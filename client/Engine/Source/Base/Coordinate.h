#ifndef ENGINE_SOURCE_BASE_COORDINATE_H
#define ENGINE_SOURCE_BASE_COORDINATE_H

float CoordinateGetAspectCompensation();
float CoordinateGetAspectRatio();
void CoordinateSetAspectRatio(float aspect);
void DDCToNDC(float ddcx, float ddcy, float* ndcx, float* ndcy);
float DDCToNDCHeight(float ddcy);
float DDCToNDCWidth(float ddcx);
void NDCToDDC(float ndcx, float ndcy, float* ddcx, float* ddcy);
float NDCToDDCHeight(float ndcy);
float NDCToDDCWidth(float ndcx);

#endif
