# $FreeBSD$

onetime_test()
{
	cipher=$1
	secsize=$2
	ealgo=${cipher%%:*}
	keylen=${cipher##*:}

	atf_check -s exit:0 -o ignore -e ignore \
		geli onetime -e $ealgo -l $keylen -s $secsize ${md}

	secs=`diskinfo /dev/${md}.eli | awk '{print $4}'`

	atf_check dd if=/dev/random of=rnd bs=${secsize} count=${secs} status=none
	atf_check dd if=rnd of=/dev/${md}.eli bs=${secsize} count=${secs} status=none

	md_rnd=`dd if=rnd bs=${secsize} count=${secs} status=none | md5`
	atf_check_equal 0 $?
	md_ddev=`dd if=/dev/${md}.eli bs=${secsize} count=${secs} status=none | md5`
	atf_check_equal 0 $?
	md_edev=`dd if=/dev/${md} bs=${secsize} count=${secs} status=none | md5`
	atf_check_equal 0 $?

	if [ ${md_rnd} != ${md_ddev} ]; then
		atf_fail "geli did not return the original data"
	fi
	if [ ${md_rnd} == ${md_edev} ]; then
		atf_fail "geli did not encrypt the data"
	fi
}
atf_test_case onetime cleanup
onetime_head()
{
	atf_set "descr" "geli onetime can create temporary providers"
	atf_set "require.user" "root"
	atf_set "timeout" 1800
}
onetime_body()
{
	. $(atf_get_srcdir)/conf.sh
	sectors=100

	for_each_geli_config_nointegrity onetime_test
}
onetime_cleanup()
{
	. $(atf_get_srcdir)/conf.sh
	geli_test_cleanup
}

onetime_a_test()
{
	cipher=$1
	aalgo=$2
	secsize=$3
	ealgo=${cipher%%:*}
	keylen=${cipher##*:}

	atf_check -s exit:0 -o ignore -e ignore \
		geli onetime -a $aalgo -e $ealgo -l $keylen -s $secsize ${md}

	secs=`diskinfo /dev/${md}.eli | awk '{print $4}'`

	atf_check dd if=rnd of=/dev/${md}.eli bs=${secsize} count=${secs} status=none

	md_rnd=`dd if=rnd bs=${secsize} count=${secs} status=none | md5`
	atf_check_equal 0 $?
	md_ddev=`dd if=/dev/${md}.eli bs=${secsize} count=${secs} status=none | md5`
	atf_check_equal 0 $?

	if [ ${md_rnd} != ${md_ddev} ]; then
		atf_fail "Miscompare for aalgo=${aalgo} ealgo=${ealgo} keylen=${keylen} sec=${secsize}"
	fi
}
atf_test_case onetime_a cleanup
onetime_a_head()
{
	atf_set "descr" "geli onetime with HMACs"
	atf_set "require.user" "root"
	atf_set "timeout" 1800
}
onetime_a_body()
{
	. $(atf_get_srcdir)/conf.sh
	sectors=8

	atf_check dd if=/dev/random of=rnd bs=1024 count=1024 status=none
	for_each_geli_config onetime_a_test
}
onetime_a_cleanup()
{
	. $(atf_get_srcdir)/conf.sh
	geli_test_cleanup
}

atf_test_case onetime_d cleanup
onetime_d_head()
{
	atf_set "descr" "geli onetime -d will create providers that detach on last close"
	atf_set "require.user" "root"
}
onetime_d_body()
{
	. $(atf_get_srcdir)/conf.sh

	sectors=100
	md=$(attach_md -t malloc -s $sectors)

	atf_check geli onetime -d ${md}
	if [ ! -c /dev/${md}.eli ]; then
		atf_fail "Provider not created, or immediately detached"
	fi

	# Be sure it doesn't detach on read.
	atf_check dd if=/dev/${md}.eli of=/dev/null status=none
	sleep 1
	if [ ! -c /dev/${md}.eli ]; then
		atf_fail "Provider detached when a reader closed"
	fi

	# It should detach when a writer closes
	true > /dev/${md}.eli
	sleep 1
	if [ -c /dev/${md}.eli ]; then
		atf_fail "Provider didn't detach on last close of a writer"
	fi
}
onetime_d_cleanup()
{
	. $(atf_get_srcdir)/conf.sh
	geli_test_cleanup
}

atf_init_test_cases()
{
	atf_add_test_case onetime
	atf_add_test_case onetime_a
	atf_add_test_case onetime_d
}
