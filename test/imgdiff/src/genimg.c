/* 
 * _GENIMG_C_ 
 * 
 * HMCSIM PHYSRAND TEST 
 * 
 * FUNCTIONS TO GENERATE RANDOM 
 * ADDRESS INPUTS FOR TEST
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* --------------------------------------------- MACROS */
#define ONEGB	1073741824


/* --------------------------------------------- LONGRANDS */
/* 
 * LONGRANDS
 * 
 */
static long longrands( long max_rand )
{
	/* vars */
	long newrand	= 0x00l;
	long a		= 0x00l;
	long b		= 0x00l;
	long shiftvar	= 0x00l;
	/* ---- */

	/* 
	 * decide if we need to bound the rand
	 *
	 */
	if( max_rand > (long)(RAND_MAX) ){

		/* 
	 	 * no need to bound the compute
		 * 
		 */
		a = (long)(rand());
		b = (long)(rand());
		shiftvar = max_rand - (long)(RAND_MAX);	
		shiftvar = shiftvar % (long)(8);
		newrand = a | (b << shiftvar);

	}else{
		/* 
	 	 * bound the compute 
		 *
		 */
		a = (long)(rand() % (int)(max_rand));

		/* 
		 * no need for 'b'
		 *
		 */

		newrand = a;
	}


	return newrand;
}


/* --------------------------------------------- GENIMG */
/* 
 * GENIMG
 * 
 */
extern int genimg( 	uint64_t *addr, 
			uint32_t seed, 
			uint32_t num_devs,
			uint32_t capacity, 
            uint64_t imgsize,
			uint32_t shiftamt )
{
	// vars
    int i    		= 0;
	long max_slot	= 0x00l;

	// sanity check 
	if( addr == NULL ){ return -1;}


    
	
	// figure out where the bounds of the memory is, represent it as the offset in 8 byte quantities
	max_slot = (long)( ((long)(ONEGB)*(long)(capacity)*(long)(num_devs))/(long)(8) );

    // Subtract image size so it doesn't fall off //Wes
     max_slot = max_slot - (long)(imgsize) ; 
	
	// seed the random number generator
	srand( (unsigned int)(seed) );

	// generate the address for the images bases via a randomly generated offset
    // Two for the input images and one for the output
    for(i = 0; i < 3; i++){
	    addr[i] = (uint64_t)(longrands( max_slot ) * (long)(8))<<(uint64_t)(shiftamt);
	}

	return 0;
}



/* EOF */

