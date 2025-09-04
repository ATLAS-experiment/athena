// Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#include "./TrigDBHelper.h"
#include "TrigConfIO/TrigDBHLTPrescalesSetLoader.h"

TrigConf::TrigDBHLTPrescalesSetLoader::TrigDBHLTPrescalesSetLoader(const std::string & connection) : 
   TrigDBLoader("TrigDBHLTPrescalesSetLoader", connection)
{
   { // query for all schema versions
      auto & q = m_queries[1];
      // tables
      q.addToTableList ( "HLT_PRESCALE_SET" );
      // bind vars
      q.extendBinding<int>("key");
      // conditions
      q.extendCondition("HPS_ID = :key");
      // attributes
      q.extendOutput<coral::Blob>( "HPS_DATA" );
      // the field with the data
      q.setDataName("HPS_DATA");
   }
}

// Destructor defined here because QueryDefinition is an incomplete type in the header
TrigConf::TrigDBHLTPrescalesSetLoader::~TrigDBHLTPrescalesSetLoader() = default;

bool
TrigConf::TrigDBHLTPrescalesSetLoader::loadHLTPrescales ( unsigned int psk, TrigConf::HLTPrescalesSet & pss,
                                                          const std::string & outFileName ) const
{
   // load data into ptree
   boost::property_tree::ptree pt;
   if(useCrest()) {
      loadFromCrest(psk, pt, outFileName, "HLT prescales", "HLTPS");
   } else {
      loadFromOracle(psk, pt, outFileName, "HLT prescales", m_queries);
   }
    
   // fill HLTPrescaleSet with data
   try {
      pss.setData(std::move(pt));
      pss.setPSK(psk);
   }
   catch(std::exception & ex) {
      pss.clear();
      TRG_MSG_ERROR("When reading HLT prescales for HLT PSK " << psk << " a parsing error occured ( " << ex.what() <<" )" );
      throw TrigConf::ParsingException("TrigDBHLTPrescalesSetLoader: parsing error " + std::string(ex.what()));
   }
   return true;
}
