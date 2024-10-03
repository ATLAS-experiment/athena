/*
  Copyright (C) 2020-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <CrestApi/CrestRequest.h>
#include <iostream>
#include "nlohmann/json.hpp"

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>
#include <boost/asio.hpp>
#include <fstream>

#include <string>
#include <map>
#include <CrestApi/CrestException.h>

namespace Crest
{

  CrestRequest::CrestRequest()
  {
    curl_global_init(CURL_GLOBAL_ALL);
    m_CREST_PROXY = std::getenv(m_CREST_PROXY_VAR);
  }

  CrestRequest::~CrestRequest()
  {
    curl_global_cleanup();
  }

  void CrestRequest::setHost(const std::string &host)
  {
    m_host = host;
  }

  void CrestRequest::setPort(const std::string &port)
  {
    m_port = port;
  }

  void CrestRequest::setPrefix(const std::string &prefix)
  {
    m_prefix = prefix;
  }
  
  std::string CrestRequest::make_url(const std::string &address) const
  {
    std::string str = m_prefix;
    str += m_host;
    str += ':';
    str += m_port;
    str += address;
    return str;
  }

  size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::vector<char> *output)
  {
    size_t total_size = size * nmemb;
    output->insert(output->end(), reinterpret_cast<char *>(contents), reinterpret_cast<char *>(contents) + total_size);
    return total_size;
  }

  size_t curlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s)
  {
    size_t newLength = size * nmemb;

    try
    {
      s->append((char *)contents, newLength);
    }
    catch (std::bad_alloc &e)
    {
      std::cout << "Not enough memory to allocate "<<newLength<<" bytes "<< std::endl;	    
      return 0;
    }
    return newLength;
  }

  // Callback function to provide data for the request
  size_t ReadCallback(void *contents, size_t size, size_t nmemb, std::string *input)
  {
    size_t totalSize = size * nmemb;
    if (!input->empty())
    {
      // Copy data from the input string to the buffer
      size_t toCopy = std::min(totalSize, input->size());
      std::memcpy(contents, input->c_str(), toCopy);
      input->erase(0, toCopy);
      return toCopy;
    }
    return 0;
  }

  std::string CrestRequest::performRequest(const std::string &current_path, Action action, nlohmann::json &js,
                                           const char *method_name, const std::string& header_params)
  {
    CURL *curl;
    CURLcode res;

    /* Enable tracing of ascii data */
    // sanitise the current path
    std::string sanitisedPath(current_path);
    auto caretPosition = sanitisedPath.find('^');
    if (caretPosition != std::string::npos)
      sanitisedPath.replace(caretPosition, 1, "%5E");
    /* get a curl handle */
    curl = curl_easy_init();
    std::string stt;
    struct curl_slist *headers = NULL;
    if (curl)
    {
      std::string url = make_url(sanitisedPath);
      std::string s;
      std::cout << "cURL request to " << url << std::endl;

      if (m_CREST_PROXY)
      {
        // Set the proxy type (replace with your proxy type details)
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);

        // Set the proxy address and port (replace with your SOCKS proxy details)
        curl_easy_setopt(curl, CURLOPT_PROXY, m_CREST_PROXY);
      }

      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      if (js.is_null())
      {
        if (action == Action::DELETE)
          curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        if (header_params != "")
        {
          headers = curl_slist_append(headers, header_params.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
      }
      else
      {
        stt = js.dump();

        if (action == Action::PUT)
          curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        headers = curl_slist_append(headers, "Accept: */*");
        headers = curl_slist_append(headers, "Content-Type: application/json");

        if (header_params != "")
        {
          headers = curl_slist_append(headers, header_params.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        /* set where to read from (on Windows you need to use READFUNCTION too) */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, stt.c_str());
      }

      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWrite_CallbackFunc_StdString);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

      /* Perform the request, res will get the return code */
      res = curl_easy_perform(curl);

      // data to check the errors in the server response:
      long response_code;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

      /* always cleanup */
      curl_easy_cleanup(curl);
      curl_slist_free_all(headers);

      curl_global_cleanup();

      // error checking in the server response:
      checkResult(res, response_code, s, method_name);
      
      return s;
    }

    throw CrestException(std::string("ERROR in ") + std::string(method_name) + " | CURL not init");
  }

  std::vector<char> CrestRequest::getPayloadRequest(const std::string &current_path) 
  {
    // Initialize libcurl
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
      std::string url = make_url(current_path);
      std::string response;

      std::string content_type = "application/octet-stream";

      if (m_CREST_PROXY)
      {
        // Set the proxy type (replace with your proxy type details)
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);

        // Set the proxy address and port (replace with your SOCKS proxy details)
        curl_easy_setopt(curl, CURLOPT_PROXY, m_CREST_PROXY);
      }
      else
      {
        // The environment variable does not exist
        std::cerr << "SOCKS_PROXY environment variable not found." << std::endl;
      }
      //

      // First set the URL that is about to receive the GET. This URL can
      // just as well be a https:
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

      // Set the write callback function to handle the response
      std::vector<char> response_data;
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
      // Perform the request
      res = curl_easy_perform(curl);

      // data to check the errors in the server response:
      long response_code;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

      curl_easy_cleanup(curl);
      
      // error checking in the server response:
      const char *method_name = "CrestRequest::getPayloadRequest";
      checkResult(res, response_code, response, method_name);
      
      return response_data;
    }
    throw CrestException(std::string("ERROR in ") + "getPayloadRequest CURL not init");
  }


  std::string CrestRequest::uploadPayload(const std::string &current_path, const std::string &tag, uint64_t endtime, const nlohmann::json &js,
                                          const std::string& objectType, const std::string& compressionType, const std::string& version,
                                          const std::vector<std::string> &files)
  {

    CURL *curl;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    struct curl_slist *headers = NULL;

    if (curl)
    {
      std::string url = make_url(current_path);
      std::string response;

      std::string content_type = "application/octet-stream";

      if (m_CREST_PROXY)
      {
        // Set the proxy type (replace with your proxy type details)
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);

        // Set the proxy address and port (replace with your SOCKS proxy details)
        curl_easy_setopt(curl, CURLOPT_PROXY, m_CREST_PROXY);
      }

      //

      // First set the URL that is about to receive our POST. This URL can
      // just as well be a https:
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      std::string js_str = js.dump();

      // Enable verbose mode
      // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

      std::string format = "X-Crest-PayloadFormat: ";
      if (files.size() > 0)
      {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        format += "FILE";
        content_type = "application/form-data";
      }
      else
      {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        format += "JSON";
      }
      headers = curl_slist_append(headers, format.c_str());
      headers = curl_slist_append(headers, "Accept: application/json");
      headers = curl_slist_append(headers, "Content-Type:  multipart/form-data");

      // Create the form  for new version
#if LIBCURL_VERSION_MAJOR < 8
      struct curl_httppost *formpost = NULL;
      struct curl_httppost *lastptr = NULL;
      curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "tag", CURLFORM_COPYCONTENTS, tag.c_str(), CURLFORM_END);
      curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "endtime", CURLFORM_COPYCONTENTS, std::to_string(endtime).c_str(), CURLFORM_END);
      // Set the read callback function
      // Add the "storeset" field with the JSON data directly
      curl_formadd(&formpost, &lastptr,
                   CURLFORM_COPYNAME, "storeset",
                   CURLFORM_BUFFER, "json_data",
                   CURLFORM_BUFFERPTR, js_str.c_str(),
                   CURLFORM_BUFFERLENGTH, js_str.size(),
                   CURLFORM_CONTENTTYPE, content_type.c_str(),
                   CURLFORM_END);

      curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "version", CURLFORM_COPYCONTENTS, version.c_str(), CURLFORM_END);
      curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "objectType", CURLFORM_COPYCONTENTS, objectType.c_str(), CURLFORM_END);
      curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "compressionType", CURLFORM_COPYCONTENTS, compressionType.c_str(), CURLFORM_END);

      for (auto &file : files)
      {
        curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "files", CURLFORM_FILE, file.c_str(), CURLFORM_END);
      }
      curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

