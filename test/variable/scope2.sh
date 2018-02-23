# @file scope2.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-28
# @version 2016-12-28

scope2_cleanup() {
	rm -rf scope2
	return 0
}

to_var t_create <<"_END"
d scope2
f scope2/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./(?<path>scope2)/in(?<ext>\\.txt)"

ext = ".dat"

process : A { foreach {
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
f scope2/out.txt
_END

t_exit="0"

t_before="scope2_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="scope2_cleanup"
fi
