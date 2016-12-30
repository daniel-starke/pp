# @file scope3.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-28
# @version 2016-12-28

scope3_cleanup() {
	rm -rf scope3
	return 0
}

to_var t_create <<"_END"
d scope3
f scope3/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

input = "\\./(?<path>scope3)/in\\.txt"

ext = ".txt"

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
f scope3/out.txt
_END

t_exit="0"

t_before="scope3_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="scope3_cleanup"
fi
