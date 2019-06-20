#include "_libctf.h"

#include <stdio.h>

#include <dwarf.h>
#include <libdwarf.h>

static unsigned int
strhash(const char *s)
{
	unsigned int hash;

	for (hash = 2166136261u; *s != '\0'; s++)
		hash = (hash ^ *s) * 16777619;
	return (hash);
}

static void *
xmalloc(size_t sz)
{
	void *ret;

	ret = malloc(sz);
	assert(ret != NULL); /* XXX */
	return (ret);
}

static Dwarf_Unsigned
die_byte_size(Dwarf_Die die)
{
	Dwarf_Unsigned byte_size;
	int error;

	error = dwarf_attrval_unsigned(die, DW_AT_byte_size, &byte_size, NULL);
	assert(error == DW_DLV_OK);
	return (byte_size);
}

static Dwarf_Die
die_child(Dwarf_Die die)
{
	Dwarf_Die child;
	int error;

	error = dwarf_child(die, &child, NULL);
	if (error == DW_DLV_OK)
		return (child);
	assert(error == DW_DLV_NO_ENTRY);
	return (NULL);
}

static Dwarf_Off
die_dieoffset(Dwarf_Die die)
{
	Dwarf_Off ret;
	int error;

	error = dwarf_dieoffset(die, &ret, NULL);
	assert(error == DW_DLV_OK);
	return (ret);
}

static Dwarf_Signed
die_encoding(Dwarf_Die die)
{
	Dwarf_Signed encoding;
	int error;

	error = dwarf_attrval_signed(die, DW_AT_encoding, &encoding, NULL);
	assert(error == DW_DLV_OK);
	return (encoding);
}

static bool
die_hasattr(Dwarf_Die die, Dwarf_Half tag)
{
	Dwarf_Bool present;
	int error;

	error = dwarf_hasattr(die, tag, &present, NULL);
	assert(error == DW_DLV_OK);
	return ((bool)present);
}

static const char *
die_name(Dwarf_Die die)
{
	char *ret;
	int error;

	error = dwarf_diename(die, &ret, NULL);
	if (error == DW_DLV_OK)
		return (ret);
	assert(error == DW_DLV_NO_ENTRY);
	return (NULL);
}

static Dwarf_Half
die_tag(Dwarf_Die die)
{
	Dwarf_Half ret;
	int error;

	error = dwarf_tag(die, &ret, NULL);
	assert(error == DW_DLV_OK);
	return (ret);
}

static Dwarf_Off
die_type(Dwarf_Die die)
{
	Dwarf_Attribute attr;
	Dwarf_Half form;
	Dwarf_Off off;
	int error;

	error = dwarf_attr(die, DW_AT_type, &attr, NULL);
	assert(error == DW_DLV_OK);
	error = dwarf_whatform(attr, &form, NULL);
	assert(error == DW_DLV_OK);
	error = dwarf_global_formref(attr, &off, NULL);
	assert(error == DW_DLV_OK);
	return (off);
}

struct tnode {
	struct ctf_imtype	t;
	struct ctf_convert_cu	*cu;

	uint64_t		gen;
	bool			canonical;

	SLIST_HEAD(, tnode)	crefs;
	SLIST_HEAD(, tnode)	refs;
	SLIST_ENTRY(tnode)	reflink;
	LIST_ENTRY(tnode)	hashlink;
};
SLIST_HEAD(tnode_slist, tnode);
LIST_HEAD(tnode_list, tnode);

struct doff {
	uint64_t	d_off;
	struct tnode	*d_ref;
	RB_ENTRY(doff)	d_link;
};

static int
doff_cmp(struct doff *a, struct doff *b)
{

	if (a->d_off < b->d_off)
		return (-1);
	return (a->d_off > b->d_off);
}
RB_HEAD(doffmap, doff);
RB_GENERATE_STATIC(doffmap, doff, d_link, doff_cmp);

struct ctf_convert {
	Ctf		*ctf;		/* dst CTF container */
	Dwarf_Debug	dbg;		/* input debug info */
	Elftc_String_Table *strtab;	/* CTF string table */

