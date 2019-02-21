#  lib_generic.mk -- generic makefile for libraries
#
#  PROVIEW/R
#  Copyright (C) 1996 by Comator Process AB.
#
#  To build a library.
#
ifndef jpwr_generic_mk
jpwr_generic_mk := 1

ifndef variables_mk
 include $(pwre_kroot)/tools/bld/src/variables.mk
endif

ifndef rules_mk
 include $(pwre_kroot)/tools/bld/src/rules.mk
endif

vpath %.java $(co_source)
vpath %.gif $(co_source)
vpath %.png $(co_source)

source_dirs := $(co_source)

java_sources := $(sort \
             $(foreach file, \
               $(foreach dir, \
                 $(source_dirs), \
                 $(wildcard $(dir)/*.java) \
               ), $(notdir $(file)) \
             ) \
           )

gif_sources := $(sort \
             $(foreach file, \
               $(foreach dir, \
                 $(source_dirs), \
                 $(wildcard $(dir)/*.gif) \
               ), $(notdir $(file)) \
             ) \
           )

gif_exe_sources := $(sort \
             $(foreach file, \
               $(foreach dir, \
                 $(source_dirs), \
                 $(wildcard $(dir)/$(comp_name)_*.gif) \
               ), $(notdir $(file)) \
             ) \
           )

png_sources := $(sort \
             $(foreach file, \
               $(foreach dir, \
                 $(source_dirs), \
                 $(wildcard $(dir)/*.png) \
               ), $(notdir $(file)) \
             ) \
           )

png_exe_sources := $(sort \
             $(foreach file, \
               $(foreach dir, \
                 $(source_dirs), \
                 $(wildcard $(dir)/$(comp_name)_*.png) \
               ), $(notdir $(file)) \
             ) \
           )

jpwr_name   := pwr_$(comp_name)
export_lib := $(lib_dir)/$(jpwr_name).jar

java_classes := $(addsuffix .class, $(basename $(local_java_sources)))
gif_objects := $(addprefix $(bld_dir)/, $(gif_sources))
gif_exe_objects := $(addprefix $(exe_dir)/, $(gif_exe_sources))
png_objects := $(addprefix $(bld_dir)/, $(png_sources))
png_exe_objects := $(addprefix $(exe_dir)/, $(png_exe_sources))
java_objects := $(addprefix $(bld_dir)/, $(java_classes))
objects := $(gif_objects) $(gif_exe_objects) $(png_objects) $(png_exe_objects) $(java_objects)

source_dependencies := $(notdir $(basename $(sources)))
source_dependencies := $(addprefix $(bld_dir)/, $(source_dependencies))
source_dependencies := $(addsuffix $(d_ext), $(source_dependencies))

.PHONY : all init copy lib exe clean realclean \
	 dirs clean_includes \
	 clean_bld clean_dirs clean_lib

#	 dirs $(clean_h_includes) $(clean_hpp_includes)

all : init copy lib exe | silent

init : dirs | silent

copy : silent

lib : $(export_lib) | silent

exe : silent

dirs : $(bld_dir)$(dir_ext) | silent

$(bld_dir)$(dir_ext) :
	@ $(mkdir) $(mkdirflags) $(basename $@)/

clean : clean_bld clean_lib

realclean : clean clean_includes clean_dirs

clean_bld :
	@ echo "Cleaning build directory"
	@ $(rm) $(rmflags) $(bld_dir)/*

clean_dirs :
	@ echo "Removing build directory"
	@ $(rm) -r $(rmflags) $(bld_dir)

clean_lib :
	@ echo "Removing archive $(notdir $(export_lib))"
	@ $(rm) $(rmflags) $(export_lib)

clean_includes : $(clean_h_includes) $(clean_hpp_includes)

silent :
	@ :

$(clean_h_includes) : clean_%.h : %.h
	@ echo "Removing h-includes"
	@ $(rm) $(rmflags) $(inc_dir)/$*.h

$(clean_hpp_includes) : clean_%.hpp : %.hpp
	@ echo "Removing hpp-includes"
	@ $(rm) $(rmflags) $(inc_dir)/$*.hpp

$(export_lib) : $(objects)
	@ echo "Building archive $(notdir $(export_lib))"
	@ if [ -e $(export_lib) ]; then \
		$(rm) $(export_lib); \
	  fi
	jar cfm $(export_lib) manifest.stub
	@ cd $(pwre_broot)/$(pwre_target)/bld; \
	echo "Inserting png and gif files"; \
	jar uf $(export_lib) jpwr/$(comp_name)/*.gif; \
	jar uf $(export_lib) jpwr/$(comp_name)/*.png; \
	find jpwr/$(comp_name) -name "*.class" -exec jar uf $(export_lib) {} \;; \
	chmod a+r $(export_lib);
#	@ jar cvfm $(export_lib) manifest.stub jpwr/$(comp_name)/*.class jpwr/$(comp_name)/*.gif

#-include $(source_dependencies)

endif
