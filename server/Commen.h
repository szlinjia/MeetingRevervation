#ifndef _COMMEN_H_
#define _COMMEN_H_

//some useful Linux commands:
//check all ports is occupy: lsof -i
//check one port is occupy: lsof -i:portNum
//kill process: kill pid

//primary server ip/port
#define SVR_IP "127.0.0.1"
#define SVR_PORT 5390


//sequencer server ip/port
#define SVR_SEQUENCER_IP   "127.0.0.1"
#define SVR_SEQUENCER_PORT 9876

//replica ip/port
#define SVR_REPLICA_IP "127.0.0.1"
#define REPLICA_PORT   9877

#define SOCKET_UI_PORT	8888

#define RELICA_NUM     10

#define SCHEDULE_DAYS  5
#define SCHEDULE_HOURS 9

#define ROOM_NUMBER 8
#define DAY			5
#define PERIOD		9

//Server send to UI Message flag
#define SERVER_BUF_CLEAR		0xf8
#define SERVER_TOTAL_ORDER		0xf7
#define SERVER_ROOM_INFOR		0xf6
#define SERVER_ROOM_FAILED		0xf5

typedef enum
{
	ENM_REQUEST = 0,
	ENM_REPLY,
	ENM_RELEASE,
}EnmMsgType;

typedef struct
{
	int status;      // room schedule status, 0 for free, 1 for reserve.
	int reserveDept; // if status is set to 1, record which dept. order this time.
}ScheduleStatus;

typedef struct
{
	int room_id;
	ScheduleStatus day[DAY][PERIOD];

}STRoom;

typedef struct
{
	EnmMsgType		type;					//0 for request, 1 for reply, 2 for release
	unsigned int	mTimestamp;				//timestamp for each commit using total order.
	int				uPort;					//current commit port.
	int				mID;					//commet dept.ID
	bool			bIsSuccess;				//whether current commit is successful or not 
	int				what_status;			//reserve type,0 for cancel, 1 for ordering, 
	int				room_id;				//0-7 for 8 meeting room
	int				which_day;				//1-5 for Monday to Friday
	int				which_time;				//1-9 from 8 am to 5 pm
}STMsg;

#endif