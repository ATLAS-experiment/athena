/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigConfIO/TrigDBLoader.h"

#include "./TrigDBHelper.h"

#include "CoralBase/Exception.h"
#include "CoralBase/Blob.h"

#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ISchema.h"

#include "CrestApi/CrestApiBase.h"
#include "CrestApi/CrestRequest.h"

#include "boost/property_tree/ptree.hpp"
#include <fstream>
#include <format>

using ptree = boost::property_tree::ptree;

TrigConf::TrigDBLoader::TrigDBLoader(const std::string & loaderName, const std::string & connection) : 
   TrigConfMessaging(loaderName),
   m_connection(connection)
{


}

// Destructor defined here because QueryDefinition is an incomplete type in the header
TrigConf::TrigDBLoader::~TrigDBLoader() = default;

size_t
TrigConf::TrigDBLoader::schemaVersion(coral::ISessionProxy* session) const {

   static const std::string versionTagPrefix("Trigger-Run3-Schema-v");

   // if database has no schema version, then we return 0
   if(! session->nominalSchema().existsTable("TRIGGER_SCHEMA") ) {
      throw std::runtime_error( "Trigger schema has no schema version table" );
   }
   
   TrigConf::QueryDefinition qdef;
   // tables
   qdef.addToTableList ( "TRIGGER_SCHEMA" );
   // attributes
   qdef.extendOutput<std::string>( "TS_TAG" );

   auto query = qdef.createQuery( session );
   auto & cursor = query->execute();
   if ( ! cursor.next() ) {
      throw std::runtime_error( "Trigger schema has schema version table but it is empty" );
   }

   const coral::AttributeList& row = cursor.currentRow();
   std::string versionTag = row["TS_TAG"].data<std::string>();
   if( ! versionTag.starts_with(versionTagPrefix)) {
      throw std::runtime_error(std::format("Tag format error: Trigger schema version tag {} does not start with {}", versionTag, versionTagPrefix));      
   }
   
   std::string vstr = versionTag.substr(versionTagPrefix.size()); // the part of the string containing the version
   size_t schemaVersion{0};
   try {
      schemaVersion = std::stoi(vstr);
   }
   catch (const std::invalid_argument& ia) {
      TRG_MSG_ERROR("Invalid argument when interpreting the version part " << vstr << " of schema tag " << versionTag << ". " << ia.what());
      throw;
   }

   TRG_MSG_INFO("TriggerDB schema version: " << schemaVersion);
   return schemaVersion;
}

void
TrigConf::TrigDBLoader::setCrestConnection(const std::string & server, const std::string & version) {
   // server
   m_crestServer = server;
   if(m_crestServer.ends_with('/')) { // remove trailing '/'
      m_crestServer.pop_back();
   }

   // use crest flag
   m_useCrest = ! m_crestServer.empty();

   // version
   if(version.empty()) {
      m_crestVersion = DEFAULT_CREST_API_VERSION; // defined in CrestApi/CrestApiBase.h
   } else {
      m_crestVersion = version;
   }
   if(!m_crestVersion.starts_with('/')) { // prepend '/' if not existent
      m_crestVersion = "/" + m_crestVersion;
   }
}

void
TrigConf::TrigDBLoader::setCrestTrigDB(const std::string & crestTrigDB) {
   m_crestTrigDb = crestTrigDB;
}

std::unique_ptr<coral::ISessionProxy>
TrigConf::TrigDBLoader::createDBSession() const {

   coral::ConnectionService connSvc;
   coral::IConnectionServiceConfiguration& csc = connSvc.configuration();
   csc.setConnectionRetrialPeriod( m_retrialPeriod );
   csc.setConnectionRetrialTimeOut( m_retrialTimeout );
   csc.setConnectionTimeOut( m_connectionTimeout );

   /* TODO
   if(csc.replicaSortingAlgorithm() == nullptr) { // likely to be standalone, create our own
      TRG_MSG_INFO("Create own ReplicaSortingAlgorithm");
      m_replicaSorter = new TrigConf::ReplicaSorter();
      csc.setReplicaSortingAlgorithm(*m_replicaSorter);
   }
   */

   TRG_MSG_INFO("Connecting to " << m_connection);

   auto proxy = std::unique_ptr<coral::ISessionProxy>( connSvc.connect(m_connection, coral::AccessMode::ReadOnly) );

   TRG_MSG_INFO("Opened session " << m_connection << " with  retrialPeriod/retrialTimeout/connectionTimeout: " 
                << m_retrialPeriod << "/" << m_retrialTimeout << "/" << m_connectionTimeout);

   return proxy;
}

