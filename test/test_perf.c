#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "pmi.h"


double get_time()
{
	struct timeval val;
	gettimeofday(&val, NULL);
	return (double)val.tv_sec + 1e-6 * val.tv_usec;
}

#define NUM 16384

//#define NUM 262144

int main(int argc, char ** argv )
{
	if( PMI_Init() != PMI_SUCCESS )
	{
		fprintf(stderr, "Could not init PMI\n");
		return 1;
	}

	int rank = 0;
        int size = 1;
	if( PMI_Get_size(&size) != PMI_SUCCESS )
	{
		fprintf(stderr, "Could not get PMI size\n");
		return 1;;
	}

	if( PMI_Get_rank(&rank) != PMI_SUCCESS )
	{
		fprintf(stderr, "Could not get PMI rank\n");
		return 1;
	}

	if( size != 2 )
	{
		fprintf(stderr, "Run this performance test on two processes\n");
		PMI_Finalize();
		return 1;
	}


	char key[PMI_STRING_LEN];
	char val[PMI_STRING_LEN];
	double start, end;
	double time;
	int i;
	
	if( rank == 0 )
	{
		time = 0.0;
		printf("Rank 0 sets %d keys ..", NUM);
		PMI_Lock();
		for( i = 0 ; i < NUM; i++)
		{
			if(!(i%1000))
				printf("%d / %d\n", i, NUM);
			snprintf(key, PMI_STRING_LEN, "iter_%d", i);
			snprintf(val, PMI_STRING_LEN, "%d", i);
			start = get_time();
			PMI_KVS_Put(key, val, strlen(val) + 1);
                        end = get_time();
			time += (end - start);

		}
		PMI_Unlock();
		fprintf(stderr, "Writing a key takes %g usec\n", 1e6*(time)/(double)NUM); 
	}


	/* Do a BARRIER */
	PMI_Barrier();

	/*if( rank == 1 )
	{
                long msg_size;
		time = 0.0;
		printf("Rank 1 reads %d keys ..", NUM);
		for( i = 0 ; i < NUM; i++)
		{
			if(!(i%1000))
				printf("%d / %d\n", i, NUM);
			snprintf(key, PMI_STRING_LEN, "iter_%d", i);
			snprintf(val, PMI_STRING_LEN, "%d", i);
			
			char retval[PMI_STRING_LEN];
			
			start = get_time();
			PMI_KVS_Get(key, retval, &msg_size);
			end = get_time();
			time += (end - start);

			if(strcmp(val, retval))
			{
                                fprintf(stderr, "size read : %ld\n", msg_size);
				fprintf(stderr, "Error in data %s instead of %s\n", retval, val);
                                //PMI_Finalize();
				//return 1;
			}
		}
		fprintf(stderr, "Reading a key takes %g usec\n", 1e6*(time)/(double)NUM); 
	}*/

        if( rank == 1 )
	{
		PMI_Lock();
                int errors = 0;
                long msg_size;
		time = 0.0;
		printf("Rank 1 reads %d keys ..", NUM);
		for( i = 0 ; i < NUM; i++)
		{
			snprintf(key, PMI_STRING_LEN, "iter_%d", i);
			
			start = get_time();
			PMI_KVS_Get_rqst(key);
			end = get_time();
			time += (end - start);
		}

		PMI_Unlock();
		PMI_Lock();

                for( i = 0 ; i < NUM; i++)
		{
                        if(!(i%1000))
				printf("%d / %d\n", i, NUM);
			char retval[PMI_STRING_LEN];

                        snprintf(val, PMI_STRING_LEN, "%d", i);
			
			start = get_time();
			int ret = PMI_KVS_Get_wait(retval, &msg_size);
			if(ret == PMI_NO_KEY)
				fprintf(stderr, "Could not get key\n");
			end = get_time();
			time += (end - start);

			//if(strcmp(val, retval))
                        if(memcmp(val, retval, strlen(val)))
			{
                                //PMI();
                                fflush(stdout);
                                fprintf(stderr, "size read : %ld\n", msg_size);
				fprintf(stderr, "Error in data %s instead of %s\n", retval, val);

                                for(int i = 0; i < msg_size; i++)
                                    fprintf(stderr, "%d ", retval[i]);

                                //PMI_Finalize();
				//return 1;
                                errors++;
			}
		}
		PMI_Unlock();

		fprintf(stderr, "Reading a key takes %g usec, with %d errors on %d tests\n", 1e6*(time)/(double)NUM, errors, NUM); 
	}


	if( PMI_Finalize() != PMI_SUCCESS )
	{
		fprintf(stderr, "Could not finalize PMI\n");
	}

	return 0;
}
