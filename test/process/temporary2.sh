# @file temporary2.sh
# @author Daniel Starke
# @copyright Copyright 2017-2018 Daniel Starke
# @date 2017-01-03
# @version 2017-09-16

temporary2_before() {
	rm -rf temporary2
	return 0
}

temporary2_after() {
	local aPattern
	to_var aPattern <<"_END"
temporary2/l1_1\\.txt
temporary2/l2_2\\.txt
temporary2/l2_3\\.txt
_END
	sub_check_pattern "final output file" "temporary2/out.txt" 0 "${aPattern}"
	[ "${clean}" -eq 1 ] && rm -rf temporary2
	return 0
}

to_var t_create <<"_END"
d temporary2
f temporary2/in1.txt
f temporary2/in2.txt
f temporary2/in3.txt
_END

to_var t_script <<"_END"
@enable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "temporary2/in(?<num>[0-9]+)\\.txt"

process : A { foreach {
	destination = "{?:directory}/l1_{num}.txt"
	echo {?} > "{destination}"
} }

process : B {
	foreach ".*l1_(?<num>[23]).txt" {
		~destination = "{?:directory}/l2_(?<num>{num}).txt"
		echo {?} > "{destination}"
	}
	foreach ".*l1_(?<num>1).txt" {
		destination = "{?}"
	}
}

process : C { all {
	destination = "{?:directory}/out.txt"
	echo {*} > "{destination}"
} }

execution : default {
	A("{input}") > B > C
}

_END

t_runs="2"

to_var t_ntext <<"_END"
echo
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f temporary2/l1_1.txt
f temporary2/l1_2.txt
f temporary2/l1_3.txt
f temporary2/out.txt
_END

to_var t_ncheck <<"_END"
f temporary2/l2_1.txt
f temporary2/l2_2.txt
f temporary2/l2_3.txt
_END

t_exit="0"

t_before="temporary2_before"
t_after="temporary2_after"
