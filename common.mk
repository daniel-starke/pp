APPS = pp

pp_version = 1.3.4
pp_version_date = 2018-02-28
pp_author = Daniel Starke

CPPFLAGS += '-DPP_VERSION="$(pp_version) ($(pp_version_date))"' '-DPP_AUTHOR="$(pp_author)"'

pp_obj = \
	$(patsubst $(SRCDIR)/%$(CEXT),$(DSTDIR)/%$(OBJEXT),$(wildcard $(SRCDIR)/libpcf/*$(CEXT))) \
	$(patsubst $(SRCDIR)/%$(CXXEXT),$(DSTDIR)/%$(OBJEXT),$(wildcard $(SRCDIR)/pcf/*/*$(CXXEXT))) \
	$(patsubst $(SRCDIR)/%$(CXXEXT),$(DSTDIR)/%$(OBJEXT),$(wildcard $(SRCDIR)/pp/parser/*$(CXXEXT))) \
	$(patsubst $(SRCDIR)/%$(CXXEXT),$(DSTDIR)/%$(OBJEXT),$(wildcard $(SRCDIR)/pp/*$(CXXEXT))) \
	$(patsubst $(SRCDIR)/%$(CXXEXT),$(DSTDIR)/%$(OBJEXT),$(wildcard $(SRCDIR)/*$(CXXEXT))) \

pp_lib = \
	libboost_program_options \
	libboost_locale \
	libboost_filesystem \
	libboost_iostreams \
	libboost_date_time \
	libboost_thread \
	libboost_regex \
	libboost_system \
	libsqlite3 \
	libws2_32

all: $(DSTDIR) $(LIBDIR) $(INCDIR) $(addprefix $(DSTDIR)/,$(addsuffix $(BINEXT),$(APPS)))

setup: all $(SCRIPTDIR)/setup.nsi
	$(NSIS) //DVERSION=$(pp_version) $(SCRIPTDIR)/setup.nsi

.PHONY: check
check: script/perform-tests.sh
	export "pp=$(DSTDIR)/pp$(BINEXT)"; $<

.PHONY: $(DSTDIR)
$(DSTDIR):
	mkdir -p $(DSTDIR)

.PHONY: $(LIBDIR)
$(LIBDIR):
	mkdir -p $(LIBDIR)

.PHONY: $(INCDIR)
$(INCDIR):
	mkdir -p $(INCDIR)

.PHONY: clean
clean:
	$(RM) $(DSTDIR)/*$(OBJEXT)
	$(RM) $(addprefix $(DSTDIR)/,$(addsuffix $(BINEXT),$(APPS)))
	$(RM) -r $(DSTDIR)

$(DSTDIR)/pp$(LIBEXT): $(pp_obj)
	$(AR) qs $@ $+

$(DSTDIR)/pp$(BINEXT): $(DSTDIR)/pp$(LIBEXT)
	$(LD) $(LDFLAGS) -o $@ $+ $(pp_lib:lib%=-l%)

$(DSTDIR)/%$(OBJEXT): $(SRCDIR)/%$(CEXT)
	mkdir -p "$(dir $@)"
	$(CC) $(CWFLAGS) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

$(DSTDIR)/%$(OBJEXT): $(SRCDIR)/%$(CXXEXT)
	mkdir -p "$(dir $@)"
	$(CXX) $(CWFLAGS) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

$(SRCDIR)/license.hpp: $(DOCDIR)/COPYING $(SCRIPTDIR)/create-hex-include.sh
	pp_author="$(pp_author)" $(SCRIPTDIR)/create-hex-include.sh $(DOCDIR)/COPYING $@ licenseText

# dependencies
$(DSTDIR)/pp/parser/Utility$(OBJEXT): \
	$(SRCDIR)/pp/parser/Utility.hpp \
	$(SRCDIR)/pp/Shell.hpp \
	$(SRCDIR)/pp/Type.hpp \
	$(SRCDIR)/pp/Variable.hpp
$(DSTDIR)/pp/Execution$(OBJEXT): \
	$(SRCDIR)/pp/Command.hpp \
	$(SRCDIR)/pp/Execution.cpp \
	$(SRCDIR)/pp/Execution.hpp \
	$(SRCDIR)/pp/Process.hpp \
	$(SRCDIR)/pp/ProcessBlock.hpp \
	$(SRCDIR)/pp/ProcessNode.hpp \
	$(SRCDIR)/pp/Shell.hpp \
	$(SRCDIR)/pp/Type.hpp \
	$(SRCDIR)/pp/Utility.hpp \
	$(SRCDIR)/pp/Variable.hpp
$(DSTDIR)/pp/Script$(OBJEXT): \
	$(SRCDIR)/pp/parser/Script.hpp \
	$(SRCDIR)/pp/parser/StringLiteral.hpp \
	$(SRCDIR)/pp/parser/Utility.hpp \
	$(SRCDIR)/pp/Command.hpp \
	$(SRCDIR)/pp/Execution.hpp \
	$(SRCDIR)/pp/Process.hpp \
	$(SRCDIR)/pp/ProcessBlock.hpp \
	$(SRCDIR)/pp/ProcessNode.hpp \
	$(SRCDIR)/pp/Script.cpp \
	$(SRCDIR)/pp/Script.hpp \
	$(SRCDIR)/pp/Shell.hpp \
	$(SRCDIR)/pp/Type.hpp \
	$(SRCDIR)/pp/Utility.hpp \
	$(SRCDIR)/pp/Variable.hpp
$(DSTDIR)/pp/Shell$(OBJEXT): \
	$(SRCDIR)/pp/Shell.cpp \
	$(SRCDIR)/pp/Shell.hpp \
	$(SRCDIR)/pp/Variable.hpp
$(DSTDIR)/pp/Type$(OBJEXT): \
	$(SRCDIR)/pp/Shell.hpp \
	$(SRCDIR)/pp/Type.cpp \
	$(SRCDIR)/pp/Type.hpp \
	$(SRCDIR)/pp/Variable.hpp
$(DSTDIR)/pp/Variable$(OBJEXT): \
	$(SRCDIR)/pp/parser/StringLiteral.hpp \
	$(SRCDIR)/pp/parser/Utility.hpp \
	$(SRCDIR)/pp/Shell.hpp \
	$(SRCDIR)/pp/Type.hpp \
	$(SRCDIR)/pp/Variable.cpp \
	$(SRCDIR)/pp/Variable.hpp
$(DSTDIR)/posix_main$(OBJEXT): \
	$(SRCDIR)/posix_main.cpp \
	$(SRCDIR)/posix_main.hpp
$(DSTDIR)/pp$(OBJEXT): \
	$(SRCDIR)/pp/Command.hpp \
	$(SRCDIR)/pp/Execution.hpp \
	$(SRCDIR)/pp/Process.hpp \
	$(SRCDIR)/pp/ProcessBlock.hpp \
	$(SRCDIR)/pp/ProcessNode.hpp \
	$(SRCDIR)/pp/Script.hpp \
	$(SRCDIR)/pp/Shell.hpp \
	$(SRCDIR)/pp/Type.hpp \
	$(SRCDIR)/pp/Utility.hpp \
	$(SRCDIR)/pp/Variable.hpp \
	$(SRCDIR)/posix_main.hpp \
	$(SRCDIR)/pp.cpp \
	$(SRCDIR)/pp.hpp \
	$(SRCDIR)/license.hpp
