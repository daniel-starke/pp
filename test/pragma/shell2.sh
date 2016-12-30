# @file shell2.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-21
# @version 2016-12-21

shell2_cleanup() {
	rm -rf shell2
	return 0
}

to_var t_create <<"_END"
d shell2
f shell2/in.txt
_END

to_var t_script <<"_END"
input = "shell2/in\\.txt"

@shell : test {
	path = "shell"
	commandLine = "shell -c \"{?}\""
	replace[1] = /([\\])/\\\1/
	replace[2] = /(["])/\\\1/
}
@shell test

process : A { foreach {
	destination = "{?:directory}/out.txt"
	echo "output" > {destination:win}
} }

execution : default { A("{input}") }

_END

t_cmdline="-n"

to_var t_text <<"_END"
shell -c "echo \\\\"output\\\\" > shell2\\\\\\\\out\.txt"$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*:.*$
_END

t_exit="0"

t_before="shell2_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="shell2_cleanup"
fi
