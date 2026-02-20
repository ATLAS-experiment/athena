/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGDECISIONINTERFACE_ITRIGDECISIONTOOLLITE_H
#define TRIGDECISIONINTERFACE_ITRIGDECISIONTOOLLITE_H

// Framework include(s):
#include "AsgTools/IAsgTool.h"

#include <string>
#include <vector>

// Forward declarations
namespace HLT {
  class Identifier;
}

namespace TrigCompositeUtils {
  struct TypelessLinkInfo;
}

namespace TrigCompositeUtils {
  template<typename T>
  struct LinkInfo;
}

namespace Trig {
  class FeatureRequestDescriptor;
}

class EventContext;

namespace Trig {

   /**
    * @brief isPassed and features interfaces for the TrigDecisionToolLite
    *
    * Typed feature containers are obtained via a wrapper which is in-lined to this interface definition.
    * The concrete class is required to only implement the private type erased implementation.
    **/
   class ITrigDecisionToolLite : virtual public asg::IAsgTool {
     ASG_TOOL_INTERFACE(ITrigDecisionToolLite)

   public:
      /// Get the physics decision for a HLT trigger chain, by identifier
      virtual bool isPassed( const HLT::Identifier& chain, const EventContext& ctx ) const = 0;

      /// Get the physics decision for a HLT trigger chain, by string
      virtual bool isPassed( const std::string& chain, const EventContext& ctx ) const = 0;

      /// Get the physics decision for the OR of a number of HLT trigger chains, by identifier
      virtual bool isPassed( const std::vector<HLT::Identifier>& chains, const EventContext& ctx ) const = 0;

      /// Get the physics decision for the OR of a number of HLT trigger chains, by string
      virtual bool isPassed( const std::vector<std::string>& chains, const EventContext& ctx ) const = 0;

      /// Obtain features from the navigation, as specified in the supplied feature request descriptor
      template<class CONTAINER>
      std::vector<TrigCompositeUtils::LinkInfo<CONTAINER>> 
      features( const Trig::FeatureRequestDescriptor& frd, const EventContext& ctx = Gaudi::Hive::currentContext() ) const;

      /// Obtain features from the navigation for the supplied chain, using default values for all other feature request descriptor parameters
      template<class CONTAINER>
      std::vector<TrigCompositeUtils::LinkInfo<CONTAINER>> 
      features( const std::string& chain, const EventContext& ctx = Gaudi::Hive::currentContext() ) const;

    private:
      /// Internal type erased features call 
      virtual std::vector<TrigCompositeUtils::TypelessLinkInfo>
      typelessFeatures( const Trig::FeatureRequestDescriptor& frd, const CLID clid, const EventContext& ctx) const = 0;

      /// Internal call to obtain event store pointer from concrete tool implementation. Note that evtStore() is not declared under IAsgTool.
     virtual const asg::EventStoreType* getEventStore() const = 0;

   }; // class ITrigDecisionToolLite

} // namespace Trig

#include "ITrigDecisionToolLite.icc"

#endif // TRIGDECISIONINTERFACE_ITRIGDECISIONTOOLLITE_H
