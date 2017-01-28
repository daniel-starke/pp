# @file foreach-sequence5.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-29
# @version 2016-12-29

foreach_sequence5_cleanup() {
	rm -rf foreach-sequence5
	return 0
}

to_var t_create <<"_END"
d foreach-sequence5
f foreach-sequence5/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "foreach-sequence5/in\\.txt"

process : A { foreach {
	destination = "{?:directory}/intermediate{num}.txt"
	echo intermediate{num} > "{destination}"
} }

process : B {
	@include A
}

process : C {
	foreach ".*intermediate(?<num>[0-9]+).txt" {
		destination = "{?:directory}/intermediate1{num}.txt"
		echo intermediate1{num} > "{destination}"
	}
	foreach ".*in.txt" {
		destination = "{?:directory}/intermediate4.txt"
		echo intermediate4 > "{destination}"
	}
}

process : D { foreach ".*intermediate(?<num>[0-9]+)\\.txt" {
	destination = "{?:directory}/out{num}.txt"
	echo output > "{destination}"
} }

execution : default {
	(A("{input}", num = "1") > A(num = "2") > B(num = "3")) > C(+,"{input}") > D
}

_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f foreach-sequence5/intermediate1.txt
f foreach-sequence5/intermediate2.txt
f foreach-sequence5/intermediate3.txt
f foreach-sequence5/intermediate4.txt
f foreach-sequence5/intermediate13.txt
f foreach-sequence5/out4.txt
f foreach-sequence5/out13.txt
_END

to_var t_ncheck <<"_END"
f foreach-sequence5/out1.txt
f foreach-sequence5/out2.txt
f foreach-sequence5/out3.txt
_END

t_exit="0"

t_before="foreach_sequence5_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_sequence5_cleanup"
fi
