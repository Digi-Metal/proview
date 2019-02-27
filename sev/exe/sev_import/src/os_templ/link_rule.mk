ifndef link_rule_mk
link_rule_mk := 1

ifndef pwre_cxx
  ifeq ($(PWRE_CONF_LIBHDF5),1)
    ldsev = mpic++
  else
    ldsev = $(ldxx)
  endif
else
  ldsev = $(pwre_cxx)
endif

link = $(ldsev) $(elinkflags) $(domap) -o $(export_exe) \
	$(export_obj) $(objects) $(wb_msg_eobjs) $(rt_msg_eobjs) \
	$(pwr_eobj)/rt_io_user.o \
        $(pwre_conf_libdir) $(pwre_conf_libpwrsev) $(pwre_conf_libpwrrt) $(pwre_conf_lib)

endif
