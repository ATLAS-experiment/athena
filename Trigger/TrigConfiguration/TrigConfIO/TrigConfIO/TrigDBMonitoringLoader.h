/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @brief Loader class for HLT monitoring configuration from the Trigger DB
 */

#ifndef TRIGCONFIO_TRIGDBMONITORINGLOADER_H
#define TRIGCONFIO_TRIGDBMONITORINGLOADER_H

#include "TrigConfIO/TrigDBLoader.h"

#include "boost/property_tree/ptree.hpp"

#include "TrigConfData/HLTMonitoring.h"

namespace TrigConf {

   class QueryDefinition;

   /**
    * @brief Loader of trigger menu configurations from the database
    */
   class TrigDBMonitoringLoader : public TrigConf::TrigDBLoader {
   public:

      /** Constructor */
      TrigDBMonitoringLoader(const std::string & connection);

      /** Destructor - cannot be defined here because QueryDefinition is an incomplete type */
      virtual ~TrigDBMonitoringLoader() override;

      /**
       * @brief Load HLT menu content from the Trigger DB into two ptrees for a given SuperMasterKey (SMK)
       * @param smk [in] the SMK that should be loaded
       * @param hltmonitoring [out] the loaded HLT menu
       * @param outFileName [in] name of file to write out the loaded data (optional, by default no file will be written)
       * @return true if loading (and optional writing) was successfull
       */
      bool loadHLTMonitoring ( unsigned int smk,
                               boost::property_tree::ptree & hltmonitoring,
                               const std::string & outFileName = "") const;

      /**
       * @brief Load content from the Trigger DB into an HLTMenu for a given SuperMasterKey (SMK)
       * @param smk [in] the SMK that should be loaded
       * @param hltmonitoring [out] the loaded HLT menu
       * @param outFileName [in] name of file to write out the loaded data (optional, by default no file will be written)
       * @return true if loading (and optional writing) was successfull
       */
      bool loadHLTMonitoring ( unsigned int smk,
                               HLTMonitoring & hltmonitoring,
                               const std::string & outFileName = "") const;
   private:
      std::map<size_t, QueryDefinition> m_queries;
   };

}

#endif

