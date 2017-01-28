# @file scope9.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-28
# @version 2016-12-28

scope9_cleanup_0() {
	rm -rf scope9
	return 0
}

scope9_cleanup_1() {
	unset ext
	rm -rf scope9
	return 0
}

to_var t_create <<"_END"
d scope9
f scope9/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes
@enable environment-variables

input = "\\./scope9/in\\.txt"

process : A { foreach "\\./(?<path>scope9)/in\\.txt" {
	destination = "{path}/out{ext}"
	echo output > "{destination}"
} }

execution : default {
	A("{input}")
}

_END

export ext=""
t_cmdline="ext=.txt"

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f scope9/out.txt
_END

t_exit="0"

t_before="scope9_cleanup_0"
if [ "${clean}" -eq 1 ]; then
	t_after="scope9_cleanup_1"
fi
