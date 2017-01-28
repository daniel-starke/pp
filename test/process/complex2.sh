# @file complex2.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-30
# @version 2017-01-03

complex2_before() {
	rm -rf complex2
	return 0
}

complex2_after() {
	local aPattern
	to_var aPattern <<"_END"
"\\?complex2/l2_2\\.txt"\\?
"\\?complex2/l1_all\\.txt"\\?
_END
	sub_check_pattern "final output file" "complex2/out.txt" 0 "${aPattern}"
	[ "${clean}" -eq 1 ] && rm -rf complex2
	return 0
}

to_var t_create <<"_END"
d complex2
f complex2/in1.txt
f complex2/in2.txt
f complex2/in3.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

base = "complex2"
input = "{base}/in(?<num>[0-9]+)\\.txt"

process : A {
	none {
		destination = "{base}/l1_10.txt"
		echo none > "{destination}"
	}
	foreach {
		destination = "{?:directory}/l1_{num}.txt"
		echo {?} > "{destination}"
	}
}

process : B { all ".*in[13]\\.txt" {
	destination = "{base}/l1_all.txt"
	echo {*} > "{destination}"
} }

process : C {
	foreach ".*l1_(?<num>[23]).txt" {
		destination = "{?:directory}/l2_{num}.txt"
		echo {?} > "{destination}"
	}
	foreach !".*l1_[0-9]+.txt" {
		destination = "{?}"
	}
}

process : D { all ".*(l1_all|l2_2)\\.txt" {
	destination = "{?:directory}/out.txt"
	echo {@*} > "{destination}"
} }

execution : default {
	(A("{input}") | B("{input}"))
	> (C > D)
}

_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f complex2/l1_all.txt
f complex2/l1_1.txt
f complex2/l1_2.txt
f complex2/l1_3.txt
f complex2/l1_10.txt
f complex2/l2_2.txt
f complex2/l2_3.txt
f complex2/out.txt
_END

to_var t_ncheck <<"_END"
f complex2/l2_all.txt
f complex2/l2_1.txt
f complex2/l2_10.txt
_END

t_exit="0"

t_before="complex2_before"
t_after="complex2_after"
