# @file preDefined3.sh
# @author Daniel Starke
# @copyright Copyright 2017 Daniel Starke
# @date 2017-01-10
# @version 2017-01-14

preDefined3_cleanup() {
	rm -rf preDefined3
	return 0
}

to_var t_create <<"_END"
d preDefined3
f preDefined3/in.txt
_END

to_var t_script <<"_END"
@disable remove-temporaries
@disable remove-remains
@disable clean-up-incompletes

base = "preDefined3"
input = "{base}/in\\.txt"

process : A { foreach {
	destination = "{?:directory}/out.txt"
	echo "{PP_THREAD}" > "{destination}"
} }

execution : default {
	A("{input}")
}
_END

t_cmdline="-n"

to_var t_text <<"_END"
echo \\\\\\?"\\(0x\\)\\?[0-9a-fA-F]\\+\\\\\\?" >
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_ncheck <<"_END"
f preDefined3/out.txt
_END

t_exit="0"

t_before="preDefined3_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="preDefined3_cleanup"
fi