std::string 
TrigConf::TrigDBLoader::getTrigDataCrest(const std::string & type, int key) const {
   /*
   To get the trigger data from the TriggerDB using CrestApi, it has been agreed to 
   use the API's payload query with a special specifier

   triggerdb://<TrigDBSpec>/<TypeSpec>/<DBKey>

   Possible TrigDBSpec: CONF_DATA_RUN3, CONF_MC_RUN3, CONF_REPR_RUN3
   Possible TypeSpec: L1PS, HLTM, L1M, HLTPS, BGS, MGS, JO
   */

   std::string url = m_crestServer + m_crestVersion;
   std::string query = std::format("triggerdb://{}/{}/{}", m_crestTrigDb, type, key);

#if 0
   /*
   the final implementation should be the code below
   however, in the version used by 24.0 the function CrestApi.getPayload(hash) does not work
   for the trigger: it includes a hash validation that is not applicable for the trigger
   but only for conditions payload queries by hash
   */ 
   Crest::CrestApi capi = Crest::CrestApi(url);
   std::string payload = capi.getPayload(query);
#else
   /*
   so for release 24.0 and until the access to the trigger payload is implemented in CrestApi
   the CrestRequest is build manually. Luckily all needed functionality is accessible
   (this code is a copy of CrestApi::getPayload() without the checkHash())
   */
   Crest::CrestRequest request = Crest::CrestRequest();
   request.setUrl(url);
   std::string current_path = "payloads/data?format=BLOB&hash=" + query;
   nlohmann::json js = nullptr;
   std::string payload = request.performRequest(current_path, Crest::Action::GET, js, "TrigDbLoader");
#endif

   return payload;
}

TrigConf::QueryDefinition
TrigConf::TrigDBLoader::getQueryDefinition(size_t schemaVersion,
                                           const std::map<size_t, TrigConf::QueryDefinition> & queries) const
{
   // find the largest version key in the map of defined queries that is <= the schemaVersion
   size_t maxDefVersion = 0;
   for(auto & entry : queries) {
      size_t vkey = entry.first;
      if(vkey>maxDefVersion and vkey<=schemaVersion) {
         maxDefVersion = vkey;
      }
   }
   // if nothing found, throw an error
   if( maxDefVersion==0 ) {
      TRG_MSG_ERROR("No query for schema version " << schemaVersion << " defined" );
      throw TrigConf::NoQueryException(std::format("No query available for schema version {}", schemaVersion));
   }
   return queries.at(maxDefVersion);
}

void 
TrigConf::TrigDBLoader::loadFromCrest(unsigned int key, boost::property_tree::ptree & pt, 
                                      const std::string & outFileName, const std::string & description,
                                      const std::string & query_type) const
{
   std::string payload;
   try {
      payload = getTrigDataCrest(query_type, key);
   }
   catch(Crest::CrestException & ex) {
      TRG_MSG_ERROR("When reading " << description << " for key " << key << " from crest a CrestException was caught ( " << ex.what() <<" )" );
      throw TrigConf::CrestLoadingException(std::format("{}: {}", getName(), ex.what()));
   }
   if(!outFileName.empty()) {
      writeRawFile(payload, outFileName);
      TRG_MSG_INFO("Wrote file " << outFileName);
   }
   try {
      stringToPtree(payload, pt);
   }
   catch(boost::property_tree::json_parser_error & ex) {
      TRG_MSG_ERROR("When reading " << description << " for key " << key << " from crest a ptree json parser error was caught ( " << ex.what() <<" )" );
      throw TrigConf::JsonParsingException(std::format("{}: {}", getName(), ex.what()));
   }
}

void 
TrigConf::TrigDBLoader:: loadFromOracle(unsigned int key, boost::property_tree::ptree & pt, 
                                        const std::string & outFileName, const std::string & description, 
                                        const std::map<size_t, QueryDefinition> & queries) const
{
   auto session = createDBSession();
   session->transaction().start( /*bool readonly=*/ true);
   const size_t sv = schemaVersion(session.get());
   QueryDefinition qdef = getQueryDefinition(sv, queries);
   try {
      qdef.setBoundValue<int>("key", key);
      auto q = qdef.createQuery( session.get() );
      auto & cursor = q->execute();
      if ( ! cursor.next() ) {
         TRG_MSG_ERROR("Tried reading " << description << ", but key " << key << " is not available" );
         throw TrigConf::NoKeyException(std::format("{}: key {} not available", getName(), key));
      }
      const coral::AttributeList& row = cursor.currentRow();
      const coral::Blob& dataBlob = row[qdef.dataName()].data<coral::Blob>();

      if(!outFileName.empty()) {
         writeRawFile( dataBlob, outFileName );
         TRG_MSG_INFO("Wrote file " << outFileName);
      }
      blobToPtree( dataBlob, pt );
   }
   catch(coral::QueryException & ex) {
      TRG_MSG_ERROR("When reading " << description << " for key " << key << " a coral::QueryException was caught ( " << ex.what() <<" )" );
      throw TrigConf::QueryException(std::format("{}: {}", getName(), ex.what()));
   }
   catch(boost::property_tree::json_parser_error & ex) {
      TRG_MSG_ERROR("When reading " << description << " for key " << key << " a ptree json parser error was caught ( " << ex.what() <<" )" );
      throw TrigConf::JsonParsingException(std::format("{}: {}", getName(), ex.what()));
   }
}