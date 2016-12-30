# @file directory.sh
# @author Daniel Starke
# @copyright Copyright 2016 Daniel Starke
# @date 2016-11-27
# @version 2016-12-30

to_var t_script <<"_END"
a1 = "/root/parent/child.ext"
b1 = "/root/parent/"
c1 = "/root/parent"
d1 = "/root/"
e1 = "/root"
f1 = "/"
g1 = ""

a2 = "C:\\root\\parent\\child.ext"
b2 = "C:\\root\\parent\\"
c2 = "C:\\root\\parent"
d2 = "C:\\root\\"
e2 = "C:\\root"
f2 = "C:\\"
g2 = ""

@warn "{a1}"
@warn "{a1:directory}"
@warn "{b1:directory}"
@warn "{c1:directory}"
@warn "{d1:directory}"
@warn "{e1:directory}"
@warn "{f1:directory}"
@warn "{g1:directory}"

@warn "{a2}"
@warn "{a2:native:directory:win}"
@warn "{b2:native:directory:win}"
@warn "{c2:native:directory:win}"
@warn "{d2:native:directory:win}"
@warn "{e2:native:directory:win}"
@warn "{f2:native:directory:win}"
@warn "{g2:native:directory:win}"

execution : default {}
_END

to_var t_error <<"_END"
^process\\.parallel:17:.*: Warn: /root/parent/child\\.ext$
^process\\.parallel:18:.*: Warn: /root/parent$
^process\\.parallel:19:.*: Warn: /root/parent$
^process\\.parallel:20:.*: Warn: /root$
^process\\.parallel:21:.*: Warn: /root$
^process\\.parallel:22:.*: Warn: /$
^process\\.parallel:23:.*: Warn: $
^process\\.parallel:24:.*: Warn: $
^process\\.parallel:26:.*: Warn: C:\\\\root\\\\parent\\\\child\\.ext$
^process\\.parallel:27:.*: Warn: C:\\\\root\\\\parent$
^process\\.parallel:28:.*: Warn: C:\\\\root\\\\parent$
^process\\.parallel:29:.*: Warn: C:\\\\root$
^process\\.parallel:30:.*: Warn: C:\\\\root$
^process\\.parallel:31:.*: Warn: C:\\\\\\?$
^process\\.parallel:32:.*: Warn: C:$
^process\\.parallel:33:.*: Warn: $
_END

t_exit="0"
