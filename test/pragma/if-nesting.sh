# @file if-nesting.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-11-30
# @version 2016-12-02

to_var t_script <<"_END"
@if "true" = "true"
	@warn "ok"
@end

@if "true" != "true"
	@warn "nok"
@end

@if "true" = "true"
	@warn "ok"
@else
	@warn "nok"
@end

@if "true" != "true"
	@warn "nok"
@else
	@warn "ok"
@end

@if "true" = "true"
	@if "true" = "true"
		@warn "ok"
	@else
		@warn "nok"
	@end
@else
	@warn "nok"
@end

@if "true" != "true"
	@if "true" != "true"
		@warn "nok"
	@else
		@warn "nok"
	@end
@else
	@if "true" != "true"
		@warn "nok"
	@else
		@warn "ok"
	@end
@end

@if "true" = "true"
	@warn "ok"
@elseif "a" = "a"
	@warn "nok"
@else
	@warn "nok"
@end

@if "true" != "true"
	@warn "nok"
@elseif "a" = "a"
	@warn "ok"
@else
	@warn "nok"
@end

@if "true" != "true"
	@warn "nok"
@elseif "a" != "a"
	@warn "nok"
@else
	@warn "ok"
@end

@if "true" != "true"
	@warn "nok"
@elseif "a" != "a"
	@warn "nok"
@else if "b" = "b"
	@warn "ok"
@else
	@warn "nok"
@end

@if "true" != "true"
	@warn "nok"
@elseif "a" = "a"
	@if "true" != "true"
		@warn "nok"
	@elseif "a" = "a"
		@warn "ok"
	@else
		@warn "nok"
	@end
@else
	@warn "nok"
@end

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:2:.*: Warn: ok$
^process\\.parallel:10:.*: Warn: ok$
^process\\.parallel:18:.*: Warn: ok$
^process\\.parallel:23:.*: Warn: ok$
^process\\.parallel:41:.*: Warn: ok$
^process\\.parallel:46:.*: Warn: ok$
^process\\.parallel:56:.*: Warn: ok$
^process\\.parallel:66:.*: Warn: ok$
^process\\.parallel:74:.*: Warn: ok$
^process\\.parallel:85:.*: Warn: ok$
_END

to_var t_nerror <<"_END"
^process\\.parallel:.*:.*: Warn: nok$
_END

t_exit="0"
