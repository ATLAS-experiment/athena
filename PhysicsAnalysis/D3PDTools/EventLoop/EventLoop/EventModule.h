/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/



#ifndef EVENT_LOOP_EVENT_SVC_H
#define EVENT_LOOP_EVENT_SVC_H

#include <EventLoop/Global.h>

#include <AsgTools/PropertyWrapper.h>
#include <EventLoop/Module.h>
#include <memory>

namespace asg
{
  class SgEvent;
}

namespace xAOD
{
  class Event;
  class TStore;
}

namespace EL
{
  namespace Detail
  {
    class EventModule : public Module
    {
      //
      // public interface
      //

    public:

      /// effects: standard constructor.
      /// guarantee: no-fail
      EventModule (const std::string& name);


      /// effects: standard destructor.
      /// guarantee: no-fail
      ~EventModule ();



      //
      // interface inherited from Algorithm
      //

    public:

      virtual StatusCode postFinalize (ModuleData& data) override;
      virtual StatusCode onFirstInputFile (ModuleData& data) override;
      virtual StatusCode onNextInputFile (ModuleData& data) override;
      virtual StatusCode onNewInputFile (ModuleData& data) override;
      virtual StatusCode postCloseInputFile (ModuleData& data) override;
      virtual StatusCode onExecute (ModuleData& data) override;

      //
      // private interface
      //

      /// description: the event structure used
    private:
      std::unique_ptr<xAOD::Event>   m_event; //!
      std::unique_ptr<xAOD::TStore>  m_store; //!
      std::unique_ptr<asg::SgEvent>  m_evtStore; //!

      /// description: whether we collect D3PDPerfStats statistics
    private:
      Gaudi::Property<bool> m_useStats {this, "useStats", false}; //!
      Gaudi::Property<bool> m_summaryReport {this, "summaryReport", true}; //!
    };
  }
}

#endif
