#!/bin/sh
# $FreeBSD$

. $(dirname $0)/conf.sh

base=`basename $0`
keyfile=`mktemp $base.XXXXXX` || exit 1
sector=`mktemp $base.XXXXXX` || exit 1

echo "1..600"

do_test() {
	cipher=$1
	aalgo=$2
	secsize=$3
	ealgo=${cipher%%:*}
	keylen=${cipher##*:}

	md=$(attach_md -t malloc -s `expr $secsize \* 2 + 512`b)
	geli init -B none -a $aalgo -e $ealgo -l $keylen -P -K $keyfile -s $secsize ${md} 2>/dev/null

	# Corrupt 8 bytes of HMAC.
	dd if=/dev/${md} of=${sector} bs=512 count=1 >/dev/null 2>&1
	dd if=/dev/random of=${sector} bs=1 count=16 conv=notrunc >/dev/null 2>&1
	dd if=${sector} of=/dev/${md} bs=512 count=1 >/dev/null 2>&1
	geli attach -p -k $keyfile ${md}

	dd if=/dev/${md}.eli bs=${secsize} count=1 >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		echo "ok $i - aalgo=${aalgo} ealgo=${ealgo} keylen=${keylen} sec=${secsize}"
	else
		echo "not ok $i - aalgo=${aalgo} ealgo=${ealgo} keylen=${keylen} sec=${secsize}"
	fi
	i=$((i+1))

	geli detach ${md}
	mdconfig -d -u ${md}
}


i=1
dd if=/dev/random of=${keyfile} bs=512 count=16 >/dev/null 2>&1

for_each_geli_config do_test

rm -f $keyfile $sector
