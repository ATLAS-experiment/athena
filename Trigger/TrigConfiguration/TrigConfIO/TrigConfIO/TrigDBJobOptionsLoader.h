/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TrigConfIO/TrigDBJobOptionsLoader.h
 * @author J. Stelzer
 * @date Sep 2019
 * @brief Loader class for Trigger configuration from the Trigger DB
 */

#ifndef TRIGCONFIO_TRIGDBJOBOPTIONSLOADER_H
#define TRIGCONFIO_TRIGDBJOBOPTIONSLOADER_H

#include "TrigConfData/DataStructure.h"

#include "boost/property_tree/ptree.hpp"

#include "TrigConfIO/TrigDBLoader.h"

#include <map>

namespace TrigConf {

   /**
    * @brief Loader of trigger configurations from Json files
    */
   class TrigDBJobOptionsLoader : public TrigDBLoader {
   public:

      /** Constructor */
      TrigDBJobOptionsLoader(const std::string & connection);

      /** Destructor - cannot be defined here because QueryDefinition is an incomplete type */
      virtual ~TrigDBJobOptionsLoader() override;

      /**
       * @brief Load job options from the Trigger DB into a ptree for a given SuperMasterKey (SMK)
       * @param smk [in] the SMK that should be loaded
       * @param jobOptions [out] the loaded job options
       * @param outFileName [in] name of file to write out the loaded data (optional, by default no file will be written)
       * @return true if loading (and optional writing) was successfull
       */
      bool loadJobOptions ( unsigned int smk,
                            boost::property_tree::ptree & jobOptions,
                            const std::string & outFileName = "") const;

      /**
       * @brief Load job options from the Trigger DB into a ptree for a given SuperMasterKey (SMK)
       * @param smk [in] the SMK that should be loaded
       * @param jobOptions [out] the loaded job options
       * @param outFileName [in] name of file to write out the loaded data (optional, by default no file will be written)
       * @return true if loading (and optional writing) was successfull
       */
      bool loadJobOptions ( unsigned int smk,
                            DataStructure & jobOptions,
                            const std::string & outFileName = "") const;
   private:

      std::map<size_t, QueryDefinition> m_queries;
   };

}

#endif