	uint64_t	currgen;
	uint64_t	gen;
	struct tnode	voidtype;

	struct tnode_list basehash[1024];
	struct tnode_list enumhash[1024];
	struct tnode_list structhash[1024];
	struct tnode_list unionhash[1024];

	void		(*errcb)(const char *); /* error callback */
};

struct ctf_convert_cu {
	struct ctf_convert	*cvt;
	struct doffmap		dmap;
	struct tnode_slist	dangling;
};

static void	ctf_convert_dwarf_die(struct ctf_convert_cu *, Dwarf_Die);
static size_t	ctf_convert_str_insert(struct ctf_convert *, const char *);
static const char *ctf_convert_str_lookup(struct ctf_convert *cu, size_t off);

static void	tnode_canonicalize_or_discard(struct tnode *, struct tnode *);
static void	tnode_canonicalize_refs(struct tnode *);
static void	tnode_remap_refs(struct tnode *, struct tnode *);

static void
doffmap_add_ref(struct doffmap *dmap, struct tnode *t, uint64_t id)
{
	struct doff *d;

	d = xmalloc(sizeof(*d));
	d->d_off = id;
	d->d_ref = t;
	RB_INSERT(doffmap, dmap, d);
}

static struct tnode *
doffmap_find(struct doffmap *m, uint64_t id)
{
	struct doff *d, key;

	key.d_off = id;
	d = RB_FIND(doffmap, m, &key);
	return (d != NULL ? d->d_ref : NULL);
}

static void
doffmap_remap(struct tnode *n, struct tnode *t)
{
	struct doff *d, key;

	key.d_off = t->t.t_id;
	d = RB_FIND(doffmap, &t->cu->dmap, &key);
	assert(d->d_ref == t);
	d->d_ref = n;
}

static struct tnode *
tnode_alloc(struct ctf_convert_cu *cu, uint64_t id, const char *name,
    char kind)
{
	struct tnode *t;

	t = xmalloc(sizeof(*t));
	t->t.t_name = ctf_convert_str_insert(cu->cvt, name);
	t->t.t_id = id;
	t->t.t_kind = kind;
	t->canonical = false;
	t->cu = cu;
	t->gen = 0;
	SLIST_INIT(&t->crefs);
	SLIST_INIT(&t->refs);

	switch (kind) {
	case CTF_K_ENUM:
		ctf_imtelem_list_init(&t->t.t_enum.vals, 8);
		break;
	case CTF_K_FUNCTION:
		ctf_imtelem_list_init(&t->t.t_func.params, 8);
		t->t.t_func.variadic = false;
		break;
	case CTF_K_STRUCT:
	case CTF_K_UNION:
		ctf_imtelem_list_init(&t->t.t_sou.members, 8);
		break;
	}
	return (t);
}

static void
tnode_free(struct tnode *t)
{

	assert(!t->canonical);
	assert(SLIST_EMPTY(&t->crefs));
	assert(SLIST_EMPTY(&t->refs));

	switch (t->t.t_kind) {
	case CTF_K_ENUM:
		free(t->t.t_enum.vals.el_list);
		break;
	case CTF_K_FUNCTION:
		free(t->t.t_func.params.el_list);
		break;
	case CTF_K_STRUCT:
	case CTF_K_UNION:
		free(t->t.t_sou.members.el_list);
		break;
	}
	free(t);
}

static void
tnode_ref_copy(struct ctf_imtype *dst, struct ctf_imtype *src)
{

	switch (src->t_kind) {
	case CTF_K_ARRAY:
		dst->t_array = src->t_array;
		break;
	case CTF_K_TYPEDEF:
		dst->t_name = src->t_name; /* XXX put this field in the union */
		/* FALLTHROUGH */
	case CTF_K_CONST:
	case CTF_K_POINTER:
	case CTF_K_RESTRICT:
	case CTF_K_VOLATILE:
		dst->t_ref.ref = src->t_ref.ref;
		break;
	default:
		errx(1, "unhandled CTF kind %d", src->t_kind);
	}
}

