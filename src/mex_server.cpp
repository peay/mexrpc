#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdio>
#include <iostream>
#include <ctime>

#include <dlfcn.h>
#include <capnp/ez-rpc.h>
#include <capnp/message.h>

#include "mex.h"
#include "proto.capnp.h"
#include "matlab_serialize.h"

#define VERBOSE

// MEX shared library
typedef int (*mexFunctionEntryPoint)(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs);
mexFunctionEntryPoint func = 0;

class CallImpl final: public Call::Server
{
public:
    kj::Promise<void> call(CallContext context) override
    {
        clock_t time_a = clock();

#ifdef VERBOSE
        std::cout << "Query.." << std::endl;

        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];

        time (&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer,80,"%d-%m-%Y %I:%M:%S",timeinfo);
        std::string str(buffer);

        std::cout << str;
#endif

        const int nlhs = context.getParams().getOut_size();

        const capnp::List<ArgList::Arg>::Reader arg_list = context.getParams().getIn().getArgs();
        const int nrhs = arg_list.size();
        const int typei = (int) context.getParams().getIn().getType();

        uint8_t** rhs_serial = new uint8_t*[nrhs];
        size_t* rhs_sizes = new size_t[nrhs];
        size_t i = 0;

        for (ArgList::Arg::Reader arg : arg_list)
        {
            rhs_sizes[i] = (size_t) arg.getSize();
            rhs_serial[i] = new uint8_t[arg.getSize()];
            memcpy(rhs_serial[i], arg.getData().begin(), arg.getSize() * sizeof(uint8_t));
            ++i;
        }

        // deserialize
        mxArray** prhs = new mxArray*[nrhs];
        deserialize(nrhs, rhs_serial, rhs_sizes, prhs);

        mxArray** plhs = new mxArray*[nlhs];

        // call mexFunction
        uint8_t** lhs_serial = new uint8_t*[nlhs];
        size_t* lhs_sizes = new size_t[nlhs];

        for (int i = 0; i < nlhs; ++i)
        {
            plhs[i] = 0;
            lhs_serial[i] = 0;
            lhs_sizes[i] = 0;
        }


        // make a query, submit to the queue, and wait
#ifdef VERBOSE
        std::cout << "Running MEX" << std::endl;
#endif

        try {
            func(nlhs, plhs, nrhs, (const mxArray**) prhs);
        }
        catch (...) {
            std::cerr << "Query failed!" << std::endl;
        }

#ifdef VERBOSE
        std::cout << "Done running MEX" << std::endl;
#endif

        // reserialize, step 1
        serialize(nlhs, (const mxArray**) plhs, lhs_serial, lhs_sizes);

        // reserialize, step 2
        capnp::MallocMessageBuilder message;

        // Make RHS argument list
        ArgList::Builder arg_out = message.initRoot<ArgList>();
        capnp::List<ArgList::Arg>::Builder args_out = arg_out.initArgs(nlhs);

        for (size_t i = 0; i < nlhs; ++i)
        {
            args_out[i].setSize(lhs_sizes[i]);
            args_out[i].setData(capnp::Data::Reader(reinterpret_cast<const unsigned char*>(lhs_serial[i]), lhs_sizes[i] * sizeof(uint8_t)));
        }

        arg_out.setType(ArgList::Type::LEFT);
        context.getResults().setOut(arg_out);

        clock_t time_b = clock();
        unsigned int total_time_ticks = (unsigned int)(time_b - time_a);

#ifdef VERBOSE
        std::cout << "Served in " << total_time_ticks << " ticks" << std::endl;
#endif

        for (int i = 0; i < nlhs; ++i)
        {
            if (plhs[i])
                mxDestroyArray(plhs[i]);

            if (lhs_serial[i])
                delete[] lhs_serial[i];
        }

        delete[] plhs;
        delete[] lhs_serial;
        delete[] lhs_sizes;


        for (int i = 0; i < nrhs; ++i)
        {
            if (prhs[i])
                mxDestroyArray(prhs[i]);

            if (rhs_serial[i])
                delete[] rhs_serial[i];
        }

        delete[] prhs;
        delete[] rhs_serial;
        delete[] rhs_sizes;

        return kj::READY_NOW;
    }

private:

};

int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "usage: " << argv[0] << " ADDRESS[:PORT] MEXFILE" << std::endl;
        return 1;
    }

    void *myso = dlopen(argv[2], RTLD_NOW);
    func = (mexFunctionEntryPoint) dlsym(myso, "mexFunction");

    if (!func)
    {
        std::cerr << "cannot load mex file! aborting." << std::endl;
        return 1;
    }

    std::cout << "running." << std::endl;

    capnp::EzRpcServer server(argv[1], 2360);
    auto& waitScope = server.getWaitScope();

    server.exportCap("call", kj::heap<CallImpl>());

    kj::NEVER_DONE.wait(waitScope);
}
