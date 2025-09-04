// Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#include "./TrigDBHelper.h"
#include "TrigConfIO/TrigDBMenuLoader.h"

TrigConf::TrigDBMenuLoader::TrigDBMenuLoader(const std::string & connection) : 
   TrigDBLoader("TrigDBMenuLoader", connection)
{
   /*
     L1 menu queries
   */
   { // for schema version 1
      auto & q = m_l1queries[1];
      // tables
      q.addToTableList ( "SUPER_MASTER_TABLE", "SMT" );
      q.addToTableList ( "L1_MASTER_TABLE", "L1MT" );
      // bind vars
      q.extendBinding<int>("key");
      // conditions
      q.extendCondition("SMT.SMT_ID = :key");
      q.extendCondition(" AND SMT.SMT_L1_MASTER_TABLE_ID = L1MT.L1MT_ID");
      // attributes
      q.extendOutput<std::string>( "SMT.SMT_NAME" );
      q.extendOutput<int>        ( "SMT.SMT_L1_MASTER_TABLE_ID" );
      q.extendOutput<coral::Blob>( "L1MT.L1MT_MENU" );
      // the field with the data
      q.setDataName("L1MT.L1MT_MENU");
   }
   { // for schema version 2
      auto & q = m_l1queries[2];
      // tables
      q.addToTableList ( "SUPER_MASTER_TABLE", "SMT" );
      q.addToTableList ( "L1_MENU", "L1TM" );
      // bind vars
      q.extendBinding<int>("key");
      // conditions
      q.extendCondition("SMT.SMT_ID = :key");
      q.extendCondition(" AND SMT.SMT_L1_MENU_ID = L1TM.L1TM_ID");
      // attributes
      q.extendOutput<std::string>( "SMT.SMT_NAME" );
      q.extendOutput<int>        ( "SMT.SMT_VERSION" );
         q.extendOutput<int>        ( "SMT.SMT_L1_MENU_ID" );
         q.extendOutput<coral::Blob>( "L1TM.L1TM_DATA" );
         // the field with the data
         q.setDataName("L1TM.L1TM_DATA");
   }

   /*
     HLT menu queries
   */
   { // for schema version 1
      auto & q = m_hltqueries[1];
      // tables
      q.addToTableList ( "SUPER_MASTER_TABLE", "SMT" );
      q.addToTableList ( "HLT_MASTER_TABLE", "HMT" );
      // bind vars
      q.extendBinding<int>("key");
      // conditions
      q.extendCondition("SMT.SMT_ID = :key");
      q.extendCondition(" AND SMT.SMT_HLT_MASTER_TABLE_ID = HMT.HMT_ID");
      // attributes
      q.extendOutput<std::string>( "SMT.SMT_NAME" );
      q.extendOutput<int>        ( "SMT.SMT_HLT_MASTER_TABLE_ID" );
      q.extendOutput<coral::Blob>( "HMT.HMT_MENU" );
      // the field with the data
      q.setDataName("HMT.HMT_MENU");
   }
   { // for schema version 2
      auto & q = m_hltqueries[2];
      // tables
      q.addToTableList ( "SUPER_MASTER_TABLE", "SMT" );
      q.addToTableList ( "HLT_MENU", "HTM" );
      // bind vars
      q.extendBinding<int>("key");
      // conditions
      q.extendCondition("SMT.SMT_ID = :key");
      q.extendCondition(" AND SMT.SMT_HLT_MENU_ID = HTM.HTM_ID");
      // attributes
      q.extendOutput<std::string>( "SMT.SMT_NAME" );
      q.extendOutput<int>        ( "SMT.SMT_VERSION" );
      q.extendOutput<int>        ( "SMT.SMT_HLT_MENU_ID" );
      q.extendOutput<coral::Blob>( "HTM.HTM_DATA" );
      // the field with the data
      q.setDataName("HTM.HTM_DATA");
   }
}


// Destructor defined here because QueryDefinition is an incomplete type in the header
TrigConf::TrigDBMenuLoader::~TrigDBMenuLoader() = default;

bool
TrigConf::TrigDBMenuLoader::loadL1Menu( unsigned int smk,
                                        boost::property_tree::ptree & l1menu,
                                        const std::string & outFileName ) const
{
   if(useCrest()) {
      loadFromCrest(smk, l1menu, outFileName, "L1 Menu", "L1M");
   } else {
      loadFromOracle(smk, l1menu, outFileName, "L1 menu", m_l1queries);
   }
   return true;
}

bool
TrigConf::TrigDBMenuLoader::loadHLTMenu( unsigned int smk,
                                         boost::property_tree::ptree & hltmenu,
                                         const std::string & outFileName ) const
{
   if(useCrest()) {
      loadFromCrest(smk, hltmenu, outFileName, "HLT Menu", "HLTM");
   } else {
      loadFromOracle(smk, hltmenu, outFileName, "HLT menu", m_hltqueries);
   }
   return true;
}

bool
TrigConf::TrigDBMenuLoader::loadL1Menu( unsigned int smk, L1Menu & l1menu,
                                        const std::string & outFileName ) const
{
   boost::property_tree::ptree ptl1;
   loadL1Menu( smk, ptl1, outFileName );
   try {
      l1menu.setData(std::move(ptl1));
      l1menu.setSMK(smk);
   }
   catch(std::exception & ex) {
      l1menu.clear();
      TRG_MSG_ERROR("When reading L1 menu for SMK " << smk << " a parsing error occured ( " << ex.what() <<" )" );
      throw TrigConf::ParsingException("TrigDBMenuLoader: parsing error " + std::string(ex.what()));
   }
   return true;
}


bool
TrigConf::TrigDBMenuLoader::loadHLTMenu( unsigned int smk, HLTMenu & hltmenu,
                                         const std::string & outFileName ) const
{
   boost::property_tree::ptree pthlt;
   loadHLTMenu( smk, pthlt, outFileName );
   try {
      hltmenu.setData(std::move(pthlt));
      hltmenu.setSMK(smk);
   }
   catch(std::exception & ex) {
      hltmenu.clear();
      TRG_MSG_ERROR("When reading HLT menu for SMK " << smk << " a parsing error occured ( " << ex.what() <<" )" );
      throw TrigConf::ParsingException("TrigDBMenuLoader: parsing error " + std::string(ex.what()));
   }
   return true;
}
