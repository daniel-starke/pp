# @file foreach-parallel2.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-29
# @version 2016-12-29

foreach_parallel2_cleanup() {
	rm -rf foreach-parallel2
	return 0
}

to_var t_create <<"_END"
d foreach-parallel2
f foreach-parallel2/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "foreach-parallel2/in\\.txt"

process : A { foreach {
	destination = "{?:directory}/intermediate{num}.txt"
	echo intermediate{num} > "{destination}"
} }

process : B {
	@include A
}

process : C { foreach {
	destination = "{?:directory}/intermediate4.txt"
	echo intermediate4 > "{destination}"
} }

process : D { foreach {
	destination = "{?:directory}/out.txt"
	echo output > "{destination}"
} }

execution : default {
	A("{input}", num = "1")
	A("{input}", num = "2")
	B("{input}", num = "3")
	C("{input}")
	D("{input}")
}

_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f foreach-parallel2/intermediate1.txt
f foreach-parallel2/intermediate2.txt
f foreach-parallel2/intermediate3.txt
f foreach-parallel2/intermediate4.txt
f foreach-parallel2/out.txt
_END

t_exit="0"

t_before="foreach_parallel2_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_parallel2_cleanup"
fi
