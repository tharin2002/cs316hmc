/* 
 * _HMC_DRE_C_
 * 
 * HYBRID MEMORY CUBE SIMULATION LIBRARY 
 * 
 * DRE QUEUE HANDLER
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hmc_sim.h"

extern int hmcsim_util_zero_packet( struct hmc_queue_t *queue );
extern int	hmcsim_decode_rsp_cmd( 	hmc_response_t rsp_cmd, 
					uint8_t *cmd );
/* 
 * HMCSIM_PROCESS_DRE_QUEUE
 * 
 */
extern int	hmcsim_process_dre_queue( struct hmcsim_t *hmc )
{
	struct hmc_queue_t *queue = NULL;
	uint32_t i 				= 0;
	uint32_t j 				= 0;
	uint32_t k 				= 0;
	uint32_t x 				= 0;
	uint64_t head 			= 0x00ll;
	uint64_t tail			= 0x00ll;
	uint32_t cmd 			= 0x00;
	uint32_t tag 			= 0x00;
	uint32_t length			= 0x00;
	uint64_t newhead 		= 0x00ll;
	uint64_t newtail 		= 0x00ll;
	uint64_t addr 			= 0x00ll;
	uint64_t newaddr 		= 0x00ll;
	uint64_t rsp_head		= 0x00ll;
	uint64_t rsp_tail		= 0x00ll;
	uint64_t rsp_slid		= 0x00ll; 
	uint64_t rsp_tag		= 0x00ll;
	uint64_t rsp_crc		= 0x00ll;
	uint64_t rsp_rtc		= 0x00ll;
	uint64_t rsp_seq		= 0x00ll;
	uint64_t rsp_frp		= 0x00ll;
	uint64_t rsp_rrp		= 0x00ll;
	uint32_t rsp_len		= 0x00;
	uint32_t r_link			= 0;
	uint32_t r_slot			= hmc->xbar_depth+1;
	uint32_t slot   		= 0;
	hmc_response_t rsp_cmd	= RSP_ERROR;
	uint8_t tmp8			= 0x0;
	uint64_t packet[HMC_MAX_UQ_PACKET];
	uint32_t cur			= 0;

	/* Loop through each DRE */
	for (i=0; i<hmc->num_dres; i++) {

		/* Process FILL and READ commands and generate new requests */
		
		if (hmc->devs[0].dres[i].rqst_queue[0].valid == HMC_RQST_VALID) {
			queue	= &(hmc->devs[0].dres[i].rqst_queue[0]);
			head	= queue->packet[0];
			cmd 	= (uint32_t)(head & 0x3F);
			tag		= (uint32_t)((head >> 15) & 0x1FF);
			length 	= (uint32_t)( (head >> 7) & 0x0F );
			tail	= queue->packet[ ((length*2)-1) ];

			switch(cmd)
			{
				case 0x39: /* Fill */
					/* Generate new requests */
					addr = hmc->devs[0].dres[i].baseAddr;
					for (x=0; x<hmc->devs[0].dres[i].numAccess; x++) {
						newaddr = addr+(x*4*hmc->devs[0].dres[i].stride);
						/* CMD field - Hard coded to a read */
						newhead |= (0x31 & 0x3F);
						/* LNG field in flits */
						newhead |= ( (uint64_t)(1 & 0xF) << 7 );
						/* Duplicate LNG field in flits */
						newhead |= ( (uint64_t)(1 & 0xF) << 11 );
						/* Tag field, duplicating the tag on the original fill */
						newhead |= ( (uint64_t)(tag & 0x1FF) << 15 );
						/* Address based on calculated base and stride */
						newhead |= ( (uint64_t)(newaddr & 0x3FFFFFFFF) << 24 );

						/* RRP value */
						newtail |= 0x03;
						/* FRP value */
						newtail |= ( (uint64_t)(0x02 & 0xFF) << 8 );
						/* Sequence number */
						newtail |= ( (uint64_t)(0x01 & 0x7) << 16 );
						/* Set bit 23 to 1 for DRE internal request */
						newtail |= ( (uint64_t)(1 & 0x1) << 23 );
						/* Source link ID */
						newtail |= ( (uint64_t)(i & 0x7) << 24 );
						/* Return token count */
						newtail |= ( (uint64_t)(0x01 & 0x1F) << 27 );;
						/* CRC (Not checked for sim) */
						newtail |= ( (uint64_t)(0x11111111 & 0xFFFFFFFF) << 32 );

						packet[0] = newhead;
						packet[1] = newtail;
						/* Send new requests to xbar queue for processing */
						hmcsim_send( hmc, &(packet[0]) );
						hmcsim_util_zero_packet( queue );
						hmc->devs[0].dres[i].rqst_queue[0].valid = HMC_RQST_STALLED;
					}
					
					break;

				case 0x31: /* RD32 */
					/* Create the response */
					rsp_slid 	= ((tail>>24) & 0x07);
					rsp_tag		= ((head>>15) & 0x1FF );
					rsp_crc		= ((tail>>32) & 0xFFFFFFFF);
					rsp_rtc		= ((tail>>27) & 0x3F);
					rsp_seq		= ((tail>>16) & 0x07);
					rsp_frp		= ((tail>>8) & 0xFF);
					rsp_rrp		= (tail & 0xFF);

					/* -- decode the repsonse command : see hmc_response.c */
					hmcsim_decode_rsp_cmd( rsp_cmd, &(tmp8) );

					/* -- packet head */
					rsp_head	|= (tmp8 & 0x3F);
					rsp_head	|= (rsp_len<<8);
					rsp_head	|= (rsp_len<<11);
					rsp_head	|= (rsp_tag<<15);
					rsp_head	|= (rsp_slid<<39); 

					/* -- packet tail */
					rsp_tail	|= (rsp_rrp);
					rsp_tail	|= (rsp_frp<<8);
					rsp_tail	|= (rsp_seq<<16);
					rsp_tail	|= (rsp_rtc<<27);
					rsp_tail	|= (rsp_crc<<32); 

					packet[0] 		= rsp_head;
					packet[((rsp_len*2)-1)]	= rsp_tail;

					r_link = (uint32_t)((head>>39) & 0x7);
					/* Send the read response to the xbar queue */
					r_slot = hmc->xbar_depth+1;
					cur = hmc->xbar_depth-1;
					for( x=0; x<hmc->xbar_depth; x++ ){	
						if( hmc->devs[0].xbar[r_link].xbar_rsp[cur].valid == 
								HMC_RQST_INVALID ){
							/* empty queue slot */
							r_slot = cur;
						}
						cur--;
					}
					
					/* 
					 * if we found a good slot, insert it
					 * and zero the vault response slot
					 *
					 */
					if( r_slot != (hmc->xbar_depth+1) ){

						/*
						 * slot found!
						 * transfer the data
						 *
						 */
						hmc->devs[0].xbar[r_link].xbar_rsp[r_slot].valid = 
								HMC_RQST_VALID;
						for( x=0; x<HMC_MAX_UQ_PACKET; x++){ 
							hmc->devs[0].xbar[r_link].xbar_rsp[r_slot].packet[x] = 
									packet[x];
							packet[x]	= 0x00ll;
						}
						hmcsim_util_zero_packet( queue );

					}else{

						/* 
						 * STALL! 
						 *
						 */

						/*if( (hmc->tracelevel & HMC_TRACE_STALL)>0 ){ */
						
							/*
							 * print a trace signal 
							 *
							 */
							/*hmcsim_trace_stall( hmc, 
									i, 
									j, 
									k,
									0,
									0,
									0, 
									x, 
									2 );
						}*/
					}
					break;
			}
		}
		


		/* Check the response queue for finished responses */

		for ( j=0; j<hmc->dre_depth; j++ ) {
			if (hmc->devs[0].dres[i].rsp_queue[j].valid == HMC_RQST_VALID) {
				hmc->devs[0].dres[i].numBack += 1;
				hmc->devs[0].dres[i].rsp_queue[j].valid = 0;
			}
		}

		/* When number of finished responses is equal to the number of requests, complete the FILL */
		if ( hmc->devs[0].dres[i].numBack == hmc->devs[0].dres[i].numAccess && hmc->devs[0].dres[i].rqst_queue[0].valid == HMC_RQST_STALLED) {
			/* Generate a response stating that the fill is complete */
			queue	= &(hmc->devs[0].dres[i].rqst_queue[0]);
			head	= queue->packet[0];
			cmd 	= (uint32_t)(head & 0x3F);
			tag		= (uint32_t)((head >> 15) & 0x1FF);
			length 	= (uint32_t)( (head >> 7) & 0x0F );
			tail	= queue->packet[ ((length*2)-1) ];

			/* Create the response */
			rsp_slid 	= ((tail>>24) & 0x07);
			rsp_tag		= ((head>>15) & 0x1FF );
			rsp_crc		= ((tail>>32) & 0xFFFFFFFF);
			rsp_rtc		= ((tail>>27) & 0x3F);
			rsp_seq		= ((tail>>16) & 0x07);
			rsp_frp		= ((tail>>8) & 0xFF);
			rsp_rrp		= (tail & 0xFF);

			/* -- decode the repsonse command : see hmc_response.c */
			hmcsim_decode_rsp_cmd( rsp_cmd, &(tmp8) );

			/* -- packet head */
			rsp_head	|= (tmp8 & 0x3F);
			rsp_head	|= (rsp_len<<8);
			rsp_head	|= (rsp_len<<11);
			rsp_head	|= (rsp_tag<<15);
			rsp_head	|= (rsp_slid<<39); 

			/* -- packet tail */
			rsp_tail	|= (rsp_rrp);
			rsp_tail	|= (rsp_frp<<8);
			rsp_tail	|= (rsp_seq<<16);
			rsp_tail	|= (rsp_rtc<<27);
			rsp_tail	|= (rsp_crc<<32); 

			packet[0] 		= rsp_head;
			packet[((rsp_len*2)-1)]	= rsp_tail;

			r_link = (uint32_t)((head>>39) & 0x7);
			/* Send the read response to the xbar queue */
			r_slot = hmc->xbar_depth+1;
			cur = hmc->xbar_depth-1;
			for( x=0; x<hmc->xbar_depth; x++ ){	
				if( hmc->devs[0].xbar[r_link].xbar_rsp[cur].valid == 
						HMC_RQST_INVALID ){
					/* empty queue slot */
					r_slot = cur;
				}
				cur--;
			}
			
			/* 
			 * if we found a good slot, insert it
			 * and zero the vault response slot
			 *
			 */
			if( r_slot != (hmc->xbar_depth+1) ){

				/*
				 * slot found!
				 * transfer the data
				 *
				 */
				hmc->devs[0].xbar[r_link].xbar_rsp[r_slot].valid = 
						HMC_RQST_VALID;
				for( x=0; x<HMC_MAX_UQ_PACKET; x++){ 
					hmc->devs[0].xbar[r_link].xbar_rsp[r_slot].packet[x] = 
							packet[x];
					packet[x]	= 0x00ll;
				}
				hmcsim_util_zero_packet( queue );
				/* Clear numBack */
				hmc->devs[0].dres[i].numBack = 0;

			}else{

				/* 
				 * STALL! 
				 *
				 */

				/*if( (hmc->tracelevel & HMC_TRACE_STALL)>0 ){ */
				
					/*
					 * print a trace signal 
					 *
					 */
					/*hmcsim_trace_stall( hmc, 
							i, 
							j, 
							k,
							0,
							0,
							0, 
							x, 
							2 );
				}*/
			}
		}

		/* Shift items up in the request queue */
		for( j=1; j<hmc->dre_depth; j++ )
		{
			/* 
			 * if the slot is valid, look upstream in the queue
			 *
			 */
			if( hmc->devs[0].dres[i].rqst_queue[j].valid == HMC_RQST_VALID ) {

				/*
				 * find the lowest appropriate slot
				 *
				 */
				slot = j;
				for( k=(j-1); k>0; k-- ){
					if( hmc->devs[0].dres[i].rqst_queue[j].valid == HMC_RQST_INVALID ){
						slot = k;
					}
				}

				/* 
			 	 * check to see if a new slot was found
				 * if so, perform the swap 
				 *
				 */
				if( slot != i ){

					/* 
					 * perform the swap 
					 *
					 */
					for( k=0; k<HMC_MAX_UQ_PACKET; k++ ){ 

						hmc->devs[0].dres[i].rqst_queue[slot].packet[k] = 
							hmc->devs[0].dres[i].rqst_queue[j].packet[k];	

						hmc->devs[0].dres[i].rqst_queue[j].packet[k] =0x00ll;	
						
					}

					hmc->devs[0].dres[i].rqst_queue[slot].valid = 1;	
					hmc->devs[0].dres[i].rqst_queue[j].valid    = 0;	
				}
			} /* else, slot not valid.. move along */
		}

	}
	return 0;
}
