#include "RequestManager.h"
#include "Request.h"
#include "http.h"
#include "defines.h"
#include "common/Platform.h"

RequestManager::RequestManager():
	threadStarted(false),
	lastUsed(time(NULL)),
	managerRunning(false),
	managerShutdown(false),
	requests(std::vector<Request*>()),
	downloadsAddQueue(std::vector<Request*>())
{
	http_init(http_proxy_string[0] ? http_proxy_string : NULL);

	pthread_mutex_init(&requestLock, NULL);
	pthread_mutex_init(&requestAddLock, NULL);
}

RequestManager::~RequestManager()
{

}

void RequestManager::Shutdown()
{
	pthread_mutex_lock(&requestLock);
	pthread_mutex_lock(&requestAddLock);
	for (std::vector<Request*>::iterator iter = requests.begin(); iter != requests.end(); ++iter)
	{
		Request *download = (*iter);
		if (download->http)
			http_force_close(download->http);
		download->requestCanceled = true;
		delete download;
	}
	requests.clear();
	downloadsAddQueue.clear();
	managerShutdown = true;
	pthread_mutex_unlock(&requestAddLock);
	pthread_mutex_unlock(&requestLock);
	pthread_join(requestThread, NULL);

	http_done();
}

//helper function for download
TH_ENTRY_POINT void* DownloadManagerHelper(void* obj)
{
	RequestManager *temp = (RequestManager*)obj;
	temp->Update();
	return NULL;
}

void RequestManager::Start()
{
	managerRunning = true;
	lastUsed = time(NULL);
	pthread_create(&requestThread, NULL, &DownloadManagerHelper, this);
}

void RequestManager::Update()
{
	unsigned int numActiveDownloads;
	while (!managerShutdown)
	{
		pthread_mutex_lock(&requestAddLock);
		if (downloadsAddQueue.size())
		{
			for (size_t i = 0; i < downloadsAddQueue.size(); i++)
			{
				requests.push_back(downloadsAddQueue[i]);
			}
			downloadsAddQueue.clear();
		}
		pthread_mutex_unlock(&requestAddLock);
		if (requests.size())
		{
			numActiveDownloads = 0;
			pthread_mutex_lock(&requestLock);
			for (size_t i = 0; i < requests.size(); i++)
			{
				Request *download = requests[i];
				if (download->CheckCanceled())
				{
					if (download->http && download->CheckStarted())
						http_force_close(download->http);
					delete download;
					requests.erase(requests.begin()+i);
					i--;
				}
				else if (download->CheckStarted() && !download->CheckDone())
				{
					if (http_async_req_status(download->http) != 0)
					{
						download->requestData = http_async_req_stop(download->http, &download->requestStatus, &download->RequestSize);
						download->requestFinished = true;
						if (!download->keepAlive)
							download->http = NULL;
					}
					lastUsed = time(NULL);
					numActiveDownloads++;
				}
			}
			pthread_mutex_unlock(&requestLock);
		}
		if (time(NULL) > lastUsed+HTTP_TIMEOUT*2 && !numActiveDownloads)
		{
			pthread_mutex_lock(&requestLock);
			managerRunning = false;
			pthread_mutex_unlock(&requestLock);
			return;
		}
		Platform::Millisleep(1);
	}
}

void RequestManager::EnsureRunning()
{
	pthread_mutex_lock(&requestLock);
	if (!managerRunning)
	{
		if (threadStarted)
			pthread_join(requestThread, NULL);
		else
			threadStarted = true;
		Start();
	}
	pthread_mutex_unlock(&requestLock);
}

void RequestManager::AddDownload(Request *download)
{
	pthread_mutex_lock(&requestAddLock);
	downloadsAddQueue.push_back(download);
	pthread_mutex_unlock(&requestAddLock);
	EnsureRunning();
}

void RequestManager::Lock()
{
	pthread_mutex_lock(&requestAddLock);
}

void RequestManager::Unlock()
{
	pthread_mutex_unlock(&requestAddLock);
}