static bool
tnode_ref_equiv(struct ctf_imtype *t1, struct ctf_imtype *t2)
{

	if (t1->t_kind != t2->t_kind)
		return (false);

	switch (t1->t_kind) {
	case CTF_K_ARRAY:
		return (t1->t_array.tindex == t2->t_array.tindex &&
		    t1->t_array.count == t2->t_array.count);
	case CTF_K_CONST:
	case CTF_K_POINTER:
	case CTF_K_RESTRICT:
	case CTF_K_VOLATILE:
		return (true);
	case CTF_K_TYPEDEF:
		return (t1->t_name == t2->t_name);
	default:
		errx(1, "unhandled CTF kind %d", t1->t_kind);
	}
}

static struct tnode *
tnode_find_or_add_ref(struct ctf_convert_cu *cu, struct ctf_imtype *search,
    Dwarf_Off toff)
{
	struct tnode *t, *target;

	t = NULL;
	target = doffmap_find(&cu->dmap, toff);
	if (target != NULL && target->canonical) {
		SLIST_FOREACH(t, &target->crefs, reflink)
			if (tnode_ref_equiv(&t->t, search))
				break;
	}
	if (t == NULL) {
		t = tnode_alloc(cu, search->t_id, NULL, search->t_kind);
		tnode_ref_copy(&t->t, search);
		if (target != NULL) {
			if (target->canonical) {
				SLIST_INSERT_HEAD(&target->crefs, t, reflink);
				t->canonical = true;
			} else {
				SLIST_INSERT_HEAD(&target->refs, t, reflink);
			}
		} else {
			SLIST_INSERT_HEAD(&cu->dangling, t, reflink);
		}
	}
	return (t);
}

static struct tnode *
tnode_from_anon_ref_type(struct ctf_convert_cu *cu, Dwarf_Die die, char ctfkind)
{
	Dwarf_Off toff;
	struct ctf_imtype search;

	if (die_hasattr(die, DW_AT_type))
		toff = die_type(die);
	else
		toff = 0;

	search.t_id = die_dieoffset(die);
	search.t_kind = ctfkind;
	search.t_ref.ref = toff;
	return (tnode_find_or_add_ref(cu, &search, toff));
}

static struct tnode *
tnode_from_array_type(struct ctf_convert_cu *cu, Dwarf_Die die)
{
	Dwarf_Die child;
	Dwarf_Off ioff, toff;
	Dwarf_Unsigned u;
	struct ctf_imtype search;
	int count, error;

	/*
	 * Array size and index type.
	 */
	child = die_child(die);
	assert(child != NULL);
	assert(die_tag(child) == DW_TAG_subrange_type);
	ioff = die_type(child);
	if (die_hasattr(child, DW_AT_upper_bound)) {
		error = dwarf_attrval_unsigned(child, DW_AT_upper_bound, &u,
		    NULL);
		assert(error == DW_DLV_OK);
		count = (int)u + 1;
	} else if (die_hasattr(child, DW_AT_count)) {
		error = dwarf_attrval_unsigned(child, DW_AT_count, &u, NULL);
		assert(error == DW_DLV_OK);
		count = (int)u;
	} else {
		count = 0;
	}
	dwarf_dealloc(cu->cvt->dbg, child, DW_DLA_DIE);

	toff = die_type(die);

	search.t_id = die_dieoffset(die);
	search.t_kind = CTF_K_ARRAY;
	search.t_array.tref = toff;
	search.t_array.tindex = ioff;
	search.t_array.count = count;
	return (tnode_find_or_add_ref(cu, &search, toff));
}

static uint32_t
dwarf_enc2ctf(Dwarf_Die die)
{
	uint32_t enc;

	enc = 0;
	switch (die_encoding(die)) {
	case DW_ATE_boolean:
		enc |= CTF_INT_BOOL;
		break;
	case DW_ATE_signed:
		enc |= CTF_INT_SIGNED;
		/* FALLTHROUGH */
	case DW_ATE_unsigned:
		break;
	case DW_ATE_signed_char:
		enc |= CTF_INT_SIGNED;
		/* FALLTHROUGH */
	case DW_ATE_unsigned_char:
		enc |= CTF_INT_CHAR;
		break;
	default:
		warnx("unhandled base type encoding at %#lx",
		    die_dieoffset(die));
	}
	return (enc);
}

