# @file temporary3.sh
# @author Daniel Starke
# @copyright Copyright 2017 Daniel Starke
# @date 2017-01-03
# @version 2017-01-03

temporary3_before() {
	rm -rf temporary3
	return 0
}

temporary3_after() {
	local aPattern
	to_var aPattern <<"_END"
temporary3/l1_1\\.txt
temporary3/l2_2\\.txt
temporary3/l2_3\\.txt
_END
	sub_check_pattern "final output file" "temporary3/out.txt" 0 "${aPattern}"
	[ "${clean}" -eq 1 ] && rm -rf temporary3
	return 0
}

to_var t_create <<"_END"
d temporary3
f temporary3/in1.txt
f temporary3/in2.txt
f temporary3/in3.txt
_END

to_var t_script <<"_END"
@enable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "temporary3/in(?<num>[0-9]+)\\.txt"

process : A { foreach {
	~destination = "{?:directory}/l1_{num}.txt"
	echo {?} > "{destination}"
} }

process : B1 {
	foreach ".*l1_(?<num>1).txt" {
		destination = "{?}"
	}
}

process : B2 {
	foreach ".*l1_(?<num>[23]).txt" {
		destination = "{?:directory}/l2_(?<num>{num}).txt"
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
	@if TEST_RUN is "1"
		A("{input}") > B1 > C
	@else
		A("{input}") > B2 > C
	@end
}

_END

t_runs="2"

to_var t_text <<"_END"
^deleting "temporary3/l1_2\\.txt": ok$
^deleting "temporary3/l1_3\\.txt": ok$
_END

to_var t_ntext <<"_END"
l1_1.txt"\\?$
^deleting "temporary3/l1_1\\.txt": ok$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f temporary3/l2_2.txt
f temporary3/l2_3.txt
f temporary3/out.txt
_END

to_var t_ncheck <<"_END"
f temporary3/l1_1.txt
f temporary3/l1_2.txt
f temporary3/l1_3.txt
_END

t_exit="0"

t_before="temporary3_before"
t_after="temporary3_after"
