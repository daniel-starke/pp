# @file import3.sh
# @author Daniel Starke
# @copyright Copyright 2016-2018 Daniel Starke
# @date 2016-12-22
# @version 2016-12-22

import3_cleanup() {
	rm -rf "import3.parallel"
	return 0
}

import3_init() {
	import3_cleanup
	cat > "import3.parallel" <<"_END"
@warn "include file {a}"
a = "2"
_END
	return 0
}

to_var t_script <<"_END"
@warn "main file"
a = "1"
@import "import3.parallel"
@import "import3.parallel"
@warn "main file"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:1:.*: Warn: main file$
^import3.parallel:1:.*: Warn: include file 1$
^process\\.parallel:5:.*: Warn: main file$
_END

to_var t_nerror <<"_END"
^import3.parallel:1:.*: Warn: include file 2$
_END

t_exit="0"

t_before="import3_init"
if [ "${clean}" -eq 1 ]; then
	t_after="import3_cleanup"
fi