#else

      curl_mime *mime = curl_mime_init(curl);
      {
        curl_mimepart *part = curl_mime_addpart(mime);
        curl_mime_name(part, "tag");
        curl_mime_data(part, tag.c_str(), tag.size());

        part = curl_mime_addpart(mime);
        std::string endtime_s = std::to_string(endtime);
        curl_mime_name(part, "endtime");
        curl_mime_data(part, endtime_s.c_str(), endtime_s.size());

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "version");
        curl_mime_data(part, version.c_str(), version.size());

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "objectType");
        curl_mime_data(part, objectType.c_str(), objectType.size());

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "compressionType");
        curl_mime_data(part, compressionType.c_str(), compressionType.size());

        part = curl_mime_addpart(mime);
        curl_mime_name(part, "storeset");
        curl_mime_type(part, content_type.c_str());
        curl_mime_data(part, js.dump().c_str(), js.dump().size());

        for (auto &file : files)
        {
          curl_mimepart *part = curl_mime_addpart(mime);
          curl_mime_name(part, "files");
          curl_mime_filedata(part, file.c_str());
        }
      }
      curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
#endif
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWrite_CallbackFunc_StdString);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

      // Perform the request
      CURLcode res = curl_easy_perform(curl);

      // data to check the errors in the server response:
      long response_code;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
      const char *method_name = "CrestRequest::uploadPayload";

      /* always cleanup */
      curl_easy_cleanup(curl);
