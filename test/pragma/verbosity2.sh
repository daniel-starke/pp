# @file verbosity1.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-11
# @version 2016-12-11

to_var t_script <<"_END"
ok = "ok"
nok = "nok"

@verbosity DEBUG
@warn ok
@info ok
@debug ok

@verbosity INFO
@warn ok
@info ok
@debug nok

@verbosity WARN
@warn ok
@info nok
@debug nok

@verbosity ERROR
@warn nok
@info nok
@debug nok

@error ok

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:5:.*: Warn: ok$
^process\\.parallel:7:.*: Debug: ok$
^process\\.parallel:10:.*: Warn: ok$
^process\\.parallel:15:.*: Warn: ok$
^process\\.parallel:24:.*: Error: ok$
_END

to_var t_text <<"_END"
^process\\.parallel:6:.*: Info: ok$
^process\\.parallel:11:.*: Info: ok$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*: .*: nok$
_END

to_var t_ntext <<"_END"
^process\\.parallel:.*:.*: .*: nok$
_END

t_exit="1"
