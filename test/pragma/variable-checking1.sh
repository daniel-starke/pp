# @file variable-checking1.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-08
# @version 2016-12-08

to_var t_script <<"_END"
variable = "content"
nothing = ""
unset nothing

@enable variable-checking

ok = "{variable}"
nok = "{nothing}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:8:.*: Error: Trying to access unknown variable "nothing".$
_END

t_exit="1"
