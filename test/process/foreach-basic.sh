# @file foreach-basic.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-27
# @version 2016-12-27

foreach_basic_cleanup() {
	rm -rf foreach-basic
	return 0
}

to_var t_create <<"_END"
d foreach-basic
f foreach-basic/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "foreach-basic/in\\.txt"

process : A { foreach {
	destination = "{?:directory}/out.txt"
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
f foreach-basic/out.txt
_END

t_exit="0"

t_before="foreach_basic_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="foreach_basic_cleanup"
fi
