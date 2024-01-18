/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#ifndef EVENT_SELECTOR_MISSINGETSELECTORALG_H
#define EVENT_SELECTOR_MISSINGETSELECTORALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

// Framework includes
#include <xAODMissingET/MissingETContainer.h>
#include <xAODEventInfo/EventInfo.h>

#include <EventSelectionAlgorithms/SignEnums.h>

namespace CP {

  /// \brief an algorithm to select an event with missing E_T above or below
  /// a specified value

  class MissingETSelectorAlg final : public EL::AnaAlgorithm {

    /// \brief the standard constructor
    public:
      MissingETSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);
      virtual StatusCode initialize() override;
      virtual StatusCode execute() override;

    private:
    
      /// \brief the MET threshold against which to compare
      Gaudi::Property<float> m_metref {this, "refMET", 0., "MET cut (in MeV)"};

      /// \brief the comparison (GT, LT, etc)
      Gaudi::Property<std::string> m_sign {this, "sign", "SetMe", "comparison sign to use"};

      /// \brief the operator version of the comparison (>, <, etc)
      SignEnum::ComparisonOperator m_signEnum;

      /// \brief the systematics
      CP::SysListHandle m_systematicsList {this};

      /// \brief the MET input handle
      CP::SysReadHandle<xAOD::MissingETContainer> m_metHandle {
        this, "met", "SetMe", "the MET container to use"
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
        this, "decorationName", "SetMe", "decoration name for the MET selector"
      };

  }; // class
} // namespace CP

#endif // EVENT_SELECTOR_MISSINGETSELECTORALG_H
