# @file import2.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-12-22
# @version 2016-12-22

import2_cleanup() {
	rm -rf "import2.parallel"
	return 0
}

import2_init() {
	import2_cleanup
	cat > "import2.parallel" <<"_END"
@warn "include file"
_END
	return 0
}

to_var t_script <<"_END"
@warn "main file"
@import "import2_invalid.parallel"
@warn "main file"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:1:.*: Warn: main file$
^process\\.parallel:2:.*: Error: Invalid file path "import2_invalid.parallel"\\.$
_END

to_var t_nerror <<"_END"
^import2.parallel:1:.*: Warn: include file$
^process\\.parallel:3:.*: Warn: main file$
_END

t_exit="1"

t_before="import2_init"
if [ "${clean}" -eq 1 ]; then
	t_after="import2_cleanup"
fi
