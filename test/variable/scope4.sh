# @file scope4.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-28
# @version 2016-12-28

scope4_cleanup() {
	rm -rf scope4
	return 0
}

to_var t_create <<"_END"
d scope4
f scope4/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./scope4/in\\.txt"

ext = ".txt"

process : A { foreach "\\./(?<path>scope4)/in\\.txt" {
	destination = "{path}/out{ext}"
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
f scope4/out.txt
_END

t_exit="0"

t_before="scope4_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="scope4_cleanup"
fi
