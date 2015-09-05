#include "MyClient.h"
#include <atlconv.h>
#include <process.h>

#define SOCKET_CLASSNAME "MYCLIENTSOCKET"

static const TCHAR* szDays[] = { TEXT("Mon"), TEXT("Tue"), TEXT("Wed"), TEXT("Thur"), TEXT("Fri") };
MyClient::MyClient ()
{
	m_curSelRoom = 0;
	m_bGetTotolOrder = false;
	memset (m_szFailureMsg, 0, sizeof(m_szFailureMsg));
	memset (m_szTotalOrderBuf, 0, sizeof(m_szTotalOrderBuf));

	for (int i = 0; i < ROOM_NUM; i++)
	{
		STRoom& room = m_roomSchedule[i];
		room.room_id = i;
		memset (room.day, 0, sizeof(room.day));
	}
}

MyClient::~MyClient ()
{
	m_isTerminate = true;
}

void MyClient::CreateBMP (HWND hWnd)
{
	HINSTANCE hInst = GetModuleHandle (NULL);
	m_bmpreserve = (HBITMAP)LoadImage (hInst, L"reserve.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	m_bmpidle = (HBITMAP)LoadImage (hInst, L"idle.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	m_hWnd = hWnd;

	CreateThread ();
}

void MyClient::DrawTable (HDC hdc)
{
	PAINTSTRUCT 	ps;
	BITMAP 			bmpIdle;
	HDC 			hdcMem;
	HGDIOBJ 		oldBitmap;
	RECT			rec;

	hdcMem = CreateCompatibleDC (hdc);
	oldBitmap = SelectObject (hdcMem, m_bmpidle);
	GetObject (m_bmpidle, sizeof(bmpIdle), &bmpIdle);
	SetBkMode (hdc, TRANSPARENT);
	SetTextColor (hdc, RGB (0, 0, 0));
	int time = 8;
	STRoom room = m_roomSchedule[m_curSelRoom];
	for (int i = 0; i < TIME; i++)
	{
		
		int x = TABLE_X;
		int y = TABLE_Y + i*bmpIdle.bmHeight;

		for (int j = 0; j < DAY; j++)
		{
			if (i == 0)
			{
				//draw day text
				SetRect (&rec, x, 0, x + bmpIdle.bmWidth, 20);
				DrawText (hdc, szDays[j], _tcslen(szDays[j]), &rec, DT_CENTER | DT_CENTER);
			}

			if (room.day[j][i].status == 1)
			{
				DrawRoomReserve (hdc, hdcMem, x, y, room.day[j][i].reserveDept);
			}
			else
			{
				BitBlt (hdc, x, y, bmpIdle.bmWidth, bmpIdle.bmHeight, hdcMem, 0, 0, SRCCOPY);
			}
			
			x += bmpIdle.bmWidth;
		}

		//DRAW TIME TEXT
		TCHAR szTime[128] = { 0 };
		_stprintf (szTime, TEXT ("%d:00-%d:00"), time + i, time + i + 1);
		SetRect (&rec, 5, y+10, 100, y+50);
		DrawText (hdc, szTime, _tcslen (szTime), &rec, DT_LEFT | DT_LEFT);
	}
	

	//draw selected room id:
	SetTextColor (hdc, RGB (255, 0, 0));
	int x = TABLE_X + DAY*bmpIdle.bmWidth + 30;
	TCHAR szBuf[BUF_LEN] = { 0 };
	_stprintf (szBuf, TEXT ("Selected Room ID:%d"), m_curSelRoom);
	SetRect (&rec, x, TABLE_Y, x + 500, TABLE_Y + 20);
	DrawText (hdc, szBuf, _tcslen (szBuf), &rec, DT_LEFT | DT_LEFT);

	//draw total order
	if(m_bGetTotolOrder)
	{
		SetRect (&rec, x, TABLE_Y+30, x + 500, TABLE_Y + 500);
		DrawText (hdc, m_szTotalOrderBuf, _tcslen (m_szTotalOrderBuf), &rec, DT_LEFT | DT_LEFT);
	}

	SelectObject (hdcMem, oldBitmap);
	DeleteDC (hdcMem);
}

void MyClient::DrawRoomReserve (HDC hdc, HDC hdcMem, int x, int y, int reserveDpt)
{
	BITMAP 			bmpReverve;
	BITMAP 			bmpIdle;
	RECT			rec;

	SelectObject (hdcMem, m_bmpreserve);

	HFONT hFont = (HFONT)GetStockObject (DEFAULT_GUI_FONT);
	LOGFONT logfont;
	GetObject (hFont, sizeof(LOGFONT), &logfont);
	HFONT hNewFont = CreateFontIndirect (&logfont);
	HFONT hOldFont = (HFONT)SelectObject (hdc, hNewFont);

	GetObject (m_bmpreserve, sizeof(bmpReverve), &bmpReverve);
	BitBlt (hdc, x, y, bmpReverve.bmWidth, bmpReverve.bmHeight, hdcMem, 0, 0, SRCCOPY);

	SelectObject (hdcMem, m_bmpidle);

	SetRect (&rec, x, y, x + bmpReverve.bmWidth, y + bmpReverve.bmHeight);
	TCHAR szbuf[128] = { 0 };
	_stprintf (szbuf, L"Dept.%d book", reserveDpt);
	DrawText (hdc, szbuf, _tcslen (szbuf), &rec, DT_CENTER | DT_CENTER);

	SelectObject (hdc, hOldFont);
	DeleteObject (hNewFont);
}

void MyClient::OnClickRoom (int roomIndex)
{
	m_curSelRoom = roomIndex;
	int x = TABLE_X + 84 * DAY;
	int y = TABLE_Y;
	RECT rect = {x, y, x+500, y+20};
	InvalidateRect (m_hWnd, &rect, TRUE);

	x = TABLE_X;
	y = TABLE_Y;
	rect = { x, y, x + DAY * 84, y + 41 * TIME };
	InvalidateRect (m_hWnd, &rect, TRUE);
}

void MyClient::OutputTraceInfo (LPCTSTR lpszFormat, ...)
{
	va_list pArg;
	va_start (pArg, lpszFormat);
	TCHAR szFormat[1024] = { 0 };
	TCHAR szMessage[1024] = { 0 };

	_sntprintf (szFormat, 1024 - 1, _T ("%s: %s\n"),
		_T ("TRACE"), lpszFormat);
	_vsntprintf (szMessage, 1024 - 1, szFormat, pArg);
	OutputDebugString (szMessage);

	va_end (pArg);
}

void MyClient::CreateThread ()
{
	UINT dwThreadID = 0;

	m_isTerminate = false;
	m_hThread = (HANDLE)_beginthreadex (
		NULL,
		0,
		CreateClientSocket,
		(LPVOID)this,
		0,
		&dwThreadID);

	m_dwThreadID = dwThreadID;

}

UINT MyClient::CreateClientSocket (LPVOID lParam)
{
	USES_CONVERSION;

	MyClient *pObj = (MyClient *)lParam;

	WSADATA wsaData;
	WORD sockVersion = MAKEWORD (2, 2);
	if (WSAStartup (sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	SOCKET serSocket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serSocket == INVALID_SOCKET)
	{
		pObj->OutputTraceInfo (L"socket error !");
		return 0;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons (8888);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind (serSocket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		pObj->OutputTraceInfo (L"bind error !");
		closesocket (serSocket);
		return 0;
	}

	sockaddr_in remoteAddr;
	int nAddrLen = sizeof(remoteAddr);
	while (!pObj->m_isTerminate)
	{
		char recvData[1024] = {0};
		int ret = recvfrom (serSocket, recvData, 255, 0, (sockaddr *)&remoteAddr, &nAddrLen);
		if (ret > 0)
		{
			unsigned char flag = recvData[0];
			if (flag == SERVER_BUF_CLEAR)
			{
				pObj->m_bGetTotolOrder = false;
				memset (pObj->m_szFailureMsg, 0, BUF_LEN*sizeof(TCHAR));
				memset (pObj->m_szTotalOrderBuf, 0, BUF_LEN*sizeof(TCHAR));
			}
			else if (flag == SERVER_TOTAL_ORDER)
			{
				pObj->m_bGetTotolOrder = true;
				memset (pObj->m_szTotalOrderBuf, 0, BUF_LEN*sizeof(TCHAR));
				_stprintf (pObj->m_szTotalOrderBuf, L"Total order from server:\n\n%s", A2T (&recvData[1]));
				
				PostMessage (pObj->m_hWnd, USER_MSG_UPDATE, NULL, NULL);
				
			}
			else if (flag == SERVER_ROOM_INFOR)
			{
				static int nCount = 0;
				nCount++;
				int index = (int)recvData[1];
				int blockIndex = (int)recvData[2];
				int blocksize = (int)recvData[3];

				char *tmp = (char *)&(pObj->m_roomSchedule[index]);
				memcpy (&tmp[blockIndex*100], &recvData[4], blocksize);
				if (nCount == 4 * ROOM_NUM)
				{
					//receive pacages all
					STRoom room = pObj->m_roomSchedule[index];
					nCount = 0;
					PostMessage (pObj->m_hWnd, USER_MSG_ROOMUPDATE, NULL, NULL);
				}
			}
			else if (flag == SERVER_ROOM_FAILED)
			{
				_stprintf (pObj->m_szFailureMsg, L"\n\nError Message From Other Sever:\n%s\n",A2T (&recvData[1]));
				PostMessage (pObj->m_hWnd, USER_MSG_BOOKFAILED, NULL, NULL);
			}
			pObj->OutputTraceInfo (L"Receive a connection%s \r\n", inet_ntoa (remoteAddr.sin_addr));
			pObj->OutputTraceInfo (A2T(recvData));
			
		}
		

	}
	closesocket (serSocket);
	WSACleanup ();
	return 0;
}

