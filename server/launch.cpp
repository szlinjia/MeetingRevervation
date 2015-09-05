#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "Commen.h"

int main (int argc, char *argv[])
{
	
	if (argc != 4)
	{
		printf ("usage: %d Test case ID.\n", argc);
		exit (0);
	}


	// kill all existing car processes, if any
	if (0 == fork ())
	{
		execlp ("/usr/bin/killall", "killall", "-9", "replica", NULL);
		exit (0);
	}

	sleep (2);
	printf ("Start to test case %s\n", argv[2]);
	// to launch 10 processes now
	for (int i = 0; i <RELICA_NUM; i++)//
	{
		char ID[8] = { 0 };
		sprintf (ID, "%d", i);
		if (0 == fork ())
		{
			execlp ("./replica", "replica", ID, argv[2], argv[3], NULL);
			printf ("fatal error happened! if you see this.\n");
			exit (-1);
		}

		printf ("Replica %s is created successfully!Client ip=%s\n", ID, argv[3]);
	}

	// parent process has nothing to do, goes infinite loop
	while (true) {};
	// this return will not ever happen
	printf ("fatal error happened! if you see this, too.\n");
	return 0;
}