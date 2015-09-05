#ifndef Network_h
#define Network_h

#ifndef MAX_DEPT_NUM 
#define MAX_DEPT_NUM 10
#endif

#include "Commen.h"
class NetworkCallback
{
public:
	virtual void OnRecieveMessage (void *msg) = 0;
};

class NetworkHelper {
public:
	NetworkHelper();
	~NetworkHelper();
	void SendMessageToSequencer (int ID, const char *message, int msgLen);
	void RegesterEvent (NetworkCallback* pEvent){ m_pEvent = pEvent; }
	void UnRegisterEvent (){ m_pEvent = NULL; }
	void CreateSocket (int uID, const char *szUIIP);
	void SendMessagetoUI (const char *message, int msgLen);
public:
	bool m_isReadytoRecive;
private:
	static void* onRecivefromSequencer (void *param);

	int mSockDept;
	int mSockSequencer;
	int mSockUI;
	struct sockaddr_in mSockAddr;
	struct sockaddr_in mSockAddrSequencer;
	struct sockaddr_in mSockAddrUI;


	NetworkCallback *m_pEvent;
	int m_DeptID;
	pthread_t mThread;
	
};

#endif
