# @file extension.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-11-28
# @version 2016-11-28

to_var t_script <<"_END"
a1 = "/root/parent/child.ext"
b1 = "/root/parent/"
c1 = "/root/parent.long-extension"
d1 = "/root/"
e1 = "/root"
f1 = "/"
g1 = ""

a2 = "C:\\root\\parent\\child.ext"
b2 = "C:\\root\\parent\\"
c2 = "C:\\root\\parent.long-extension"
d2 = "C:\\root\\"
e2 = "C:\\root"
f2 = "C:\\"
g2 = ""

@warn "{a1}"
@warn "{a1:extension}"
@warn "{b1:extension}"
@warn "{c1:extension}"
@warn "{d1:extension}"
@warn "{e1:extension}"
@warn "{f1:extension}"
@warn "{g1:extension}"

@warn "{a2}"
@warn "{a2:extension}"
@warn "{b2:extension}"
@warn "{c2:extension}"
@warn "{d2:extension}"
@warn "{e2:extension}"
@warn "{f2:extension}"
@warn "{g2:extension}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:17:.*: Warn: /root/parent/child\\.ext$
^process\\.parallel:18:.*: Warn: \\.ext$
^process\\.parallel:19:.*: Warn: $
^process\\.parallel:20:.*: Warn: \\.long-extension$
^process\\.parallel:21:.*: Warn: $
^process\\.parallel:22:.*: Warn: $
^process\\.parallel:23:.*: Warn: $
^process\\.parallel:24:.*: Warn: $
^process\\.parallel:26:.*: Warn: C:\\\\root\\\\parent\\\\child\\.ext$
^process\\.parallel:27:.*: Warn: \\.ext$
^process\\.parallel:28:.*: Warn: $
^process\\.parallel:29:.*: Warn: \\.long-extension$
^process\\.parallel:30:.*: Warn: $
^process\\.parallel:31:.*: Warn: $
^process\\.parallel:32:.*: Warn: $
^process\\.parallel:33:.*: Warn: $
_END

t_exit="0"
