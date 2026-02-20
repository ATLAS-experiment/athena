#include "TrigConfSvcHelper.h"

#include <format>
#include <vector>
#include <sstream>

namespace TrigConf {

bool isCrestConnection(const std::string& db_connection_string, 
                       std::string& crest_server, std::string& crest_api, std::string& dbname) {
   // Implementation of the function
   if(!db_connection_string.starts_with("http")) {
      return false;
   }

   std::string url = db_connection_string;
   std::size_t protocol_end = url.find("://");
   std::string protocol;

   // --- 1. Extract protocol ---
   if (protocol_end != std::string::npos) {
      protocol = url.substr(0, protocol_end);
   } else {
      protocol = ""; // no protocol given
      protocol_end = -3; // so that host_start = 0 below
   }

   // --- 2. Extract host ---
   std::size_t host_start = protocol_end + 3;
   std::size_t host_end = url.find('/', host_start);
   std::string host = (host_end == std::string::npos) ? \
      url.substr(host_start) : url.substr(host_start, host_end - host_start);
   // server is protocol + host
   crest_server = std::format("{}://{}", protocol, host);

   // --- 3. Extract path ---
   std::string path = (host_end != std::string::npos) ? url.substr(host_end) : "";

   // --- 4. Remove trailing slashes ---
   while (!path.empty() && path.back() == '/')
      path.pop_back();

   // --- 5. Split path into non-empty parts ---
   std::vector<std::string> path_parts;
   std::stringstream ss(path);
   std::string segment;

   while (std::getline(ss, segment, '/')) {
      if (!segment.empty()) {
         path_parts.push_back(segment);
      }
   }
   if(path_parts.empty()) {
      throw std::runtime_error("TrigConfJobOptionsSvc: crest connection '" + db_connection_string + "' is missing the database name.");
   }
   crest_api = "";
   dbname = "";
   if(path_parts.size()==1) {
      dbname = path_parts[0];
   } else if(path_parts.size()==2) {
      crest_api = path_parts[0];
      dbname = path_parts[1];
   }
   return true;
}

} // namespace TrigConf