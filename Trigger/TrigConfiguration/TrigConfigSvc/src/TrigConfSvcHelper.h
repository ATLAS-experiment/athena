/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigConfigSvc_TrigConfSvcHelper
#define TrigConfigSvc_TrigConfSvcHelper

#include <string>

namespace TrigConf {

   /**
   * @brief Function to interpret the trigger connection string for CREST connections
   * Format of the connections string:
   *  - `TRIGGERDB` for connections using CORAL
   *  - `https://crest.cern.ch/api-v5.0//CONF_DATA_RUN3` for CREST connections
   *
   * @param db_connection_string The database connection string
   * @param crest_server The server address (including protocol such as https://)
   * @param crest_api The API version
   * @param dbname The database name
   * @return true if the connection is a CREST connection
   */
   bool isCrestConnection(const std::string& db_connection_string, 
   std::string& crest_server, std::string& crest_api, std::string& dbname);

}

#endif