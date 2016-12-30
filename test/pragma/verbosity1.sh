# @file verbosity1.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-11
# @version 2016-12-11

to_var t_script <<"_END"
@verbosity DEBUG
@warn "ok"
@info "ok"
@debug "ok"

@verbosity INFO
@warn "ok"
@info "ok"
@debug "nok"

@verbosity WARN
@warn "ok"
@info "nok"
@debug "nok"

@verbosity ERROR
@warn "nok"
@info "nok"
@debug "nok"

@error "ok"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:2:.*: Warn: ok$
^process\\.parallel:4:.*: Debug: ok$
^process\\.parallel:7:.*: Warn: ok$
^process\\.parallel:12:.*: Warn: ok$
^process\\.parallel:21:.*: Error: ok$
_END

to_var t_text <<"_END"
^process\\.parallel:3:.*: Info: ok$
^process\\.parallel:8:.*: Info: ok$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*: .*: nok$
_END

to_var t_ntext <<"_END"
^process\\.parallel:.*:.*: .*: nok$
_END

t_exit="1"
