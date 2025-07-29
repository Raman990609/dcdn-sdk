include ../config.mk

INCFLAGS = -I.. 

LIBFLAGS = \
		   -L../deps/lib -lcurl -lspdlog -lz \

PLT ?= $(shell uname)
ifeq ($(PLT), Darwin)
	OpenSslDir=$(shell brew --prefix openssl@3)
	INCFLAGS += -I$(OpenSslDir)/include
    LIBFLAGS += -L$(OpenSslDir)/lib -lssl -lcrypto
	LIBFLAGS += -framework CoreFoundation -framework SystemConfiguration
else
    LIBFLAGS += -lssl -lcrypto
endif	

testTargets = \
			  HttpClientTest \

test: $(testTargets)

HttpClientTest: HttpClientTest.o
	$(CXX) -o $@ $^ $(LIBFLAGS)

%.o : %.cpp
	$(CXX) -c $(INCFLAGS) $^

clean:
	rm -rf $(testTargets) *.o
