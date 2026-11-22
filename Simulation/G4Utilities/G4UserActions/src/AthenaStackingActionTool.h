/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASALG_ATLASSTACKINGACTIONTOOL_H
#define G4ATLASALG_ATLASSTACKINGACTIONTOOL_H

// G4Atlas includes
#include "G4AtlasInterfaces/IUserActionTool.h"
#include "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "AthenaStackingAction.h"


namespace G4UA
{

  /// @class AthenaStackingActionTool
  /// @brief Tool which manages the AthenaStackingAction
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class AthenaStackingActionTool : public UserActionToolBase<AthenaStackingAction>
  {

    public:

      using UserActionToolBase<AthenaStackingAction>::UserActionToolBase;

      /// Initialize the tool
      virtual StatusCode initialize() override final;

    protected:

      // Setup the user action for current thread
      virtual std::unique_ptr<AthenaStackingAction>
      makeAndFillAction(G4AtlasUserActions& actionLists) override final;

    private:

      /// Configuration parameters
      Gaudi::Property<bool> m_killAllNeutrinos{this, "KillAllNeutrinos", false,
          "Toggle killing of all neutrinos"};
      Gaudi::Property<double> m_photonEnergyCut{this, "PhotonEnergyCut", -1.,
          "Energy threshold for tracking photons"};
      Gaudi::Property<bool> m_applyNRR{this, "ApplyNRR", false,
          "Apply the Neutron Russian Roulette"};
      Gaudi::Property<double> m_russianRouletteNeutronThreshold{this, "NRRThreshold", -1.,
          "Energy threshold for the Neutron Russian Roulette"};
      Gaudi::Property<double> m_russianRouletteNeutronWeight{this, "NRRWeight", -1.,
          "Weight for the Neutron Russian Roulette"};
      Gaudi::Property<bool> m_applyPRR{this, "ApplyPRR", false,
          "Apply the Photon Russian Roulette"};
      Gaudi::Property<double> m_russianRoulettePhotonThreshold{this, "PRRThreshold", -1.,
          "Energy threshold for the Photon Russian Roulette"};
      Gaudi::Property<double> m_russianRoulettePhotonWeight{this, "PRRWeight", -1.,
          "Weight for the Photon Russian Roulette"};
      Gaudi::Property<bool> m_isISFJob{this, "IsISFJob", false, ""};

      /// Use the debug version of the stacking action
      Gaudi::Property<bool> m_useDebugAction{this, "UseDebugAction", false};

  }; // class AthenaStackingActionTool

} // namespace G4UA

#endif
