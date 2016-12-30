# @file command-checking3.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-16
# @version 2016-12-16

command_checking3_cleanup() {
	rm -rf command-checking3
	return 0
}

to_var t_create <<"_END"
d command-checking3
f command-checking3/in0.txt
f command-checking3/in1.txt
_END

to_var t_script <<"_END"
@enable command-checking

input = "command-checking3/in[01]\\.txt"

process : A { foreach {
	destination = "{?:directory}/out{?:filename:2}"
	exit {?:filename:2,1}
	echo output > "{destination}"
} }

execution : default { A("{input}") }

_END

t_cmdline="-j 1"

to_var t_text <<"_END"
^Error: Missing output path: command-checking3.out1\\.txt$
^Error: Command was not executed: .*$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f command-checking3/out0.txt
_END

to_var t_ncheck <<"_END"
f command-checking3/out1.txt
_END

t_exit="0"

t_before="command_checking3_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="command_checking3_cleanup"
fi
