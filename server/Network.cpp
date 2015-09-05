#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "Replica.h"
#include "Network.h"


using namespace std;

#define DBG printf
//#define TCP	

void* NetworkHelper::onRecivefromSequencer (void *param)
{
	struct sockaddr_in my_addr, cli_addr;
	int sockfd, i;
	socklen_t slen = sizeof(cli_addr);
	char buf[128] = { 0 };

	NetworkHelper *obj = (NetworkHelper *)param;
	if ((sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		printf ("create socket error.\n");

	bzero (&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons (REPLICA_PORT + obj->m_DeptID);
	my_addr.sin_addr.s_addr = htonl (INADDR_ANY);

	if (bind (sockfd, (struct sockaddr*) &my_addr, sizeof(my_addr)) == -1)
		printf ("bind error\n");

	obj->m_isReadytoRecive = true;
	printf ("Server : create socket successfully:ip:%s, port:%d\n", SVR_REPLICA_IP, REPLICA_PORT + obj->m_DeptID);

	while (1)
	{
		if (recvfrom (sockfd, buf, 128, 0, (struct sockaddr*)&cli_addr, &slen) == -1)
			printf ("recevie data error.\n");
		
		if (obj->m_pEvent != NULL)
		{
			obj->m_pEvent->OnRecieveMessage ((void *)&buf);
		}
		memset (buf, 0, 128);
	}

	close (sockfd);
	return 0;
}

NetworkHelper::NetworkHelper()
{
	m_pEvent = NULL;
}

NetworkHelper::~NetworkHelper()
{
}

void NetworkHelper::CreateSocket (int uID, const char *szUIIP)
{
	m_DeptID = uID;
	//printf ("\nCreateSocket.id:%d\n",uID);
	m_isReadytoRecive = false;
	mSockSequencer = socket (AF_INET, SOCK_DGRAM, 0);
	assert (mSockSequencer != -1);

	bzero (&mSockAddrSequencer, sizeof(mSockAddrSequencer));
	mSockAddrSequencer.sin_family = AF_INET;
	mSockAddrSequencer.sin_port = htons (SVR_SEQUENCER_PORT);
	mSockAddrSequencer.sin_addr.s_addr = inet_addr (SVR_SEQUENCER_IP);


	mSockUI = socket (AF_INET, SOCK_DGRAM, 0);
	assert (mSockUI != -1);
	
	
	bzero (&mSockAddrUI, sizeof(mSockAddrUI));
	mSockAddrUI.sin_family = AF_INET;
	mSockAddrUI.sin_port = htons (SOCKET_UI_PORT);
	mSockAddrUI.sin_addr.s_addr = inet_addr (szUIIP);
	
	//send msg to UI to clear buffer
	if (uID == 0)
	{
		printf ("create ui socket successfull.ip:%s,port:%d\n", szUIIP, SOCKET_UI_PORT);
		char flag = SERVER_BUF_CLEAR;
		SendMessagetoUI ((char*)&flag, sizeof(char));
	}
	

	int ret = pthread_create (&mThread, NULL, onRecivefromSequencer, (void *)this);
	assert (0 == ret);
}

void NetworkHelper::SendMessageToSequencer(int ID,const char *message, int msgLen)
{
	int ret = sendto (mSockSequencer, message, msgLen, 0,
		(struct sockaddr *)&mSockAddrSequencer, (socklen_t)sizeof(sockaddr_in));

}

void NetworkHelper::SendMessagetoUI (const char *message, int msgLen)
{
	int ret = sendto (mSockUI, message, msgLen, 0,
		(struct sockaddr *)&mSockAddrUI, (socklen_t)sizeof(sockaddr_in));
	printf ("Send message to UI,flag:%2x\n", message[0]);

}

