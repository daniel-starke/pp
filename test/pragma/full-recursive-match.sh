# @file full-recursive-match.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-10
# @version 2018-06-18

full_recursive_match_cleanup() {
	rm -rf full.recursive.match
	return 0
}

to_var t_create <<"_END"
d full.recursive.match
d full.recursive.match/sub
f full.recursive.match/sub/file0.ext
f full.recursive.match/sub/file1.ext
_END

to_var t_script <<"_END"
match = "{PP_SCRIPT:directory:unix}/full.recursive.match/.*\\.ext"

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

t_before="full_recursive_match_cleanup"
if [ "${clean}" -eq 1 ]; then
	t_after="full_recursive_match_cleanup"
fi
