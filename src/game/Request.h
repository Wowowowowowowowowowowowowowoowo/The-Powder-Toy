#ifndef REQUEST_H
#define REQUEST_H
#include <map>
#include <string>

class RequestManager;
class Request
{
	std::string uri;
	void *http;
	bool keepAlive;

	char *requestData;
	int RequestSize;
	int requestStatus;

	std::string postData;
	std::string postDataBoundary;

	const char *userID;
	const char *userSession;

	volatile bool requestFinished;
	volatile bool requestCanceled;
	volatile bool requestStarted;

public:
	Request(std::string uri, bool keepAlive = false);
	~Request();

	void AddPostData(std::map<std::string, std::string> data);
	void AddPostData(std::pair<std::string, std::string> data);
	void AuthHeaders(const char *ID, const char *session);
	void Start();
	bool Reuse(std::string newuri);
	char* Finish(int *length, int *status);
	void Cancel();

	void CheckProgress(int *total, int *done);
	bool CheckDone();
	bool CheckCanceled();
	bool CheckStarted();

	static std::string GetStatusCodeDesc(int code);

	static char* Simple(std::string uri, int *length, int *status, std::map<std::string, std::string> post_data = std::map<std::string, std::string>{});
	static char* SimpleAuth(std::string uri, int *length, int *status, const char *ID, const char *session, std::map<std::string, std::string> post_data = std::map<std::string, std::string>{});

	friend class RequestManager;
};

#endif
