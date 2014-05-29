#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "mex.h"
#include "matlab_serialize.h"


// See undocumented MATLAB website
#if defined(__cplusplus) && defined(MATRIX_DLL_EXPORT_SYM)
    #define EXTERN_C extern
namespace matrix{ namespace detail{ namespace noninlined{ namespace mx_array_api{
#endif

    EXTERN_C mxArray* mxSerialize(mxArray const *);
    EXTERN_C mxArray* mxDeserialize(const void *, size_t);

#if defined(__cplusplus) && defined(MATRIX_DLL_EXPORT_SYM)
}}}}
using namespace matrix::detail::noninlined::mx_array_api;
#endif

// In order to check for ctrl-c (in the future)
#ifdef __cplusplus
extern "C" bool utIsInterruptPending();
#else
extern bool utIsInterruptPending();
#endif

void serialize(const int in_size, const mxArray** in, uint8_t** out, size_t* out_sizes)
{
    for (int i = 0; i < in_size; ++i)
    {
        if (!in[i])
        {
            out[i] = 0;
            out_sizes[i] = 0;
        }
        else
        {
            mxArray* serialized = mxSerialize(in[i]);
            size_t num_els = mxGetNumberOfElements(serialized);

            out[i] = new uint8_t[num_els];
            memcpy(out[i], mxGetData(serialized), sizeof(uint8_t) * num_els);
            mxDestroyArray(serialized);

            out_sizes[i] = num_els;
        }
    }
}

void deserialize(const int in_size, uint8_t** in, size_t* in_sizes, mxArray** out)
{
    for (int i = 0; i < in_size; ++i)
    {
        size_t num_els = in_sizes[i];
        mxArray* serialized = mxCreateNumericMatrix(1, num_els, mxUINT8_CLASS, mxREAL);

        memcpy(mxGetData(serialized), in[i], sizeof(uint8_t) * num_els);
        out[i] = (mxArray *) mxDeserialize(mxGetData(serialized), num_els);
        mxDestroyArray(serialized);
    }
}