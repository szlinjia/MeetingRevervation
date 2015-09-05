#include "Replica.h"
#include "Commen.h"
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static const char* szDays[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};

#ifndef min
#define min(a,b) a<b?a:b
#endif

pthread_mutex_t lock;

Replica::Replica (unsigned int uID, const char *szUIIP)
{
	m_curID = uID;
	m_TimeStamp = 0;
	m_bSendRoomInfo2UI = false;

	for (int i = 0; i < ROOM_NUMBER; i++)
	{
		STRoom& room = m_roomSchedule[i];
		room.room_id = i;
		memset (room.day, 0, sizeof(room.day));		
	}

	m_cNetwork.CreateSocket (m_curID, szUIIP);
	while (!m_cNetwork.m_isReadytoRecive)
	{
		usleep (10);
	}
	
	m_cNetwork.RegesterEvent (this);
}

Replica::~Replica ()
{

}

/*
 *	@function: Test_Case1: only one Dept.[0] want to reserve room
 *  @input	 : void
 *  @output	 : void
 *  @describe: Only one request has been send to primary server and 
				will be ordered successfully
 */
void Replica::Test_Case1()
{
	m_concurrencyNumber = 1;

	STMsg msg;
	msg.type = ENM_REQUEST;
	msg.mTimestamp = 0;
	msg.mID = m_curID;
	msg.uPort = REPLICA_PORT + m_curID;

	if (m_curID == 0)
	{
		printf ("Dept.[%d] Request Timestamp to PrimaryServer.\n", m_curID);
		msg.room_id = 0;
		msg.which_day = 3;
		msg.which_time = 8;
		msg.what_status = 1;
		m_cNetwork.SendMessageToSequencer (m_curID, (const char*)&msg, sizeof(msg));
	}
}

/*
 *	@function: Test_Case2: 5 Depts need to reserve rooms concurrency.
 *  @input	 : void 
 *  @output	 : void
 *  @describe: Multiple Dept. don't conflict with each other, which means that all of 
				them will reserve successfully eventually
 */
void Replica::Test_Case2 ()
{
	m_concurrencyNumber = 5;
	STMsg msg;
	msg.type = ENM_REQUEST;
	msg.mTimestamp = 0;
	msg.mID = m_curID;
	msg.uPort = REPLICA_PORT + m_curID;

	if (m_curID % 2 == 0 )
	{
		printf ("Dept.[%d] Request Timestamp to PrimaryServer.\n", m_curID);
		msg.room_id = m_curID%ROOM_NUMBER;
		msg.which_day = m_curID%5;
		msg.which_time = m_curID%9;
		msg.what_status = 1;
		m_cNetwork.SendMessageToSequencer (m_curID, (const char*)&msg, sizeof(msg));
	}
}

/*
 *	@function:Test_Case3:Five Depts need to reserve rooms concurrency. 
 *  @input	 : void
 *  @output	 : void
 *  @describe:Some Dept. conflict with each other at the same time, which means that only one of
		them will reserve successfully and the rest will fail at the end.
 */
void Replica::Test_Case3 ()
{
	m_concurrencyNumber = 5;
	STMsg msg;
	msg.type = ENM_REQUEST;
	msg.mTimestamp = 0;
	msg.mID = m_curID;
	msg.uPort = REPLICA_PORT + m_curID;

	if (m_curID % 2 != 0)
	{
		//Dept[1] and Dept[3] intend to order the same room at the same time. 
		if (m_curID < 5)
		{
			msg.room_id = 1;
			msg.which_day = 1;
			msg.which_time = 1;
		}
		else
		{
			msg.room_id = m_curID%ROOM_NUMBER;
			msg.which_day = m_curID%5;
			msg.which_time = m_curID%9;
		}
		printf ("Dept.[%d] Request Timestamp to PrimaryServer.\n", m_curID);
		msg.what_status = 1;
		m_cNetwork.SendMessageToSequencer (m_curID, (const char*)&msg, sizeof(msg));
	}
}