static struct tnode *
tnode_from_base_type(struct ctf_convert_cu *cu, Dwarf_Die die)
{
	struct tnode *t;
	const char *name;
	size_t nameoff;
	unsigned int hash;
	uint32_t enc;

	name = die_name(die);
	assert(name != NULL);

	nameoff = ctf_convert_str_insert(cu->cvt, name);

	/* XXX */
	enc = CTF_INT_DATA(dwarf_enc2ctf(die), 0, die_byte_size(die) * NBBY);
	hash = strhash(name);
	LIST_FOREACH(t, &cu->cvt->basehash[hash & 1023 /* XXX */], hashlink) {
		if (t->t.t_name == nameoff && t->t.t_integer.enc == enc)
			break;
	}
	if (t == NULL) {
		t = tnode_alloc(cu, die_dieoffset(die), name, CTF_K_INTEGER);
		t->t.t_integer.enc = enc;
		LIST_INSERT_HEAD(&cu->cvt->basehash[hash & 1023], t, hashlink);
		t->canonical = true;
	}
	return (t);
}

static struct tnode *
tnode_from_compound_type(struct ctf_convert_cu *cu, Dwarf_Die die, char kind)
{
	Dwarf_Debug dbg;
	Dwarf_Die child, child1;
	struct ctf_imtelem e;
	struct tnode *t;
	struct tnode_list *l;
	const char *name;
	unsigned int hash;
	int error;

	assert(kind == CTF_K_STRUCT || kind == CTF_K_UNION);

	dbg = cu->cvt->dbg;
	name = die_name(die);

	t = tnode_alloc(cu, die_dieoffset(die), name, kind);

	child = die_child(die);
	if (name != NULL) {
		hash = strhash(name);
		if (child == NULL)
			goto done;
	} else if (child != NULL) {
		/* XXX this is wrong we should recurse. */
		name = die_name(child);
		if (name != NULL)
			hash = strhash(name);
		else
			hash = 0;
	} else {
		hash = 0;
		goto done;
	}

	child1 = NULL;
	do {
		if (child1 != NULL) {
			dwarf_dealloc(dbg, child, DW_DLA_DIE);
			child = child1;
		}

		switch (die_tag(child)) {
		case DW_TAG_member:
			name = die_name(child);
			e.e_name = ctf_convert_str_insert(cu->cvt, name);
			e.e_type = die_type(child);
			/* XXX handle bit fields */
			e.e_off = 0; /* XXX */
			ctf_imtelem_list_add(&t->t.t_sou.members, &e);
			break;
		case DW_TAG_enumeration_type:
		case DW_TAG_structure_type:
		case DW_TAG_union_type:
			ctf_convert_dwarf_die(cu, child);
			break;
		default:
			errx(1, "unhandled child tag at offset %#lx",
			    die_dieoffset(child));
		}
	} while ((error = dwarf_siblingof(dbg, child, &child1, NULL)) ==
	    DW_DLV_OK);
	assert(error == DW_DLV_NO_ENTRY);
	dwarf_dealloc(dbg, child, DW_DLA_DIE);

done:
	if (kind == CTF_K_STRUCT)
		l = &cu->cvt->structhash[hash & 1023];
	else
		l = &cu->cvt->unionhash[hash & 1023];
	LIST_INSERT_HEAD(l, t, hashlink);
	return (t);
}

