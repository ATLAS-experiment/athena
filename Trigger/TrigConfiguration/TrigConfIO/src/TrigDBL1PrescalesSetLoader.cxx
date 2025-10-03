// Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#include "./TrigDBHelper.h"
#include "TrigConfIO/TrigDBL1PrescalesSetLoader.h"

TrigConf::TrigDBL1PrescalesSetLoader::TrigDBL1PrescalesSetLoader(const std::string & connection) : 
   TrigDBLoader("TrigDBL1PrescalesSetLoader", connection)
{
   { // query for all schema versions
      auto & q = m_queries[1];
      // tables
      q.addToTableList ( "L1_PRESCALE_SET" );
      // bind vars
      q.extendBinding<int>("key");
      // conditions
      q.extendCondition("L1PS_ID = :key");
      // attributes
      q.extendOutput<coral::Blob>( "L1PS_DATA" );
      // the field with the data
      q.setDataName("L1PS_DATA");
   }
}

// Destructor defined here because QueryDefinition is an incomplete type in the header
TrigConf::TrigDBL1PrescalesSetLoader::~TrigDBL1PrescalesSetLoader() = default;

bool
TrigConf::TrigDBL1PrescalesSetLoader::loadL1Prescales ( unsigned int psk, TrigConf::L1PrescalesSet & pss,
                                                        const std::string & outFileName ) const
{
   // load data into ptree
   boost::property_tree::ptree pt;
   if(useCrest()) {
      loadFromCrest(psk, pt, outFileName, "L1 prescales", "L1PS");
   } else {
      loadFromOracle(psk, pt, outFileName, "L1 prescales", m_queries);
   }
    
   // fill L1PrescaleSet with data
   try {
      pss.setData(std::move(pt));
      pss.setPSK(psk);
   }
   catch(std::exception & ex) {
      pss.clear();
      TRG_MSG_ERROR("When reading L1 prescales for L1 PSK " << psk << " a parsing error occured ( " << ex.what() <<" )" );
      throw TrigConf::ParsingException("TrigDBL1PrescalesSetLoader: parsing error " + std::string(ex.what()));
   }

   return true;
}
