#pragma once

/**
 * This is the header file for the wrapper class around libcurl. Libcurl is in c, so
 * this class is a simple wrapper to have a C++ interface.
*/

#include <string>
#include <iostream>
#include <curl/curl.h>
#include <memory>

namespace meteringclientlib
{
namespace detail
{
class CurlWrapper
{
public:
	CURL *easyCurlPtr;
	struct curl_slist *slist;

private:
	std::string requestType;
	std::string curlAgent;
	long bufferSize;
	long maxRedirs;
	long curlTcpKeepAlive;
	std::unique_ptr<CurlWrapper> curlPtr;
	std::string caPath;

public:
	CurlWrapper()
		: slist(NULL),
		  requestType("GET"),
		  curlAgent("curl/7.63.0"),
		  bufferSize(102400L),
		  maxRedirs(50L),
		  curlTcpKeepAlive(1L)
	{
		easyCurlPtr = curl_easy_init();
	}

	CurlWrapper(const CurlWrapper &inputCurl)
		: slist(NULL),
		  requestType(inputCurl.requestType),
		  curlAgent(inputCurl.curlAgent),
		  bufferSize(inputCurl.bufferSize),
		  maxRedirs(inputCurl.maxRedirs),
		  curlTcpKeepAlive(inputCurl.curlTcpKeepAlive),
		  caPath(inputCurl.caPath)
	{
		easyCurlPtr = curl_easy_init();
		setCaInfo();
	}

	CurlWrapper(std::string inputRequestType, std::string inputCurlAgent, long inputBufferSize, long inputMaxRedirs,
				long inputCurlTcp, std::string caPath)
		: slist(NULL),
		  requestType(inputRequestType),
		  curlAgent(inputCurlAgent),
		  bufferSize(inputBufferSize),
		  maxRedirs(inputMaxRedirs),
		  curlTcpKeepAlive(inputCurlTcp),
		  caPath(caPath)
	{
		easyCurlPtr = curl_easy_init();
		setCaInfo();
	}

	virtual ~CurlWrapper()
	{
		curl_easy_cleanup(easyCurlPtr);
		easyCurlPtr = NULL;
		curl_slist_free_all(slist);
		slist = NULL;
	}

	virtual void appendSlist(const char *stringToAppend);
	virtual void setUrl(const char *inputUrl);
	virtual void setWriteFunc(size_t (&write_callback)(char *ptr, size_t size, size_t nmemb, std::string *userdata));
	virtual void setWriteData(std::string &inputWrite);
	virtual void setPostField(std::string &inputPostField);
	virtual void setNoProgress(long progressFlag);
	virtual CURLcode easyPerform();
	virtual long getResponseCode();
	virtual void resetOpt();
	virtual void setPostOpt();
	virtual void setCaInfo();

private:
	void setOpt();
};
} // namespace detail
} // namespace meteringclientlib
