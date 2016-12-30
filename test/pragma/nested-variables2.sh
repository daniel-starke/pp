# @file nested-variables2.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-19
# @version 2016-12-21

nested_variables2_cleanup() {
	rm -rf nested-variables2
	return 0
}

to_var t_create <<"_END"
d nested-variables2
f nested-variables2/in.txt
_END

to_var t_script <<"_END"
@disable variable-checking
@disable nested-variables

input = "nested-variables2/in\\.txt"

@shell : test {
	path = "shell"
	commandLine = "shell -c {?}"
}
@shell test

process : A { foreach {
	destination = "{?:directory}/out.txt"
	text = "output"
	echo {text} > {destination}
} }

execution : default { A("{input}") }

_END

t_cmdline="-n"

to_var t_text <<"_END"
shell -c text = "output"$
shell -c echo  > nested-variables2/out\\.txt$
_END

to_var t_error <<"_END"
^process\\.parallel:15:.*: Warning: Trying to access unknown variable "text"\\.$
_END

t_exit="0"

t_before="nested_variables2_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="nested_variables2_cleanup"
fi