static struct tnode *
tnode_from_enumeration_type(struct ctf_convert_cu *cu, Dwarf_Die die)
{
	Dwarf_Debug dbg;
	Dwarf_Die child, child1;
	Dwarf_Signed sval;
	Dwarf_Unsigned uval;
	struct tnode *t, *s;
	struct ctf_imtelem e, *e1, *e2;
	struct ctf_imtelem_list *l1, *l2;
	const char *name;
	unsigned int hash;
	int error, i;

	dbg = cu->cvt->dbg;
	name = die_name(die);

	t = tnode_alloc(cu, die_dieoffset(die), name, CTF_K_ENUM);

	child = die_child(die);
	assert(child != NULL);

	if (name != NULL)
		hash = strhash(name);
	else
		hash = strhash(die_name(child));

	child1 = NULL;
	do {
		if (child1 != NULL) {
			dwarf_dealloc(dbg, child, DW_DLA_DIE);
			child = child1;
		}

		name = die_name(child);
		assert(name != NULL);
		e.e_name = ctf_convert_str_insert(cu->cvt, name);
		e.e_type = -1;
		assert(die_tag(child) == DW_TAG_enumerator);
		error = dwarf_attrval_unsigned(child, DW_AT_const_value, &uval,
		    NULL);
		if (error == DW_DLV_OK) {
			e.e_val = (int)uval;
		} else {
			error = dwarf_attrval_signed(child, DW_AT_const_value,
			    &sval, NULL);
			assert(error == DW_DLV_OK);
			e.e_val = (int)sval;
		}
		ctf_imtelem_list_add(&t->t.t_enum.vals, &e);
	} while ((error = dwarf_siblingof(dbg, child, &child1, NULL)) ==
	    DW_DLV_OK);
	assert(error == DW_DLV_NO_ENTRY);
	dwarf_dealloc(dbg, child, DW_DLA_DIE);

	l1 = &t->t.t_enum.vals;
	LIST_FOREACH(s, &cu->cvt->enumhash[hash & 1023 /* XXX */], hashlink) {
		l2 = &s->t.t_enum.vals;
		if (l1->el_count != l2->el_count)
			continue;
		for (i = 0; i < l1->el_count; i++) {
			e1 = &l1->el_list[i];
			e2 = &l2->el_list[i];
			if (e1->e_name != e2->e_name || e1->e_val != e2->e_val)
				break;
		}
		if (i == l1->el_count)
			break;
	}

	if (s != NULL) {
		tnode_free(t);
		t = s;
	} else {
		LIST_INSERT_HEAD(&cu->cvt->enumhash[hash & 1023], t, hashlink);
		t->canonical = true;
	}

	return (t);
}

static struct tnode *
tnode_from_subroutine_type(struct ctf_convert_cu *cu, Dwarf_Die die)
{
	Dwarf_Debug dbg;
	Dwarf_Die child, child1;
	struct ctf_imtelem e;
	struct tnode *t;
	int error;

	dbg = cu->cvt->dbg;

	t = tnode_alloc(cu, die_dieoffset(die), NULL, CTF_K_FUNCTION);

	child = die_child(die);
	if (child == NULL)
		goto done;
	child1 = NULL;
	do {
		if (child1 != NULL) {
			dwarf_dealloc(dbg, child, DW_DLA_DIE);
			child = child1;
		}

		switch (die_tag(child)) {
		case DW_TAG_formal_parameter:
			e.e_name = 0;
			e.e_type = die_type(child);
			e.e_val = 0;
			ctf_imtelem_list_add(&t->t.t_func.params, &e);
			break;
		case DW_TAG_unspecified_parameters:
			assert(!t->t.t_func.variadic);
			t->t.t_func.variadic = true;
			break;
		default:
			errx(1, "unhandled tag at offset %#lx",
			    die_dieoffset(child));
		}
	} while ((error = dwarf_siblingof(dbg, child, &child1, NULL)) ==
	    DW_DLV_OK);
	assert(error == DW_DLV_NO_ENTRY);
	dwarf_dealloc(dbg, child, DW_DLA_DIE);

done:
	return (t);
}

static struct tnode *
tnode_from_typedef(struct ctf_convert_cu *cu, Dwarf_Die die)
{
	Dwarf_Off toff;
	struct ctf_imtype search;
	const char *name;

	name = die_name(die);
	assert(name != NULL);

	if (die_hasattr(die, DW_AT_type))
		toff = die_type(die);
	else
		toff = 0;
	search.t_name = ctf_convert_str_insert(cu->cvt, name);
	search.t_id = die_dieoffset(die);
	search.t_kind = CTF_K_TYPEDEF;
	search.t_ref.ref = toff;
	return (tnode_find_or_add_ref(cu, &search, toff));
}

