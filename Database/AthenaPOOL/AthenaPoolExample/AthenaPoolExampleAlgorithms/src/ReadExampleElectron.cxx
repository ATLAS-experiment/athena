/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ReadExampleElectron.h"

#include <set>

#include "AthenaPoolExampleData/ExampleElectron.h"
#include "AthenaPoolExampleData/ExampleElectronContainer.h"
#include "EventBookkeeperMetaData/EventBookkeeperCollection.h"
#include "EventInfo/EventID.h"
#include "EventInfo/EventStreamInfo.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandle.h"

using namespace AthPoolEx;

//___________________________________________________________________________
ReadExampleElectron::ReadExampleElectron(const std::string& name,
                                         ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}
//___________________________________________________________________________
StatusCode ReadExampleElectron::initialize() {
  ATH_MSG_INFO(name() << ": in initialize()");

  ATH_CHECK(m_exampleElectronContainerKey.initialize());
  ATH_CHECK(m_decor1Key.initialize());

  return StatusCode::SUCCESS;
}
//___________________________________________________________________________
StatusCode ReadExampleElectron::execute(const EventContext& ctx) const {
  ATH_MSG_INFO(name() << ": is executing ...");
  size_t idx = 0;

  // Setup the handle for the ElectronContainer
  SG::ReadHandle<xAOD::ExampleElectronContainer> objs =
      SG::makeHandle(m_exampleElectronContainerKey, ctx);

  // Handle for the decoration
  SG::ReadDecorHandle<xAOD::ExampleElectronContainer, float> hdl1(m_decor1Key,
                                                                  ctx);

  if (objs.isValid()) {

    // Access the example electrons
    for (const xAOD::ExampleElectron* obj : *objs) {
      ATH_MSG_INFO("ExampleElectron #" << idx << " {pt = " << obj->pt()
                                       << ", charge = " << obj->charge()
                                       << "} ");

      // check to see if the first decor was written and print it out
      if (hdl1.isPresent()) {
        ATH_MSG_INFO("Decoration reader: decor1 = " << hdl1(*obj));
      } else {
        ATH_MSG_INFO("Decoration reader decor1 hasn't been written");
      }

      ++idx;  // iterate the index
    }

  } else {
    ATH_MSG_ERROR(" Could not get example electron");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}
//___________________________________________________________________________
StatusCode ReadExampleElectron::finalize() {
  ATH_MSG_INFO(name() << "in finalize()");
  return StatusCode::SUCCESS;
}
