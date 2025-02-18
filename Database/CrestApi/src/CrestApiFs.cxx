/*
  Copyright (C) 2020-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <CrestApi/CrestApiFs.h>
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

#include <algorithm>

namespace Crest
{

  /**
   * CrestFsClient constructor for Internet mode. If CrestClient is created with this method the data will be sent to the
   * CREST Server.
   * @param rewriteIfExists - Boolean flag to reinitialize the file system.
   * @param root_folder - root directory of the CREST storage.
   */
  CrestFsClient::CrestFsClient(bool rewriteIfExists, const std::string &root_folder)
      : m_root_folder(root_folder), m_isRewrite(rewriteIfExists)
  {
    if (m_root_folder == "") {
      m_root_folder = std::filesystem::current_path();
      m_root_folder += "/crest";
    } 
    if (!std::filesystem::exists(std::filesystem::path(m_root_folder)))
    {
      std::filesystem::create_directory(std::filesystem::path(m_root_folder));
    }
    m_data_folder = m_root_folder + "/data";
    if (!std::filesystem::exists(std::filesystem::path(m_data_folder)))
    {
      std::filesystem::create_directory(std::filesystem::path(m_data_folder));
    }
  }

  /**
   * CrestClient destructor.
   */
  CrestFsClient::~CrestFsClient() {}

  // UTILITIES Methods

  std::string CrestFsClient::getFileString(const std::string& path) {
    std::ifstream ifs(path);
    std::stringstream buf;
    buf << ifs.rdbuf();
    return buf.str();
  }

  std::string CrestFsClient::buildPath(const std::string &path, const std::string &name)
  {
    std::string p = m_root_folder;
    if (!std::filesystem::exists(std::filesystem::path(m_root_folder)))
    {
      std::filesystem::create_directory(std::filesystem::path(m_root_folder));
    }
    p += path;
    if (!std::filesystem::exists(std::filesystem::path(p)))
    {
      std::filesystem::create_directory(std::filesystem::path(p));
    }
    p += '/';
    p += name;
    if (!std::filesystem::exists(std::filesystem::path(p)))
    {
      std::filesystem::create_directory(std::filesystem::path(p));
    }
    return p;
  }

  void CrestFsClient::getFileList(const std::string &path)
  {
    std::filesystem::path p(path);
    for (auto i = std::filesystem::directory_iterator(p); i != std::filesystem::directory_iterator(); i++)
    {
      std::string file = i->path().filename().string();
      if (file != "data")
      {
        std::cout << file << std::endl;
      }
      else{
        continue;
      }
    }
  }

  // Global tag methods:

  void CrestFsClient::createGlobalTag(GlobalTagDto &globalTag)
  {
    std::string name = "";
    if (globalTag.name != "")
    {
      name = globalTag.name;
    }
    else
    {
      throw CrestException("ERROR in CrestFsClient::createGlobalTag: global tag name is not set.");
    }

    std::string workDir = buildPath(s_FS_GLOBALTAG_PATH, name);
    std::string globalTagFile = workDir + s_FS_GLOBALTAG_FILE;

    if (m_isRewrite)
    {
      if (std::filesystem::exists(std::filesystem::path(globalTagFile)))
      {
        std::filesystem::remove(std::filesystem::path(globalTagFile));
      }

      std::ofstream outFile;

      outFile.open(globalTagFile.c_str());
      outFile << globalTag.to_json();
      outFile.close();
    }

    return;
  }

  GlobalTagDto CrestFsClient::findGlobalTag(const std::string &name)
  {
    nlohmann::json js = nullptr;
    GlobalTagDto dto;

    std::string workDir = buildPath(s_FS_GLOBALTAG_PATH, name);
    std::string file_path = workDir + s_FS_GLOBALTAG_FILE;

    try
    {
      std::string tag = getFileString(file_path);
      js = nlohmann::json::parse(tag);
      dto = GlobalTagDto::from_json(js);
    }
    catch (...)
    {
      throw CrestException(
          "ERROR in CrestFsClient::findGlobalTag: cannot get the global tag " + name + " form the file storage.");
    }

    return dto;
  }

  void CrestFsClient::removeGlobalTag(const std::string &)
  {
    checkFsException("CrestFsClient::removeGlobalTag");
  }

  GlobalTagSetDto CrestFsClient::listGlobalTags(const std::string &name, int size, int page, const std::string &sort)
  {

    std::string folder = m_root_folder + s_FS_GLOBALTAG_PATH;

    GlobalTagSetDto tagSet;

    bool ascending = true;
    if (sort == "name:ASC")
    {
      ascending = true;
    }
    else if (sort == "name:DESC")
    {
      ascending = false;
    }
    else
    {
      throw CrestException(
          "ERROR in CrestFsClient::listTags: wrong sort parameter." + sort);
    }

    try
    {
      std::vector<std::string> taglist = nameList(folder, ascending);
      std::vector<std::string> clearedTaglist;

      for (const std::string& tag : taglist)
      {
        std::string file_name = folder + "/" + tag + "/" + s_FS_GLOBALTAG_FILE;

        if (std::filesystem::exists(file_name))
        {
          if (name != "")
          {
            if (isMatch(tag, name))
            {
              clearedTaglist.push_back(tag);
            }
          }
          else
          {
            clearedTaglist.push_back(tag);
          }
        }
      }

      taglist = getVectorPage(clearedTaglist, size, page);
      for (const std::string& tag : taglist)
      {
        std::string file_name = folder + "/" + tag + "/" + s_FS_GLOBALTAG_FILE;
        GlobalTagDto dto = findGlobalTag(tag);
        tagSet.resources.push_back(dto);
      }
      tagSet.datatype = "";
    }
    catch (const std::exception &e)
    {
      throw CrestException(
          "ERROR in CrestFsClient::listGlobalTags: cannot get the tag list.");
    }

    return tagSet;
  }

  // Tag methods:

  void CrestFsClient::createTag(TagDto &tag)
  {
    std::string name = tag.name;
    std::string workDir = buildPath(s_FS_TAG_PATH, name);

    if (name.compare(m_currentTag) != 0 && m_isRewrite)
    {
      flush();
    }
    m_currentTag = name;
    std::string tagFile = workDir + s_FS_TAG_FILE;
    std::string iovFile = workDir + s_FS_IOV_FILE;

    if (m_isRewrite)
    {
      if (std::filesystem::exists(std::filesystem::path(tagFile)))
      {
        std::filesystem::remove(std::filesystem::path(tagFile));
      }
      if (std::filesystem::exists(std::filesystem::path(iovFile)))
      {
        std::filesystem::remove(std::filesystem::path(iovFile));
      }
      std::ofstream outFile;
      outFile.open(tagFile.c_str());
      outFile << tag.to_json();
      outFile.close();
    }
    if (m_data.find(name) == m_data.end())
    {
      m_data.insert(std::pair<std::string, nlohmann::json>(name, nlohmann::json(nlohmann::json::value_t::array)));
    }
  }

  TagDto CrestFsClient::findTag(const std::string &name)
  {
    nlohmann::json js = nullptr;
    TagDto dto;

    std::string workDir = buildPath(s_FS_TAG_PATH, name);
    std::string file_path = workDir + s_FS_TAG_FILE;

    try
    {
      std::string tag = getFileString(file_path);
      js = nlohmann::json::parse(tag);
      dto = TagDto::from_json(js);
    }
    catch (...)
    {
      throw CrestException(
          "ERROR in CrestFsClient::findTag: cannot get the tag " + name + " form the file storage.");
    }

    return dto;
  }

  void CrestFsClient::removeTag(const std::string &)
  {
    checkFsException("CrestFsClient::removeTag");
  }

  TagSetDto CrestFsClient::listTags(const std::string &name, int size, int page, const std::string &sort)
  {

    std::string folder = m_root_folder + s_FS_TAG_PATH;

    TagSetDto tagSet;

    bool ascending = true;
    if (sort == "name:ASC")
    {
      ascending = true;
    }
    else if (sort == "name:DESC")
    {
      ascending = false;
    }
    else
    {
      throw CrestException(
          "ERROR in CrestFsClient::listTags: wrong sort parameter." + sort);
    }

    try
    {
      std::vector<std::string> taglist = nameList(folder, ascending);
      std::vector<std::string> clearedTaglist;

      for (const std::string& tag : taglist)
      {
        std::string file_name = folder + "/" + tag + "/" + s_FS_TAG_FILE;

        if (std::filesystem::exists(file_name))
        {
          if (name != "")
          {
            if (isMatch(tag, name))
            {
              clearedTaglist.push_back(tag);
            }
          }
          else
          {
            clearedTaglist.push_back(tag);
          }
        }
      }

      taglist = getVectorPage(clearedTaglist, size, page);
      for (const std::string& tag : taglist)
      {
        std::string file_name = folder + "/" + tag + "/" + s_FS_TAG_FILE;
        TagDto dto = findTag(tag);
        tagSet.resources.push_back(dto);
      }
      tagSet.datatype = "";
    }
    catch (const std::exception &e)
    {
      throw CrestException(
          "ERROR in CrestFsClient::listTags: cannot get the tag list.");
    }

    return tagSet;
  }

  void CrestFsClient::flush()
  {

    for (auto &item : m_data)
    {
      nlohmann::json iov = item.second;
      std::string name = item.first;
      std::string workDir = m_root_folder + s_FS_TAG_PATH + '/' + name;
      std::ofstream outFile;
      std::string tagFile = workDir + s_FS_IOV_FILE;
      outFile.open(tagFile.c_str());
      outFile << iov;
      outFile.close();
    }

    m_data.clear();
  }

  // TagMeta methods
  void CrestFsClient::createTagMeta(TagMetaDto &tag)
  {
    std::string name = tag.tagName;

    std::string workDir = buildPath(s_FS_TAG_PATH, name);
    std::string tagMetaFile = workDir + s_FS_TAGMETAINFO_FILE;

    if (m_isRewrite)
    {
      if (std::filesystem::exists(std::filesystem::path(tagMetaFile)))
      {
        std::filesystem::remove(std::filesystem::path(tagMetaFile));
      }

      std::ofstream outFile;

      outFile.open(tagMetaFile.c_str());
      outFile << tag.to_json();
      outFile.close();
    }
  }

  void CrestFsClient::updateTagMeta(TagMetaDto &)
  {
    checkFsException("CrestFsClient::updateTagMeta");
  }

  TagMetaDto CrestFsClient::findTagMeta(const std::string &name)
  {
    nlohmann::json js = nullptr;
    TagMetaDto dto;

    std::string workDir = buildPath(s_FS_TAG_PATH, name);
    std::string file_path = workDir + s_FS_TAGMETAINFO_FILE;

    try
    {
      std::string tag = getFileString(file_path);
      js = nlohmann::json::parse(tag);
      dto = TagMetaDto::from_json(js);
    }
    catch (...)
    {
      throw CrestException(
          "ERROR in CrestFsClient::findTagMeta: cannot get the tag " + name + " form the file storage.");
    }

    return dto;
  }

  // GlobalTagMap methods

  void CrestFsClient::createGlobalTagMap(GlobalTagMapDto &globalTagMap)
  {
    nlohmann::json js = globalTagMap.to_json();

    // global tag name:
    std::string name = "";
    try
    {
      name = static_cast<std::string>(js["globalTagName"]);
    }
    catch (...)
    {
      throw CrestException("ERROR in CrestClient::createGlobalTagMap: cannot get the global tag name from JSON.");
    }

    // tag name:
    std::string tagname = "";
    try
    {
      tagname = static_cast<std::string>(js["tagName"]);
    }
    catch (...)
    {
      throw CrestException("ERROR in CrestClient::createGlobalTagMap: cannot get the tag name from JSON.");
    }

    std::string workDir = buildPath(s_FS_GLOBALTAG_PATH, name);
    std::string catalogFile = workDir + s_FS_MAP_FILE;

    if (std::filesystem::exists(std::filesystem::path(catalogFile)))
    {
      // cathalogue file exists:

      std::string array_lst = getFileString(catalogFile);

      nlohmann::json cathalogue;
      try
      {
        cathalogue = nlohmann::json::parse(array_lst);
      }
      catch (...)
      {
        throw CrestException("ERROR in CrestFsClient::createGlobalTagMap: global tag map file corrupted.");
      }

      if (std::filesystem::exists(std::filesystem::path(catalogFile)))
      {
        // the file storage contains the record of the global tag map:
        int m = cathalogue.size();
        for (int i = 0; i < m; i++)
        {
          const std::string &tn = cathalogue[i]["tagName"];
          if (tn == tagname)
          {
            cathalogue.erase(i);
          }
        }
      }
      else
      {
        // the file storage does not contain the record of the global tag map:
      }
      std::filesystem::remove(std::filesystem::path(catalogFile));
      cathalogue.push_back(js);
      std::ofstream outFile;

      outFile.open(catalogFile.c_str());
      outFile << cathalogue;
      outFile.close();
    }
    else
    {
      // cathalogue file does not exist (creation):

      nlohmann::json cathalogue = nlohmann::json::array();
      cathalogue.push_back(js);

      std::ofstream outFile;

      outFile.open(catalogFile.c_str());
      outFile << cathalogue;
      outFile.close();
    }

    return;
  }

  void CrestFsClient::removeGlobalTagMap(const std::string &, const std::string &, const std::string &, const std::string &)
  {
    checkFsException("CrestFsClient::removeGlobalTagMap");
  }

  void CrestFsClient::checkFsException(const char *method_name)
  {
    throw CrestException("ERROR in " + std::string(method_name) + " This methods is unsupported for FILESYSTEM mode");
  }

  GlobalTagMapSetDto CrestFsClient::findGlobalTagMap(const std::string &name, const std::string &xCrestMapMode)
  {

    nlohmann::json js = nullptr;

    if (xCrestMapMode != "Trace")
    {
      throw CrestException(
          "ERROR in CrestFsClient::getGlobalTagMap: not supported value for the parameter xCrestMapMode = " + xCrestMapMode);
    }

    std::string workDir = m_root_folder + s_FS_GLOBALTAG_PATH;
    workDir += '/';
    workDir += name;
    workDir += s_FS_MAP_FILE;

    try
    {
      std::string tag = getFileString(workDir);
      js = nlohmann::json::parse(tag);
    }
    catch (...)
    {
      throw CrestException(
          "ERROR in CrestFsClient::getGlobalTagMap: cannot get the global tag map " + name +
          " form the file storage.");
    }

    GlobalTagMapSetDto dto;
    dto = GlobalTagMapSetDto::from_fs_json(js);
    return dto;
  }

  // Iovs methods

  IovSetDto CrestFsClient::selectIovs(const std::string &name, uint64_t since, uint64_t until, long , int size, int page, const std::string &sort)
  {
    IovSetDto dto;

    nlohmann::json js = nlohmann::json::array();

    try
    {
      nlohmann::json iovList = findAllIovs(name);
      int niovs = iovList.size();

      for (int i = 0; i < niovs; i++)
      {
        if (iovList[i].find("since") != iovList[i].end())
        {
          uint64_t currentS = iovList[i]["since"];

          if (until != static_cast<uint64_t>(-1))
          {
            if (currentS >= since && currentS <= until)
            {
              js.push_back(iovList[i]);
            }
          }
          else
          { // until == -1, Infinity
            if (currentS >= since)
            {
              js.push_back(iovList[i]);
            }
          } // until == -1
        }
      } // for
    }
    catch (...)
    {
      throw CrestException("ERROR in CrestClient::selectIovsFS : cannot get the iov list form file storage");
    }

    bool ascending = true;
    if (sort == "id.since:ASC")
      ascending = true;
    else if (sort == "id.since:DESC")
      ascending = false;
    else
    {
      throw CrestException(
          "ERROR in CrestFsClient::selectIovs: wrong sort parameter." + sort);
    }

    nlohmann::json sorted = sortIOVJson(std::move(js), ascending);
    nlohmann::json ext = getPage(std::move(sorted), size, page);
    dto = IovSetDto::from_fs_json(ext);

    return dto;
  }

  nlohmann::json CrestFsClient::findAllIovs(const std::string &tagname)
  {
    nlohmann::json js = nullptr;
    std::string file_path = m_root_folder;
    file_path += '/';
    file_path += s_FS_TAG_PATH;
    file_path += '/';
    file_path += tagname;
    file_path += s_FS_IOV_FILE;

    try
    {
      std::string tag = getFileString(file_path);
      js = nlohmann::json::parse(tag);
    }
    catch (const std::exception &e)
    {
      throw CrestException("ERROR in CrestFsClient::findAllIovs : cannot get the iov information form file storage "); 
    }

    return js;
  }

  int CrestFsClient::getSize(const std::string &tagname)
  {
    int res = 0;

    try
    {
      nlohmann::json iovs = findAllIovs(tagname);
      int length = iovs.size();
      return length;
    }
    catch (...)
    {
      return res;
    }

    return res;
  }

  IovSetDto CrestFsClient::selectGroups(const std::string &, long , int , int , const std::string &)
  {
    checkFsException("CrestFsClient::selectGroups");
    return IovSetDto();
  }

  // Payload methods
  void CrestFsClient::storeData(const std::string &tag,
                                const StoreSetDto &storeSetJson,
                                const std::string &payloadFormat,
                                const std::string &objectType,
                                const std::string &compressionType,
                                const std::string &version,
                                uint64_t)
  {
    std::string name = tag;

    std::string workDir = buildPath(s_FS_TAG_PATH, name);
    std::string tagFile = workDir + s_FS_TAG_FILE;
    std::string iovFile = workDir + s_FS_IOV_FILE;

    nlohmann::json js_data = storeSetJson.to_json();

    nlohmann::json res;
    auto it = js_data.find("resources");
    if (it != js_data.end())
    {
      res = js_data["resources"];
    }

    try
    {
      for (auto &kvp : res)
      {
        std::string payload = kvp.value("data", "");
        int since = kvp.value("since", 0);
        std::string streamer = kvp.value("streamerInfo", "");
        // Register everything on the file system.
        storePayloadDump(tag, since, payload, payloadFormat,
                         objectType, compressionType, version, streamer);
      }
    } // end of try
    catch (...)
    {
      throw CrestException("ERROR in CrestFsClient::storeData cannot store the data in a file");
    } // end of catch
    flush();
  }

  std::string CrestFsClient::getDateAndTime()
  {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];

    localtime_r(&now, &tstruct);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    return buf;
  }

  std::string CrestFsClient::getFirstLetters(const std::string &str)
  {
    std::string result = str.substr(0, s_FS_PREFIX_LENGTH);
    return result;
  }

  void CrestFsClient::storePayloadDump(const std::string &tag,
                                       uint64_t since,
                                       const std::string &js,
                                       const std::string &payloadFormat,
                                       const std::string &objectType,
                                       const std::string &compressionType,
                                       const std::string &version,
                                       const std::string &streamerInfo)
  {

    std::ofstream outFile;

    std::string hashCode;
    std::string payloadLocalFile;

    // payload file:
    if (payloadFormat == "JSON")
    {
      hashCode = getHash(js);
    }
    else
    {

      int found_dots = js.find_first_of(':');
      int word_size = js.size();
      payloadLocalFile = js.substr(found_dots + 3, word_size);

      hashCode = getHashForFile(payloadLocalFile);
    }

    std::string workDir = m_data_folder;
    workDir += '/';
    workDir += getFirstLetters(hashCode);
    if (!std::filesystem::exists(std::filesystem::path(workDir)))
    {
      std::filesystem::create_directory(std::filesystem::path(workDir));
    }
    workDir += '/';
    workDir += hashCode;
    if (!std::filesystem::exists(std::filesystem::path(workDir)))
    {
      std::filesystem::create_directory(std::filesystem::path(workDir));
    }
    std::string tagFile = workDir + "/payload.json";

    if (payloadFormat == "JSON")
    {
      outFile.open(tagFile);
      outFile << js;
      outFile.close();
    }
    else
    {
      try
      {
        std::filesystem::copy_file(payloadLocalFile, tagFile);
      }
      catch (std::filesystem::filesystem_error &e)
      {
        throw CrestException("ERROR in CrestFsClient::storePayloadDump cannot not save payload file: " + tagFile + e.what());
      }
    }

    // Define the meta info for the payload:
    nlohmann::json jsn =
        {
            {"hash", hashCode},
            {"checkSum", "SHA-256"},
            {"objectType", objectType},
            {"version", version},
            {"size", js.size()},
            {"streamerInfo", streamerInfo},
            {"compressionType", compressionType},
            {"insertionTime", getDateAndTime()}};

    // payload meta info file:
    std::string metaFile = workDir + "/meta.json";

    outFile.open(metaFile);
    outFile << jsn.dump();
    outFile.close();

    // check if data exists

    if (m_data.find(tag) == m_data.end())
    {
      try
      {
        nlohmann::json jsi = findAllIovs(tag);
        m_data.insert(std::pair<std::string, nlohmann::json>(tag, jsi));
      }
      catch (...)
      {
        try
        { // tag exists, but there are no IOVs
          nlohmann::json tagJS = findTag(tag).to_json();
          nlohmann::json jsFree = nlohmann::json::array({});
          m_data.insert(std::pair<std::string, nlohmann::json>(tag, jsFree));
        }
        catch (...)
        {
          throw CrestException(
              "ERROR in CrestFsClient::storePayloadDump cannot get data for tag \"" + tag + "\" from file storage.");
        }
      }
    }

    std::map<std::string, nlohmann::json>::iterator it = m_data.find(tag);
    if (it != m_data.end())
    {
      std::string link = std::move(hashCode);
      nlohmann::json iovs = it->second;
      nlohmann::json obj(nlohmann::json::value_t::object);
      obj["tagName"] = tag;
      obj["since"] = since;
      obj["insertionTime"] = getDateAndTime();
      obj["payloadHash"] = link;
      iovs.push_back(obj);
      m_data[it->first] = std::move(iovs);
    }
  }


  // Payload methods

  std::string CrestFsClient::getPayload(const std::string &hash)
  {
    std::string workDir = m_data_folder;
    workDir += '/';
    workDir += getFirstLetters(hash);
    workDir += '/';
    workDir += hash;
    std::string filePath = workDir + "/payload.json";
    std::string res = "";

    try
    {
      if (std::filesystem::exists(filePath))
      {
        res = getFileString(filePath);
      }
      else
      {
        throw CrestException("payload with hash " + hash + " does not exist.");
      }
    }
    catch (const std::exception &e)
    {
      std::string message = e.what();
      throw CrestException("ERROR in CrestFsClient::getPayload cannot get the payload form file storage, " + message);
    }

    return res;
  }

  PayloadDto CrestFsClient::getPayloadMeta(const std::string &hash)
  {
    nlohmann::json js = nullptr;
    PayloadDto dto;

    std::string workDir = m_data_folder;
    workDir += '/';
    workDir += getFirstLetters(hash);
    workDir += '/';
    workDir += hash;

    std::string filePath = std::move(workDir);
    filePath += "/meta.json";

    std::string res = "";

    try
    {
      if (std::filesystem::exists(filePath))
      {
        res = getFileString(filePath);
        js = nlohmann::json::parse(res);
        dto = PayloadDto::from_json(js);
      }
      else
      {
        throw CrestException("payload meta info with hash " + hash + " does not exist.");
      }
    }
    catch (const std::exception &e)
    {
      std::string message = e.what();
      throw CrestException("ERROR in CrestClient::getPayloadMeta cannot get the payload meta info form file storage, " + message);
    }

    return dto;
  }

  nlohmann::json CrestFsClient::getPage(nlohmann::json data, int size, int page)
  {
    nlohmann::json js = nlohmann::json::array();
    int dataSize = data.size();

    if (dataSize == 0)
      return js; // the data is absent

    // index interval to load the data from JSON array:
    int kmin = size * page;
    int kmax = size * (page + 1);

    // check if the interval is correct:
    if (kmin > dataSize - 1)
      return js; // out of range

    if (kmax > dataSize - 1)
    { // this step is not full
      kmax = dataSize;
    }

    for (int i = kmin; i < kmax; i++)
    {
      js.push_back(data[i]);
    }
    return js;
  }

  std::vector<std::string> CrestFsClient::getVectorPage(const std::vector<std::string>& data, int size, int page)
  {
    std::vector<std::string> res;
    int dataSize = data.size();

    if (dataSize == 0)
      return res; // the data is absent

    // index interval to load the data from JSON array:
    int kmin = size * page;
    int kmax = size * (page + 1);

    // check if the interval is correct:
    if (kmin > dataSize - 1)
      return res; // out of range

    if (kmax > dataSize - 1)
    { // this step is not full
      kmax = dataSize;
    }

    for (int i = kmin; i < kmax; i++)
    {
      res.push_back(data[i]);
    }
    return res;
  }

  std::vector<std::string> CrestFsClient::nameList(std::string &folder, bool ascending)
  {
    std::vector<std::string> tag_list;
    std::filesystem::path p(folder);

    for (auto i = std::filesystem::directory_iterator(p); i != std::filesystem::directory_iterator(); i++)
    {
      std::string file = i->path().filename().string();
      tag_list.push_back(file);
    }

    std::sort(tag_list.begin(), tag_list.end());

    if (ascending == false)
    {
      std::reverse(tag_list.begin(), tag_list.end());
    }

    return tag_list;
  }

  // method to sort JSON array by the key (par) value
  nlohmann::json CrestFsClient::sortJson(nlohmann::json js, const std::string &par, bool order)
  {
    nlohmann::json respond = nlohmann::json::array();
    std::vector<std::string> parlist;
    std::map<std::string, nlohmann::json> m;

    int size = js.size();
    for (int i = 0; i < size; i++)
    {
      nlohmann::json elem = js[i];

      auto res = elem.find(par);

      if (res != elem.end())
      {
        std::string par_val = elem[par];
        parlist.push_back(par_val);
        m[par_val] = std::move(elem);
      }
    }

    std::sort(parlist.begin(), parlist.end());

    if (order == false)
    {
      std::reverse(parlist.begin(), parlist.end());
    }

    for (std::string item : parlist)
    {
      respond.push_back(m[item]);
    }

    return respond;
  }

  // method to sort JSON array with IOVs by the since value
  nlohmann::json CrestFsClient::sortIOVJson(nlohmann::json js, bool order)
  {
    std::string par = "since";
    nlohmann::json respond = nlohmann::json::array();
    std::vector<double> parlist;
    std::map<double, nlohmann::json> m;

    int size = js.size();
    for (int i = 0; i < size; i++)
    {
      nlohmann::json elem = js[i];

      auto res = elem.find(par);

      if (res != elem.end())
      {
        double par_val = elem[par];
        parlist.push_back(par_val);
        m[par_val] = std::move(elem);
      }
    }

    std::sort(parlist.begin(), parlist.end());

    if (order == false)
    {
      std::reverse(parlist.begin(), parlist.end());
    }

    for (double item : parlist)
    {
      respond.push_back(m[item]);
    }

    return respond;
  }

  bool CrestFsClient::isMatch(const std::string& word, long unsigned int n, const std::string& pattern, long unsigned int m)
  {
    if (m == pattern.size())
    {
      return n == word.size();
    }

    if (n == word.size())
    {
      for (long unsigned int i = m; i < pattern.size(); i++)
      {
        if (pattern[i] != '%')
        {
          return false;
        }
      }

      return true;
    }

    if (pattern[m] == '?' || pattern[m] == word[n])
    {
      return isMatch(word, n + 1, pattern, m + 1);
    }

    if (pattern[m] == '%')
    {
      return isMatch(word, n + 1, pattern, m) || isMatch(word, n, pattern, m + 1);
    }

    return false;
  }

  bool CrestFsClient::isMatch(const std::string& word, const std::string& pattern)
  {
    return isMatch(word, 0, pattern, 0);
  }

  std::string CrestFsClient::getCrestVersion()
  {
    throw CrestException(
      "ERROR in CrestFsClient::getCrestVersion: cannot get the CREST server version for file storage.");
  }

} // namespace Crest

