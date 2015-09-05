#ifndef _REPLICA_H_
#define _REPLICA_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include "Network.h"

using namespace std;

class Replica : public NetworkCallback
{
public:
	Replica (unsigned int uID, const char *szUIIP);
	virtual ~Replica ();

	void Test_Case1 ();
	void Test_Case2 ();
	void Test_Case3 ();
	void UpdateData ();
	void doUpdateMeetingRoomStatus (STRoom& room, STMsg& msg, Replica *pObj);
	void PrintTotalOrder ();

	//override NetworkCallback
	virtual void OnRecieveMessage (void *msg);

private:
	static void*onUpdateThread (void* paramer);

private:
	unsigned int m_curID;
	int m_concurrencyNumber;
	NetworkHelper m_cNetwork;
	int m_TimeStamp;
	list<STMsg> m_lstTime;
	list<STMsg> m_releaseRequest;
	STRoom m_roomSchedule[ROOM_NUMBER];
	bool m_bContinue;
	bool m_bSendRoomInfo2UI;
};
#endif