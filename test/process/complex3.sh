# @file complex3.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-30
# @version 2017-01-03

complex3_before() {
	rm -rf complex3
	return 0
}

complex3_after() {
	local aPattern
	to_var aPattern <<"_END"
"\\?complex3/l2_2\\.txt"\\?
"\\?complex3/l2_3\\.txt"\\?
_END
	sub_check_pattern "final output file" "complex3/out.txt" 0 "${aPattern}"
	[ "${clean}" -eq 1 ] && rm -rf complex3
	return 0
}

to_var t_create <<"_END"
d complex3
f complex3/in1.txt
f complex3/in2.txt
f complex3/in3.txt
_END

to_var t_script <<"_END"
@enable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "complex3/in(?<num>[0-9]+)\\.txt"

process : A { foreach {
	~destination = "{?:directory}/l1_{num}.txt"
	echo {?} > "{destination}"
} }

process : B { foreach ".*l1_(?<num>[23]).txt" {
	destination = "{?:directory}/l2_{num}.txt"
	echo {?} > "{destination}"
} }

process : C { all {
	destination = "{?:directory}/out.txt"
	echo {@*} > "{destination}"
} }

execution : default {
	A("{input}") > B > C
}

_END

t_runs="2"

to_var t_touch1 <<"_END"
f complex3/in2.txt
_END

to_var t_text <<"_END"
^deleting "complex3/l1_2\\.txt": ok$
_END

to_var t_ntext <<"_END"
l[12]_[13]\\.txt.$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f complex3/l2_2.txt
f complex3/l2_3.txt
f complex3/out.txt
_END

to_var t_ncheck <<"_END"
f complex3/l1_1.txt
f complex3/l1_2.txt
f complex3/l1_3.txt
f complex3/l2_1.txt
_END

t_exit="0"

t_before="complex3_before"
t_after="complex3_after"
