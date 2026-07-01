/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef ASG_ANALYSIS_ALGORITHMS__EVENT_DECORATOR_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__EVENT_DECORATOR_ALG_H

#include <xAODEventInfo/EventInfo.h>
#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgTools/PropertyWrapper.h>
#include <map>
#include <functional>
#include <cstdint> //for uint32_t

namespace CP
{
  /// \brief an algorithm for decorating EventInfo with constant values

  class EventDecoratorAlg final : public EL::AnaReentrantAlgorithm
  {
  public:

    /// \brief the standard constructor
    using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
    StatusCode initialize () override;
    StatusCode execute (const EventContext& ctx) const override;



  private:

    /// \brief the name of the event info object
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {this, "eventInfo", "EventInfo", "the input EventInfo object"};

    /// \brief the uint32_t decorations to add
    Gaudi::Property<std::map<std::string, uint32_t>> m_uint32Decorations {this, "uint32Decorations", {}, "the uint32_t decorations to add"};

    /// \brief the functions to add decorations
    std::vector<std::function<void(const xAOD::EventInfo&)>> m_decFunctions {};
  };
}

#endif
