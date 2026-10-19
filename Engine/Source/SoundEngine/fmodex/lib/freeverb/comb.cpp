// Comb filter implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include "comb.h"

// VERIFY: 1ac812b2 auto
// OFFSET: 0x009450D0
comb::comb()
{
	filterstore = 0;
	bufidx = 0;
}

// VERIFY: 984df88f auto
// OFFSET: 0x009450E0
void comb::setbuffer(float *buf, int size) 
{
	buffer = buf; 
	bufsize = size;
}

// VERIFY: 47f33d47 auto
// OFFSET: 0x00945100
void comb::mute()
{
	for (int i=0; i<bufsize; i++)
		buffer[i]=0;
}

// VERIFY: 079b42a3 auto
// OFFSET: 0x00945120
void comb::setdamp(float val) 
{
	damp1 = val; 
	damp2 = 1-val;
}

float comb::getdamp() 
{
	return damp1;
}

void comb::setfeedback(float val) 
{
	feedback = val;
}

float comb::getfeedback() 
{
	return feedback;
}

// ends
