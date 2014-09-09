/*
 * This file was generated by LibCSS gen_parser 
 * 
 * Generated from:
 *
 * text_align:CSS_PROP_TEXT_ALIGN IDENT:( INHERIT: LEFT:0,TEXT_ALIGN_LEFT RIGHT:0,TEXT_ALIGN_RIGHT CENTER:0,TEXT_ALIGN_CENTER JUSTIFY:0,TEXT_ALIGN_JUSTIFY LIBCSS_LEFT:0,TEXT_ALIGN_LIBCSS_LEFT LIBCSS_CENTER:0,TEXT_ALIGN_LIBCSS_CENTER LIBCSS_RIGHT:0,TEXT_ALIGN_LIBCSS_RIGHT IDENT:)
 * 
 * Licensed under the MIT License,
 *		  http://www.opensource.org/licenses/mit-license.php
 * Copyright 2010 The NetSurf Browser Project.
 */

#include <assert.h>
#include <string.h>

#include "bytecode/bytecode.h"
#include "bytecode/opcodes.h"
#include "parse/properties/properties.h"
#include "parse/properties/utils.h"

/**
 * Parse text_align
 *
 * \param c	  Parsing context
 * \param vector  Vector of tokens to process
 * \param ctx	  Pointer to vector iteration context
 * \param result  resulting style
 * \return CSS_OK on success,
 *	   CSS_NOMEM on memory exhaustion,
 *	   CSS_INVALID if the input is not valid
 *
 * Post condition: \a *ctx is updated with the next token to process
 *		   If the input is invalid, then \a *ctx remains unchanged.
 */
css_error css__parse_text_align(css_language *c,
		const parserutils_vector *vector, int *ctx,
		css_style *result)
{
	int orig_ctx = *ctx;
	css_error error;
	const css_token *token;
	bool match;

	token = parserutils_vector_iterate(vector, ctx);
	if ((token == NULL) || ((token->type != CSS_TOKEN_IDENT))) {
		*ctx = orig_ctx;
		return CSS_INVALID;
	}

	if ((lwc_string_caseless_isequal(token->idata, c->strings[INHERIT], &match) == lwc_error_ok && match)) {
			error = css_stylesheet_style_inherit(result, CSS_PROP_TEXT_ALIGN);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[LEFT], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_TEXT_ALIGN, 0,TEXT_ALIGN_LEFT);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[RIGHT], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_TEXT_ALIGN, 0,TEXT_ALIGN_RIGHT);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[CENTER], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_TEXT_ALIGN, 0,TEXT_ALIGN_CENTER);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[JUSTIFY], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_TEXT_ALIGN, 0,TEXT_ALIGN_JUSTIFY);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[LIBCSS_LEFT], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_TEXT_ALIGN, 0,TEXT_ALIGN_LIBCSS_LEFT);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[LIBCSS_CENTER], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_TEXT_ALIGN, 0,TEXT_ALIGN_LIBCSS_CENTER);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[LIBCSS_RIGHT], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_TEXT_ALIGN, 0,TEXT_ALIGN_LIBCSS_RIGHT);
	} else {
		error = CSS_INVALID;
	}

	if (error != CSS_OK)
		*ctx = orig_ctx;
	
	return error;
}

