# @file foreach-capture2.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-27
# @version 2016-12-27

foreach_capture2_cleanup() {
	rm -rf foreach-capture2
	return 0
}

to_var t_create <<"_END"
d foreach-capture2
f foreach-capture2/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./(foreach-capture2)/in(\\.txt)"

process : A { foreach {
	destination = "{1}/out{2}"
	echo output > "{destination}"
} }

execution : default {
	A("{input}")
}

_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f foreach-capture2/out.txt
_END

t_exit="0"

t_before="foreach_capture2_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_capture2_cleanup"
fi
