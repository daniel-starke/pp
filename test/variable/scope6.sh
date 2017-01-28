# @file scope6.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-28
# @version 2016-12-28

scope6_cleanup() {
	rm -rf scope6
	return 0
}

to_var t_create <<"_END"
d scope6
f scope6/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./scope6/in\\.txt"

ext = ".dat"

process : A { foreach "\\./(?<path>scope6)/in(?<ext>\\.txt)" {
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
f scope6/out.txt
_END

t_exit="0"

t_before="scope6_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="scope6_cleanup"
fi
