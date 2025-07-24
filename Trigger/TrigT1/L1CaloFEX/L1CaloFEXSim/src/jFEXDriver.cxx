/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include "jFEXDriver.h"
#include "L1CaloFEXSim/jFEXOutputCollection.h"
#include "StoreGate/WriteHandle.h"
#include "SGTools/TestStore.h"


#define DEBUG_VHB 1


namespace LVL1 {

jFEXDriver::jFEXDriver(const std::string& name, ISvcLocator* pSvcLocator):  AthAlgorithm(name, pSvcLocator){}


 jFEXDriver::~jFEXDriver()
{
  ATH_MSG_DEBUG("Destroying " << name() << "...");
}


StatusCode jFEXDriver::initialize()
{

  ATH_CHECK( m_jFEXSysSimTool.retrieve() );
  ATH_CHECK( m_jFEXOutputCollectionSGKey.initialize() );
  
  return StatusCode::SUCCESS;

}


StatusCode jFEXDriver::finalize()
{
  ATH_MSG_DEBUG("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}


StatusCode jFEXDriver::execute() {
    
    
    // STEP 1 - Set up the jFEXSysSim
    m_jFEXSysSimTool->init();
    
    // STEP 2 - Do some monitoring
    jFEXOutputCollection* my_jFEXOutputCollection = new jFEXOutputCollection();
    my_jFEXOutputCollection->setdooutput(true);  
    
    // STEP 3 - Run the jFEXSysSim
    ATH_CHECK(m_jFEXSysSimTool->execute(my_jFEXOutputCollection));      
    
    // STEP 4 - Close and clean the event  
    m_jFEXSysSimTool->cleanup();
    
    // STEP 5 - Write the completed jFEXOutputCollection into StoreGate (move the local copy in memory)
    std::unique_ptr<jFEXOutputCollection> local_jFEXOutputCollection = std::unique_ptr<jFEXOutputCollection>(my_jFEXOutputCollection);
    SG::WriteHandle<LVL1::jFEXOutputCollection> jFEXOutputCollectionSG(m_jFEXOutputCollectionSGKey);
    ATH_CHECK(jFEXOutputCollectionSG.record(std::move(local_jFEXOutputCollection)));
    
    return StatusCode::SUCCESS;
}

} // end of LVL1 namespace
