# @file if-operand.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-03
# @version 2016-12-03

to_var t_script <<"_END"
a = "true"
b = "false"
c = ""
unset c

@if "true" = set
	@warn "ok"
@else
	@warn "nok"
@end

@if "true" is set
	@warn "ok"
@else
	@warn "nok"
@end

@if a = set
	@warn "ok"
@else
	@warn "nok"
@end

@if a is set
	@warn "ok"
@else
	@warn "nok"
@end

@if c != set
	@warn "ok"
@else
	@warn "nok"
@end

@if c is not set
	@warn "ok"
@else
	@warn "nok"
@end

@if PP_SCRIPT is file
	@warn "ok"
@else
	@warn "nok"
@end

@if "{PP_SCRIPT}.not" is not file
	@warn "ok"
@else
	@warn "nok"
@end

@if "{PP_SCRIPT:directory}" is directory
	@warn "ok"
@else
	@warn "nok"
@end

@if "{PP_SCRIPT:directory}.not" is not directory
	@warn "ok"
@else
	@warn "nok"
@end

@if "(?:abc)?_[0-9]+\\.ext" is regex
	@warn "ok"
@else
	@warn "nok"
@end

@if "(?:abc)?_[0-9+\\.ext" is not regex
	@warn "ok"
@else
	@warn "nok"
@end

@if a is "true"
	@warn "ok"
@else
	@warn "nok"
@end

@if a is not b
	@warn "ok"
@else
	@warn "nok"
@end

@if a is like "^(?i:TRUE)$"
	@warn "ok"
@else
	@warn "nok"
@end

@if a is not like "^(?i:FALSE)$"
	@warn "ok"
@else
	@warn "nok"
@end

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:7:.*: Warn: ok$
^process\\.parallel:13:.*: Warn: ok$
^process\\.parallel:19:.*: Warn: ok$
^process\\.parallel:25:.*: Warn: ok$
^process\\.parallel:31:.*: Warn: ok$
^process\\.parallel:37:.*: Warn: ok$
^process\\.parallel:43:.*: Warn: ok$
^process\\.parallel:49:.*: Warn: ok$
^process\\.parallel:55:.*: Warn: ok$
^process\\.parallel:61:.*: Warn: ok$
^process\\.parallel:67:.*: Warn: ok$
^process\\.parallel:73:.*: Warn: ok$
^process\\.parallel:79:.*: Warn: ok$
^process\\.parallel:85:.*: Warn: ok$
^process\\.parallel:91:.*: Warn: ok$
^process\\.parallel:97:.*: Warn: ok$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*: Warn: nok$
_END

t_exit="0"
