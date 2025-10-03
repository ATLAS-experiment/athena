/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "./TrigDBHelper.h"
#include "TrigConfIO/TrigDBJobOptionsLoader.h"

TrigConf::TrigDBJobOptionsLoader::TrigDBJobOptionsLoader(const std::string & connection) : 
   TrigDBLoader("TrigDBJobOptionsLoader", connection)
{
   { // query for schema version 1
      auto & q = m_queries[1];
      // tables
      q.addToTableList ( "SUPER_MASTER_TABLE", "SMT" );
      q.addToTableList ( "JO_MASTER_TABLE", "JOMT" );
      // bind vars
      q.extendBinding<int>("key");
      // conditions
      q.extendCondition("SMT.SMT_ID = :key");
      q.extendCondition(" AND SMT.SMT_JO_MASTER_TABLE_ID = JOMT.JO_ID");
      // attributes
      q.extendOutput<std::string>( "SMT.SMT_NAME" );
      q.extendOutput<int>        ( "SMT.SMT_JO_MASTER_TABLE_ID" );
      q.extendOutput<coral::Blob>( "JOMT.JO_CONTENT" );
      // the field with the data
      q.setDataName("JOMT.JO_CONTENT");
   }
   { // query for schema version 2
      auto & q = m_queries[2];
      // tables
      q.addToTableList ( "SUPER_MASTER_TABLE", "SMT" );
      q.addToTableList ( "HLT_JOBOPTIONS", "HJO" );
      // bind vars
      q.extendBinding<int>("key");
      // conditions
      q.extendCondition("SMT.SMT_ID = :key");
      q.extendCondition("AND HJO.HJO_ID=SMT.SMT_HLT_JOBOPTIONS_ID");
      // attributes
      q.extendOutput<std::string>( "SMT.SMT_NAME" );
      q.extendOutput<int>        ( "SMT.SMT_HLT_JOBOPTIONS_ID" );
      q.extendOutput<coral::Blob>( "HJO.HJO_DATA" );
      // the field with the data
      q.setDataName("HJO.HJO_DATA");
   }
}

// Destructor defined here because QueryDefinition is an incomplete type in the header
TrigConf::TrigDBJobOptionsLoader::~TrigDBJobOptionsLoader() = default;

bool
TrigConf::TrigDBJobOptionsLoader::loadJobOptions ( unsigned int smk,
                                                   boost::property_tree::ptree & jobOptions,
                                                   const std::string & outFileName ) const
{
    // load data into ptree
   if(useCrest()) {
      loadFromCrest(smk, jobOptions, outFileName, "HLT job options", "JO");
   } else {
      loadFromOracle(smk, jobOptions, outFileName, "HLT job options", m_queries);
   }
   return true;
}

bool
TrigConf::TrigDBJobOptionsLoader::loadJobOptions ( unsigned int smk,
                                                   DataStructure & jobOptions,
                                                   const std::string & outFileName ) const
{

   boost::property_tree::ptree ptJobOptions;
   loadJobOptions( smk, ptJobOptions, outFileName );
   try {
      jobOptions.setData(std::move(ptJobOptions));
   }
   catch(std::exception & ex) {
      jobOptions.clear();
      TRG_MSG_ERROR("When reading HLT job options for SMK " << smk << " a parsing error occured ( " << ex.what() <<" )" );
      throw TrigConf::ParsingException("TrigDBJobOptionsLoader: parsing error " + std::string(ex.what()));
   }
   return true;
}

