#pragma once

#include <winsock2.h>
#include <windows.h>
#include "stdafx.h"

#define DAY  5
#define TIME 9
#define TABLE_X 100
#define TABLE_Y 30
#define ROOM_NUM 8
#define BUF_LEN	128

#define SERVER_BUF_CLEAR		0xf8
#define SERVER_TOTAL_ORDER		0xf7
#define SERVER_ROOM_INFOR		0xf6
#define SERVER_ROOM_FAILED		0xf5

#define USER_MSG_UPDATE			WM_USER + 1
#define USER_MSG_ROOMUPDATE		WM_USER + 2
#define USER_MSG_BOOKFAILED		WM_USER + 3

typedef struct
{
	int status;      // room schedule status, 0 for free, 1 for reserve.
	int reserveDept; // if status is set to 1, record which dept. order this time.
}ScheduleStatus;

typedef struct
{
	int room_id;
	ScheduleStatus day[DAY][TIME];

}STRoom;


class MyClient
{
public:
	MyClient ();
	virtual ~MyClient ();

	void CreateBMP (HWND hWnd);
	void DrawTable (HDC hdc);
	void DrawRoomReserve (HDC hdc, HDC hdcMem, int x, int y, int reserveDpt);
	void OnClickRoom (int roomIndex);
	static UINT  _stdcall CreateClientSocket (LPVOID lParam);
	void OutputTraceInfo (LPCTSTR lpszFormat, ...);
	void CreateThread ();

private:
	HBITMAP m_bmpreserve;
	HBITMAP m_bmpidle;
	STRoom  m_roomSchedule[ROOM_NUM];
	int		m_curSelRoom;
	HANDLE  m_hThread;
	DWORD	m_dwThreadID;
	bool    m_isTerminate;
	HWND	m_hWnd;
	HWND	m_hSocketWindow;
	bool	m_bGetTotolOrder;
	TCHAR   m_szTotalOrderBuf[BUF_LEN];
	TCHAR   m_szFailureMsg[BUF_LEN];
	
};