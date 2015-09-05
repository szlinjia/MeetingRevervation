#ifndef _SEQUENCER_H_
#define _SEQUENCER_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <pthread.h>
#include "Replica.h"
#include "Commen.h"

using namespace std;

class PrimarySvr
{
public:
	PrimarySvr ();
	virtual ~PrimarySvr ();
	void CreateGroupSocket ();
	void CreateThread ();
	void Broadcast (void *buf);

public:
	pthread_t tid;
private:
	void AssignSeqNumber();
	static void* RecivingPkg (void *context);
	void err (const char *s);

private:
	unsigned long m_curSeq;
	int mSockDept[MAX_DEPT_NUM];
	struct sockaddr_in m_servaddr[MAX_DEPT_NUM];
	
};
#endif // !_SEQUENCER_H_
