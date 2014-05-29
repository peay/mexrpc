#include <capnp/ez-rpc.h>
#include <capnp/message.h>
#include "proto.capnp.h"

void rpc_client(int nlhs, uint8_t** lhs_serial, size_t* lhs_sizes,
                        int nrhs, uint8_t** rhs_serial, size_t* rhs_sizes,
                        const char* host_port)
{
    capnp::EzRpcClient client(host_port, 2360);
    auto& waitScope = client.getWaitScope();

    Call::Client cap = client.importCap<Call>("call");

    capnp::MallocMessageBuilder message;

    // Make RHS argument list
    ArgList::Builder in = message.initRoot<ArgList>();
    capnp::List<ArgList::Arg>::Builder arg_list = in.initArgs(nrhs);

    for (size_t i = 0; i < (unsigned int) nrhs; ++i)
    {
        arg_list[i].setSize(rhs_sizes[i]);
        arg_list[i].setData(capnp::Data::Reader(reinterpret_cast<const unsigned char*>(rhs_serial[i]), rhs_sizes[i] * sizeof(uint8_t)));
    }

    in.setType(ArgList::Type::RIGHT);

    // Make request
    auto request = cap.callRequest();
    request.setOut_size(nlhs);
    request.setIn(in);

    auto promise = request.send();
    auto response = promise.wait(waitScope);
    auto out = response.getOut();

    // Populate lhs_serial and lhs_sizes
    capnp::List<ArgList::Arg>::Reader out_list = out.getArgs();

    for (size_t i = 0; i < (unsigned int) nlhs; ++i)
    {
        capnp::Data::Reader out_arg = out_list[i].getData();
        size_t num_els = out_list[i].getSize();

        lhs_serial[i] = new uint8_t[num_els];
        lhs_sizes[i] = num_els;

        memcpy(lhs_serial[i], out_arg.begin(), num_els * sizeof(uint8_t));
    }
}
