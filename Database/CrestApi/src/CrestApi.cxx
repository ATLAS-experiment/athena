/*
  Copyright (C) 2020-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <CrestApi/CrestApi.h>
#include <CrestApi/CrestRequest.h>
#include <CrestApi/CrestModel.h>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>
#include <boost/asio.hpp>

#include <fstream>
#include <filesystem>
#include <iostream>

#include <CrestApi/picosha2.h>

#include <cstdio>
#include <ctime>
#include <cstdlib>

namespace Crest
{

    /**
     * CrestClient constructor for Internet mode. If CrestClient is created with this method the data will be sent to the
     * CREST Server.
     * @param _host - host name of the CREST Server.
     * @param _port - port of the CREST Server.
     * @param check_version - the parameter to switch CREST version checking, if this parameter is true,
     * the CREST version test will be executed.
     */
    CrestClient::CrestClient(const std::string &_host, const std::string &_port, bool check_version) : m_host(_host), m_port(_port)
    {
        if (check_version == true)
        {
            checkCrestVersion();
        }
        m_request.setHost(m_host);
        m_request.setPort(m_port);
	m_request.setPrefix(m_prefix);
    }

    /**
     * CrestClient constructor for Internet mode. If CrestClient is created with this method the data will be sent to the
     * CREST Server.
     * @param url - URL address of the CREST Server (with port).
     * @param check_version - the parameter to switch CREST version checking, if this parameter is true,
     * the CREST version test will be executed.
     * <br> <br>
     * Example:
     * <br>
     * <pre>
     *    std::string url = "http://mvg-test-pc-03.cern.ch:8090";
     *    CrestClient myCrestClient = CrestClient(url);
     * </pre>
     */
    CrestClient::CrestClient(std::string_view url, bool check_version)
    {
        size_t found = url.find_first_of(':');
        size_t n = url.size();

        std::string_view url_new = url.substr(found + 3); // url_new is the url excluding the http part
        m_prefix = url.substr(0,found + 3); // URL prefix "http://" or "https://"
	
        size_t found1 = url_new.find_first_of(':');
        size_t found2 = url_new.find_first_of('/');

        std::string_view host;
        std::string_view port;
        std::string_view resources;
        if (found1 != std::string::npos && found2 != std::string::npos) {
          host = url_new.substr(0, found1);
          port = url_new.substr(found1 + 1, found2 - found1 - 1);
        } else if (found1 != std::string::npos) {
          host = url_new.substr(0, found1);
          port = url_new.substr(found1 + 1);
        } else if (found2 != std::string::npos) {
          if (m_prefix == "https://") {
    	    port = "443";
          }
          else port = "80";
          host = url_new.substr(0, found2);
        } else {
          if (m_prefix == "https://") {
	      port = "443";
          }
          else port = "80";
          host = url_new;
        }

        if (found2 < n - 1)
        {
            resources = url_new.substr(found2, n - 1);
            m_PATH = resources;
        }

        m_port = std::string(port);
        m_host = std::string(host);
        m_request.setHost(m_host);
        m_request.setPort(m_port);
	m_request.setPrefix(m_prefix);

        if (check_version == true)
        {
            checkCrestVersion();
        }
        std::cout << "CrestClient::CrestClient: host = " << m_host << ", port = " << m_port << ", path = " << m_PATH << std::endl;
    }

    /**
     * CrestClient destructor.
     */
    CrestClient::~CrestClient() {}

  
    // The auxillary method to remove XML/HTML tags from a std::string
    std::string CrestClient::parseXMLOutput(const std::string_view xmlBuffer) const
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

 
    // The auxillary method to remove carridge returns from a std::string
    std::string CrestClient::removeCR(const std::string &str) const
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


    /**
     * CrestClient::getJson method is used to convert the string to the json object.
     * @param str - string to be converted to the json object.
     * @param method - name of the method which calls this method.
     * @return json object.
     */
    nlohmann::json CrestClient::getJson(const std::string &str, const char *method) const
    {
        nlohmann::json js = nullptr;
        try
        {
            js = nlohmann::json::parse(str);
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
        catch (const std::exception &e)
        {
            throw CrestException("ERROR in CrestClient::" + (std::string)method + ": " + e.what());
        }
        return js;
    }

    nlohmann::json CrestClient::getMgmtInfo()
    {
        const char *method_name = "getMgmtInfo";

        std::string current_path = s_MGMT_PATH + s_MGMT_INFO_PATH;
        std::string retv;
        nlohmann::json js = nullptr;

        retv = m_request.performRequest(current_path, Action::GET, js, method_name);

        nlohmann::json respond = getJson(retv, method_name);

        return respond;
    }

    std::string CrestClient::getCrestVersion()
    {

        std::string version = "";
        nlohmann::json buildInfo;
        nlohmann::json info = getMgmtInfo();
        // The CREST Server response has the following structure:
        // { "build": { "artifact": "crestdb", "name": "crest", "version": "4.0", "group": "hep.crest"} }
        try
        {
            buildInfo = info["build"];
            version = buildInfo["version"];
        }
        catch (const std::exception &e)
        {
            throw CrestException("ERROR in CrestClient::getCrestVersion: " + to_string(info) + " does not contain version info.");
        }
        return version;
    }

    std::string CrestClient::getClientVersion()
    {
        return s_CREST_CLIENT_VERSION;
    }

    int CrestClient::getMajorVersion(std::string &str)
    {
        int v = -1;
        int n = str.find('.');

        if (n < 1)
        {
            throw CrestException("ERROR in CrestClient::getMajorVersion: string \"" + str + "\" does not contain major version.");
        }

        std::string vers = str.substr(0, n);

        try
        {
            v = std::stoi(str);
        }
        catch (const std::exception &e)
        {
            throw CrestException("ERROR in CrestClient::getMajorVersion: string \"" + str + "\" does not contain major version.");
        }

        return v;
    }

    void CrestClient::checkCrestVersion()
    {
        std::string client = getClientVersion();
        std::string server = getCrestVersion();

        int clientVersion = getMajorVersion(client);
        int serverVersion = getMajorVersion(server);

        if (clientVersion != serverVersion)
        {
	  throw CrestException("ERROR in CrestClient::checkCrestVersion: CrestApi version \"" + client + "\" does not correspond to the server version \"" + server + "\".");
        }
    }

    std::string CrestClient::makeUrl(const std::string &address) const
    {
        std::string str = m_prefix;
        str += m_host;
        str += ':';
        str += m_port;
        str += address;
        return str;
    }
    //==============================================================================================================
    // GlobalTag methods:
    void CrestClient::createGlobalTag(GlobalTagDto &dto)
    {
        const char *method_name = "CrestClient::createGlobalTag";

        std::string current_path = m_PATH;
        current_path += s_GLOBALTAG_PATH;
        std::string retv;
        nlohmann::json js = dto.to_json();
        retv = m_request.performRequest(current_path, Action::POST, js, method_name);
    }

    GlobalTagDto CrestClient::findGlobalTag(const std::string &name)
    {
        const char *method_name = "CrestClient::findGlobalTag";

        std::string current_path = m_PATH;
        current_path += s_GLOBALTAG_PATH;
        current_path += '/';
        current_path += name;
        std::string retv;

        nlohmann::json js = nullptr;
        retv = m_request.performRequest(current_path, Action::GET, js, method_name);
        nlohmann::json response = getJson(retv, method_name);

        GlobalTagSetDto dto = GlobalTagSetDto::from_json(response);
        std::vector<GlobalTagDto> v = dto.resources;
        // Only one element is expected:
        return v[0];
    }

    GlobalTagSetDto CrestClient::listGlobalTags(const std::string &name, int size, int page, const std::string &sort)
    {
        const char *method_name = "CrestClient::listGlobalTags";

        std::string current_path = m_PATH;
        current_path += s_GLOBALTAG_PATH;
        current_path += '?';
        if (name != "")
        {
            current_path += "name=";
            current_path += name;
            current_path += '&';
        }
        if (size != -1)
        {
            current_path += "size=";
            current_path += std::to_string(size);
            current_path += '&';
        }
        if (page != -1)
        {
            current_path += "page=";
            current_path += std::to_string(page);
            current_path += '&';
        }
        if (sort != "")
        {
            current_path += "sort=";
            current_path += sort;
            current_path += '&';
        }
        std::string retv;

        nlohmann::json js = nullptr;
        retv = m_request.performRequest(current_path, Action::GET, js, method_name);
        nlohmann::json response = getJson(retv, method_name);

        GlobalTagSetDto dto = GlobalTagSetDto::from_json(response);

        return dto;
    }

    // The method to remove a global tag:
    void CrestClient::removeGlobalTag(const std::string &name)
    {
        const char *method_name = "CrestClient::removeGlobalTag";

        std::string current_path = m_PATH;
        current_path += s_ADMIN_PATH;
        current_path += s_GLOBALTAG_PATH;
        current_path += '/';
        current_path += name;
        std::string retv;
        nlohmann::json js = nullptr;
        retv = m_request.performRequest(current_path, Action::DELETE, js, method_name);
    }

    //==============================================================================================================
    // Tag methods:
    void CrestClient::createTag(TagDto &dto)
    {
        const char *method_name = "CrestClient::createTag";

        std::string current_path = m_PATH;
        current_path += s_TAG_PATH;
        std::string retv;
        nlohmann::json js = dto.to_json();
        retv = m_request.performRequest(current_path, Action::POST, js, method_name);
    }

    // The method to find a tag:
    TagDto CrestClient::findTag(const std::string &name)
    {
        const char *method_name = "CrestClient::findTag";

        std::string current_path = m_PATH;
        current_path += s_TAG_PATH;
        current_path += '/';
        current_path += name;
        std::string retv;
        nlohmann::json js = nullptr;
        retv = m_request.performRequest(current_path, Action::GET, js, method_name);
        nlohmann::json response = getJson(retv, method_name);
        TagSetDto dto = TagSetDto::from_json(response);

        std::vector<TagDto> v = dto.resources;
        // Only one element is expected:
        return v[0];
    }

    TagSetDto CrestClient::listTags(const std::string &name, int size, int page, const std::string &sort)
    {
        const char *method_name = "CrestClient::listTags";

        std::string current_path = m_PATH;
        current_path += s_TAG_PATH;
        current_path += '?';
        if (name != "")
        {
            current_path += "name=";
            current_path += name;
            current_path += '&';
        }
        if (size != -1)
        {
            current_path += "size=";
            current_path += std::to_string(size);
            current_path += '&';
        }
        if (page != -1)
        {
            current_path += "page=";
            current_path += std::to_string(page);
            current_path += '&';
        }
        if (sort != "")
        {
            current_path += "sort=";
            current_path += sort;
            current_path += '&';
        }
        std::string retv;

        nlohmann::json js = nullptr;
        retv = m_request.performRequest(current_path, Action::GET, js, method_name);
        nlohmann::json response = getJson(retv, method_name);

        TagSetDto dto = TagSetDto::from_json(response);

        return dto;
    }

    // The method to remove a tag:
    void CrestClient::removeTag(const std::string &name)
    {
        const char *method_name = "CrestClient::removeTag";

        std::string current_path = m_PATH;
        current_path += s_ADMIN_PATH;
        current_path += s_TAG_PATH;
        current_path += '/';
        current_path += name;
        std::string retv;
        nlohmann::json js = nullptr;
        retv = m_request.performRequest(current_path, Action::DELETE, js, method_name);
    }

    //==============================================================================================================
    // TagMeta methods:
    void CrestClient::createTagMeta(TagMetaDto &dto)
    {
        const char *method_name = "CrestClient::createTagMeta";

        std::string current_path = m_PATH;
        current_path += s_TAG_PATH;
        current_path += '/';
        current_path += dto.tagName;
        current_path += s_META_PATH;

        std::string retv;
        nlohmann::json js = dto.to_json();
        retv = m_request.performRequest(current_path, Action::POST, js, method_name);
    }

    void CrestClient::updateTagMeta(TagMetaDto &dto)
    {
        const char *method_name = "CrestClient::updateTagMeta";

        std::string current_path = m_PATH;
        current_path += s_TAG_PATH;
        current_path += '/';
        current_path += dto.tagName;
        current_path += s_META_PATH;

        std::string retv;
        nlohmann::json js = dto.to_json();
        retv = m_request.performRequest(current_path, Action::PUT, js, method_name);
    }

    TagMetaDto CrestClient::findTagMeta(const std::string &name)
    {
        const char *method_name = "CrestClient::findTagMeta";

        std::string current_path = m_PATH;
        current_path += s_TAG_PATH;
        current_path += '/';
        current_path += name;
        current_path += s_META_PATH;
        std::string retv;
        nlohmann::json js = nullptr;
        retv = m_request.performRequest(current_path, Action::GET, js, method_name);
        nlohmann::json response = getJson(retv, method_name);

        TagMetaSetDto dto = TagMetaSetDto::from_json(response);
        std::vector<TagMetaDto> v = dto.resources;
        // Only one element is expected:
        return v[0];
    }


    int CrestClient::getSize(const std::string &tagname)
    {
        const char *method_name = "CrestClient::getSize";

        std::string current_path = m_PATH;
        current_path += s_IOV_PATH;
        current_path += s_IOV_SIZE_PATH;
        current_path += "?tagname=";
        current_path += tagname;

        std::string retv;

        nlohmann::json js = nullptr;
        retv = m_request.performRequest(current_path, Action::GET, js, method_name);
        nlohmann::json respond = getJson(retv, method_name);

        auto res = respond.find("resources");
        nlohmann::json r;

        if (res != respond.end())
        {
            r = respond["resources"][0];
        }

        int result = 0;
        if (r.find("niovs") != r.end())
        {
            result = r["niovs"];
        }
        else
        {
            throw CrestException("ERROR in CrestClient::getSize CREST Server JSON response has no \"size\" key.");
        }

        return result;
    }

    //==============================================================================================================
    // GlobalTagMaps methods:
    void CrestClient::createGlobalTagMap(GlobalTagMapDto &dto)
    {
        const char *method_name = "CrestClient::createGlobalTagMap";

        std::string current_path = m_PATH;
        current_path += s_GLOBALTAG_MAP_PATH;
        std::string retv;
        nlohmann::json js = dto.to_json();
        retv = m_request.performRequest(current_path, Action::POST, js, method_name);
    }

    GlobalTagMapSetDto CrestClient::findGlobalTagMap(const std::string &name, const std::string &xCrestMapMode)
    {
        const char *method_name = "CrestClient::findGlobalTagMap";

        std::string current_path = m_PATH;
        current_path += s_GLOBALTAG_MAP_PATH;
        current_path += '/';
        current_path += name;
        std::string headers_params = "";

        if (xCrestMapMode != "")
        {
            headers_params += "X-Crest-MapMode: ";
            headers_params += xCrestMapMode;
        }
        else
        {
            headers_params += "X-Crest-MapMode: ";
            headers_params += "Trace";
        }
        std::string retv;
        nlohmann::json js = nullptr;
        retv = m_request.performRequest(current_path, Action::GET, js, method_name, headers_params);
        nlohmann::json response = getJson(retv, method_name);
        GlobalTagMapSetDto dto = GlobalTagMapSetDto::from_json(response);

        return dto;
    }

    void CrestClient::removeGlobalTagMap(const std::string &name, const std::string &record, const std::string &label, const std::string &tagname)
    {
        const char *method_name = "CrestClient::removeGlobalTagMap";

        std::string current_path = m_PATH;
        current_path += s_GLOBALTAG_MAP_PATH;
        current_path += '/';
        current_path += name;
        current_path += '?';
        if (tagname != "")
        {
            current_path += "tagname=";
            current_path += tagname;
            current_path += '&';
        }
        if (record != "")
        {
            current_path += "record=";
            current_path += record;
            current_path += '&';
        }
        if (label != "")
        {
            current_path += "label=";
            current_path += label;
            current_path += '&';
        }

        std::string retv;
        nlohmann::json js = nullptr;
        retv = m_request.performRequest(current_path, Action::DELETE, js, method_name);
    }

    //==============================================================================================================
    // Iovs methods:
    IovSetDto CrestClient::selectIovs(const std::string &name, uint64_t since, uint64_t until, long snapshot, int size, int page, const std::string &sort)
    {
        const char *method_name = "CrestClient::selectIovs";

        std::string current_path = m_PATH;
        current_path += s_IOV_PATH;

        current_path += "?method=";
        current_path += s_METHOD_IOVS;
        current_path += "&tagname=";

        current_path += name;
        current_path += "&since=";
        current_path += std::to_string(since);
        current_path += "&until=";
        if (until == static_cast<uint64_t>(-1))
        {
            current_path += "INF";
        }
        else
        {
            current_path += std::to_string(until);
        }
        current_path += "&snapshot=";
        current_path += std::to_string(snapshot);
	//
        current_path += "&size=";
        current_path += std::to_string(size);
        current_path += "&page=";
        current_path += std::to_string(page);
        current_path += "&sort=";
        current_path += sort;
	//

        std::string retv;

        nlohmann::json js = nullptr;
        retv = m_request.performRequest(current_path, Action::GET, js, method_name);
        nlohmann::json response = getJson(retv, method_name);

        IovSetDto dto = IovSetDto::from_json(response);

        return dto;
    }

    IovSetDto CrestClient::selectGroups(const std::string &name, long snapshot, int size, int page, const std::string &sort)
    {
        const char *method_name = "CrestClient::selectGroups";

        std::string current_path = m_PATH;
        current_path += s_IOV_PATH;

        current_path += "?method=";
        current_path += s_METHOD_GROUPS;
        current_path += "&tagname=";

        current_path += name;
        current_path += "&snapshot=";
        current_path += std::to_string(snapshot);

	//
        current_path += "&size=";
        current_path += std::to_string(size);
        current_path += "&page=";
        current_path += std::to_string(page);
        current_path += "&sort=";
        current_path += sort;
	//
	
        std::string retv;

        nlohmann::json js = nullptr;
        retv = m_request.performRequest(current_path, Action::GET, js, method_name);
        nlohmann::json response = getJson(retv, method_name);

        IovSetDto dto = IovSetDto::from_json(response);

        return dto;
    }

    //==============================================================================================================
    // Payload methods:
    void CrestClient::storeData(const std::string &tag,
                                const StoreSetDto &storeSetJson,
				                const std::string &payloadFormat,
                                const std::string &objectType,
                                const std::string &compressionType,
                                const std::string &version,
                                uint64_t endTime)
    {

        std::string current_path = m_PATH;
        current_path += s_PAYLOAD_PATH;
        std::vector<std::string> files;
        std::string retv;
        nlohmann::json js = storeSetJson.to_json();
        if (payloadFormat == "JSON")
        {
            retv = m_request.uploadPayload(current_path, tag, endTime, js, objectType, compressionType, version, files);
        }
        else
        {
            // Assumes the data content in the JSON is just a file path.
            nlohmann::json resources = nullptr;

            auto subjectIdIter1 = js.find("resources");
            if (subjectIdIter1 != js.end())
            {
                resources = js["resources"];
            }
            int partN = resources.size();
            for (int i = 0; i < partN; i++)
            {
                nlohmann::json element = resources[i];
                std::string file_param;

                auto subjectIdIter1 = element.find("data");
                if (subjectIdIter1 != element.end())
                {
                    file_param = element["data"];
                }
                //
                int found_dots = file_param.find_first_of(':');
                int word_size = file_param.size();
                std::string data_file = file_param.substr(found_dots + 3, word_size);
                files.push_back(data_file);
            }
            retv = m_request.uploadPayload(current_path, tag, endTime, js, objectType, compressionType, version, files);
        }
    }

    std::string CrestClient::getPayload(const std::string &hash)
    {
        const char *method_name = "CrestClient::getPayload";

        std::string current_path = m_PATH;
        current_path += s_PAYLOAD_PATH;
        current_path += "/data?format=BLOB&hash=";
        current_path += hash;
        std::string retv;
        nlohmann::json js = nullptr;
        retv = m_request.performRequest(current_path, Action::GET, js, method_name);
        return retv;
    }

    PayloadDto CrestClient::getPayloadMeta(const std::string &hash)
    {
        const char *method_name = "CrestClient::getPayloadMeta";

        std::string current_path = m_PATH;
        current_path += s_PAYLOAD_PATH;
        current_path += '/';
        current_path += hash;
        current_path += "?format=META";
        std::string retv;
        nlohmann::json js = nullptr;
        retv = m_request.performRequest(current_path, Action::GET, js, method_name);
        nlohmann::json response = getJson(retv, method_name);

        PayloadSetDto dto = PayloadSetDto::from_json(response);
        std::vector<PayloadDto> v = dto.resources;
        // Only one element is expected:
        return v[0];
    }
} // namespace Crest

