# @file command-checking1.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-16
# @version 2016-12-16

command_checking1_cleanup() {
	rm -rf command-checking1
	return 0
}

to_var t_create <<"_END"
d command-checking1
f command-checking1/in.txt
_END

to_var t_script <<"_END"
@enable command-checking

input = "command-checking1/in\\.txt"

process : A { foreach {
	destination = "{?:directory}/out.txt"
	exit 1
	echo output > "{destination}"
} }

execution : default { A("{input}") }

_END

t_cmdline="-j 1"

to_var t_text <<"_END"
^Error: Missing output path: command-checking1.out\\.txt$
^Error: Command was not executed: .*$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

to_var t_ncheck <<"_END"
f command-checking1/out.txt
_END

t_exit="0"

t_before="command_checking1_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="command_checking1_cleanup"
fi
