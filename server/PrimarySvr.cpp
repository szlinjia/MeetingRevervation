#include "PrimarySvr.h"

PrimarySvr::PrimarySvr ()
{
	m_curSeq = 0;
}

PrimarySvr::~PrimarySvr ()
{
	printf ("close socket....\n");
	for (int i = 0; i < MAX_DEPT_NUM; i++)
	{
		close (mSockDept[i]);
	}
}

void PrimarySvr::CreateGroupSocket ()
{
	for (int i = 0; i < MAX_DEPT_NUM; i++)
	{
		mSockDept[i] = socket (AF_INET, SOCK_DGRAM, 0);
		if (mSockDept[i] == -1)
		{
			err ("create socket fail");
		}

		bzero (&m_servaddr[i], sizeof(m_servaddr[i]));
		m_servaddr[i].sin_family = AF_INET;
		m_servaddr[i].sin_port = htons (REPLICA_PORT + i);
		m_servaddr[i].sin_addr.s_addr = inet_addr (SVR_REPLICA_IP);
	}
}

void PrimarySvr::Broadcast (void *buf)
{
	STMsg *pmsg = (STMsg *)buf;

	printf ("Ready to broadcast message, type:%d msg from Dept.[%d]\n",pmsg->type, pmsg->mID);
	if (pmsg->type == ENM_REQUEST)
	{
		pmsg->type = ENM_REPLY; //set msg to reply;
		pmsg->mTimestamp = ++m_curSeq;
	}
	else if (pmsg->type == ENM_RELEASE)
	{
		usleep (1000);
	}
	
	for (int i = 0; i < MAX_DEPT_NUM; i++)//
	{
		int ret = sendto (mSockDept[i], pmsg, sizeof(STMsg), 0,
			(struct sockaddr *)&m_servaddr[i], (socklen_t)sizeof(sockaddr_in));
		printf ("Send msg back to port:%d red=%d\n", ntohs (m_servaddr[i].sin_port), ret);

	}

	printf ("broadcast over.\n");

}

void PrimarySvr::CreateThread ()
{
	printf ("[PrimarySvr::CreateThread]create.\n");

	int err = pthread_create (&tid, NULL, &RecivingPkg, this);

	if (err != 0){
		printf ("can't create thread :[%s]\n", strerror (err));
	}
	else
	{
		printf ("[[SockThread::CreateThread]]Thread created successfully\n");
	}
	void *red;
	pthread_join (tid, &red);
	printf ("[SockThread::CreateThread] out\n");

}

void PrimarySvr::err (const char *s)
{
	perror (s);
	exit (1);
}

void* PrimarySvr::RecivingPkg (void *context)
{
	struct sockaddr_in my_addr, cli_addr;
	int sockfd, i;
	socklen_t slen = sizeof(cli_addr);
	char buf[128] = { 0 };

	PrimarySvr *obj = (PrimarySvr *)context;
	if ((sockfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		obj->err ("socket");
	else
		printf ("Server : Socket() successful:ip:%s, port:%d\n", SVR_IP, SVR_PORT);

	bzero (&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons (SVR_SEQUENCER_PORT);
	my_addr.sin_addr.s_addr = htonl (INADDR_ANY);

	if (bind (sockfd, (struct sockaddr*) &my_addr, sizeof(my_addr)) == -1)
		obj->err ("bind");
	else
		printf ("Server : bind() successful\n");

	while (1)
	{
		if (recvfrom (sockfd, buf, 128, 0, (struct sockaddr*)&cli_addr, &slen) == -1)
			obj->err ("recvfrom()");
		printf ("Received packet from %s:%d\n",
			inet_ntoa (cli_addr.sin_addr), ntohs(cli_addr.sin_port));
		obj->Broadcast ((void *)buf);
		memset (buf, 0, 128);
	}

	close (sockfd);
	return 0;
}

int main (void)
{
	PrimarySvr mySeq;
	printf ("PrimarySvr start to run.\n");
	mySeq.CreateGroupSocket ();
	mySeq.CreateThread ();
	pthread_join (mySeq.tid, NULL);
}