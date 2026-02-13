/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


//package includes
#include "HGTDMappingAlg.h"


//Athena
#include "PathResolver/PathResolver.h"
// for std::ifstream
#include <fstream> 


HGTDMappingAlg::HGTDMappingAlg(const std::string& name, ISvcLocator* pSvcLocator):
  AthReentrantAlgorithm(name, pSvcLocator)
{
}


StatusCode HGTDMappingAlg::initialize() {
  m_source = PathResolver::find_file(m_source.value(), "DATAPATH");
  if (m_source.empty()) {
    ATH_MSG_FATAL("The HGTD data file for cabling, " << m_source.value() << ", was not found.");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Reading cabling from " << m_source.value());

  ATH_CHECK(detStore()->retrieve(m_idHelper, "HGTD_ID"));
  ATH_CHECK(m_writeKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode HGTDMappingAlg::execute(const EventContext& ctx) const {
  
  SG::WriteCondHandle<HGTDMappingData> writeHandle = SG::makeHandle(m_writeKey, ctx);

  // Construct the output Cond Object and fill it in
  std::unique_ptr<HGTDMappingData> pMapping = std::make_unique<HGTDMappingData>();
  auto inputFile = std::ifstream(m_source.value());
  if (not inputFile.good()){
    ATH_MSG_ERROR("The HGTD mapping file "<<m_source.value()<<" could not be opened.");
    return StatusCode::FAILURE;
  }
  else{
    ATH_MSG_DEBUG("input file is good " << m_source.value());
  }

  inputFile>>*pMapping;
  const int numEntries = pMapping->size(); 

  // Define validity of the output cond object and record it
  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 0, 0, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDRange rangeW{start, stop};
  if (writeHandle.record(rangeW, std::move(pMapping)).isFailure()) {
    ATH_MSG_FATAL("Could not record MappingData " << writeHandle.key() 
                  << " with EventRange " << rangeW
                  << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_VERBOSE("recorded new conditions data object " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");
  return (numEntries==0) ? (StatusCode::FAILURE) : (StatusCode::SUCCESS);
}
