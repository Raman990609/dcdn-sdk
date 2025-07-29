
target = \
		 lib/libdatachannel.a include/nlohmann \
		 lib/libcurl.a \
		 lib/libsqlite3.a \
		 lib/libspdlog.a \

InstallDir=$(shell pwd)

PLT ?= $(shell uname)
ifeq ($(PLT), Darwin)
    OpenSsl=--with-openssl=$(shell brew --prefix openssl@3)
else
	OpenSsl=--with-openssl
endif

all: $(target)
	@echo done

lib/libdatachannel.a: src/libdatachannel-0.23.1/build/libdatachannel.a
	cd src/libdatachannel-0.23.1/build; cmake --install . --prefix $(shell pwd)

include/nlohmann: src/libdatachannel-0.23.1/deps/json/single_include/nlohmann
	cp -r src/libdatachannel-0.23.1/deps/json/single_include/nlohmann include/

src/libdatachannel-0.23.1/build/libdatachannel.a: src/libdatachannel-0.23.1/build/Makefile
	cd src/libdatachannel-0.23.1/build; cmake --build .

src/libdatachannel-0.23.1/build/Makefile: src/libdatachannel-0.23.1/CMakeLists.txt
	cd src/libdatachannel-0.23.1; cmake -B build -DUSE_GNUTLS=0 -DUSE_NICE=0 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=0 -DNO_MEDIA=ON -DNO_EXAMPLES=ON -DNO_TESTS=ON

src/libdatachannel-0.23.1/CMakeLists.txt: src/libdatachannel-0.23.1.tar.bz2
	cd src; tar jxvf libdatachannel-0.23.1.tar.bz2
	@touch $@

lib/libcurl.a: src/curl-8.15.0/lib/.libs/libcurl.a
	cd src/curl-8.15.0; make install

src/curl-8.15.0/lib/.libs/libcurl.a: src/curl-8.15.0/Makefile
	cd src/curl-8.15.0; make

src/curl-8.15.0/Makefile: src/curl-8.15.0/configure
	cd src/curl-8.15.0; ./configure --prefix=$(InstallDir) --enable-shared=no $(OpenSsl) --disable-ldap --disable-ldaps --disable-ftp --disable-ipfs --disable-rtsp --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smb --disable-smtp --disable-gopher --disable-mqtt --disable-docs --without-libpsl

src/curl-8.15.0/configure: src/curl-8.15.0.tar.bz2
	cd src; tar jxvf curl-8.15.0.tar.bz2
	@touch $@

lib/libsqlite3.a: src/sqlite-autoconf-3500300/libsqlite3.a
	cd src/sqlite-autoconf-3500300; make install

src/sqlite-autoconf-3500300/libsqlite3.a: src/sqlite-autoconf-3500300/Makefile
	cd src/sqlite-autoconf-3500300; make

src/sqlite-autoconf-3500300/Makefile: src/sqlite-autoconf-3500300/configure
	cd src/sqlite-autoconf-3500300; ./configure --prefix=$(InstallDir) --disable-shared
	@touch $@

src/sqlite-autoconf-3500300/configure: src/sqlite-autoconf-3500300.tar.gz
	cd src; tar jxvf sqlite-autoconf-3500300.tar.gz
	@touch $@

lib/libspdlog.a: src/spdlog-1.15.3/build/libspdlog.a
	cd src/spdlog-1.15.3; cmake --install build --prefix $(InstallDir)

src/spdlog-1.15.3/build/libspdlog.a: src/spdlog-1.15.3/build/Makefile
	cd src/spdlog-1.15.3; cmake --build build

src/spdlog-1.15.3/build/Makefile: src/spdlog-1.15.3/CMakeLists.txt
	cd src/spdlog-1.15.3; cmake -B build
	@touch $@

src/spdlog-1.15.3/CMakeLists.txt: src/spdlog-1.15.3.tar.gz
	cd src; tar zxvf spdlog-1.15.3.tar.gz
	@touch $@

clean:
	rm -rf bin include lib share

distclean:
	rm -rf bin include lib share src/libdatachannel-0.23.1 src/curl-8.15.0 src/spdlog-1.15.3 src/sqlite-autoconf-3500300

