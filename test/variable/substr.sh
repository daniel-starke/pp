# @file substr.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-11-26
# @version 2016-11-28

to_var t_script <<"_END"
a = "0123456789"
b = "äÖêÉß"

@warn "{a}"
@warn "{a:1}"
@warn "{a:-1}"
@warn "{a:3}"
@warn "{a:-3}"
@warn "{a:12}"
@warn "{a:-12}"
@warn "{a:3,2}"
@warn "{a:-3,2}"
@warn "{a:3,-2}"
@warn "{a:-3,-2}"
@warn "{a:3,12}"
@warn "{a:-3,-12}"

@warn "{b:1,3}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:4:.*: Warn: 0123456789$
^process\\.parallel:5:.*: Warn: 123456789$
^process\\.parallel:6:.*: Warn: 9$
^process\\.parallel:7:.*: Warn: 3456789$
^process\\.parallel:8:.*: Warn: 789$
^process\\.parallel:9:.*: Warn: $
^process\\.parallel:10:.*: Warn: 0123456789$
^process\\.parallel:11:.*: Warn: 34$
^process\\.parallel:12:.*: Warn: 78$
^process\\.parallel:13:.*: Warn: 12$
^process\\.parallel:14:.*: Warn: 56$
^process\\.parallel:15:.*: Warn: 3456789$
^process\\.parallel:16:.*: Warn: 0123456$
^process\\.parallel:18:.*: Warn: ÖêÉ$
_END

t_exit="0"
