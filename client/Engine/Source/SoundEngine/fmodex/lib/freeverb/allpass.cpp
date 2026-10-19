// Allpass filter implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include "allpass.h"

// VERIFY: ebfd92d9 auto
// OFFSET: 0x00945150
allpass::allpass()
{
	bufidx = 0;
}

// VERIFY: 7bd3ba61 auto
// OFFSET: 0x00945160
void allpass::setbuffer(float *buf, int size) 
{
	buffer = buf; 
	bufsize = size;
}

// VERIFY: 3df7356d auto
// OFFSET: 0x00945180
void allpass::mute()
{
	for (int i=0; i<bufsize; i++)
		buffer[i]=0;
}

// VERIFY: 92ab74ed auto
// OFFSET: 0x00945140
void allpass::setfeedback(float val) 
{
	feedback = val;
}

float allpass::getfeedback() 
{
	return feedback;
}

//ends
