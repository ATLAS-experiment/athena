/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PathResolver/PathResolver.h"
#include "CxxUtils/checker_macros.h"

#include <cstdlib>
#include <format>
#include <fstream>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <memory>
#include <mutex>

#include <curl/curl.h>

#include "TFile.h"
#include "TSystem.h"


namespace fs = std::filesystem;

namespace {
  const char path_separator = ':'; // Linux and MacOS
  const char* const pathResolverEnvVar = "PATHRESOLVER_DEVAREARESPONSE";

  // Callback function to write received data into an ofstream
  size_t write_data(void* ptr, size_t size, size_t nmemb, void* userdata) {
    std::ostream* stream = static_cast<std::ostream*>(userdata);
    size_t total_size = size * nmemb;
    stream->write(static_cast<char*>(ptr), total_size);
    return total_size;
  }
  bool download_file(const std::string& url, const std::string& output_path,
    asg::AsgMessaging& asgmsg)
  {

    // We intentionally skip curl_global_cleanup to avoid races in
    // multithreaded use. This leaks a small amount of global state,
    // but the OS reclaims it at process exit.
    static std::once_flag curl_setup;
    std::call_once(curl_setup, curl_global_init, CURL_GLOBAL_DEFAULT);

    using owner_t = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;
    owner_t curl_owner(curl_easy_init(), curl_easy_cleanup);

    auto* curl = curl_owner.get();

    if (!curl) {
      asgmsg.msg(MSG::WARNING) << "unable to setup curl" << endmsg;
      return false;
    }

    std::ofstream file(output_path, std::ios::binary);
    if (!file.is_open()) {
      asgmsg.msg(MSG::WARNING) << "unable to open " << output_path << endmsg;
      return false;
    }
    auto setCurlOption =[curl](auto option, const auto &value)->bool{
      CURLcode ret = curl_easy_setopt(curl, option, value);
      return (ret == CURLE_OK);
    };
    bool setupOk = setCurlOption(CURLOPT_TIMEOUT, 60L);
    setupOk &= setCurlOption(CURLOPT_URL, url.c_str());
    setupOk &= setCurlOption(CURLOPT_WRITEFUNCTION, write_data);
    setupOk &= setCurlOption(CURLOPT_WRITEDATA, &file);
    // Optional: follow redirects
    setupOk &= setCurlOption(CURLOPT_FOLLOWLOCATION, 1L);
    if (not setupOk) {
      asgmsg.msg(MSG::WARNING) << "curl setup failed in PathResolver." <<endmsg;
      return false;
    }
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      asgmsg.msg(MSG::WARNING) << "error downloading file: "
                             << curl_easy_strerror(res)
                             << " (" << res << ")" << endmsg;
      return false;
    }

    return true;
  }

  /// Check if a file from "dev/" is loaded and warn/throw if requested
  void checkForDev(asg::AsgMessaging& asgmsg,
                   const std::string& logical_file_name) {

    asgmsg.msg(MSG::DEBUG) << "Trying to locate " << logical_file_name << endmsg;

    if (logical_file_name.starts_with("dev/")) {
      const char* env = std::getenv(pathResolverEnvVar);
      const std::string dev_area_response = env ? env : "DEFAULT";

      MSG::Level level{};
      if (dev_area_response == "SILENT") {
        return;
      }
      else if (dev_area_response == "THROW") {
        throw std::runtime_error(
          "Loading dev area file " + logical_file_name + " is not allowed! "
          "To override this error set the environment variable " +
          pathResolverEnvVar + " to SILENT, DEFAULT, INFO, WARNING or ERROR");
      }
      else if (dev_area_response == "DEFAULT") {
        #ifdef XAOD_ANALYSIS
          level = MSG::WARNING;
        #else
          level = MSG::ERROR;
        #endif
      }
      else if (dev_area_response == "INFO")    level = MSG::INFO;
      else if (dev_area_response == "WARNING") level = MSG::WARNING;
      else if (dev_area_response == "ERROR")   level = MSG::ERROR;
      else {
        throw std::runtime_error(std::format("{} set to '{}', not sure what to do. "
                                             "Options are DEFAULT, THROW, INFO, WARNING, ERROR or SILENT",
                                             pathResolverEnvVar, dev_area_response));
      }

      // Print message at appropriate level
      asgmsg.msg(level) << "Locating dev file " << logical_file_name << ". Do not let this propagate to a release!" << endmsg;
    }
  }
}


asg::AsgMessaging& PathResolver::asgMsg() {
#ifdef XAOD_STANDALONE
   static thread_local asg::AsgMessaging asgMsg("PathResolver");
#else
   static asg::AsgMessaging asgMsg ATLAS_THREAD_SAFE ("PathResolver");
#endif
/// In AnalysisBase this method is not available
#ifndef XAOD_ANALYSIS
   asgMsg.setLevel(m_level);   
#else
   asgMsg.msg().setLevel(m_level);
#endif
   return asgMsg;
}


/**
 * Main private search method used by all public methods.
 */