/* XXX this is not as general as its name implies */
static bool
tnode_equiv(struct ctf_convert *cvt, struct tnode *t1, struct tnode *t2)
{
	struct ctf_imtelem *e1, *e2;
	struct ctf_imtelem_list *l1, *l2;
	struct tnode *n1, *n2;
	int i;
	char kind;

	if (t1 == t2)
		return (true);
	if (t1->t.t_kind != t2->t.t_kind)
		return (false);
	if (t1->t.t_name != t2->t.t_name)
		return (false);

	if (t1->gen > cvt->gen && t2->gen > cvt->gen)
		return (t1->gen == t2->gen);
	t1->gen = t2->gen = ++cvt->currgen;

	kind = t1->t.t_kind;
	switch (kind) {
	case CTF_K_ENUM:
	case CTF_K_FLOAT:
	case CTF_K_INTEGER:
		assert(t1->canonical);
		assert(t2->canonical);
		/* It is sufficient to verify that t1 != t2. */
		return (false);
	case CTF_K_ARRAY:
		if (t1->t.t_array.count != t2->t.t_array.count)
			return (false);
		/* FALLTHROUGH */
	case CTF_K_TYPEDEF:
		if (kind == CTF_K_TYPEDEF && t1->t.t_name != t2->t.t_name)
			return (false);
		/* FALLTHROUGH */
	case CTF_K_CONST:
	case CTF_K_POINTER:
	case CTF_K_RESTRICT:
	case CTF_K_VOLATILE:
		n1 = doffmap_find(&t1->cu->dmap, t1->t.t_ref.ref);
		n2 = doffmap_find(&t2->cu->dmap, t2->t.t_ref.ref);
		return (tnode_equiv(cvt, n1, n2));
	case CTF_K_FUNCTION:
		l1 = &t1->t.t_func.params;
		l2 = &t2->t.t_func.params;
		break;
	case CTF_K_STRUCT:
	case CTF_K_UNION:
		l1 = &t1->t.t_sou.members;
		l2 = &t2->t.t_sou.members;
		break;
	}

	if (l1->el_count != l2->el_count) {
		if ((kind == CTF_K_STRUCT || kind == CTF_K_UNION) &&
		    (l1->el_count == 0 || l2->el_count == 0))
			return (true);
		else
			return (false);
	}
	for (i = 0; i < l1->el_count; i++) {
		e1 = &l1->el_list[i];
		e2 = &l2->el_list[i];

		if (e1->e_name != e2->e_name)
			return (false);
		if ((kind == CTF_K_STRUCT || kind == CTF_K_UNION) &&
		    e1->e_off != e2->e_off)
			return (false);

		n1 = doffmap_find(&t1->cu->dmap, e1->e_type);
		n2 = doffmap_find(&t2->cu->dmap, e2->e_type);
		if (!tnode_equiv(cvt, n1, n2))
			return (false);
	}

	return (true);
}

static void
tnode_canonicalize_or_discard(struct tnode *t, struct tnode *ref)
{
	struct tnode *cref;

	assert(t->canonical);
	assert(!ref->canonical);

	SLIST_FOREACH(cref, &t->crefs, reflink) {
		if (tnode_ref_equiv(&cref->t, &ref->t))
			break;
	}
	if (cref != NULL) {
		doffmap_remap(cref, ref);
		tnode_remap_refs(cref, ref);
		tnode_free(ref);
	} else {
		ref->canonical = true;
		tnode_canonicalize_refs(ref);
		SLIST_INSERT_HEAD(&t->crefs, ref, reflink);
	}
}

static void
tnode_canonicalize_refs(struct tnode *t)
{
	struct tnode *ref;

	assert(t->canonical);

	while ((ref = SLIST_FIRST(&t->refs)) != NULL) {
		SLIST_REMOVE_HEAD(&t->refs, reflink);
		tnode_canonicalize_or_discard(t, ref);
	}
}

static void
tnode_remap_refs(struct tnode *cref, struct tnode *ref)
{
	struct tnode *t;

	assert(cref->canonical);
	assert(!ref->canonical);

	while ((t = SLIST_FIRST(&ref->refs)) != NULL) {
		SLIST_REMOVE_HEAD(&ref->refs, reflink);
//		t->t.t_ref.ref = cref->t.t_id;
		tnode_canonicalize_or_discard(cref, t);
	}
}