#if LIBCURL_VERSION_MAJOR < 8
      curl_formfree(formpost);
#else
      curl_mime_free(mime);
#endif
      curl_slist_free_all(headers);
      curl_global_cleanup();

      // error checking in the server response:
      checkResult(res, response_code, response, method_name);

      return response;
    }
    std::string mes = "ERROR in CrestRequest::uploadPayload";
    throw CrestException(mes + " | CURL not init");
  }
  // end of REQUEST METHODS

  // The auxillary method to remove XML/HTML tags from a std::string

  std::string CrestRequest::parseXMLOutput(std::string_view xmlBuffer)
  {
    bool copy = true;

    std::string plainString = "";

    // remove all xml tags
    for (long unsigned int i = 0; i < xmlBuffer.length(); i++)
    {
      char convertc = xmlBuffer[i];

      if (convertc == '<')
        copy = false;
      else if (convertc == '>')
      {
        copy = true;
        continue;
      }

      if (copy)
        plainString += convertc;
    }

    return plainString;
  }

  std::string CrestRequest::removeCR(const std::string &str)
  {
    std::string str2 = str;
    std::replace(str2.begin(), str2.end(), '\n', '|');
    char needle = '\r';
    size_t pos;
    while ((pos = str2.find(needle)) != str2.npos)
    {
      str2.erase(pos, 1);
    }
    return str2;
  }

  bool CrestRequest::isJson(const std::string &str)
  {
    try
    {
      nlohmann::json js = nlohmann::json::parse(str);
      return true;
    }
    catch (nlohmann::json::parse_error &e)
    {
      return false;
    }
  }

  void CrestRequest::checkResult(CURLcode res, const long response_code, const std::string &st,
                                 const char *method_name)
  {
    // Bad HTTP response:

    if (res != CURLE_OK)
    {
      std::string mes = "ERROR in ";
      mes += method_name;
      mes += " | ";
      throw CrestException(mes + std::to_string(response_code));
    }

    // Errors, decoded from JSON CREST Server messages:

    if (isJson(st))
    {
      nlohmann::json respond = getJson(st);
      checkErrors(respond, method_name);
    }

    // HTTP response code error interval:

    if (response_code >= 400 || response_code == 303)
    {
      std::string s = parseXMLOutput(st); // to remove HTML tags
      s = removeCR(s);                    // to remove end lines and carridge returns
      std::string mes = "ERROR in ";
      mes += method_name;
      mes += " | ";
      throw CrestException(mes + "CREST Server response : " + s);
    }
  }

  nlohmann::json CrestRequest::getJson(const std::string &str, const char *method)
  {
    try
    {
      nlohmann::json js = nlohmann::json::parse(str);
      return js;
    }
    catch (nlohmann::json::parse_error &e)
    {
      if (method == nullptr || *method == '\0')
      {
        // method name is undefined

        std::string wh = e.what();
        throw CrestException("ERROR in JSON conversion: " + wh + " | In string: " + str);
      }
      else
      {
        std::string str2 = parseXMLOutput(str); // to remove HTML tags use this function
        std::string str3 = removeCR(str2);      // to remove carridge return
        throw CrestException("ERROR in " + std::string(method) + " | CREST Server response : " + str3);
      }
    }
  }

  nlohmann::json CrestRequest::getJson(const std::string &str)
  {
    return getJson(str, "");
  }

  int CrestRequest::checkErrors(const nlohmann::json &js, const char *method)
  {
    int result = 0;
    auto res = js.find("type");

    // Old format parsing:

    if (res != js.end())
    {
      std::string type = js.value("type", " unknown type ");
      if (type == "error" || type == "info")
      {
        std::string message = js.value("message", " unknown message ");
        throw CrestException("ERROR in " + std::string(method) + " | CREST response: " + message);
      }
    }

    // New format parsing:

    auto r1 = js.find("error");
    auto r2 = js.find("message");

    if (r1 != js.end())
    {
      std::string error = js.value("error", " unknown error ");
      if (r2 != js.end())
      {
        std::string message = js.value("message", "");
	throw CrestException("ERROR in " + std::string(method) + " | CREST response: " + (message.empty() ?error : message));
      }
    }
    return result;
  }

}
