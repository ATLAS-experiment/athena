/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#ifndef EVENT_SELECTOR_EVENTSCALARSELECTORALG_H
#define EVENT_SELECTOR_EVENTSCALARSELECTORALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>

// Framework includes
#include <xAODEventInfo/EventInfo.h>

#include <EventSelectionAlgorithms/SignEnums.h>

namespace CP {

  /// \brief an algorithm to cut on a scalar quantity decorating EventInfo
  /// (e.g. a DNN/BDT classifier discriminant), comparing it to a threshold
  /// with a configurable sign operator. The stored type is configurable: the
  /// Python configuration populates exactly one of the typed handles below.

  class EventScalarSelectorAlg final : public EL::AnaAlgorithm {

    public:
      EventScalarSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);
      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) override;

    private:

      /// \brief the comparison sign (GT, LT, etc)
      Gaudi::Property<std::string> m_sign {this, "sign", "SetMe", "comparison sign to use"};

      /// \brief the reference value against which to compare
      Gaudi::Property<float> m_refValue {this, "refValue", 0., "reference value to compare against"};

      /// \brief the operator version of the comparison (>, <, etc)
      SignEnum::ComparisonOperator m_signEnum{};

      /// \brief the systematics list
      CP::SysListHandle m_systematicsList {this};

      /// \brief the event info handle
      CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
        this, "eventInfo", "EventInfo", "the EventInfo container to read from"
      };

      /// \brief the EventInfo scalar decoration to cut on; exactly one of the
      /// typed handles below is configured, according to the stored type
      CP::SysReadDecorHandle<float> m_floatVariable {
        this, "floatVariable", "", "EventInfo float decoration to cut on"
      };
      CP::SysReadDecorHandle<int> m_intVariable {
        this, "intVariable", "", "EventInfo int decoration to cut on"
      };
      CP::SysReadDecorHandle<double> m_doubleVariable {
        this, "doubleVariable", "", "EventInfo double decoration to cut on"
      };

      /// \brief the preselection
      CP::SysReadSelectionHandle m_preselection {
        this, "eventPreselection", "SetMe", "name of the preselection to check before applying this one"
      };

      /// \brief the output selection decoration
      CP::SysWriteSelectionHandle m_decoration {
        this, "decorationName", "SetMe", "decoration name for the EVENTVAR selector"
      };

  }; // class
} // namespace CP

#endif // EVENT_SELECTOR_EVENTSCALARSELECTORALG_H
