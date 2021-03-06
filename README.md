MEXRPC
======

This is a small MEX client + server, which allows to call MEX files remotely. This piece of code has flexibility in mind, not performance. In particular, there are no restrictions to input/output arguments (which can be structs, cell arrays, or any valid MATLAB construct). This is achieved through serialization, but requires copies which may incurr a performance overhead when using MEXRPC on large input/output data.

Usage
-----
On the server side,

    ./mex_server hostname:port path-to-mex-file

This starts a server bound on hostname:port, that will accept requests from the client,
call the MEX file with the arguments provided by the client, and send the results back
to the client.

On the client side, within MATLAB,

    [out1, out2] = mex_client('hostname:port', arg1, arg2);

There are no restrictions on input or output arguments - these can be cells, structs, matrices, sparse arrays, strings, whatever.

Current restrictions: cannot interrupt with  Ctrl-C, there is no error reporting (printfs and errors while running the MEX on the server are NOT transfered back to the client, which will only say that the call failed)

Installation
------------
The RPC aspects are based on Cap'n Proto. Install Cap'n Proto:

    curl -O https://capnproto.org/capnproto-c++-0.4.1.tar.gz
    tar zxf capnproto-c++-0.4.1.tar.gz
    cd capnproto-c++-0.4.1
    ./configure
    make -j6 check
    sudo make install

To compile, edit Makefile to specify paths to MATLAB and Cap'n Proto, and run

    make

When running MATLAB or the server, make sure that library path contains the Cap'n Proto libraries, as well MATLAB libraries:

    libmx, libmex, libmat, libeng

and librpc_client (in MEXRPC's folder). See start_server.sh (generated by `make`) for an example.




