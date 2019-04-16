#include <stdlib.h>
#include "defines.h"
#include "Request.h"
#include "RequestManager.h"
#include "http.h"
#include "common/Platform.h"

Request::Request(std::string uri, bool keepAlive):
	http(NULL),
	keepAlive(keepAlive),
	requestData(NULL),
	RequestSize(0),
	requestStatus(0),
	postData(""),
	postDataBoundary(""),
	userID(NULL),
	userSession(NULL),
	requestFinished(false),
	requestCanceled(false),
	requestStarted(false)
{
	this->uri = std::string(uri);
	RequestManager::Ref().AddDownload(this);
}

// called by download thread itself if download was canceled
Request::~Request()
{
	if (http && (keepAlive || requestCanceled))
		http_async_req_close(http);
	if (requestData)
		free(requestData);
}

// add post data to a request
void Request::AddPostData(std::map<std::string, std::string> data)
{
	postDataBoundary = FindBoundary(data, "");
	postData = GetMultipartMessage(data, postDataBoundary);
}
void Request::AddPostData(std::pair<std::string, std::string> data)
{
	std::map<std::string, std::string> postData;
	postData.insert(data);
	AddPostData(postData);
}

// add userID and sessionID headers to the download. Must be done after download starts for some reason
void Request::AuthHeaders(const char *ID, const char *session)
{
	userID = ID;
	userSession = session;
}

// start the download thread
void Request::Start()
{
	if (CheckStarted() || CheckDone())
		return;
	http = http_async_req_start(http, uri.c_str(), postData.c_str(), postData.length(), keepAlive ? 1 : 0);
	// add the necessary headers
	if (userID || userSession)
		http_auth_headers(http, userID, NULL, userSession);
	if (postDataBoundary.length())
		http_add_multipart_header(http, postDataBoundary);
	RequestManager::Ref().Lock();
	requestStarted = true;
	RequestManager::Ref().Unlock();
}

// for persistent connections (keepAlive = true), reuse the open connection to make another request
bool Request::Reuse(std::string newuri)
{
	if (!keepAlive || !CheckDone() || CheckCanceled())
	{
		return false;
	}
	uri = std::string(newuri);
	RequestManager::Ref().Lock();
	requestFinished = false;
	RequestManager::Ref().Unlock();
	Start();
	RequestManager::Ref().EnsureRunning();
	return true;
}

// finish the download (if called before the download is done, this will block)
char* Request::Finish(int *length, int *status)
{
	if (CheckCanceled())
		return NULL; // shouldn't happen but just in case
	while (!CheckDone()); // block
	RequestManager::Ref().Lock();
	requestStarted = false;
	if (length)
		*length = RequestSize;
	if (status)
		*status = requestStatus;
	char *ret = requestData;
	requestData = NULL;
	if (!keepAlive)
		requestCanceled = true;
	RequestManager::Ref().Unlock();
	return ret;
}

// returns the download size and progress (if the download has the correct length headers)
void Request::CheckProgress(int *total, int *done)
{
	RequestManager::Ref().Lock();
	if (!requestFinished && http)
		http_async_get_length(http, total, done);
	else
		*total = *done = 0;
	RequestManager::Ref().Unlock();
}

// returns true if the download has finished
bool Request::CheckDone()
{
	RequestManager::Ref().Lock();
	bool ret = requestFinished;
	RequestManager::Ref().Unlock();
	return ret;
}

// returns true if the download was canceled
bool Request::CheckCanceled()
{
	RequestManager::Ref().Lock();
	bool ret = requestCanceled;
	RequestManager::Ref().Unlock();
	return ret;
}

// returns true if the download is running
bool Request::CheckStarted()
{
	RequestManager::Ref().Lock();
	bool ret = requestStarted;
	RequestManager::Ref().Unlock();
	return ret;
}

// cancels the download, the download thread will delete the Download* when it finishes (do not use Download in any way after canceling)
void Request::Cancel()
{
	RequestManager::Ref().Lock();
	requestCanceled = true;
	RequestManager::Ref().Unlock();
}

std::string Request::GetStatusCodeDesc(int code)
{
	return http_ret_text(code);
}

char* Request::Simple(std::string uri, int *length, int *status, std::map<std::string, std::string> post_data)
{
	Request *request = new Request(uri);
	request->AddPostData(post_data);
	request->Start();
	while (!request->CheckDone())
	{
		Platform::Millisleep(1);
	}
	return request->Finish(length, status);
}

char* Request::SimpleAuth(std::string uri, int *length, int *status, const char* ID, const char* session, std::map<std::string, std::string> post_data)
{
	Request *request = new Request(uri);
	request->AddPostData(post_data);
	request->AuthHeaders(ID, session);
	request->Start();
	while (!request->CheckDone())
	{
		Platform::Millisleep(1);
	}
	return request->Finish(length, status);
}

