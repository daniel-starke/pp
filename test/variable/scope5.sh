# @file scope5.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-28
# @version 2016-12-28

scope5_cleanup() {
	rm -rf scope5
	return 0
}

to_var t_create <<"_END"
d scope5
f scope5/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./scope5/in\\.txt"

ext = ".dat"

process : A { foreach "\\./(?<path>scope5)/in\\.txt" {
	ext = ".txt"
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
f scope5/out.txt
_END

t_exit="0"

t_before="scope5_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="scope5_cleanup"
fi
