/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "gFEXDriver.h"
#include "L1CaloFEXSim/gFEXOutputCollection.h"
#include "StoreGate/WriteHandle.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

namespace LVL1 {

  gFEXDriver::gFEXDriver(const std::string& name, ISvcLocator* pSvcLocator)
    :  AthAlgorithm(name, pSvcLocator)
  {

  }


 gFEXDriver::~gFEXDriver()
{
  ATH_MSG_DEBUG("Destroying " << name() << "...");
}


StatusCode gFEXDriver::initialize()
{
  ServiceHandle<ITHistSvc> histSvc("THistSvc","");

  ATH_CHECK( histSvc.retrieve() );

  ATH_CHECK( m_gFEXSysSimTool.retrieve() );

  ATH_CHECK( m_gFEXOutputCollectionSGKey.initialize() );

  return StatusCode::SUCCESS;
}


  StatusCode gFEXDriver::execute() //const
{
  // STEP 1 - Do some monitoring
  gFEXOutputCollection* my_gFEXOutputCollection = new gFEXOutputCollection();
  my_gFEXOutputCollection->setdooutput(true);

  // STEP 2 - Run the gFEXSysSim
  ATH_CHECK(m_gFEXSysSimTool->execute(my_gFEXOutputCollection));

  // STEP 3 - Close and clean the event
  m_gFEXSysSimTool->cleanup();

  // STEP 4 - Write the completed gFEXOutputCollection into StoreGate (move the local copy in memory)
  std::unique_ptr<gFEXOutputCollection> local_gFEXOutputCollection = std::unique_ptr<gFEXOutputCollection>(my_gFEXOutputCollection);
  SG::WriteHandle<LVL1::gFEXOutputCollection> gFEXOutputCollectionSG(m_gFEXOutputCollectionSGKey);
  ATH_CHECK(gFEXOutputCollectionSG.record(std::move(local_gFEXOutputCollection)));

  return StatusCode::SUCCESS;
}


} // end of LVL1 namespace
