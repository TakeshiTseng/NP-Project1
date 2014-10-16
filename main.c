#include <stdio.h>
#include "main.h"
#include <unistd.h>
#include <stdlib.h>
#include "server.h"


int main(int argc, const char *argv[])
{
	char* env_path = getenv("PATH");
	if(env_path){
		printf("%s\n", env_path);
	} else {
		printf("can't get path\n");
	}
	return 0;
}


