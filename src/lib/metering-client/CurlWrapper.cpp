/**
 * This is the cpp file for the wrapper class around libcurl. These functions provide
 * C++ interface. These are geared towards creating a curl call to the RPS.
*/

#include "CurlWrapper.h"
#include <stdexcept> // exception errors

namespace meteringclientlib
{
namespace detail
{
void CurlWrapper::appendSlist(const char *stringToAppend)
{
	slist = curl_slist_append(slist, stringToAppend);
}

void CurlWrapper::setUrl(const char *inputUrl)
{
	setOpt();
	curl_easy_setopt(easyCurlPtr, CURLOPT_URL, inputUrl);
}

void CurlWrapper::setWriteFunc(size_t (&write_callback)(char *ptr, size_t size, size_t nmemb, std::string *userdata))
{
	curl_easy_setopt(easyCurlPtr, CURLOPT_WRITEFUNCTION, write_callback);
}

void CurlWrapper::setWriteData(std::string &inputWrite)
{
	curl_easy_setopt(easyCurlPtr, CURLOPT_WRITEDATA, &inputWrite);
}

void CurlWrapper::setPostField(std::string &inputPostField)
{
	curl_easy_setopt(easyCurlPtr, CURLOPT_POSTFIELDS, inputPostField.c_str());
	curl_easy_setopt(easyCurlPtr, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(inputPostField.size()));
}

void CurlWrapper::setNoProgress(long progressFlag)
{
	curl_easy_setopt(easyCurlPtr, CURLOPT_NOPROGRESS, progressFlag);
}

CURLcode CurlWrapper::easyPerform()
{
	CURLcode retCurl = curl_easy_perform(easyCurlPtr);
	if (retCurl != CURLE_OK)
	{
		throw std::runtime_error(curl_easy_strerror(retCurl));
	}
	return retCurl;
}

long CurlWrapper::getResponseCode()
{
	long response_code;
	curl_easy_getinfo(easyCurlPtr, CURLINFO_RESPONSE_CODE, &response_code);
	return response_code;
}

void CurlWrapper::setPostOpt()
{
	curl_easy_setopt(easyCurlPtr, CURLOPT_CUSTOMREQUEST, "POST");
}

void CurlWrapper::resetOpt()
{
	curl_easy_reset(easyCurlPtr);
}

void CurlWrapper::setOpt()
{
	curl_easy_setopt(easyCurlPtr, CURLOPT_BUFFERSIZE, bufferSize);
	curl_easy_setopt(easyCurlPtr, CURLOPT_HTTPHEADER, slist);
	curl_easy_setopt(easyCurlPtr, CURLOPT_USERAGENT, curlAgent.c_str());
	curl_easy_setopt(easyCurlPtr, CURLOPT_MAXREDIRS, maxRedirs);
	curl_easy_setopt(easyCurlPtr, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(easyCurlPtr, CURLOPT_CUSTOMREQUEST, requestType.c_str());
	curl_easy_setopt(easyCurlPtr, CURLOPT_TCP_KEEPALIVE, curlTcpKeepAlive);
}

void CurlWrapper::setCaInfo()
{
	if (!caPath.empty())
	{
		curl_easy_setopt(easyCurlPtr, CURLOPT_CAINFO, caPath.c_str());
	}
	#ifdef WIN32
	// Use native ca root on Windows
	else
	{
		curl_easy_setopt(easyCurlPtr, CURLOPT_SSL_OPTIONS , CURLSSLOPT_NATIVE_CA);
	}
	#endif
}

} // namespace detail
} // namespace meteringclientlib
