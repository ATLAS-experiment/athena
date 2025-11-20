/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DerivationKernel.cxx
///////////////////////////////////////////////////////////////////
// Author: James Catmore (James.Catmore@cern.ch)
// Based on the Integrated Simulation Framework
// This code loops over tools defining skimming, slimming and thinning
// operations, sets the final filter decision, applies the overall slimming
// and thinning, and passes on to the persistency

#include "DerivationFrameworkCore/DerivationKernel.h"

#include <sstream>                                      // C++ utilities
#include <string>
#include <algorithm>
#include <fstream>

#include "GaudiKernel/ISvcLocator.h"
#include "AthContainers/DataVector.h"
#include "AthLinks/ElementLink.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/Chrono.h"
#include "GaudiKernel/ToolVisitor.h"
#include "GaudiKernel/ConcurrencyFlags.h"

#include "StoreGate/StoreGateSvc.h"             // Storegate stuff
#include "StoreGate/DataHandle.h"
#include "AthenaKernel/DefaultKey.h"
#include "SGTools/StlVectorClids.h"

///////////////////////////////////////////////////////////////////////////////
namespace {
  inline void renounceInputs([[maybe_unused]] const std::unordered_set<std::string> &outputs, std::vector< const DataObjID *> &output_ids, AlgTool *a_tool) {
    for ( const DataObjID *a_data_id : output_ids ) {
      a_tool->renounceInput( *a_data_id );
    }
  }
  inline void collectOutputs(std::unordered_set<std::string> &outputs, std::vector< const DataObjID *> &output_ids, const AlgTool *a_tool) {
    for ( const DataObjID &a_data_id : a_tool->outputDataObjs() ) {
      if (outputs.insert( a_data_id.key() ).second) {
        output_ids.push_back( &a_data_id );
      }
    }
  }
  template <typename Callable, typename = std::enable_if_t<std::is_invocable_r_v<void, Callable, IAlgTool*>>>
  inline void visitTools(IAlgTool &a_tool_interface, Callable &func) {
    const AlgTool *alg_tool = dynamic_cast<AlgTool *>(&a_tool_interface);
    if (alg_tool) {
      func(&a_tool_interface);
      std::vector<IAlgTool *> &non_const_tools ATLAS_THREAD_SAFE = const_cast< std::vector<IAlgTool *> &>( alg_tool->tools() );
      ToolVisitor::visit(  non_const_tools, func);
    }
  }
}

