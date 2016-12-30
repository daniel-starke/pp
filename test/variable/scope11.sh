# @file scope11.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-28
# @version 2016-12-28

scope11_cleanup_0() {
	rm -rf scope11
	return 0
}

scope11_cleanup_1() {
	unset ext
	rm -rf scope11
	return 0
}

to_var t_create <<"_END"
d scope11
f scope11/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes
@enable environment-variables

input = "\\./scope11/in\\.txt"

process : A { foreach "\\./(?<path>scope11)/in\\.txt" {
	destination = "{path}/out{ext}"
	echo output > "{destination}"
} }

execution : default {
	A("{input}")
}

_END

export ext=".txt"

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f scope11/out.txt
_END

t_exit="0"

t_before="scope11_cleanup_0"
if [ "${clean}" -eq 1 ]; then
	t_after="scope11_cleanup_1"
fi
