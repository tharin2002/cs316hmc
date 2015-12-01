/* 
 * _PHYSRAND_MAIN_C
 * 
 * HMCSIM PHYSRAND TEST DRIVER 
 * 
 * Attempts to initialize a HMC instantiation
 * Randomly sends data to the device and 
 * extracts output 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hmc_sim.h"

/* ----------------------------------------------------- FUNCTION PROTOTYPES */
extern int genimg( 	uint64_t *addr, 
			uint32_t seed, 
			uint32_t num_dev, 
			uint32_t capacity, 
            uint64_t imgsize,
			uint32_t shiftamt );
extern int execute_test( struct hmcsim_t *hmc, 
			uint64_t *addr,
            uint64_t imgsize,
            uint64_t width, 
			uint32_t stride );
extern int getshiftamount( uint32_t num_links, 
			uint32_t capacity,
			uint32_t bsize, 
			uint32_t *shiftamt );

/* ----------------------------------------------------- MAIN */
/* 
 * MAIN 
 * 
 */
extern int main( int argc, char **argv )
{
	/* vars */
	int ret			= 0;
	uint32_t i		= 0;
	uint32_t num_devs	= 0;
	uint32_t num_links	= 0;
	uint32_t num_vaults	= 0;
	uint32_t queue_depth	= 0;
	uint32_t num_banks	= 0;
	uint32_t num_drams	= 0;
	uint32_t capacity	= 0;
	uint32_t xbar_depth	= 0;	
	uint32_t seed		= 0;
	uint32_t bsize		= 128;
	uint32_t shiftamt	= 0;
//	long num_req		= 0x0Fl;
	uint64_t *addr		= NULL;
    uint64_t imgsize    = 0;
    uint64_t width      = 0;
    uint32_t stride     = 0;
//	uint32_t *req		= NULL;
	struct hmcsim_t hmc;
	/* ---- */

	while(( ret = getopt( argc, argv, "b:c:d:hl:m:n:q:v:x:N:T:S:W:" )) != -1 )
	{
		switch( ret )
		{
			case 'b': 
				num_banks = (uint32_t)(atoi(optarg));
				break;
			case 'c':
				capacity = (uint32_t)(atoi(optarg));
				break;
			case 'd': 
				num_drams = (uint32_t)(atoi(optarg));
				break;
			case 'h': 
				printf( "%s%s%s\n", "usage : ", argv[0], " -bcdhlmnqvxNTSW" );
				printf( " -b <num_banks>\n" );
				printf( " -c <capacity>\n" );
				printf( " -d <num_drams>\n" );
				printf( " -h ...print help\n" );
				printf( " -l <num_links>\n" );
				printf( " -m <max_bsize>\n" );
				printf( " -n <num_devs>\n" );
				printf( " -q <queue_depth>\n" );
				printf( " -v <num_vaults>\n" );
				printf( " -x <xbar_depth>\n" );
				printf( " -N <image_size>\n" );
                printf( " -W <image_width>\n" );    
                printf( " -T <stride>\n" );
				printf( " -S <seed>\n" );
				return 0;
				break;
			case 'l':
				num_links = (uint32_t)(atoi(optarg));
				break;
			case 'm':
				bsize 	= (uint32_t)(atoi(optarg));
				break;
			case 'n':
				num_devs = (uint32_t)(atoi(optarg));
				break;
			case 'q':
				queue_depth = (uint32_t)(atoi(optarg));
				break;
			case 'v': 
				num_vaults = (uint32_t)(atoi(optarg));
				break;
			case 'x': 
				xbar_depth = (uint32_t)(atoi(optarg));
				break;
			case 'N':
				imgsize	= (uint64_t)(atol(optarg));
				break;
            case 'W':
                width = (uint64_t)(atol(optarg));
                break;
 
            case 'T':
                stride = (uint32_t)(atoi(optarg));
                break;
			case 'S':
				seed = (uint32_t)(atoi(optarg));
				break;
			case '?':
			default:
				printf( "%s%s%s\n", "Unknown option: see ", argv[0], "-bcdhlnqvxNRSW" );
				return -1;
				break;
		}
	}

	/* 
	 * sanity check the runtime args 
	 * 
	 */

	if( (num_devs < 1 ) || ( num_devs > 8 ) ){
		printf( "FAILED TO VALIDATE ARGS: NUM DEVS OUT OF BOUNDS\n" );
		return -1;
	}

	if( (num_links != 4) && (num_links != 8) ){
		printf( "FAILED TO VALIDATE ARGS: NUM LINKS OUT OF BOUNDS\n" );
		return -1;
	}

	/* 
	 * allocate memory 
	 * 
 	 */
	addr = malloc( sizeof( uint64_t ) * 3 ); // Change later from dynamic to static alloation
	if( addr == NULL ){ 
		printf( "FAILED TO ALLOCATE MEMORY FOR ADDR\n" );
		return -1;
	}


	/* 
	 * get the address shift amount based upon 
	 * the max block size, device size and link count
	 *
	 */
	if( getshiftamount( num_links, capacity, bsize, &shiftamt ) != 0 ){ 
		printf( "FAILED TO RETRIEVE SHIFT AMOUNT\n" );
		hmcsim_free( &hmc );
		free( addr );
		return -1;
	}	
	
	/* 
	 * generate the inputs
	 * 
	 */
	if( genimg( 	addr, 
			seed, 
			num_devs,
			capacity,
            imgsize,
			shiftamt ) != 0 ){
		printf( "FAILED TO GENERATE IMAGE INPUTS\n" );
		free( addr );
		addr = NULL;
//		free( req );
//		req = NULL;
		return -1;
	}
	

	
	/* 
	 * init the library 
	 * 
	 */

	ret = hmcsim_init(	&hmc, 
                num_devs,
                num_links,
                num_vaults,
                queue_depth,
                num_banks,
                num_drams,
                capacity,
                xbar_depth,
                1,4 );



	if( ret != 0 ){ 
		printf( "FAILED TO INIT HMCSIM : return code=%d\n", ret );	
		return -1;
	}else {
		printf( "SUCCESS : INITALIZED HMCSIM\n" );
	}


	/* 
	 * setup the device topology
	 * 
	 */
	if( num_devs > 1 ){ 

		/* -- TODO */			

	}else{ 
		/* 
	 	 * single device, connect everyone 
		 *
		 */
		for( i=0; i<num_links; i++ ){ 
	
			ret = hmcsim_link_config( &hmc, 
						(num_devs+1), 
						0, 
						i, 
						i,
						HMC_LINK_HOST_DEV );
	
			if( ret != 0 ){ 
				printf( "FAILED TO INIT LINK %d\n", i );
				hmcsim_free( &hmc );
				free( addr );
				addr = NULL;
//				free( req );
//				req = NULL;
				return -1;
			}else{ 
				printf( "SUCCESS : INITIALIZED LINK %d\n", i );
			}
		}
	}

	/* 
	 * init the max block size 
	 * 
 	 */
	ret = hmcsim_util_set_all_max_blocksize( &hmc, bsize );
	
	if( ret != 0 ){ 
		printf( "FAILED TO INIT MAX BLOCK SIZE\n" );
		hmcsim_free( &hmc );
		free( addr );
		addr = NULL;
//		free( req );
//		req = NULL;
		return -1;
	}else {
		printf( "SUCCESS : INITALIZED MAX BLOCK SIZE\n" );
	}

	/* 
	 * execute the test
	 * 
	 */
	if( execute_test( &hmc, addr, imgsize, width, stride ) != 0 ) {
		printf( "FAILED TO EXECUTE TEST\n" );
		hmcsim_free( &hmc );
		free( addr );
		addr = NULL;
//		free( req );
//		req = NULL;
		return -1;
	}

	/* 
	 * free the library data
	 * 
	 */	
	ret = hmcsim_free( &hmc );

	/*
	 * free the memory 
	 * 
 	 */
	free( addr );
	addr = NULL;
//	free( req );
//	req = NULL;
	
	if( ret != 0 ){ 
		printf( "FAILED TO FREE HMCSIM\n" );
		return -1;
	}else {
		printf( "SUCCESS : FREE'D HMCSIM\n" );
	}
	
	return ret;
}

/* EOF */
