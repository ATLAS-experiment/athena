/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <EventLoop/TEventModule.h>

#include <memory>
#include <AsgTools/SgTEvent.h>
#include <xAODRootAccess/TEvent.h>
#include <xAODRootAccess/tools/TFileAccessTracer.h>
#include <xAODRootAccess/TStore.h>
// #include <xAODRootAccess/D3PDPerfStats.h>
#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/ThrowMsg.h>
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
    TEventModule ::
    TEventModule (const std::string& name)
      : Module (name)
    {}



    TEventModule ::
    ~TEventModule ()
    {}



    StatusCode TEventModule ::
    onInitialize (ModuleData& data)
    {
      if (m_event != nullptr || m_store != nullptr)
      {
        ANA_MSG_ERROR ("module initialized twice");
        return StatusCode::FAILURE;
      }
      if (data.m_tevent != nullptr || data.m_tstore != nullptr)
      {
        ANA_MSG_ERROR ("duplicate TEventModule??");
        return StatusCode::FAILURE;
      }

      if (!m_modeStr.value().empty())
      {
        xAOD::TEvent::EAuxMode mode = xAOD::TEvent::kClassAccess; //compiler dummy
        if (m_modeStr.value() == Job::optXaodAccessMode_class)
          mode = xAOD::TEvent::kClassAccess;
        else if (m_modeStr.value() == Job::optXaodAccessMode_branch)
          mode = xAOD::TEvent::kBranchAccess;
        else if (m_modeStr.value() == Job::optXaodAccessMode_athena)
          mode = xAOD::TEvent::kAthenaAccess;
        else
        {
          ANA_MSG_ERROR ("unknown XAOD access mode: " << m_modeStr.value());
          return StatusCode::FAILURE;
        }
        m_event.reset (new xAOD::TEvent (mode));
      } else
      {
        m_event.reset (new xAOD::TEvent);
      }
      if (!m_summaryReport.value())
        xAOD::TFileAccessTracer::enableDataSubmission (false);

      m_store.reset (new xAOD::TStore);

      m_evtStore = std::make_unique<asg::SgTEvent> (m_event.get(), m_store.get());

      if (m_useStats.value())
        xAOD::PerfStats::instance().start();

      data.m_tevent = m_event.get();
      data.m_tstore = m_store.get();
      data.m_evtStore = m_evtStore.get();

      // Perform a sanity check.
      if (!data.m_inputFile)
      {
        ANA_MSG_ERROR ("File is not available during initialization?!?");
        return StatusCode::FAILURE;
      }
      // Set up the reading from the first input file, which should be
      // open already. But note that no event is loaded with getEntry(...)
      // yet, as we don't want to allow users to access the first event
      // during initialisation. Only the in-file metadata...
      ANA_CHECK (m_event->readFrom (data.m_inputFile.get()));

      return StatusCode::SUCCESS;
    }



    StatusCode TEventModule ::
    postFinalize (ModuleData& data)
    {
      if (m_useStats.value())
      {
        xAOD::PerfStats::instance().stop();
        std::unique_ptr<xAOD::ReadStats> stats
          (new xAOD::ReadStats (xAOD::IOStats::instance().stats()));
        stats->SetName (Job::optXAODReadStats.c_str());
        stats->Print ();
        data.addOutput (std::move (stats));
      }
      data.m_evtStore = nullptr;
      data.m_tevent = nullptr;
      data.m_tstore = nullptr;
      m_evtStore.reset ();
      m_event.reset ();
      m_store.reset ();
      return StatusCode::SUCCESS;
    }



    StatusCode TEventModule ::
    onNewInputFile (ModuleData& data)
    {
      if (m_event == nullptr || m_store == nullptr)
      {
        ANA_MSG_ERROR ("module not inititalized");
        return StatusCode::FAILURE;
      }
      ANA_CHECK (m_event->readFrom (data.m_inputFile.get()));
      if ((m_event->getEntries() > 0) && (m_event->getEntry (0) < 0))
      {
        ANA_MSG_ERROR ("Failed to load first entry from file");
        return StatusCode::FAILURE;
      }
      m_store->clear ();
      return StatusCode::SUCCESS;
    }



    StatusCode TEventModule ::
    postCloseInputFile (ModuleData& /*data*/)
    {
      if (m_event == nullptr || m_store == nullptr)
      {
        ANA_MSG_ERROR ("module not inititalized");
        return StatusCode::FAILURE;
      }
      ANA_CHECK (m_event->readFrom ((TFile *)nullptr));
      return StatusCode::SUCCESS;
    }



    StatusCode TEventModule ::
    onExecute (ModuleData& data)
    {
      m_store->clear ();
      if (m_event->getEntry (data.m_inputTreeEntry) < 0)
        RCU_THROW_MSG ("failed to read from xAOD");
      return StatusCode::SUCCESS;
    }
  }
}
