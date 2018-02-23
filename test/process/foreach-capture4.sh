# @file foreach-capture4.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-27
# @version 2016-12-27

foreach_capture4_cleanup() {
	rm -rf foreach-capture4
	return 0
}

to_var t_create <<"_END"
d foreach-capture4
f foreach-capture4/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./(foreach-capture4)/in(\\.txt)"

process : A { foreach {
	destination = "({1})/intermediate({2})"
	echo intermediate > "{destination}"
} }

process : B { foreach {
	destination = "{1}/out{2}"
	echo output > "{destination}"
} }

execution : default {
	A("{input}") > B
}

_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f foreach-capture4/intermediate.txt
f foreach-capture4/out.txt
_END

t_exit="0"

t_before="foreach_capture4_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_capture4_cleanup"
fi
