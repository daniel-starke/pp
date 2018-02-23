# @file full-recursive-match.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-10
# @version 2016-12-10

to_var t_script <<"_END"
match = "{PP_SCRIPT:directory:directory:unix}/.*\.sh"

@disable full-recursive-match

@if "{match:rexists}" is "false"
	@warn "ok"
@else
	@warn "nok"
@end

@enable full-recursive-match

@if "{match:rexists}" is "true"
	@warn "ok"
@else
	@warn "nok"
@end

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:6:.*: Warn: ok$
^process\\.parallel:14:.*: Warn: ok$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*: Warn: nok$
_END

t_exit="0"
