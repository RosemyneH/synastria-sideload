#ifndef ENGINE_SOURCE_STORM_SOURCE_SCOMP_H
#define ENGINE_SOURCE_STORM_SOURCE_SCOMP_H

#include <cstdint>

void Bzip2Compress(void* dest, uint32_t* destsize, const void* source, uint32_t sourcesize, uint32_t* hint, uint32_t optimization);
int32_t Bzip2Decompress(void* dest, uint32_t* destsize, const void* source, uint32_t sourcesize);
void HuffmanCompress(void* dest, uint32_t* destsize, const void* source, uint32_t sourcesize, uint32_t* hint, uint32_t optimization);
int32_t HuffmanDecompress(void* dest, uint32_t* destsize, const void* source, uint32_t sourcesize);
void ImaAdpcmMonoCompress(void* dest, uint32_t* destsize, const void* source, uint32_t sourcesize, uint32_t* hint, uint32_t optimization);
int32_t ImaAdpcmMonoDecompress(void* dest, uint32_t* destsize, const void* source, uint32_t sourcesize);
void ImaAdpcmStereoCompress(void* dest, uint32_t* destsize, const void* source, uint32_t sourcesize, uint32_t* hint, uint32_t optimization);
int32_t ImaAdpcmStereoDecompress(void* dest, uint32_t* destsize, const void* source, uint32_t sourcesize);
void RLEZeroCompress(void* dest, uint32_t* destsize, const void* source, uint32_t sourcesize, uint32_t* hint, uint32_t optimization);
int32_t RLEZeroDecompress(void* dest, uint32_t* destsize, const void* source, uint32_t sourcesize);
void ZlibCompress(void* dest, uint32_t* destsize, const void* source, uint32_t sourcesize, uint32_t* hint, uint32_t optimization);
int32_t ZlibDecompress(void* dest, uint32_t* destsize, const void* source, uint32_t sourcesize);

#endif
