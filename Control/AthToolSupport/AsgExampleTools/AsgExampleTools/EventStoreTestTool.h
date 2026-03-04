// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ASGEXAMPLETOOLS_EVENTSTORETESTTOOL_H
#define ASGEXAMPLETOOLS_EVENTSTORETESTTOOL_H

// Local include(s).
#include "AsgExampleTools/IEventStoreTestTool.h"

// Athena include(s).
#include "AsgTools/AsgTool.h"
#include "AsgDataHandles/WriteHandleKey.h"
#include "xAODBase/IParticleContainer.h"

namespace asg {

   /// Tool testing some of the "event store functionality" of @c asg::AsgTool
   class EventStoreTestTool : virtual public IEventStoreTestTool,
                              public AsgTool {

   public:
      // Implement all non-trivial constructors
      ASG_TOOL_CLASS( EventStoreTestTool, IEventStoreTestTool )

      /// Constructor
      EventStoreTestTool( const std::string& toolName );

      /// Initialize
      virtual StatusCode initialize() override;

      /// @name Function(s) inherited from @c asg::IEventStoreTestTool
      /// @{

      /// Function performing (a) test(s) with the event store.
      virtual StatusCode performTest() const override;

      /// @}

   private:
      SG::WriteHandleKey<xAOD::IParticleContainer> m_containerKey{this, "ContainerName", "AsgTestIParticles"};

   }; // class EventStoreTestTool

} // namespace asg

#endif // ASGEXAMPLETOOLS_EVENTSTORETESTTOOL_H