void Replica::OnRecieveMessage (void *msg)
{
	pthread_mutex_lock (&lock);
	STMsg *pMsg = (STMsg *)msg;

	if (pMsg->type == 1)
	{
		if (pMsg->mID == m_curID)
		{
			m_TimeStamp = pMsg->mTimestamp;
		}

		//process reply message
		list<STMsg>::iterator iter = m_lstTime.begin();
		for (int i = 0; iter != m_lstTime.end ();i++)
		{
			if (iter->mTimestamp < pMsg->mTimestamp)
			{
				iter++;
			}
			else
			{
				break;
			}
		}
		m_lstTime.insert (iter, *pMsg);
		PrintTotalOrder ();
	}
	else if (pMsg->type == 2 && pMsg->mID != m_curID)
	{
		//put into release queue for each replica to modify data in order
		list<STMsg>::iterator iter = m_releaseRequest.begin ();
		for (int i = 0; iter != m_releaseRequest.end (); i++)
		{
			if (iter->mTimestamp < pMsg->mTimestamp)
			{
				iter++;
			}
			else
			{
				break;
			}
		}
		m_releaseRequest.insert (iter, *pMsg);

	}
	pthread_mutex_unlock (&lock);
}

void Replica::PrintTotalOrder ()
{
	///test log print out:print total order:
	if (m_curID == 0 && m_lstTime.size () == m_concurrencyNumber)
	{
		char szbuf[256] = { 0 };
		szbuf[0] = SERVER_TOTAL_ORDER;
		list<STMsg>::iterator printIter = m_lstTime.begin ();
		printf ("\n**********************************\n");
		printf ("total order timestamp list,size:%d\n", m_lstTime.size ());
		for (; printIter != m_lstTime.end (); printIter++)
		{
			char sztmp[128] = { 0 };
			sprintf (sztmp, "Dept[%d] timestampe:%d\n", printIter->mID, printIter->mTimestamp);
			strcat (&szbuf[1], sztmp);
		}
		printf ("%s\n", &szbuf[1]);
		printf ("**********************************\n\n");
		//Send back to Client:
		m_cNetwork.SendMessagetoUI (szbuf, strlen (szbuf)+1);
		m_bSendRoomInfo2UI = true;
		m_bContinue = true;
		
	}

	
}

void Replica::UpdateData ()
{
	pthread_t tid;
	m_bContinue = true;
	if (m_curID == 0)
	{
		m_bContinue = false;
	}
	
	int ret = pthread_create (&tid, NULL, onUpdateThread, (void *)this);
}


//check each time, if current timestamp at the front of queue, modify replica data
void* Replica::onUpdateThread (void* paramer)
{
	Replica* pobj = (Replica *)paramer;

	while (1)
	{
		if (pobj->m_bContinue)
		{
			pthread_mutex_lock (&lock);

			list<STMsg>::iterator iter = pobj->m_lstTime.begin ();
			if (iter != pobj->m_lstTime.end () &&
				iter->mID == pobj->m_curID)
			{
				printf ("\n\n**********************************\n");
				printf ("Now it's Dept.[%d]'s turn to modify data.\n", pobj->m_curID);
				STRoom& room = pobj->m_roomSchedule[iter->room_id];
				pobj->doUpdateMeetingRoomStatus (room, *iter, pobj);
				pobj->m_lstTime.pop_front ();

				//release itself task id and broadcast to all replica
				STMsg msg = *iter;
				msg.type = ENM_RELEASE;
				printf ("\nDept.[%d] finish update Replica. Send release message to PrimaryServer.\n", pobj->m_curID);
				printf ("**********************************\n\n");
				usleep (1000);
				pobj->m_cNetwork.SendMessageToSequencer (pobj->m_curID, (const char*)&msg, sizeof(msg));

			}
			else if (pobj->m_releaseRequest.size () > 0)
			{
				list<STMsg>::iterator iter = pobj->m_releaseRequest.begin ();
				STRoom& room = pobj->m_roomSchedule[iter->room_id];
				pobj->doUpdateMeetingRoomStatus (room, *iter, pobj);
				pobj->m_releaseRequest.erase (iter);
				//other replica receive release message, remove their first node from m_lsttime.
				if (pobj->m_lstTime.size () > 0)
				{
					pobj->m_lstTime.pop_front ();
				}
			}

			if (pobj->m_curID == 0 && pobj->m_lstTime.size () == 0 && pobj->m_bSendRoomInfo2UI)
			{
				char szbuf[512] = { 0 };
				int blockNum = sizeof(STRoom) / 100;
				if (sizeof(STRoom) % 100 != 0)
				{
					blockNum++;
				}

				for (int i = 0; i <ROOM_NUMBER; i++)//
				{
					char *tmp = (char *)&(pobj->m_roomSchedule[i]);
					for (int j = 0; j < blockNum; j++)
					{
						memset (szbuf, 0, sizeof(szbuf));
						int blocksize = min ((sizeof(STRoom)-j * 100), 100);
						szbuf[0] = SERVER_ROOM_INFOR;
						szbuf[1] = i;
						szbuf[2] = j;
						szbuf[3] = blocksize;
						memcpy (&szbuf[4], tmp, blocksize);
						tmp += blocksize;
						/*if (i == 6)
						{
							STRoom room = pobj->m_roomSchedule[i];
							printf ("Ready to send room Information to Client UI.status=%d revD=%d roomNum=%d blockid=%d size=%d blocksize=%d\n",
								room.day[1][6].status, room.day[1][6].reserveDept,szbuf[1], szbuf[2], szbuf[3], blocksize);
						}*/
						pobj->m_cNetwork.SendMessagetoUI (szbuf, blocksize + 4);
					}
					
				}
				
				pobj->m_bSendRoomInfo2UI = false;
			}
			pthread_mutex_unlock (&lock);
		}
		
	}
}

