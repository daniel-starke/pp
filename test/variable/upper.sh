# @file upper.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-11-28
# @version 2016-11-30

to_var t_script <<"_END"
a = "abc"
b = "ABC"
c = "äÖêÉß"

@warn "{a:upper}"
@warn "{b:upper}"
@warn "{c:upper}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:5:.*: Warn: ABC$
^process\\.parallel:6:.*: Warn: ABC$
^process\\.parallel:7:.*: Warn: ÄÖÊÉ\\(ß\\|SS\\)$
_END

t_exit="0"
