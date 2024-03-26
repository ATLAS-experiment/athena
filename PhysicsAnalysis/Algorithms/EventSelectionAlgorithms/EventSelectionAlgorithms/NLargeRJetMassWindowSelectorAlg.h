/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#ifndef EVENT_SELECTOR_NLARGERJETMASSWINDOWSELECTORALG_H
#define EVENT_SELECTOR_NLARGERJETMASSWINDOWSELECTORALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

// Framework includes
#include <xAODBase/IParticleContainer.h>
#include <xAODEventInfo/EventInfo.h>

#include <EventSelectionAlgorithms/SignEnums.h>

namespace CP {

  /// \brief an algorithm to select an event with N large-R jets with a mass compared
  /// to a specified window of values "lowMass" and "highMass". Use "veto" to change
  /// the behaviour and instead veto these events.

  class NLargeRJetMassWindowSelectorAlg final : public EL::AnaAlgorithm {

    /// \brief the standard constructor
    public:
      NLargeRJetMassWindowSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);
      virtual StatusCode initialize() override;
      virtual StatusCode execute() override;

    private:
    
      /// \brief the upper limit of the Mass window
      Gaudi::Property<float> m_mupper {this, "highMass", 0., "Mass < HIGH (in MeV)"};

      /// \brief the lower limit of the Mass window
      Gaudi::Property<float> m_mlower {this, "lowMass", 0., "Mass > LOW (in MeV)"};

      /// \brief the sign against which to compare Mass (GT, LT, etc)
      Gaudi::Property<std::string> m_sign {this, "sign", "SetMe", "comparison sign to use"};

      /// \brief the operator version of the comparison (>, <, etc)
      SignEnum::ComparisonOperator m_signEnum;

      /// \brief whether to veto events instead of selecting them
      Gaudi::Property<bool> m_veto {this, "vetoMode", false, "switch to veto-mode"};

      /// \brief the count of events desired
      Gaudi::Property<int> m_count {this, "count", 0, "count value"};

      /// \brief the systematics
      CP::SysListHandle m_systematicsList {this};

      /// \brief the large-R jet handle
      CP::SysReadHandle<xAOD::IParticleContainer> m_ljetsHandle {
        this, "ljets", "", "the large-R jet container to use"
      };

      /// \brief the large-R jet selection
      CP::SysReadSelectionHandle m_ljetSelection {
        this, "ljetSelection", "", "the selection on the input large-R jets"
      };

      /// \brief the event info handle
      CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
        this, "eventInfo", "EventInfo", "the EventInfo container to read selection decisions from"
      };

      /// \brief the preselection
      CP::SysReadSelectionHandle m_preselection {
        this, "eventPreselection", "SetMe", "name of the preselection to check before applying this one"
      };

      /// \brief the output selection decoration
      CP::SysWriteSelectionHandle m_decoration {
        this, "decorationName", "SetMe", "decoration name for the NObjects by Mass Window selector"
      };

  }; // class
} // namespace CP

#endif // EVENT_SELECTOR_NLARGERJETMASSWINDOWSELECTORALG_H
