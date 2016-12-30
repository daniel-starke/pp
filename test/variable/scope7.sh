# @file scope7.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-28
# @version 2016-12-28

scope7_cleanup() {
	rm -rf scope7
	return 0
}

to_var t_create <<"_END"
d scope7
f scope7/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./scope7/in\\.txt"

process : A { foreach "\\./(?<path>scope7)/in\\.txt" {
	destination = "{path}/out{ext}"
	echo output > "{destination}"
} }

execution : default {
	A("{input}", ext = ".txt")
}

_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f scope7/out.txt
_END

t_exit="0"

t_before="scope7_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="scope7_cleanup"
fi
