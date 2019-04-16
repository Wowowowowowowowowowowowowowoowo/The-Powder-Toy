#ifndef DOWNLOAD_H
#define DOWNLOAD_H
#include <map>
#include <string>

class RequestManager;
class Request
{
	std::string uri;
	void *http;
	bool keepAlive;

	char *downloadData;
	int downloadSize;
	int downloadStatus;

	std::string postData;
	std::string postDataBoundary;

	const char *userID;
	const char *userSession;

	volatile bool downloadFinished;
	volatile bool downloadCanceled;
	volatile bool downloadStarted;

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

	static char* Simple(std::string uri, int *length, int *status, std::map<std::string, std::string> post_data = {});
	static char* SimpleAuth(std::string uri, int *length, int *status, const char *ID, const char *session, std::map<std::string, std::string> post_data = {});

	friend class RequestManager;
};

#endif
