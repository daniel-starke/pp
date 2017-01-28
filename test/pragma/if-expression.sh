# @file if-expression.sh
# @author Daniel Starke
# @copyright Copyright 2016-2017 Daniel Starke
# @date 2016-12-02
# @version 2016-12-02

to_var t_script <<"_END"
a = "true"
b = "false"

# silence warning if c was not set as environment variable
c = ""
unset c

@if a
	@warn "ok"
@end

@if c
	@warn "nok"
@end

@if !a
	@warn "nok"
@end

@if !c
	@warn "ok"
@end

@if not a
	@warn "nok"
@end

@if not c
	@warn "ok"
@end

@if a = b
	@warn "nok"
@else
	@warn "ok"
@end

@if a is b
	@warn "nok"
@else
	@warn "ok"
@end

@if a != b
	@warn "ok"
@else
	@warn "nok"
@end

@if a is not b
	@warn "ok"
@else
	@warn "nok"
@end

@if ! a = b
	@warn "ok"
@else
	@warn "nok"
@end

@if not a = b
	@warn "ok"
@else
	@warn "nok"
@end

@if not a is b
	@warn "ok"
@else
	@warn "nok"
@end

@if not a is b and a
	@warn "ok"
@else
	@warn "nok"
@end

@if not a is b and not c
	@warn "ok"
@else
	@warn "nok"
@end

@if a is b or a
	@warn "ok"
@else
	@warn "nok"
@end

@if not a is b or not c
	@warn "ok"
@else
	@warn "nok"
@end

@if a and c or b and c or not c
	@warn "ok"
@else
	@warn "nok"
@end

@if a and (c or b) and not (c or c)
	@warn "ok"
@else
	@warn "nok"
@end

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:9:.*: Warn: ok$
^process\\.parallel:21:.*: Warn: ok$
^process\\.parallel:29:.*: Warn: ok$
^process\\.parallel:35:.*: Warn: ok$
^process\\.parallel:41:.*: Warn: ok$
^process\\.parallel:45:.*: Warn: ok$
^process\\.parallel:51:.*: Warn: ok$
^process\\.parallel:57:.*: Warn: ok$
^process\\.parallel:63:.*: Warn: ok$
^process\\.parallel:69:.*: Warn: ok$
^process\\.parallel:75:.*: Warn: ok$
^process\\.parallel:81:.*: Warn: ok$
^process\\.parallel:87:.*: Warn: ok$
^process\\.parallel:93:.*: Warn: ok$
^process\\.parallel:99:.*: Warn: ok$
^process\\.parallel:105:.*: Warn: ok$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*: Warn: nok$
_END

t_exit="0"
