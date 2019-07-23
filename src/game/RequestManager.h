#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H

#include "common/tpt-thread.h"
#include <ctime>
#include <set>
#include <string>
#include "common/Singleton.h"

// minor hack so I can avoid including curl in the header file. This causes problems with mingw
typedef void CURLM;

class Request;
class RequestManager : public Singleton<RequestManager>
{
	pthread_t worker_thread;
	std::set<Request *> requests;
	int requests_added_to_multi = 0;

	std::set<Request *> requests_to_add;
	bool requests_to_start = false;
	bool requests_to_remove = false;
	bool initialized = false;
	bool rt_shutting_down = false;
	pthread_mutex_t rt_mutex;
	pthread_cond_t rt_cv;

	CURLM *multi = nullptr;

	void Start();
	void Worker();
	void MultiAdd(Request *request);
	void MultiRemove(Request *request);
	bool AddRequest(Request *request);
	void StartRequest(Request *request);
	void RemoveRequest(Request *request);

	static TH_ENTRY_POINT void *RequestManagerHelper(void *obj);

public:
	RequestManager();
	~RequestManager();

	void Initialise(std::string proxy);
	void Shutdown();

	friend class Request;
};

extern const long timeout;
extern std::string proxy;
extern std::string user_agent;

#endif // REQUESTMANAGER_H
