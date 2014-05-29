#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

extern void rpc_client(int nlhs, uint8_t** lhs_serial, size_t* lhs_sizes,
                        int nrhs, uint8_t** rhs_serial, size_t* rhs_sizes,
                        const char* host_port);

#endif