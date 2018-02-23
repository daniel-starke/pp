# @file scope10.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-28
# @version 2016-12-28

scope10_cleanup() {
	rm -rf scope10
	return 0
}

to_var t_create <<"_END"
d scope10
f scope10/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes
@enable environment-variables

input = "\\./scope10/in\\.txt"

ext = ".txt"

process : A { foreach "\\./(?<path>scope10)/in\\.txt" {
	destination = "{path}/out{ext}"
	echo output > "{destination}"
} }

execution : default {
	A("{input}")
}

_END

t_cmdline="ext=.dat"

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f scope10/out.txt
_END

t_exit="0"

t_before="scope10_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="scope10_cleanup"
fi
