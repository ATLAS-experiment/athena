/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcByteStream/ZdcByteStreamLucrodData.h"
#include "ZdcByteStream/ZdcLucrodDataContainer.h"
#include "ZdcByteStream/ZdcLucrodData.h"

#include "eformat/SourceIdentifier.h"

using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

ZdcByteStreamLucrodData::ZdcByteStreamLucrodData(const std::string& name, ISvcLocator* pSvcLocator):
  AthAlgorithm     (name                , pSvcLocator),
  m_robDataProvider("ROBDataProviderSvc", name),
  m_ZdcLucrodDecoder(eformat::FORWARD_ZDC)
 {
 }

StatusCode ZdcByteStreamLucrodData::initialize() {
  
  ATH_MSG_DEBUG("ZdcByteStreamLucrodData::initialize");

  StatusCode sc = m_robDataProvider.retrieve();
  
  if (sc.isFailure()) ATH_MSG_WARNING(" Could not retrieve ROBDataProviderSvc ");
  else                ATH_MSG_DEBUG  (" Retrieved service ROBDataProviderSvc ");

  ATH_CHECK( m_ZdcLucrodDataContainerKey.initialize() );
  
  return StatusCode::SUCCESS;
}

StatusCode ZdcByteStreamLucrodData::execute() {
  
  ATH_MSG_DEBUG(" ZdcByteStreamLucrodData::execute ");

  SG::WriteHandle<ZdcLucrodDataContainer> h_write(m_ZdcLucrodDataContainerKey);
  
  std::vector<const ROBFragment*> listOfRobf;
  std::vector<unsigned int> ROBIDs;
  
  // Set up the list of LUCROD ROBIDs 
  //
  //  High word of source ID indicates the ZDC (0x83)
  //  Low word numbers the modules: 0-5 for the 6 LUCRODs in Run 3
  //
  const unsigned int RODSourceIdShifted = eformat::FORWARD_ZDC << 16;

  for (unsigned int lucrodModuleNum = 0; lucrodModuleNum < 6; lucrodModuleNum++) {
    ROBIDs.push_back(RODSourceIdShifted + lucrodModuleNum);
  }

  ATH_MSG_DEBUG("ZdcByteStreamLucrodData::execute::getROBDATA");
  m_robDataProvider->getROBData(Gaudi::Hive::currentContext(), ROBIDs, listOfRobf);
  
  auto zdcLucrodDataContainer = std::make_unique<ZdcLucrodDataContainer>(); 
  
  ATH_MSG_DEBUG("ZdcByteStreamLucrodData::execute::filleContainer");
  StatusCode sc = fillContainer(std::move(listOfRobf),zdcLucrodDataContainer.get());
  
  if (sc.isFailure()) ATH_MSG_WARNING(" fillContainer failed ");
  else                ATH_MSG_DEBUG  (" fillContainer success ");

  ATH_MSG_DEBUG("ZdcByteStreamLucrodData::execute::record m_ZdcLucrodDataContainerKey");

  sc = h_write.record(std::move(zdcLucrodDataContainer));
  
  ATH_MSG_DEBUG("ZdcByteStreamLucrodData::execute::record testing...");

  if (sc.isFailure()) ATH_MSG_WARNING(" Could not record ZdcLucrodDataContainer in StoreGate ");
  else                ATH_MSG_DEBUG  (" ZdcLucrodDataContainer is recorded in StoreGate ");
  
  return StatusCode::SUCCESS;
}

StatusCode ZdcByteStreamLucrodData::fillContainer(std::vector<const ROBFragment*> listOfRobf, ZdcLucrodDataContainer* zdcLucrodDataContainer) {
  
  ATH_MSG_DEBUG(" ZdcByteStreamLucrodData::fillContainer ");
 
  size_t nFragments = listOfRobf.size();
    
  ATH_MSG_DEBUG(" Number of ROB fragments: " << nFragments);
  
  if (!nFragments) return StatusCode::SUCCESS;

  std::vector<const ROBFragment*>::const_iterator rob_it  = listOfRobf.begin();
  std::vector<const ROBFragment*>::const_iterator rob_end = listOfRobf.end();
  
  for (; rob_it != rob_end; ++rob_it) {
    
    // *** important: we need to use [rod] source id here ***
    //
    uint32_t robid = (*rob_it)->rod_source_id();
    
    ATH_MSG_DEBUG(" ROB Fragment with ID: 0x" << std::hex << robid << std::dec); 

    uint32_t lucrodID = robid & 0x7;

    ZdcLucrodData* zld = new ZdcLucrodData(lucrodID);
    
    StatusCode sc = m_ZdcLucrodDecoder.decode(&**rob_it, zld);
    
    if (sc.isFailure()){
      ATH_MSG_WARNING(" Conversion from ByteStream to ZdcLucrodData failed ");
      delete zld;
    } 
    else zdcLucrodDataContainer->push_back(zld);
  }

  for (auto zld : *zdcLucrodDataContainer) {zld->str();}

  if (zdcLucrodDataContainer->size() == nFragments)
    return StatusCode::SUCCESS;
  else
    return StatusCode::FAILURE;

}

StatusCode ZdcByteStreamLucrodData::finalize() { 
  
  ATH_MSG_DEBUG(" ZdcByteStreamLucrodData::finalize ");
  
  return StatusCode::SUCCESS;
}
