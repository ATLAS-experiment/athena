/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "eFEXDriver.h"
#include "eFEXSim.h"
#include "L1CaloFEXSim/eFEXOutputCollection.h"

#include "StoreGate/WriteHandle.h"


namespace LVL1 {

  eFEXDriver::eFEXDriver(const std::string& name, ISvcLocator* pSvcLocator)
    :  AthAlgorithm(name, pSvcLocator)//AthReentrantAlgorithm(name, pSvcLocator)
  { 
  
  }


 eFEXDriver::~eFEXDriver()
{
  ATH_MSG_DEBUG("Destroying " << name() << "...");
}


StatusCode eFEXDriver::initialize()
{
  ATH_CHECK( m_eFEXSysSimTool.retrieve() );

  ATH_CHECK( m_eFEXOutputCollectionSGKey.initialize() );

  return StatusCode::SUCCESS;
}


StatusCode eFEXDriver::finalize()
{
  ATH_MSG_DEBUG("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}


  StatusCode eFEXDriver::execute(/*const EventContext& ctx*/) //const
{

  // STEP 2 - Do some monitoring
  eFEXOutputCollection* my_eFEXOutputCollection = new eFEXOutputCollection();
  my_eFEXOutputCollection->setdooutput(true);

  // STEP 3 - Run THE eFEXSysSim
  ATH_CHECK(m_eFEXSysSimTool->execute(my_eFEXOutputCollection));

  // STEP 5 - Write the completed eFEXOutputCollection into StoreGate (move the local copy in memory)
  std::unique_ptr<eFEXOutputCollection> local_eFEXOutputCollection = std::unique_ptr<eFEXOutputCollection>(my_eFEXOutputCollection);
  SG::WriteHandle<LVL1::eFEXOutputCollection> eFEXOutputCollectionSG(m_eFEXOutputCollectionSGKey);
  ATH_CHECK(eFEXOutputCollectionSG.record(std::move(local_eFEXOutputCollection)));

  return StatusCode::SUCCESS;
}


} // end of LVL1 namespace