static void
ctf_convert_resolve_refs(struct ctf_convert_cu *cu)
{
	struct tnode *t, *ref;

	while ((ref = SLIST_FIRST(&cu->dangling)) != NULL) {
		SLIST_REMOVE_HEAD(&cu->dangling, reflink);

		t = doffmap_find(&cu->dmap, ref->t.t_ref.ref);
		assert(t != NULL);

		if (t->canonical)
			tnode_canonicalize_or_discard(t, ref);
		else
			SLIST_INSERT_HEAD(&t->refs, ref, reflink);
	}
}

static size_t
ctf_convert_str_insert(struct ctf_convert *cu, const char *name)
{
	size_t off;

	if (name == NULL)
		return (0);
	off = elftc_string_table_insert(cu->strtab, name);
	assert(off != 0); /* XXX */
	return (off);
}

static const char * __unused
ctf_convert_str_lookup(struct ctf_convert *cu, size_t off)
{

	return (elftc_string_table_to_string(cu->strtab, off));
}

static void
ctf_convert_subprogram(struct ctf_convert_cu *cu, Dwarf_Die die)
{
	Dwarf_Debug dbg;
	Dwarf_Die child, child1;
	int error;

	dbg = cu->cvt->dbg;

	child = die_child(die);
	if (child == NULL)
		return;
	child1 = NULL;
	do {
		if (child1 != NULL) {
			dwarf_dealloc(dbg, child, DW_DLA_DIE);
			child = child1;
		}
		ctf_convert_dwarf_die(cu, child);
	} while ((error = dwarf_siblingof(dbg, child, &child1, NULL)) ==
	    DW_DLV_OK);
	assert(error == DW_DLV_NO_ENTRY);
	dwarf_dealloc(dbg, child, DW_DLA_DIE);
}

static void
ctf_convert_dwarf_die(struct ctf_convert_cu *cu, Dwarf_Die die)
{
	struct tnode *t;

	switch (die_tag(die)) {
	case DW_TAG_array_type:
		t = tnode_from_array_type(cu, die);
		break;
	case DW_TAG_base_type:
		t = tnode_from_base_type(cu, die);
		break;
	case DW_TAG_const_type:
		t = tnode_from_anon_ref_type(cu, die, CTF_K_CONST);
		break;
	case DW_TAG_enumeration_type:
		t = tnode_from_enumeration_type(cu, die);
		break;
	case DW_TAG_pointer_type:
		t = tnode_from_anon_ref_type(cu, die, CTF_K_POINTER);
		break;
	case DW_TAG_restrict_type:
		t = tnode_from_anon_ref_type(cu, die, CTF_K_RESTRICT);
		break;
	case DW_TAG_structure_type:
		t = tnode_from_compound_type(cu, die, CTF_K_STRUCT);
		break;
	case DW_TAG_subprogram:
		ctf_convert_subprogram(cu, die);
		t = NULL;
		break;
	case DW_TAG_subroutine_type:
		t = tnode_from_subroutine_type(cu, die);
		break;
	case DW_TAG_typedef:
		t = tnode_from_typedef(cu, die);
		break;
	case DW_TAG_union_type:
		t = tnode_from_compound_type(cu, die, CTF_K_UNION);
		break;
	case DW_TAG_volatile_type:
		t = tnode_from_anon_ref_type(cu, die, CTF_K_VOLATILE);
		break;
	default:
		t = NULL;
		break;
	}

	if (t != NULL)
		doffmap_add_ref(&cu->dmap, t, die_dieoffset(die));
}

