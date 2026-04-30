/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TrigConfSvcHelper.h"

#include <format>
#include <vector>
#include <sstream>
#include <stdexcept>

namespace TrigConf {

bool isCrestConnection(const std::string& db_connection_string,
                       std::string& crest_server,
                       std::string& crest_api,
                       std::string& dbname)
{
   if (!db_connection_string.starts_with("http")) {
      return false;
   }

   const std::string& url = db_connection_string;
   const std::size_t protocol_end = url.find("://");

   std::string protocol;
   std::size_t host_start = 0;

   if (protocol_end != std::string::npos) {
      protocol = url.substr(0, protocol_end);
      host_start = protocol_end + 3;
   }

   const std::size_t host_end = url.find('/', host_start);
   const std::string host =
      (host_end == std::string::npos)
         ? url.substr(host_start)
         : url.substr(host_start, host_end - host_start);

   crest_server = protocol.empty()
      ? host
      : std::format("{}://{}", protocol, host);

   std::string path =
      (host_end != std::string::npos) ? url.substr(host_end) : "";

   while (!path.empty() && path.back() == '/') {
      path.pop_back();
   }

   std::vector<std::string> path_parts;
   std::stringstream ss(path);
   std::string segment;

   while (std::getline(ss, segment, '/')) {
      if (!segment.empty()) {
         path_parts.push_back(segment);
      }
   }

   if (path_parts.empty()) {
      throw std::runtime_error(
         "TrigConfJobOptionsSvc: crest connection '" + db_connection_string +
         "' is missing the database name.");
   }

   crest_api.clear();
   dbname.clear();

   if (path_parts.size() == 1) {
      dbname = path_parts[0];
   }
   else if (path_parts.size() == 2) {
      crest_api = path_parts[0];
      dbname = path_parts[1];
   }

   return true;
}

} // namespace TrigConf