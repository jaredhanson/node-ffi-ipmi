IPMITOOL_VER = e2c5b322d893389f15e4e9e6dd7adb84c96658d0
IPMITOOL = ./ipmitool
LIBS = $(IPMITOOL)/src/plugins/.libs/libintf.a \
	$(IPMITOOL)/src/plugins/imb/.libs/libintf_imb.a \
	$(IPMITOOL)/src/plugins/lan/.libs/libintf_lan.a \
	$(IPMITOOL)/src/plugins/lanplus/.libs/libintf_lanplus.a \
	$(IPMITOOL)/src/plugins/open/.libs/libintf_open.a \
	$(IPMITOOL)/src/plugins/serial/.libs/libintf_serial.a \
	$(IPMITOOL)/lib/.libs/libipmitool.a \
	-lcrypto


libipmi.so: ipmi.c
	@if [ ! -e $(IPMITOOL) ]; then git clone http://git.code.sf.net/p/ipmitool/source $(IPMITOOL); \
		cd $(IPMITOOL) && git reset --hard ${IPMITOOL_VER} && ./bootstrap && ./configure --quiet; fi
	@cd $(IPMITOOL) && ${MAKE} -j3
	@gcc -o libipmi.so -I $(IPMITOOL)/include/ -I $(IPMITOOL)/src/plugins/lanplus/ -shared -fpic ipmi.c $(LIBS)

clean:
	@rm -f libipmi.so
	@cd $(IPMITOOL) && ${MAKE} clean
