/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32141925
*	    Student name : Park Jong Bum
*
*   lab1_sched_test.c :
*       - Lab1 source file.
*       - Must contains scueduler algorithm test code.
*
*/

#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <asm/unistd.h>

#include "lab1_sched_types.h"

/*
 * you need to implement scheduler simlator test code.
 *
 */

int main(int argc, char *argv[]){
	
	my_proc proc[5] = { {'A', 3, 0, 0}, {'B', 6, 2, 0}, {'C', 4, 4, 0},
				{'D', 5, 6, 0}, {'E', 2, 8, 0}};
	printf("------------------------------------------------------------------\n");
	My_FCFS(proc, 5);
	My_RRq1(proc, 5);
	My_RRq4(proc, 5);
	My_SJF(proc, 5);
	My_MLFQq1(proc, 5);
	My_MLFQq2(proc, 5);
	printf("-----------------------------------------------------------------\n");
	return 0;
}

