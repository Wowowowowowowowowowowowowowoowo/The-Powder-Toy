#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H
#include "common/tpt-thread.h"
#include <time.h>
#include <vector>
#include "common/Singleton.h"

class Request;
class RequestManager : public Singleton<RequestManager>
{
private:
	pthread_t requestThread;
	pthread_mutex_t requestLock;
	pthread_mutex_t requestAddLock;
	bool threadStarted;

	int lastUsed;
	volatile bool managerRunning;
	volatile bool managerShutdown;
	std::vector<Request*> requests;
	std::vector<Request*> downloadsAddQueue;

	void Start();
public:
	RequestManager();
	~RequestManager();

	void Shutdown();
	void Update();
	void EnsureRunning();

	void AddDownload(Request *download);
	void RemoveDownload(int id);

	void Lock();
	void Unlock();
};

#endif // REQUESTMANAGER_H
