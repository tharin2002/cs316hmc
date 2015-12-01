/* 
 * _EXECUTE_TEST_C_ 
 * 
 * HMCSIM PHYSRAND TEST EXECUTION FUNCTIONS
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "hmc_sim.h"


/* ---------------------------------------------------- ZERO_PACKET */
/* 
 * ZERO_PACKET 
 * 
 */
static void zero_packet( uint64_t *packet )
{
	uint64_t i = 0x00ll;

	/* 
	 * zero the packet
	 * 
	 */
	for( i=0; i<HMC_MAX_UQ_PACKET; i++ ){
		packet[i] = 0x00ll;
	} 


	return ;
}

/* ---------------------------------------------------- EXECUTE_TEST */
/* 
 * EXECUTE TEST
 * 
 */
extern int execute_test(	struct hmcsim_t *hmc, 
				uint64_t *addr, 
                uint64_t imgsize, //In bytes ??
				uint64_t width, //In pixels ??
                uint32_t stride )
{
	/* vars */
	uint32_t z		= 0x00;
	long iter		= 0x00l;
	uint64_t head		= 0x00ll;
	uint64_t tail		= 0x00ll;
	uint64_t payload[8]	= {0x00ll,0x00ll,0x00ll,0x00ll,
				   0x00ll,0x00ll,0x00ll,0x00ll};
	uint64_t packet[HMC_MAX_UQ_PACKET];
	uint8_t	cub		= 0;
	uint16_t tag		= 0;
	uint8_t link		= 0;
	int ret			= HMC_OK;
	int stall_sig		= 0;
	
	FILE *ofile		= NULL;
	int *rtns		= NULL;
    long total_sent     =0;
    long total_recv     =0;
	int done		= 0;


    uint64_t d_response_head;
    uint64_t d_response_tail;
    hmc_response_t d_type;
    uint8_t d_length;
    uint16_t d_tag;
    uint8_t d_rtn_tag;
    uint8_t d_src_link;
    uint8_t d_rrp;
    uint8_t d_frp;
    uint8_t d_seq;
    uint8_t d_dinv;
    uint8_t d_errstat;
    uint8_t d_rtc;
    uint32_t d_crc;



    //Size of strided image
//    uint64_t stridedsize = imgsize / (stride * stride);
    uint64_t rows_processed = imgsize/(width * 4 * stride);
    long row_req = (((width * 4) / stride) / 8) + (((width * 4) / stride) % 8); //How many packets does it take to transfer one row of strided reads?
    long num_req = rows_processed * row_req * 4; //( stridedsize / (8 * 4)) + (2 * row_req); //We will have an overhead of three  packets: setup, release, and one 32 bytes read in addition to fil  




    //FSM States
    uint8_t isSetup = 1;
    uint8_t isRelease = 0;
    uint8_t isFill = 0;
    uint8_t isRead = 0;

    //FSM Control variables
    uint64_t rowbase = addr[0];
    long setup_packet = 0;
    uint64_t row = 0;
    uint64_t base_address = 0;
    uint8_t pixels = 0;

    //Keeping track of DREs
    short * waiting = NULL;
    short * dre_id = NULL; 
    long * dre_addr = NULL;




    waiting = malloc( sizeof( short ) * hmc->num_links );
    memset( waiting, -1, sizeof( short ) * hmc->num_links );

    dre_id = malloc( sizeof( short ) * hmc->num_links );
    memset( dre_id, -1, sizeof( short ) * hmc->num_links );

    dre_addr = malloc( sizeof( long ) * hmc->num_links );
    memset( dre_addr, -1, sizeof( long ) * hmc->num_links );



	/* ---- */

	rtns = malloc( sizeof( int ) * hmc->num_links );
	memset( rtns, 0, sizeof( int ) * hmc->num_links );

	/* 
	 * Setup the tracing mechanisms
	 * 
	 */
	ofile = fopen( "imgdiff.out", "w" );	
	if( ofile == NULL ){ 
		printf( "FAILED : COULD NOT OPEN OUTPUT FILE imgdiff.out\n" );
		return -1;
	}


    FILE * afile = fopen( "imgdiff_addresses.out", "w" );
    if( afile == NULL ){
        printf( "FAILED : COULD NOT OPEN OUTPUT FILE imgdiff.out\n" );
        return -1;
    }




	hmcsim_trace_handle( hmc, ofile );
	hmcsim_trace_level( hmc, (HMC_TRACE_BANK|
				HMC_TRACE_QUEUE|
				HMC_TRACE_CMD|
				HMC_TRACE_STALL|
				HMC_TRACE_LATENCY) );

	printf( "SUCCESS : INITIALIZED TRACE HANDLERS\n" );				
	

	/* 
	 * zero the packet
	 * 
	 */
	zero_packet( &(packet[0]) );

	printf( "SUCCESS : ZERO'D PACKETS\n" );
	printf( "SUCCESS : BEGINNING TEST EXECUTION\n" );




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    // Attempt to execute all the requests, Push requests into the device  until we get a stall signal 

    while (!done){

		// attempt to push a request in as long as we don't stall 		
		if( iter >= num_req ){  
			/* everything is sent, go to receive side */
			goto packet_recv;
		}

		printf( "....sending packets\n" );
		while( ret != HMC_STALL ){

				printf( "...building read request for device : %d\n", cub );                


                if(isSetup){


                    hmcsim_build_memrequest( hmc,
                                            cub,
                                            addr[iter],
                                            tag,
                                            DRE_SETUP,
                                            link,
                                            &(payload[0]),
                                            &head,
                                            &tail );


                    if (setup_packet < row_req-1)
                        pixels = 8;
                    else 
                        pixels = width % 8;            
                    
                    rowbase = addr[0] + (row * width * 4); 
                    base_address = rowbase + (setup_packet * stride * 4 * pixels);
                    

                    packet[0] = head;//UPDATE
                    packet[1] = base_address;
                    packet[2] = stride * 4; // Striding accounts for pixels occupying 4 bytes 
                    packet[3] = pixels;    //Number of elements (pixels) to be retrieved
                    packet[4] = 0; //other;
                    packet[5] = tail;//UPDATE


                    fprintf(afile, "Image address: %ld. Row address %ld. Setup msg \n", (long) addr[0], (long) rowbase );
                    fprintf(afile, "Setup seq %d requesting %d pixels from base address %d. \n", (int) setup_packet, (int) pixels, (int) base_address);
                }

               else if(isFill){

                    if(waiting[link] < 0){
                        hmcsim_build_memrequest( hmc,
                                                cub,
                                                addr[iter],
                                                tag,
                                                DRE_FILL,
                                                link,
                                                &(payload[0]),
                                                &head,
                                                &tail );

                        packet[0] = head;
                        packet[1] = dre_id[link];
                        packet[2] = 0;//other;
                        packet[3] = tail;
                    }

                }

                else if (isRead){
                    hmcsim_build_memrequest( hmc,
                                            cub,
                                            dre_addr[link],
                                            tag,
                                            RD32,
                                            link,
                                            &(payload[0]),
                                            &head,
                                            &tail );
     
                    packet[0] = head;
                    packet[1] = tail;

                    fprintf(afile, "Reading from DRE %d, address %d. \n", (int) dre_id[link], (int) dre_addr[link] );
                }


               else if(isRelease){
                    hmcsim_build_memrequest( hmc,
                                            cub,
                                            addr[iter],
                                            tag,
                                            DRE_RELEASE,
                                            link,
                                            &(payload[0]),
                                            &head,
                                            &tail );

                packet[0] = head;
                packet[1] = dre_id[link];
                packet[2] = 0;//other;
                packet[3] = tail;

                   fprintf(afile, "Releasing DRE %d. \n", (int) dre_id[link]);
                }
                        
                else {

                   fprintf(afile, "Unidentified state \n" );

                }

			
			// Step 2: Send it 
			 
			printf( "...sending packet : base addr=0x%016llx\n", (long long int )addr[iter] );

#if 0
			for( i=0; i<HMC_MAX_UQ_PACKET; i++){ 
				printf( "packet[%" PRIu64 "] = 0x%016llx\n", i, packet[i] );
			}
#endif
	
            if (isFill && (waiting[link] >=  0)){
                ret = HMC_STALL;
                fprintf(afile, "Link %d stalled Waiting for setup response, tage %d \n", (int) link, (int) waiting[link] );
            }
            else
			    ret = hmcsim_send( hmc, &(packet[0]) );

			switch( ret ){ 
				case 0: 
					printf( "SUCCESS : PACKET WAS SUCCESSFULLY SENT\n" );
					iter++;
                     
                    if(isSetup){
                        isSetup=0;
                        waiting[link] = tag;
                        isFill = 1;                        
                    }

                    else if(isFill){
                            isFill = 0;
                            isRead = 1;
                    }
                    else if(isRead){
                            isRead = 0;
                            isRelease = 1;
                    }

                    else if(isRelease){
                        row += stride;
                        isSetup = 1;
                        isRelease = 0;
                        dre_addr[link] = -1;
                        dre_id[link] = -1;
                        if (setup_packet == row_req-1){
                            row += stride;
                            setup_packet = 0;
                        }
                        else
                            setup_packet++;
                    }
                    else
                        fprintf(afile, "Unidentified msg \n" );
   
                    total_sent++;
					break;
				case HMC_STALL:
					printf( "STALLED : PACKET WAS STALLED IN SENDING\n" );
					break;
				case -1:
				default:
					printf( "FAILED : PACKET SEND FAILED\n" );
					goto complete_failure;
					break;
			}

			/* 
			 * zero the packet 
			 * 
			 */
			zero_packet( &(packet[0]) );

			tag++;
			if( tag == 2048 ){
				tag = 0;
			}	

			link++;
			if( link == hmc->num_links ){
				/* -- TODO : look at the number of connected links
				 * to the host processor
				 */
				link = 0;
			}

			
			// check to see if we're at the end of the packet queue
			if( iter >= num_req ){ 
				goto packet_recv;
			}

			/* DONE SENDING REQUESTS */
		}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



packet_recv:
		/* 
		 * reset the return code for receives
		 * 
		 */
		ret = HMC_OK;

		/* 
		 * We hit a stall or an error
		 * 
		 * Try to drain the responses off all the links
		 * 
		 */
		printf( "...reading responses\n" );
		while( ret != HMC_STALL ){ 

			for( z=0; z< hmc->num_links; z++){ 
				
				rtns[z] = hmcsim_recv( hmc, cub, z, &(packet[0]) );

				if( rtns[z] == HMC_STALL ){ 
					stall_sig++;
				}else{ 
					/* successfully received a packet */
					printf( "SUCCESS : RECEIVED A SUCCESSFUL PACKET RESPONSE\n" );	
                                        hmcsim_decode_memresponse( hmc,
                                                                  &(packet[0]), 
                                                                  &d_response_head,
                                                                  &d_response_tail,
                                                                  &d_type,
                                                                  &d_length,
                                                                  &d_tag,
                                                                  &d_rtn_tag,
                                                                  &d_src_link,
                                                                  &d_rrp,
                                                                  &d_frp,
                                                                  &d_seq,
                                                                  &d_dinv,
                                                                  &d_errstat,
                                                                  &d_rtc,
                                                                  &d_crc );
                                        printf( "RECV tag=%d; rtn_tag=%d\n", d_tag, d_rtn_tag );
				
                    total_recv++;

                    if (waiting[z] == d_rtn_tag)
                        waiting[z] = -1;
                        dre_id[z]  = packet[1];
                        dre_addr[z] = packet[2];

				}

				/* 
				 * zero the packet 
				 * 
				 */
				zero_packet( &(packet[0]) );
			}

			/* count the number of stall signals received */
			if( stall_sig == hmc->num_links ){ 
				/* 
				 * if all links returned stalls, 
				 * then we're done receiving packets
				 *
				 */
				
				printf( "STALLED : STALLED IN RECEIVING \n");
				ret = HMC_STALL;
			}

			stall_sig = 0;
			for( z=0; z<hmc->num_links; z++){
				rtns[z] = HMC_OK;
			}
		}

		/* 
		 * reset the return code
		 * 
		 */
		stall_sig = 0;
		for( z=0; z<hmc->num_links; z++ ){ 
			rtns[z] = HMC_OK;
		}
		ret = HMC_OK;
	

		/* 
	 	 * done with sending/receiving packets
		 * update the clock 
		 */
		printf( "SIGNALING HMCSIM TO CLOCK\n" );
		hmcsim_clock( hmc );

		fflush( stdout );


		if( total_sent == num_req ){ 
			if( total_recv == num_req ){ 
                done = 1;
			}
		}
	}


        printf( "TOTAL_SENT = %ld\n", total_sent );
        printf( "TOTAL_RECV = %ld\n", total_recv );
        fflush( stdout );




complete_failure:

	fclose( ofile );
	ofile = NULL;

    fclose( afile );
    afile = NULL;

	free( rtns ); 
	rtns = NULL;

	return 0;
}








/* EOF */
