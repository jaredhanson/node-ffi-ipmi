#
# IPMITOOL should be set to ipmitool-1.8.13 based source distribution 
# based on https://github.com/speedops/ipmitool.git
#

IPMITOOL = ../ipmitool

LIBS = $(IPMITOOL)/src/plugins/.libs/libintf.a \
	$(IPMITOOL)/src/plugins/imb/.libs/libintf_imb.a \
	$(IPMITOOL)/src/plugins/lan/.libs/libintf_lan.a \
	$(IPMITOOL)/src/plugins/lanplus/.libs/libintf_lanplus.a \
	$(IPMITOOL)/src/plugins/open/.libs/libintf_open.a \
	$(IPMITOOL)/src/plugins/serial/.libs/libintf_serial.a \
	$(IPMITOOL)/lib/.libs/libipmitool.a \
	-lcrypto

libipmi.so: ipmi.c
	gcc -o libipmi.so -I $(IPMITOOL)/include/ -I $(IPMITOOL)/src/plugins/lanplus/ -shared -fpic ipmi.c $(LIBS)

test: 
	node ipmi.js
