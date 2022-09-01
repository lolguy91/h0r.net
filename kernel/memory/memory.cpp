#include <memory/memory.h>

void memset(void* start, uint_64 value, uint_64 num) {

	if (num <= 8) {
		uint_8* valPtr = (uint_8*)&value;
		for (uint_8* ptr = (uint_8*)start; ptr < (uint_8*)((uint_64)start + num); ptr++) {
			*ptr = *valPtr;
			valPtr++;
		}

		return;
	}

	uint_64 proceedingBytes = num % 8;
	uint_64 newnum = num - proceedingBytes;

	for (uint_64* ptr = (uint_64*)start; ptr < (uint_64*)((uint_64)start + newnum); ptr++) {
		*ptr = value;
	}

	uint_8* valPtr = (uint_8*)&value;
	for (uint_8* ptr = (uint_8*)((uint_64)start + newnum); ptr < (uint_8*)((uint_64)start + num); ptr++)
	{
		*ptr = *valPtr;
		valPtr++;
	}
}
void memcpy(void* dest,void* src,uint_64 num){
   if(num <= 8){
        uint_8* srcptr = (uint_8*)src;
        for(uint_8* destptr = (uint_8*)dest;destptr < (uint_8*)((uint_64)dest + num);destptr++){
            *destptr = *srcptr;
            srcptr++;
        }  
       return;
    } 
    uint_64 proceedingBytes = num % 8;
    uint_64 newnum          = num - proceedingBytes;
    uint_64* srcptr          =(uint_64*)src;
    

    for(uint_64* destptr =(uint_64*)dest; destptr < (uint_64*)((uint_64)dest + newnum);destptr++){
        *destptr = *srcptr;
        srcptr++;
    }
    uint_8* srcptr8 = (uint_8*)src;
    for(uint_8* destptr = (uint_8*)((uint_64*)dest + newnum); destptr < (uint_8*)((uint_64)dest + num);destptr++){
        *destptr = *srcptr8;
        destptr++;
    }

} 
int memcmp(const unsigned char* aptr, const unsigned char* bptr, unsigned int n){
	const unsigned char* a = aptr,*b = bptr;
	for (unsigned int i = 0; i < n; i++)
	{
		if(a[i] < b[i]) return -1;
		else if(a[i] > b[i]) return 1;
	}
	return 0;
	
}