void Replica::doUpdateMeetingRoomStatus (STRoom& room, STMsg& msg, Replica *pObj)
{
	char time[128] = { 0 };
	sprintf (time, "%d:00-%d:00", msg.which_time + 8, msg.which_time + 9);
	
	if (room.day[msg.which_day][msg.which_time].status == 0 && msg.what_status == 1)
	{
		if (m_curID == 0)
		{
			printf ("Replica.[%d]update itself data::Dept.[%d] reserve successfully.\
Room id:%d, day:%s, schedule time:%s, status:%d\n", m_curID, msg.mID,
			msg.room_id, szDays[msg.which_day], time, msg.what_status);
		}


		room.day[msg.which_day][msg.which_time].status = msg.what_status;
		room.day[msg.which_day][msg.which_time].reserveDept = msg.mID;
		msg.bIsSuccess = true;

	}
	else
	{
		if (m_curID == 0)
		{
			printf ("Dept.[%d]update itself data::Dept.[%d] reserve failed. Occupy by Dept.[%d],\
Room id : %d, day:%s, schedule time:%s, status : %d\n",
				m_curID, msg.mID, room.day[msg.which_day][msg.which_time].reserveDept,
				msg.room_id, szDays[msg.which_day], time, msg.what_status);
		}

		msg.bIsSuccess = false;

		//send booking failure message to UI
		if (m_curID == 0)
		{
			char szbuf[128] = { 0 };
			szbuf[0] = SERVER_ROOM_FAILED;
			sprintf (&szbuf[1], "Dept.[%d] reserve failed. Occupy by Dept.[%d],\nRoom id : %d, day:%s, schedule time:%s, status : %d\n",
				msg.mID, room.day[msg.which_day][msg.which_time].reserveDept,
				msg.room_id, szDays[msg.which_day], time, msg.what_status);
			printf ("buf:%s\n", &szbuf[1]);
			pObj->m_cNetwork.SendMessagetoUI (szbuf, strlen (szbuf) + 1);
		}
	}
}

int main (int argc, char *argv[])
{
	if (argc != 4)
	{
		printf ("usage: %d Department ID and Test case id, and IP\n", argc);
		exit (0);
	}

	int iID = atoi (argv[1]);
	int iCaseID = atoi (argv[2]);

	if (pthread_mutex_init (&lock, NULL) != 0)
	{
		printf ("Dept.[%d] mutex init failed\n", iID);
	}
	
	Replica client (iID, argv[3]);
	usleep (6000);
	switch (iCaseID)
	{
		case 1:
			client.Test_Case1 ();
			break;
		case 2:
			client.Test_Case2 ();
			break;
		case 3:
			client.Test_Case3 ();
			break;
		default:
			printf ("Error input case id.Must be 1,2,3 icase=%d\n", iCaseID);
			break;
	}
	client.UpdateData ();

	// parent process has nothing to do, goes infinite loop
	while (true) {};
	pthread_mutex_destroy (&lock);

	printf ("\nprocess lauch exit.\n");
	return 0;
}

