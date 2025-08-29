/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#ifndef EVENT_SELECTOR_JETNGHOSTSELECTORALG_H
#define EVENT_SELECTOR_JETNGHOSTSELECTORALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

// Framework includes
#include <xAODJet/JetContainer.h>
#include <xAODEventInfo/EventInfo.h>

#include <EventSelectionAlgorithms/SignEnums.h>

namespace CP {

  /// \brief an algorithm to select an event with a specified number
  /// of objects compared to a transverse momentum value

  class JetNGhostSelectorAlg final : public EL::AnaAlgorithm {

    /// \brief the standard constructor 
    public:
      JetNGhostSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);
      virtual StatusCode initialize() override;
      virtual StatusCode execute() override;

    private:

      /// \brief the pT threshold on which to select
      Gaudi::Property<float> m_ptmin {this, "minPt", 0., "minimum pT (in MeV)"};

      /// \brief the sign against which to compare pT (GT, LT, etc)
      Gaudi::Property<std::string> m_sign {this, "sign", "SetMe", "comparison sign to use"};

      /// \brief the count of events desired
      Gaudi::Property<int> m_count {this, "count", 0, "count value"};

      /// \brief the ghost decoration
      Gaudi::Property<std::string> m_ghost {
         this, "ghost", "", "name of the ghost decoration to look up for positive object counts"
      };

      /// \brief the ghost decoration
      Gaudi::Property<std::string> m_veto {
         this, "veto", "", "name of the ghost decoration to look up for negative object counts"
      };

      /// \brief the operator version of the comparison (>, <, etc)
      SignEnum::ComparisonOperator m_signEnum{};

      /// \brief the systematics list
      CP::SysListHandle m_systematicsList {this};

      /// \brief the jet input handle
      CP::SysReadHandle<xAOD::JetContainer> m_jetsHandle {
        this, "jets", "", "the particle container to use"
      };

      /// \brief the jet selection handle
      CP::SysReadSelectionHandle m_jetSelection {
        this, "jetSelection", "", "the selection on the input particles"
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
        this, "decorationName", "SetMe", "decoration name for the NGhosts selector"
      };

      /// \brief the ghost and ghost-veto accessors
      std::unique_ptr<SG::ConstAccessor<int>> m_ghostAcc{};
      std::unique_ptr<SG::ConstAccessor<int>> m_vetoAcc{};
      bool m_doVeto{false};
  }; // class
} // namespace CP

#endif // EVENT_SELECTOR_JETNGHOSTSELECTORALG_H
