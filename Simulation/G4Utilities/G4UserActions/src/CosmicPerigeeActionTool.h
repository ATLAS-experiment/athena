/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4USERACTIONS_G4UA_COSMICPERIGEEACTIONTOOL_H
#define G4USERACTIONS_G4UA_COSMICPERIGEEACTIONTOOL_H

// Infrastructure includes
#include "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "CosmicPerigeeAction.h"

namespace G4UA
{

  /// @class CosmicPerigeeActionTool
  /// @brief Tool which manages the CosmicPerigeeAction action.
  ///
  /// Create the CosmicPerigeeAction for each worker thread
  ///
  /// @author Andrea Di Simone
  ///
  class CosmicPerigeeActionTool : public UserActionToolBase<CosmicPerigeeAction>
  {

    public:

      using UserActionToolBase<CosmicPerigeeAction>::UserActionToolBase;

    protected:

      /// Create action for this thread
      virtual std::unique_ptr<CosmicPerigeeAction>
      makeAndFillAction(G4AtlasUserActions&) override final;

    private:

      /// Configuration parameters
      Gaudi::Property<float> m_pMinPrimary{this, "pMinPrimary", 100*CLHEP::MeV};

  }; // class CosmicPerigeeActionTool

} // namespace G4UA

#endif
