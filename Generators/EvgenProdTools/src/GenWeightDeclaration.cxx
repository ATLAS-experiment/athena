/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#include "EvgenProdTools/GenWeightDeclaration.h"

#include "EventBookkeeperTools/CutFlowSvc.h"
#include "GenInterfaces/IHepMCWeightSvc.h"

GenWeightDeclaration::GenWeightDeclaration(const std::string& name,
                                           ISvcLocator* svcLoc)
  : GenBase(name, svcLoc)
{
}


StatusCode GenWeightDeclaration::initialize()
{
  ATH_CHECK(GenBase::initialize());

  // Retrieve the CutFlowSvc
  ATH_CHECK(m_cutFlowSvc.retrieve());

  // Access CutFlowSvc methods
  m_cutFlowSvcImpl = dynamic_cast<CutFlowSvc*>(&*m_cutFlowSvc);
  if (!m_cutFlowSvcImpl) {
    ATH_MSG_ERROR("Configured CutFlowSvc does not use the CutFlowSvc implementation");
    return StatusCode::FAILURE;
  }

  // Retrieve the HepMCWeightSvc
  ATH_CHECK(m_hepMCWeightSvc.retrieve());
  return StatusCode::SUCCESS;
}


StatusCode GenWeightDeclaration::execute(const EventContext& ctx)
{
  if (m_weightsDeclared) {
    return StatusCode::SUCCESS;
  }

  // Declare the number of generator weights to the CutFlowSvc
  const McEventCollection* eventCollection = events_const(ctx);
  if (!eventCollection || eventCollection->empty()) {
    ATH_MSG_ERROR("Cannot declare generator weights from an empty GEN_EVENT");
    return StatusCode::FAILURE;
  }

  const HepMC::GenEvent* event = eventCollection->front();
  if (!event) {
    ATH_MSG_ERROR("Cannot declare generator weights from a null event");
    return StatusCode::FAILURE;
  }

  const std::size_t weightCount = event->weights().size();
  if (weightCount == 0) {
    ATH_MSG_ERROR("Cannot declare an empty generator-weight vector");
    return StatusCode::FAILURE;
  }

  IHepMCWeightSvc::WeightMap weightNames = HepMC::weights_map(event);
  const auto runInfo = event->run_info();
  if (runInfo && !runInfo->weight_names().empty()) {
    const std::size_t rawNameCount = runInfo->weight_names().size();
    if (rawNameCount != weightCount) {
      ATH_MSG_ERROR("Generator weight names and values have different sizes: "
                    << rawNameCount << " and " << weightCount);
      return StatusCode::FAILURE;
    }
    if (weightNames.size() != rawNameCount) {
      ATH_MSG_ERROR("Generator weight names are not unique");
      return StatusCode::FAILURE;
    }
  }

  // Declare the number of weights to the CutFlowSvc 
  // and the weight names to the HepMCWeightSvc
  ATH_CHECK(m_cutFlowSvcImpl->setNumberOfWeightVariations(weightCount));
  if (!weightNames.empty()) {
    ATH_CHECK(m_hepMCWeightSvc->setWeightNames(weightNames, ctx));
  }

  // Set the flag to avoid re-declaring weights in subsequent events
  m_weightsDeclared = true;
  ATH_MSG_INFO("Declared " << weightCount
               << " generator weight slot" << (weightCount == 1 ? "" : "s")
               << " from the first event");
  return StatusCode::SUCCESS;
}

#endif
