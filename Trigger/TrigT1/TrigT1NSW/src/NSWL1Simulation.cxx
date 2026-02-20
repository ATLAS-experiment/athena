/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "NSWL1Simulation.h"

namespace NSWL1 {
  NSWL1Simulation::NSWL1Simulation( const std::string& name, ISvcLocator* pSvcLocator )
    : AthReentrantAlgorithm( name, pSvcLocator ),
      m_histSvc( "THistSvc/THistSvc", name )
  {}


  StatusCode NSWL1Simulation::initialize() {
    ATH_MSG_DEBUG( "initialize " << name() );
    ATH_CHECK( m_trigRdoContainer.initialize() );

    // Do not allow debug ntuple in multi-threaded mode
    if(m_doNtuple and Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents() > 1) {
      ATH_MSG_ERROR("DoNtuple is not possible in multi-threaded mode");
      return StatusCode::FAILURE;
    }

    if(m_dosTGC){
      if(m_doPad) {
        ATH_CHECK(m_pad_emulator.retrieve());
        if(m_doNtuple) ATH_CHECK(m_pad_emulator->attachBranches(m_altree));
      }
      if(m_doStrip){
        ATH_MSG_ERROR("No sTGC Strip Trigger algorithm defined. Please, check whether the hardware implementation is available before implementing any kind of tool!");
        return StatusCode::FAILURE;
      }
    }

    if(m_doMM){
      ATH_CHECK(m_mmtrigger.retrieve());
      if(m_doNtuple) ATH_CHECK(m_mmtrigger->attachBranches(m_altree));
    }

    ATH_CHECK(m_trigProcessor.retrieve());

    if(m_doNtuple) ATH_CHECK(m_altree.init(this));
    return StatusCode::SUCCESS;
  }


  StatusCode NSWL1Simulation::execute(const EventContext& ctx) const {
    auto padTriggerContainer = std::make_unique<Muon::NSW_PadTriggerDataContainer>();
    auto stripTriggerContainer = std::make_unique<Muon::NSW_TrigRawDataContainer>();
    auto MMTriggerContainer = std::make_unique<Muon::NSW_TrigRawDataContainer>();

    // sTGC Trigger
    if(m_dosTGC) {
      if(m_doPad) {
        ATH_CHECK( m_pad_emulator->emulate(ctx, padTriggerContainer.get()) );
      }
    }

    // MM Trigger
    if(m_doMM) {
      ATH_CHECK( m_mmtrigger->runTrigger(ctx, MMTriggerContainer.get(), m_doMMDiamonds) );
    }

    // Store output in debug ntuple when running in single-thread only
    if(m_doNtuple) {
      static std::mutex mutex;
      std::scoped_lock lock(mutex);
      bool success ATLAS_THREAD_SAFE = m_altree.fill(ctx);
      if(!success) return StatusCode::FAILURE;
    }

    SG::WriteHandle<Muon::NSW_TrigRawDataContainer> rdohandle( m_trigRdoContainer, ctx );
    auto trgContainer=std::make_unique<Muon::NSW_TrigRawDataContainer>();
    ATH_CHECK( m_trigProcessor->mergeRDO(padTriggerContainer.get(), stripTriggerContainer.get(), MMTriggerContainer.get(), trgContainer.get()) );
    ATH_CHECK(rdohandle.record(std::move(trgContainer)));
    return StatusCode::SUCCESS;
  }

  StatusCode NSWL1Simulation::finalize() {
    if(m_doNtuple) ATH_CHECK(m_altree.write());
    return StatusCode::SUCCESS;
  }
}
