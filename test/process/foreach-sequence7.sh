# @file foreach-sequence7.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-29
# @version 2016-12-29

foreach_sequence7_cleanup() {
	rm -rf foreach-sequence7
	return 0
}

to_var t_create <<"_END"
d foreach-sequence7
f foreach-sequence7/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "foreach-sequence7/in\\.txt"

process : A { foreach {
	destination = "{?:directory}/intermediate{num}.txt"
	echo {?} > "{destination}"
} }

process : B {
	@include A
}

process : C {
	foreach ".*intermediate(?<num>[0-9]+).txt" {
		destination = "{?:directory}/intermediate1{num}.txt"
		echo {?} > "{destination}"
	}
	foreach ".*in.txt" {
		destination = "{?:directory}/intermediate4.txt"
		echo {?} > "{destination}"
	}
}

process : D { foreach ".*intermediate(?<num>[0-9]+)\\.txt" {
	destination = "{?:directory}/out{num}.txt"
	echo {?} > "{destination}"
} }

execution : default {
	(A("{input}", num = "1") > A(num = "2") > B(num = "3")) > (C(+,"{input}") > D)
}

_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f foreach-sequence7/intermediate1.txt
f foreach-sequence7/intermediate2.txt
f foreach-sequence7/intermediate3.txt
f foreach-sequence7/intermediate4.txt
f foreach-sequence7/intermediate13.txt
f foreach-sequence7/out4.txt
f foreach-sequence7/out13.txt
_END

to_var t_ncheck <<"_END"
f foreach-sequence7/out1.txt
f foreach-sequence7/out2.txt
f foreach-sequence7/out3.txt
_END

t_exit="0"

t_before="foreach_sequence7_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_sequence7_cleanup"
fi
