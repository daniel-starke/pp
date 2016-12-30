# @file lower.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-11-28
# @version 2016-11-28

to_var t_script <<"_END"
a = "abc"
b = "ABC"
c = "äÖêÉß"

@warn "{a:lower}"
@warn "{b:lower}"
@warn "{c:lower}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:5:.*: Warn: abc$
^process\\.parallel:6:.*: Warn: abc$
^process\\.parallel:7:.*: Warn: äöêéß$
_END

t_exit="0"
