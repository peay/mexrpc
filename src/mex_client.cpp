#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <exception>

#include "mex.h"
#include "matlab_serialize.h"
#include "rpc_client.h"

void mexFunction(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs)
{
    if (nrhs < 1 || (nrhs >= 1 && mxGetClassID(prhs[0]) != mxCHAR_CLASS))
    {
        mexErrMsgTxt("MEX-RPC: syntax: mex_client <host:port> <arguments>");
        return;
    }

    const char* host_port = mxArrayToString(prhs[0]);

    nrhs -= 1;
    const mxArray** prhs2 = prhs + 1;

    uint8_t** rhs_serial = new uint8_t*[nrhs];
    size_t* rhs_sizes = new size_t[nrhs];
    uint8_t** lhs_serial = new uint8_t*[nlhs];
    size_t* lhs_sizes = new size_t[nlhs];

    try
    {
        // serialize input
        serialize(nrhs, prhs2, rhs_serial, rhs_sizes);

        // make RPC call
        for (int i = 0; i < nlhs; ++i)
        {
            lhs_serial[i] = 0;
            lhs_sizes[i] = 0;
        }

        // call rpc client
        rpc_client(nlhs, lhs_serial, lhs_sizes, nrhs, rhs_serial, rhs_sizes, host_port);

        // deserialize output of RPC call
        deserialize(nlhs, lhs_serial, lhs_sizes, plhs);
    }
    catch (std::exception& ex)
    {
        mexPrintf("MEX-RPC: caught exception: %s\n", ex.what());
        mexErrMsgTxt("MEX-RPC: RPC call failed");

        return;
    }

    //
    for (int i = 0; i < nlhs; ++i)
        delete[] lhs_serial[i];

    delete[] lhs_serial;
    delete[] lhs_sizes;

    for (int i = 0; i < nrhs; ++i)
        delete[] rhs_serial[i];

    delete[] rhs_serial;
    delete[] rhs_sizes;
}
