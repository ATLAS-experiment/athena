/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
 
#include "L0MuonRPCSim.h"

#include "xAODTrigger/MuonRoIAuxContainer.h"

namespace L0Muon {

L0MuonRPCSim::L0MuonRPCSim(const std::string& name, ISvcLocator* pSvcLocator)
: AthReentrantAlgorithm(name, pSvcLocator) {}


L0MuonRPCSim::~L0MuonRPCSim() {}


StatusCode L0MuonRPCSim::initialize() {
  ATH_MSG_INFO("Initializing " << name() << "...");

  ATH_CHECK(m_keyRpcRdo.initialize());
  ATH_CHECK(m_keyRpcDigit.initialize());
  ATH_CHECK(m_cablingKey.initialize());

  /// container of output RoIs
  ATH_CHECK(m_outputMuonRoIKey.initialize());
  
  /// retrieve the monitoring tool
  if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());
  
  return StatusCode::SUCCESS;
}


StatusCode L0MuonRPCSim::finalize() {
  ATH_MSG_INFO ("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}


StatusCode L0MuonRPCSim::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG ("Executing " << name() << "...");
  
  SG::ReadHandle<xAOD::NRPCRDOContainer> rpcRdo_handle(m_keyRpcRdo,ctx);
  const xAOD::NRPCRDOContainer* inputRDO = rpcRdo_handle.cptr();
  if ( not inputRDO ) {
    ATH_MSG_FATAL("Unable to retrieve input RPC RDO container: " << m_keyRpcRdo.key());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Number of RPC RDO: " <<  inputRDO->size());
  
  SG::ReadCondHandle cablingMap{m_cablingKey, ctx};
  ATH_CHECK(cablingMap.isValid());
  
  /// monitor RDO quantities
  if ( !m_monTool.empty() ) {
    
    auto n_of_RDO = Monitored::Scalar<unsigned int>("n_of_RDO",inputRDO->size());
    
  }

  
  /// output RoIs container
  SG::WriteHandle<xAOD::MuonRoIContainer> outputRoI_handle(m_outputMuonRoIKey, ctx);
  ATH_CHECK(outputRoI_handle.record(std::make_unique<xAOD::MuonRoIContainer>(), std::make_unique<xAOD::MuonRoIAuxContainer>()));
  auto outputRoIs = outputRoI_handle.ptr();

  /// Create an RoI
  outputRoIs->push_back(std::make_unique<xAOD::MuonRoI>()); 

  
  return StatusCode::SUCCESS;
}


}   // end of namespace
