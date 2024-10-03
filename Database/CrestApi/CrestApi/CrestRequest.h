/*
   Copyright (C) 2019-2024 CERN for the benefit of the ATLAS collaboration
 */

#ifndef CRESTAPI_REQUEST_H
#define CRESTAPI_REQUEST_H

#include <string>
#include <map>
#include <iosfwd>
#include <cstdint>
#include "nlohmann/json.hpp"
#include <iostream>
#include <curl/curl.h>

namespace Crest
{

    // The action to be performed
    enum class Action
    {
        GET = 0,
        POST = 1,
        PUT = 2,
        DELETE = 3
    };

    class urlParameters
    {
    private:
        std::string m_str{};

    public:
        void add(const std::string &key, const std::string &value)
        {
            if (m_str.empty())
            {
                m_str = key;
                m_str += '=';
                m_str += value;
            }
            else
            {
                m_str += '&';
                m_str += key;
                m_str += '=';
                m_str += value;
            }
        }

        const std::string &getParams() const
        {
            return m_str;
        }

        void reset()
        {
            m_str.clear();
        }
    };

    class CrestRequest
    {

    private:
        std::string m_prefix = "http://";
        std::string m_host;
        std::string m_port;
        std::string make_url(const std::string &address) const;

        char* m_CREST_PROXY = NULL;
        const char* m_CREST_PROXY_VAR = "SOCKS_PROXY";

        // Auxiliary method to make request to the CREST Server. This method is used by other methods realizing the
        // requests with the concrete kinds of data (iovs|payloads|tags…).
        // They are used to clean the response from the CREST Server from the additional information and to check the
        // response code.
        bool isJson(const std::string& str);
        nlohmann::json getJson(const std::string& str, const char* method);
        nlohmann::json getJson(const std::string& str);
        int checkErrors(const nlohmann::json& js, const char* method);
        std::string parseXMLOutput(const std::string_view xmlBuffer);
        std::string removeCR(const std::string& str);

        void getProxyPath();
      
    public:
        CrestRequest();
        ~CrestRequest();

        void setHost(const std::string& host);
        void setPort(const std::string& port);
        void setPrefix(const std::string& prefix);
      
        /**
         * General auxiliary method to make request to the CREST Server. This method is used by other methods realizing the
         * requests with the concrete kinds of data (iovs|payloads|tags…).
         *
         * @param current_path - URL request path
         * @param action - Action (GET|POST|DELETE)
         * @param js - a JSON object which has to be send or changed with this request. If this argument has to be void it has
         * to be set as nullptr.
         *
         */
        std::string performRequest(const std::string &current_path, Action action, nlohmann::json &js, const std::string& header_params = "");

        /**
         * General auxillary method to make request to the CREST Server. This method is used by other methods realizing the
         * requests with the concrete kinds of data (iovs|payloads|tags…).
         *
         * @param current_path - URL request path
         * @param action - Action (GET|POST|DELETE)
         * @param js - a JSON object which has to be send or changed with this request. If this argument has to be void it has
         * to be set as nullptr.
         * @param method_name - the name of method which called this method. This parameter is used in the error messages.
         *
         */
        std::string performRequest(const std::string &current_path, Action action, nlohmann::json &js,
                                   const char *method_name, const std::string& header_params = "");

    
        std::vector<char> getPayloadRequest(const std::string &current_path);


	std::string uploadPayload(const std::string &current_path, 
            const std::string& tag, uint64_t endtime, const nlohmann::json& js, 
            const std::string& objectType, const std::string& compressionType, const std::string& version, 
            const std::vector<std::string>& files);

        void checkResult(CURLcode res, long response_code, const std::string& st, const char* method_name);
    };
}

#endif // CRESTAPI_REQUEST_H
