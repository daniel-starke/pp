PREFIX = 
export CC = $(PREFIX)gcc
export CXX = $(PREFIX)g++
export LD = $(PREFIX)g++
export AR = $(PREFIX)gcc-ar
export FIND = find
export RM = rm -f
export CP = cp -p
export TAR = tar
export NSIS = nsis

ifneq (, $(findstring __amd64__,$(shell $(CXX) -dM -E - < /dev/null)))
 LIBDIRNAME = lib64
else
 LIBDIRNAME = lib
endif

CEXT = .c
CXXEXT = .cpp
OBJEXT = .o
LIBEXT = .a
BINEXT = .exe
DOCDIR = doc
SRCDIR = src
SCRIPTDIR = script
LIBDIR = $(LIBDIRNAME)
INCDIR = include
DSTDIR = bin

#DEBUG = 1
#ENABLE_LTO = 1
UNICODE = -municode -D_UNICODE

CWFLAGS = -Wall -Wextra -Wformat -pedantic -Wshadow -Wconversion -Wparentheses -Wunused
ifeq (,$(DEBUG))
 CPPFLAGS = -isystem $(INCDIR) -I $(SRCDIR)
 ifeq (,$(ENABLE_LTO))
  BASE_CFLAGS= -O3 -static -mstackrealign -fno-ident -mtune=core2 -march=core2 -fgraphite -fomit-frame-pointer -DNDEBUG -D_WIN32_WINNT=0x0501 -DWIN32_LEAN_AND_MEAN -DBOOST_THREAD_USE_LIB -DBOOST_SPIRIT_USE_PHOENIX_V3=1 $(UNICODE)
  LDFLAGS = -O3 -s -static $(UNICODE) -Wl,--allow-multiple-definition -L$(LIBDIR)
 else
  BASE_CFLAGS = -O3 -static -mstackrealign -fno-ident -mtune=core2 -march=core2 -flto -flto-partition=none -ffat-lto-objects -fgraphite -fomit-frame-pointer -DNDEBUG -D_WIN32_WINNT=0x0501 -DWIN32_LEAN_AND_MEAN -DBOOST_THREAD_USE_LIB -DBOOST_SPIRIT_USE_PHOENIX_V3=1 $(UNICODE)
  LDFLAGS = -O3 -s -static $(UNICODE) -Wl,--allow-multiple-definition -flto -flto-partition=none -L$(LIBDIR)
 endif
else
 CPPFLAGS = -isystem $(INCDIR) -I $(SRCDIR)
 BASE_CFLAGS = -Wa,-mbig-obj -Og -g3 -ggdb -gdwarf-3 -static -mstackrealign -mtune=core2 -march=core2 -D_WIN32_WINNT=0x0501 -DWIN32_LEAN_AND_MEAN -DBOOST_THREAD_USE_LIB -DBOOST_SPIRIT_USE_PHOENIX_V3=1 $(UNICODE)
 LDFLAGS = -static $(UNICODE) -Wl,--allow-multiple-definition -L$(LIBDIR)
endif
export CFLAGS = -std=gnu99 $(BASE_CFLAGS)
export CXXFLAGS = -Wcast-qual -Wnon-virtual-dtor -Wold-style-cast -Wno-unused-parameter -Wno-long-long -Wno-maybe-uninitialized -std=c++03 $(BASE_CFLAGS)

include common.mk
