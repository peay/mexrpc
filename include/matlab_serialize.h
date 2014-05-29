#ifndef MATLABSERI_H
#define MATLABSERI_H

void serialize(const int in_size, const mxArray** in, uint8_t** out, size_t* out_sizes);
void deserialize(const int in_size, uint8_t** in, size_t* in_sizes, mxArray** out);

#endif