DerivationFramework::DerivationKernel::DerivationKernel(const std::string& name, ISvcLocator* pSvcLocator) :
  AthFilterAlgorithm(name, pSvcLocator)
{
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode DerivationFramework::DerivationKernel::initialize() {

  ATH_MSG_INFO("Initializing the derivation framework kernel " << name());

  // get the skimming tools
  ATH_CHECK( m_skimmingTools.retrieve() );
  ATH_MSG_INFO("The following skimming tools will be applied....");
  ATH_MSG_INFO(m_skimmingTools);

  // get the thinning tools
  ATH_CHECK( m_thinningTools.retrieve() );
  ATH_MSG_INFO("The following thinning tools will be applied");
  ATH_MSG_INFO(m_thinningTools);

  // get the augmentation tools
  ATH_CHECK( m_augmentationTools.retrieve() );
  ATH_MSG_INFO("The following augmentation tools will be applied....");
  ATH_MSG_INFO(m_augmentationTools);

  if (m_doChronoStat) {
    //get the chrono auditor
    ATH_CHECK(m_chronoSvc.retrieve());
  }

  if (m_runSkimmingFirst) {
    ATH_MSG_INFO("Skimming will be run before augmentation. Make sure your skimming does not depend on variables calculated in the augmentation step!");
  }

  std::unordered_set<std::string> outputs;
  std::vector<const DataObjID *> output_ids;
  auto output_collector = [&outputs, &output_ids](IAlgTool *a_tool_interface) {
    const AlgTool *alg_tool = dynamic_cast<AlgTool *>(a_tool_interface);
    if (alg_tool) {
      collectOutputs(outputs,output_ids, alg_tool);
    }
  };
  auto renounce_and_collect_outputs = [&outputs, &output_ids](IAlgTool *a_tool_interface) {
    AlgTool *alg_tool = dynamic_cast<AlgTool *>(a_tool_interface);
    if (alg_tool) {
      renounceInputs(outputs,output_ids, alg_tool);
      collectOutputs(outputs,output_ids, alg_tool);
    }
  };
  auto renouncer = [&outputs, &output_ids](IAlgTool *a_tool_interface) {
    AlgTool *alg_tool = dynamic_cast<AlgTool *>(a_tool_interface);
    if (alg_tool) {
      renounceInputs(outputs,output_ids, alg_tool);
    }
  };

  // collection and renouncing has to happen in the order the tools are called
  // during execute.
  if (m_runSkimmingFirst) {
    for (ToolHandle<ISkimmingTool> &a_tool_handle : m_skimmingTools ) {
      visitTools(*a_tool_handle, output_collector);
    }
    for (ToolHandle<IAugmentationTool> &a_tool_handle : m_augmentationTools ) {
      visitTools(*a_tool_handle, renounce_and_collect_outputs);
    }
  }
  else {
    for (ToolHandle<IAugmentationTool> &a_tool_handle : m_augmentationTools ) {
      visitTools(*a_tool_handle, output_collector);
    }
    for (ToolHandle<ISkimmingTool> &a_tool_handle : m_skimmingTools ) {
      visitTools(*a_tool_handle, renounce_and_collect_outputs);
    }
  }
  for (ToolHandle<IThinningTool> &a_tool_handle : m_thinningTools ) {
    visitTools(*a_tool_handle, renouncer);
  }

  return StatusCode::SUCCESS;

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode DerivationFramework::DerivationKernel::execute() {

  IChronoSvc* cSvc=m_chronoSvc.get(); //Might be null ...
  // On your marks.... get set.... (but only if not in MT)
  Chrono chrono( cSvc , name() );
  // GO!!!

  ATH_MSG_DEBUG(name() << " is processing next event...");

  // Increment the event counter
  m_eventCounter++;

  //=============================================================================
  // AUGMENTATION ===============================================================
  //=============================================================================
  const EventContext &ctx = Gaudi::Hive::currentContext();
  if (!m_runSkimmingFirst) {
    for (const auto &  augmentationTool : m_augmentationTools) {
      ATH_MSG_DEBUG("Entering " << augmentationTool->name());
      if ( augmentationTool->addBranches(ctx).isFailure() ) {
        ATH_MSG_ERROR("Augmentation failed!");
        return StatusCode::FAILURE;
      }
    }
  }

  //=============================================================================
  //SKIMMING ===================================================================
  //=============================================================================

  // Set master flag to true
  bool acceptEvent(true);

  // Loop over the filters
  for (const auto &  skimmingTool : m_skimmingTools) {
    ATH_MSG_DEBUG("Entering " << skimmingTool->name());
    if (!(skimmingTool->eventPassesFilter())) {
      acceptEvent=false;
      ATH_MSG_DEBUG("This event failed the " << skimmingTool->name() << " filter. Therefore it will not be recorded.");
      break;
    }
  }

  // Increment local counters if event to be accepted
  if (acceptEvent) ++m_acceptCntr;

  // Set the setFilterPassed flag
  setFilterPassed(acceptEvent);

  // Return if event didn't pass
  if (!acceptEvent) return StatusCode::SUCCESS;

  // If user requested skimming first, run augmentation now...
  if (m_runSkimmingFirst) {
    for (const auto &  augmentationTool : m_augmentationTools) {
      ATH_MSG_DEBUG("Entering " << augmentationTool->name());
      if ( augmentationTool->addBranches(ctx).isFailure() ) {
        ATH_MSG_ERROR("Augmentation failed!");
        return StatusCode::FAILURE;
      }
    }
  }

  //=============================================================================
  // THINNING ===================================================================
  //=============================================================================

  for (const auto &  thinningTool : m_thinningTools) {
    ATH_MSG_DEBUG("Entering " << thinningTool->name());
    if ( thinningTool->doThinning().isFailure() ) {
      ATH_MSG_ERROR("Thinning failed!");
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode DerivationFramework::DerivationKernel::finalize() {

  ATH_MSG_INFO( "============================================================================");
  ATH_MSG_INFO( "|| SUMMARY OF THE DERIVATION FRAMEWORK KERNEL WITH NAME " << name() << " || ");
  ATH_MSG_INFO( "============================================================================");
  ATH_MSG_INFO( "============================================================================");
  ATH_MSG_INFO( "Events analyzed: " << m_eventCounter);
  ATH_MSG_INFO( "Events accepted: " << m_acceptCntr);
  ATH_MSG_INFO( "============================================================================");

  return StatusCode::SUCCESS;

}
