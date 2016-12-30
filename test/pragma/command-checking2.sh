# @file command-checking2.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-16
# @version 2016-12-16

command_checking2_cleanup() {
	rm -rf command-checking2
	return 0
}

to_var t_create <<"_END"
d command-checking2
f command-checking2/in.txt
_END

to_var t_script <<"_END"
@disable command-checking

input = "command-checking2/in\\.txt"

process : A { foreach {
	destination = "{?:directory}/out.txt"
	exit 1
	echo output > "{destination}"
} }

execution : default { A("{input}") }

_END

t_cmdline="-j 1"

to_var t_ntext <<"_END"
^Error: Missing output path: command-checking2.out\\.txt$
^Error: Command was not executed: .*$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_check <<"_END"
f command-checking2/out.txt
_END

t_exit="0"

t_before="command_checking2_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="command_checking2_cleanup"
fi
