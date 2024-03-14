#include  <stdio.h>
#include  <string.h>
#include  <sys/types.h>
#include  <sys/wait.h>
#include <unistd.h>

int main(void){

    int pidfilho;
	int status;
	
	pidfilho = fork();

	if( pidfilho != 0 ){
	
		printf("\n pid do pai %d \n", getpid());
		
		waitpid(-1, &status, 0);
		
	}
	else {
		printf("\n pid do filho %d \n", getpid());
	
	}
	return 0;
}
    