static int
ctf_convert_dwarf_cu(struct ctf_convert *cvt, Dwarf_Die cu)
{
	Dwarf_Debug dbg;
	Dwarf_Die die, die1;
	struct ctf_convert_cu *cucvt;
	int error;

	cucvt = xmalloc(sizeof(*cucvt));
	cucvt->cvt = cvt;
	RB_INIT(&cucvt->dmap);
	SLIST_INIT(&cucvt->dangling);

	doffmap_add_ref(&cucvt->dmap, &cvt->voidtype, cvt->voidtype.t.t_id);

	dbg = cvt->dbg;
	die = die_child(cu);
	assert(die != NULL);
	die1 = NULL;
	do {
		if (die1 != NULL) {
			dwarf_dealloc(dbg, die, DW_DLA_DIE);
			die = die1;
		}
		ctf_convert_dwarf_die(cucvt, die);
	} while ((error = dwarf_siblingof(dbg, die, &die1, NULL)) == DW_DLV_OK);
	assert(error == DW_DLV_NO_ENTRY);
	dwarf_dealloc(dbg, die, DW_DLA_DIE);

	ctf_convert_resolve_refs(cucvt);

	return (0);
}

Ctf *
ctf_convert_dwarf(int fd, void (*errcb)(const char *) __unused /* XXX */) /* XXX add warncb? */
{
	Dwarf_Debug dbg;
	Dwarf_Die cu;
	struct ctf_convert cvt;
	struct tnode *t, *t1, *tmp, *voidt;
	struct tnode_list *l;
	Ctf *ctf;
	size_t i;
	int error;

	ctf = calloc(1, sizeof(*ctf));
	assert(ctf != NULL); /* XXX */
	cvt.ctf = ctf;

	for (i = 0; i < nitems(cvt.basehash); i++)
		LIST_INIT(&cvt.basehash[i]);
	for (i = 0; i < nitems(cvt.enumhash); i++)
		LIST_INIT(&cvt.enumhash[i]);
	for (i = 0; i < nitems(cvt.structhash); i++)
		LIST_INIT(&cvt.structhash[i]);
	for (i = 0; i < nitems(cvt.unionhash); i++)
		LIST_INIT(&cvt.unionhash[i]);

	/*
	 * Use an initial string table size of 1MB.
	 */
	cvt.strtab = elftc_string_table_create(1024 * 1024);
	assert(cvt.strtab != NULL); /* XXX */

	/*
	 * Synthesize a "void" type.
	 */
	voidt = &cvt.voidtype;
	voidt->t.t_name = ctf_convert_str_insert(&cvt, "void");
	voidt->t.t_id = 0;
	voidt->t.t_kind = CTF_K_INTEGER;
	voidt->t.t_integer.enc = 0;
	voidt->canonical = true;
	voidt->cu = NULL;

	error = dwarf_init(fd, DW_DLC_READ, NULL, NULL, &dbg, NULL);
	assert(error == DW_DLV_OK); /* XXX */
	cvt.dbg = dbg;

	while ((error = dwarf_next_cu_header_b(dbg, NULL, NULL, NULL, NULL,
	    NULL, NULL, NULL, NULL)) == DW_DLV_OK) {
		cu = NULL;
		error = dwarf_siblingof(dbg, cu, &cu, NULL);
		assert(error == DW_DLV_OK); /* XXX */

		if (die_tag(cu) != DW_TAG_compile_unit)
			errx(1, "top-level DIE isn't a CU"); /* XXX */

		ctf_convert_dwarf_cu(&cvt, cu);

		dwarf_dealloc(dbg, cu, DW_DLA_DIE);
	}
	assert(error == DW_DLV_NO_ENTRY); /* XXX */

	cvt.gen = cvt.currgen = 1;

	int count = 0;
	for (i = 0; i < nitems(cvt.structhash); i++) {
		l = &cvt.structhash[i];
		while ((t = LIST_FIRST(l)) != NULL) {
			LIST_REMOVE(t, hashlink);
			assert(!t->canonical);
			t->canonical = true;
			/* XXX why do we do this here? */
			tnode_canonicalize_refs(t);
			count++;

			LIST_FOREACH_SAFE(t1, l, hashlink, tmp) {
				if (tnode_equiv(&cvt, t, t1)) {
					LIST_REMOVE(t1, hashlink);
					doffmap_remap(t, t1);
					tnode_remap_refs(t, t1);
					tnode_free(t1);
				}
				cvt.gen = cvt.currgen + 1;
			}
		}
	}

	printf("found %d structures\n", count);

	return (ctf);
}
