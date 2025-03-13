/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "IdDictParser/IdDictParser.h"  
#include "InDetIdentifier/SCT_ID.h"
#include "src/ITkStripCablingAlg.h"
#include "src/OnlineIdGenerator.h"

#include <iostream>
#include <fstream>
#include <set>
#include <memory>

static const std::string itkDictFilename{"InDetIdDictFiles/IdDictInnerDetector_ITK-P2-RUN4-03-00-00.xml"};
static const std::string outputFileName{"ITkStripsCabling.dat"};

int main(){

  IdDictParser parser;
  parser.register_external_entity("InnerDetector", itkDictFilename);
  IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
  auto pITkId=std::make_unique<SCT_ID>();
  pITkId->initialize_from_dictionary(idd);
  ITkStripCabling::OnlineIdGenerator gen(pITkId.get());
  std::ostringstream os;
  ExpandedIdentifier e{};
  std::set<ITkStripOnlineId> onlineIds;
  std::set<std::uint32_t> rodIds;
  for(auto i = pITkId->wafer_begin();i!=pITkId->wafer_end();++i){
    pITkId->get_expanded_id(*i,e);
    onlineIds.insert(gen(*i));
    rodIds.insert(gen.rod(*i));
    os<<*i<<" "<<e<<", "<<gen(*i)<<"\n";
  }
  std::ofstream file(outputFileName);
  file<<"#Wafer Identifiers,Expanded Identifiers {2/2/Bec/LayerDisk/Phi/Eta/Side/0/0} and OnlineId:\n";
  file<<"#Version 0\n";
  file<<os.str()<<std::endl;
  std::cout<<"File "+outputFileName+" has been created.\n";
  

  return 0;
}
