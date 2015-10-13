SHELL=/bin/bash
IPMITOOL_GIT = git://git.code.sf.net/p/ipmitool/source
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
	@if [ ! -e $(IPMITOOL) ]; then git clone ${IPMITOOL_GIT} $(IPMITOOL); \
		cd $(IPMITOOL) && git reset --hard ${IPMITOOL_VER} && ./bootstrap && ./configure --quiet; fi
	@cd $(IPMITOOL) && ${MAKE} -j2
	@gcc -o libipmi.so -I $(IPMITOOL)/include/ -I $(IPMITOOL)/src/plugins/lanplus/ -shared -fpic ipmi.c $(LIBS)


.PHONY: package
package:
	@echo "building package"; \
		export DEBEMAIL="hwimo robots <hwimo@hwimo.lab.emc.com>"; \
		export DEBFULLNAME="The HWIMO Robots"; \
		rm -f packagebuild; \
		mkdir -p packagebuild; \
		pushd packagebuild; \
		cp -f ../libipmi.so . ; \
		cp -rf ../debian . ; \
		debuild --no-lintian --no-tgz-check -us -uc; \
		popd


.PHONY: cleanall
cleanall:
	@rm -f libipmi.so
	@cd $(IPMITOOL) && ${MAKE} clean
	@rm -f *.build; rm -f *.deb; rm -f *.gz; rm -f *.changes; rm -f *.dsc; rm -fr packagebuild
.PHONY: install
install:
	@cp -f libipmi.so /usr/lib
