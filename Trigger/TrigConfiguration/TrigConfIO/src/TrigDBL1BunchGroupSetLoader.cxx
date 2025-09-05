// Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#include "./TrigDBHelper.h"
#include "TrigConfIO/TrigDBL1BunchGroupSetLoader.h"

TrigConf::TrigDBL1BunchGroupSetLoader::TrigDBL1BunchGroupSetLoader(const std::string & connection) : 
   TrigDBLoader("TrigDBL1BunchGroupSetLoader", connection)
{
   { // query for all schema versions
      auto & q = m_queries[1];
      // tables
      q.addToTableList ( "L1_BUNCH_GROUP_SET" );
      // bind vars
      q.extendBinding<int>("key");
      // conditions
      q.extendCondition("L1BGS_ID = :key");
      // attributes
      q.extendOutput<coral::Blob>( "L1BGS_DATA" );
      // the field with the data
      q.setDataName("L1BGS_DATA");
   }
}

// Destructor defined here because QueryDefinition is an incomplete type in the header
TrigConf::TrigDBL1BunchGroupSetLoader::~TrigDBL1BunchGroupSetLoader() = default;

bool
TrigConf::TrigDBL1BunchGroupSetLoader::loadBunchGroupSet ( unsigned int bgsk,
                                                           TrigConf::L1BunchGroupSet & bgs,
                                                           const std::string & outFileName ) const
{
   // load data into ptree
   boost::property_tree::ptree pt;
   if(useCrest()) {
      loadFromCrest(bgsk, pt, outFileName, "L1 bunchgroups", "BGS");
   } else {
      loadFromOracle(bgsk, pt, outFileName, "L1 bunchgroups", m_queries);
   }
    
   // fill L1BunchGroupSet with data
   try {
      bgs.setData(std::move(pt));
      bgs.setBGSK(bgsk);
   }
   catch(std::exception & ex) {
      bgs.clear();
      TRG_MSG_ERROR("When reading L1 bunchgroup set for L1 BGSK " << bgsk << " a parsing error occured ( " << ex.what() <<" )" );
      throw TrigConf::ParsingException("TrigDBL1BunchGroupSetLoader: parsing error " + std::string(ex.what()));
   }

   return true;
}
