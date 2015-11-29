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


/* 
 * HMCSIM_PROCESS_DRE_QUEUE
 * 
 */
extern int	hmcsim_process_dre_queue( struct hmcsim_t *hmc )
{
	struct hmc_queue_t *queue = NULL;
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t x = 0;
	uint64_t head = 0x00ll;
	uint32_t cmd = 0x00;
	uint32_t tag = 0x00;
	uint64_t newhead = 0x00ll;
	uint64_t newtail = 0x00ll;
	uint64_t addr = 0x00ll;
	uint64_t newaddr = 0x00ll;
	uint64_t packet[HMC_MAX_UQ_PACKET];

	/* Loop through each DRE */
	for (i=0; i<hmc->num_dres; i++) {

		/* Process FILL and READ commands and generate new requests */
		
		if (hmc->devs[0].dres[i].rqst_queue[0].valid == HMC_RQST_VALID) {
			queue	= &(hmc->devs[0].dres[i].rqst_queue[0]);
			head	= queue->packet[0];
			cmd 	= (uint32_t)(head & 0x3F);
			tag		= (uint32_t)((head >> 15) & 0x1FF);

			switch(cmd)
			{
				case 0x39: /* Fill */
					/* Generate new requests */
					addr = hmc->devs[0].dres[i].baseAddr;
					for (x=0; x<hmc->devs[0].dres[i].numAccess; x++) {
						newaddr = addr+(x*4*hmc->devs[0].dres[i].stride);
						newhead |= (0x31 & 0x3F);
						newhead |= ( (uint64_t)(1 & 0xF) << 7 );
						newhead |= ( (uint64_t)(1 & 0xF) << 11 );
						newhead |= ( (uint64_t)(tag & 0x1FF) << 15 );
						newhead |= ( (uint64_t)(newaddr & 0x3FFFFFFFF) << 24 );

						newtail |= 0x03;
						newtail |= ( (uint64_t)(0x02 & 0xFF) << 8 );
						newtail |= ( (uint64_t)(0x01 & 0x7) << 16 );
						/* Set bit 23 to 1 for DRE internal request */
						newtail |= ( (uint64_t)(1 & 0x1) << 23 );
						newtail |= ( (uint64_t)(i & 0x7) << 24 );
						newtail |= ( (uint64_t)(0x01 & 0x1F) << 27 );;
						newtail |= ( (uint64_t)(0x11111111 & 0xFFFFFFFF) << 32 );

						packet[0] = newhead;
						packet[1] = newtail;
						hmcsim_send( hmc, &(packet[0]) );
					}
					
					break;

				case 0x31: /* RD32 */

					break;
			}
		}
		

		/* Send new requests to xbar queue for processing */

		/* Check the response queue for finished responses */

		/* When number of finished responses is equal to the number of requests, complete the FILL */

		/* Generate a response for the FILL command */
	}

}
