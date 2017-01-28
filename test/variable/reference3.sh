# @file reference3.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-11-29
# @version 2016-11-29

to_var t_script <<"_END"
a = "abc"
@warn "{a}"
unset a
@warn "{a}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:2:.*: Warn: abc$
^process\\.parallel:4:.*: Error: Trying to access unknown variable "a"\\.$
_END

t_exit="1"
