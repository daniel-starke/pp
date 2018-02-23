# @file foreach-capture18.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-28
# @version 2016-12-28

foreach_capture18_cleanup() {
	rm -rf foreach-capture18
	return 0
}

to_var t_create <<"_END"
d foreach-capture18
f foreach-capture18/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes
@enable full-recursive-match

input = "(foreach-capture18)/in(\\.txt)"

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
f foreach-capture18/intermediate.txt
f foreach-capture18/out.txt
_END

t_exit="0"

t_before="foreach_capture18_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_capture18_cleanup"
fi
