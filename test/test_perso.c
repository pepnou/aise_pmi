#include "pmi.h"
#include <stdlib.h>
#include <stdio.h>


int main()
{

	PMI_Init();
	int rank, size, jobid;

	PMI_Get_size(&size);
	PMI_Get_rank(&rank);
	PMI_Get_job(&jobid);
	PMI_Barrier();


	fprintf(stderr, "%d / %d on job %d\n", rank + 1, size, jobid);

	PMI_Finalize();

	return 0;
}