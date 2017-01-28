# @file variable-checking3.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-10
# @version 2016-12-10

to_var t_script <<"_END"
variable = "{?}"

@disable variable-checking

ok = "{variable}"

@warn "{ok}"

@enable variable-checking

@warn "{ok}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:7:.*: Warn: {?}$
^process\\.parallel:11:.*: Warn: {?}$
_END

t_exit="0"
