/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef EVENT_LOOP_TEVENT_SVC_H
#define EVENT_LOOP_TEVENT_SVC_H

#include <EventLoop/Global.h>

#include <AsgTools/PropertyWrapper.h>
#include <EventLoop/Module.h>
#include <memory>

namespace asg
{
  class SgTEvent;
}

namespace xAOD
{
  class TEvent;
  class TStore;
}

namespace EL
{
  namespace Detail
  {
    class TEventModule : public Module
    {
      //
      // public interface
      //

    public:

      /// effects: standard constructor.
      /// guarantee: no-fail
      TEventModule (const std::string& name);


      /// effects: standard destructor.
      /// guarantee: no-fail
      ~TEventModule ();



      //
      // interface inherited from Algorithm
      //

    public:

      virtual StatusCode onInitialize (ModuleData& data) override;
      virtual StatusCode postFinalize (ModuleData& data) override;
      virtual StatusCode onNewInputFile (ModuleData& data) override;
      virtual StatusCode postCloseInputFile (ModuleData& data) override;
      virtual StatusCode onExecute (ModuleData& data) override;



      //
      // private interface
      //

      /// description: the event structure used
    private:
      std::unique_ptr<xAOD::TEvent> m_event; //!
      std::unique_ptr<xAOD::TStore> m_store; //!
      std::unique_ptr<asg::SgTEvent> m_evtStore; //!

      /// description: whether we collect D3PDPerfStats statistics
    private:
      Gaudi::Property<bool> m_useStats {this, "useStats", false}; //!

      Gaudi::Property<std::string> m_modeStr {this, "accessMode", ""}; //!
      Gaudi::Property<bool> m_summaryReport {this, "summaryReport", true}; //!
    };
  }
}

#endif