bool PathResolver::PR_find( const std::string& logical_file_name, const std::string& search_list,
                            fs::file_type file_type, std::string& result ) {

  // expand filename before finding
  TString tmpString(logical_file_name);
  gSystem->ExpandPathName(tmpString);

  fs::path file(tmpString.Data());
  fs::path locationToDownloadTo = "."; // will replace with first search location

  // First always search for filename as given in local directory
  const std::string searchPath = std::format("./{}{}", path_separator, search_list);

  // iterate through search list
  for (const auto r : searchPath | std::views::split(path_separator)) {
    std::string_view path(r.begin(), r.end());
    const bool is_http = path.starts_with("http//");
    if( (is_http || path.starts_with("https//")) &&
        file_type==fs::file_type::regular && std::getenv("PATHRESOLVER_ALLOWHTTPDOWNLOAD") ) { // only http download files, not directories

      // Try to do an http download to the local location.
      // Need to restore the proper http protocol (cannot use ":" in search paths)
      const std::string fileToDownload = std::format("{}://{}/{}", is_http ? "http" : "https",
                                                     path.substr(6), file.string());

      const fs::path targetPath = locationToDownloadTo / file;
      fs::path targetDir = targetPath;
      targetDir.remove_filename();
      msg(MSG::DEBUG) << "Attempting http download of " << fileToDownload << " to " << targetDir << endmsg;

      if (!is_directory(targetDir)) {
        msg(MSG::DEBUG) << "Creating directory " << targetDir  << endmsg;
        if(!fs::create_directories(targetDir)) {
          msg(MSG::ERROR) << "Unable to create directories to write file to " << targetDir << endmsg;
          return false;
        }
      }

      if (!download_file(fileToDownload, targetPath, asgMsg())) {
        msg(MSG::WARNING) << "Unable to download file " << fileToDownload << endmsg;
      } else {
        msg(MSG::DEBUG) << "Successfully downloaded " << fileToDownload << endmsg;
        result = targetPath;
        return true;
      }

    } else if (locationToDownloadTo==".") {
      // Prefer first non-pwd location (usually local build area) for downloading to.
      fs::path dummyFile = fs::path(path) / "._pathresolver_dummy";
      std::ofstream ofs(dummyFile);   // check if writable
      if (ofs.is_open()) {
        locationToDownloadTo = path;
        ofs.close();
        fs::remove(dummyFile);
      }
    }

    fs::path fp = path / file;
    try {
      if (fs::status(fp).type() == file_type) {
        result = fs::absolute(fp).string();
        return true;
      }
    } catch (const fs::filesystem_error&) {
      // file not accessible or does not exist
    }

  }

  return false; // not found
}


std::string PathResolver::find_file(const std::string& logical_file_name,
                                    const std::string& search_path) {

#ifndef XAOD_ANALYSIS
  if (logical_file_name.starts_with('/')) {
    msg(MSG::ERROR) << "Use of an absolute file name: " << logical_file_name << endmsg;
  }
#endif

  const char* path_list = std::getenv(search_path.c_str());
  if (path_list == nullptr) {
    msg(MSG::ERROR) << search_path << " environment variable not defined!" << endmsg;
    return {};
  }

  return find_file_from_list(logical_file_name, path_list);
}


std::string PathResolver::find_file_from_list (const std::string& logical_file_name,
                                               const std::string& search_list)
{
  std::string result;
  PR_find (logical_file_name, search_list, fs::file_type::regular, result);

  return result;
}


std::string PathResolver::find_directory (const std::string& logical_file_name,
                                          const std::string& search_path)
{
  const char* path_list = std::getenv(search_path.c_str());
  if(path_list == nullptr) {
    msg(MSG::ERROR) << search_path  << " environment variable not defined!" << endmsg;
    return {};
  }

  return find_directory_from_list(logical_file_name, path_list);
}


std::string PathResolver::find_directory_from_list (const std::string& logical_file_name,
                                                    const std::string& search_list)
{
  std::string result;
  PR_find(logical_file_name, search_list, fs::file_type::directory, result);

  return result;
}


std::string PathResolver::find_calib_file (const std::string& logical_file_name)
{
  checkForDev(asgMsg(), logical_file_name);

  if (logical_file_name.starts_with("root://")) {
    //xrootd access .. try to open file ...
    std::unique_ptr<TFile> fTmp{TFile::Open(logical_file_name.c_str())};
    if (!fTmp || fTmp->IsZombie()) {
      msg(MSG::WARNING) << "Could not open " << logical_file_name << endmsg;
      return {};
    }
    return logical_file_name;
  }

  std::string out = PathResolver::find_file (logical_file_name, "CALIBPATH");
  if (out.empty()) {
    msg(MSG::WARNING) << "Could not locate " << logical_file_name << endmsg;
  }
  return out;
}


std::string PathResolver::find_calib_directory (const std::string& logical_file_name)
{
  checkForDev(asgMsg(), logical_file_name);

  std::string out = PathResolver::find_directory (logical_file_name, "CALIBPATH");
  if (out.empty()) {
    msg(MSG::WARNING) << "Could not locate " << logical_file_name << endmsg;
  }
  return out;
}


void PathResolver::setOutputLevel(MSG::Level level) {
   m_level = level;
}

std::string PathResolverFindXMLFile (const std::string& logical_file_name)
{
  return PathResolver::find_file (logical_file_name, "XMLPATH");
}

std::string PathResolverFindDataFile (const std::string& logical_file_name)
{
  return PathResolver::find_file (logical_file_name, "DATAPATH");
}

std::string PathResolverFindCalibFile (const std::string& logical_file_name) {
  return PathResolver::find_calib_file(logical_file_name);
}


std::string PathResolverFindCalibDirectory (const std::string& logical_file_name) {
  return PathResolver::find_calib_directory(logical_file_name);
}


void PathResolverSetOutputLevel(int lvl) {
  PathResolver::setOutputLevel(MSG::Level(lvl));
}
