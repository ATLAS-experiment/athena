/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <EventLoop/EventModule.h>

#include <memory>
#include <AsgTools/SgEvent.h>
#include <xAODRootAccess/Event.h>
#include <xAODRootAccess/tools/TFileAccessTracer.h>
#include <xAODRootAccess/TStore.h>
#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <RootCoreUtils/Assert.h>
#include <SampleHandler/MetaObject.h>
#include <xAODCore/tools/ReadStats.h>
#include <xAODCore/tools/PerfStats.h>
#include <xAODCore/tools/IOStats.h>
#include <AsgMessaging/MessageCheck.h>
#include <TList.h>

//
// method implementations
//

namespace EL
{
  namespace Detail
  {
    EventModule::EventModule (const std::string& name)
      : Module (name)
    {}

    EventModule::~EventModule ()
    {}

    
    StatusCode EventModule::onFirstInputFile (ModuleData& data) {

      if (m_event != nullptr || m_store != nullptr) {
        ANA_MSG_ERROR ("module initialized twice");
        return StatusCode::FAILURE;
      }
      if (data.m_event != nullptr || data.m_tstore != nullptr) {
        ANA_MSG_ERROR ("duplicate EventModule??");
        return StatusCode::FAILURE;
      }

      // Perform a sanity check.
      if (!data.m_inputFile) {
        ANA_MSG_ERROR ("File is not available during initialization?!?");
        return StatusCode::FAILURE;
      }

      // Set up the reading from the first input file. This is the generic
      // interface for both TTree and RNTuple reading.  But note that no event is loaded with getEntry(...)
      // yet, as we don't want to allow users to access the first event
      // during initialisation. Only the in-file metadata...
      m_event = xAOD::Event::createAndReadFrom(*data.m_inputFile.get());
      if (!m_event) {
          ATH_MSG_ERROR( "cannot read from file: " << data.m_inputFile->GetName());
          return StatusCode::FAILURE;
      }
      // Set event in module data
      data.m_event = m_event.get();
      ATH_MSG_DEBUG( "EventModule::onFirstInputFile: opened " << data.m_inputFile->GetName());
      if (!m_summaryReport.value())
        xAOD::TFileAccessTracer::instance().enableDataSubmission (false);

      m_store.reset (new xAOD::TStore);

      m_evtStore = std::make_unique<asg::SgEvent> (m_event.get(), m_store.get());

      if (m_useStats.value())
        xAOD::PerfStats::instance().start();

      data.m_tstore   = m_store.get();
      data.m_evtStore = m_evtStore.get();
      return StatusCode::SUCCESS;
    }


    StatusCode EventModule::onNextInputFile (ModuleData& data) {

      if (m_event == nullptr || m_store == nullptr) {
        ANA_MSG_ERROR ("module not inititalized");
        return StatusCode::FAILURE;
      }
      ANA_CHECK (m_event->readFrom (*data.m_inputFile.get()));
      m_store->clear ();
      return StatusCode::SUCCESS;
    }

    StatusCode EventModule::onNewInputFile (ModuleData& /*data*/)
    {
      if (m_event == nullptr || m_store == nullptr) {
        ANA_MSG_ERROR ("module not inititalized");
        return StatusCode::FAILURE;
      }
      // readFrom has already been done in onFirstInputFile or onNextInputFile
      // Here we only need to make sure getEntry(0) is called
      if ((m_event->getEntries() > 0) && (m_event->getEntry (0) < 0)) {
        ANA_MSG_ERROR ("Failed to load first entry from file");
        return StatusCode::FAILURE;
      }
      m_store->clear ();
      return StatusCode::SUCCESS;
    }

    StatusCode EventModule::postCloseInputFile (ModuleData& /*data*/) {

      if (m_event == nullptr || m_store == nullptr) {
        ANA_MSG_ERROR ("module not inititalized");
        return StatusCode::FAILURE;
      }
      // Nothing to do post close input file
      return StatusCode::SUCCESS;
    }


    StatusCode EventModule::onExecute (ModuleData& data) {
      // move to next event
      m_store->clear ();
      if (m_event->getEntry (data.m_inputEntry) < 0)
      {
        ANA_MSG_ERROR ("failed to read from xAOD");
        return StatusCode::FAILURE;
      }
      return StatusCode::SUCCESS;
    }


    StatusCode EventModule::postFinalize (ModuleData& data) {

      // Only stop the stats if we actually started them, which happens in
      // onFirstInputFile once m_event is created.  With an empty file list
      // start() is never called, so stop() must be skipped too.
      if (m_useStats.value() && m_event != nullptr)       {
        xAOD::PerfStats::instance().stop();
        std::unique_ptr<xAOD::ReadStats> stats
          (new xAOD::ReadStats (xAOD::IOStats::instance().stats()));
        stats->SetName (Job::optXAODReadStats.c_str());
        stats->Print ();
        data.addOutput (std::move (stats));
      }
      data.m_evtStore = nullptr;
      data.m_event    = nullptr;
      data.m_tstore   = nullptr;
      m_evtStore.reset ();
      m_event.reset ();
      m_store.reset ();
      return StatusCode::SUCCESS;
    }

  }

}
