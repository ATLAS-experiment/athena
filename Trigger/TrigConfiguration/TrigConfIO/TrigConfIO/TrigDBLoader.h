/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TrigConfIO/TrigDBLoader.h
 * @author J. Stelzer
 * @date Sep 2019
 * @brief Loader class for Trigger configuration from the Trigger DB
 */

#ifndef TRIGCONFIO_TRIGDBLOADER_H
#define TRIGCONFIO_TRIGDBLOADER_H

#include "TrigConfBase/TrigConfMessaging.h"
#include "TrigConfIO/Exceptions.h"

#include <memory>
#include <map>

#include "boost/property_tree/ptree_fwd.hpp"


// namespace boost { namespace property_tree {
//    class ptree;
// }}
namespace coral {
   class ISessionProxy;
   class Blob;
}

namespace TrigConf {

   class QueryDefinition;

   /**
    * @brief Loader of trigger configurations from the Trigger database
    */
   class TrigDBLoader : public TrigConfMessaging {
   public:

      /** Constructor */
      TrigDBLoader(const std::string & loaderName, const std::string & connection);

      /** Destructor - cannot be defined here because QueryDefinition is an incomplete type */
      virtual ~TrigDBLoader();

      /** @brief declare CREST as the source of the configuration
       * An empty crest server makes it use Oracle
       * @param server The crest server. An empty string disables Crest and enables Oracle (the default)
       * @param version The version of the crest api. Usually not needed. If not given, it defaults to the default API version (see CrestApi/CrestApiBase.h)
       */
      void setCrestConnection(const std::string & server, const std::string & version = "");

      /** @brief set trigger db for the crest connection
       * @param crestTrigDB the source trigger DB. Possible values currently
       *   - CONF_DATA_RUN3 => ATLAS_CONF_TRIGGER_RUN3
       *   - CONF_MC_RUN3   => ATLAS_CONF_TRIGGER_MC_RUN3
       *   - CONF_REPR_RUN3 => ATLAS_CONF_TRIGGER_REPR_RUN3
       */
      void setCrestTrigDB(const std::string & crestTrigDB);

      /** @brief access to TriggerDB schema version
         @return version of the DB schema (0 - no version, >0 - schema version)
       */
      size_t schemaVersion(coral::ISessionProxy* session) const;

      void setLevel(MSGTC::Level lvl) { msg().setLevel(lvl); }

      MSGTC::Level outputLevel() const { return msg().level(); }

   protected:

      bool useCrest() const {
         return m_useCrest;
      }

      /** @brief Get trigger configuration from the TriggerDB through Crest
      @param type The type of trigger configuration data to access
      - L1PS => L1 prescale (with L1PS key)
      - HLTPS => HLT prescale (with HLTPS key)
      - L1M => L1 menu (with SMK)
      - HLTM => HLT menu (with SMK)
      - JO => Job options (with SMK)
      - BGS => bunch group set (with BGS key)
      - MGS => monitoring group (with SMK)
      @param key The trigger key
       */
      std::string getTrigDataCrest(const std::string & type, int key) const;

      /** @brief create (if needed) DB session and return the session proxy */
      std::unique_ptr<coral::ISessionProxy> createDBSession() const;

      /** @brief return query for given schemaVersion from possible queries */
      QueryDefinition getQueryDefinition(size_t schemaVersion,
                                         const std::map<size_t, QueryDefinition> & queries) const;

      void loadFromCrest(unsigned int key, boost::property_tree::ptree & pt,
                         const std::string & outFileName, const std::string & description,
                         const std::string & query_type) const;

      void loadFromOracle(unsigned int key, boost::property_tree::ptree & pt,
                          const std::string & outFileName, const std::string & description, 
                          const std::map<size_t, QueryDefinition> & queries) const;

   private:

      // private variables
      bool           m_useCrest {false};
      std::string    m_connection {""};
      std::string    m_crestServer {""};
      std::string    m_crestVersion {""};
      std::string    m_crestTrigDb {""};
      int            m_retrialPeriod {0};
      int            m_retrialTimeout {0};
      int            m_connectionTimeout {0};
   };

}

#endif

