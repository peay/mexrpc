
# -- MATLAB configuration
# path to MATLAB installation
MATLABROOT=/Applications/MATLAB_R2011b.app

# architecture
MATLABARCH=maci64

# -- Cap'n'proto configuration
# path to Cap'n'proto installation
CAPNPPATH=capnproto-c++-0.4.1

# path to Cap'n'proto compiler
CAPNP=capnp

# -- Compiler configuration
CXX=g++
CC=gcc
CFLAGS=-Iinclude -I$(CAPNPPATH)/src
LDFLAGS=
DEBUG=

# -- Output
SERVER=mex_server
CLIENT=mex_client

# -- MATLAB options
MEXLINKOPTS=-pthread
MEXLINKOPTS2=-L$(MATLABROOT)/bin/$(MATLABARCH)

# -- Main rules
all: $(SERVER) $(CLIENT)

debug: DEBUG += -g
debug: $(SERVER) $(CLIENT)

# Cap'n'proto interface
src/proto.capnp.c++ include/proto.capnp.h:
	$(CAPNP) compile -oc++:src/ capnp/proto.capnp; \
	mv src/proto.capnp.h include/proto.capnp.h

.PHONY: clean


# -- Server
$(SERVER): start_server.sh src/proto.capnp.c++ src/matlab_serialize.cpp src/mex_server.cpp
	$(CXX) -D_THREAD_SAFE $(LDFLAGS) $(CFLAGS) \
			-I$(MATLABROOT)/extern/include \
			-L$(MATLABROOT)/bin/$(MATLABARCH) \
			$(DEBUG) \
			-std=c++11 \
			$(MEXLINKOPTS) \
			-o $@ \
			src/proto.capnp.c++ src/matlab_serialize.cpp src/mex_server.cpp \
			$(MEXLINKOPTS2) \
			-lcapnp -lcapnp-rpc -lkj -lkj-async -lmx -leng

start_server.sh: 
	echo '#!/bin/bash' > $@
	echo 'IP=localhost' >> $@
	echo 'PORT=$$1' >> $@
	echo 'MEX="your-mex-file.mexmaci64"' >> $@
	echo "PATHADD=$(MATLABROOT)/bin/$(MATLABARCH)" >> $@
	echo 'LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:$$PATHADD ./$(SERVER) $$IP:$$PORT $$MEX' >> $@

# -- Client
$(CLIENT): librpc_client.so src/matlab_serialize.cpp src/mex_client.cpp
	$(MATLABROOT)/bin/mex $(CFLAGS) -L. src/matlab_serialize.cpp src/mex_client.cpp -o $@ -lrpc_client

librpc_client.so: rpc_client.o proto.capnp.o
	$(CXX) $(DEBUG) $(LDFLAGS) \
		-shared -o $@ \
		rpc_client.o proto.capnp.o \
		-lcapnp -lcapnp-rpc -lkj -lkj-async

rpc_client.o: src/proto.capnp.c++ src/rpc_client.cpp
	$(CXX) -c -Wall -Werror -fPIC -std=c++11 \
			$(DEBUG) $(CFLAGS) \
			src/proto.capnp.c++ src/rpc_client.cpp

clean:
		@rm -f *.o
		@rm -f *.so
		@rm -f start_server.sh
		@rm -f $(CLIENT)
		@rm -f $(SERVER)


