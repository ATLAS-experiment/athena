/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4USERACTIONS_G4UA__FASTIDKILLERTOOL_H
#define G4USERACTIONS_G4UA__FASTIDKILLERTOOL_H

// Infrastructure includes
#include "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "FastIDKiller.h"

namespace G4UA
{

  /// @brief NEEDS DOCUMENTATION
  class FastIDKillerTool: public UserActionToolBase<FastIDKiller>
  {

    public:

      using UserActionToolBase<FastIDKiller>::UserActionToolBase;

      virtual StatusCode finalize() override;

    protected:

      /// Create the action for the current thread
      virtual std::unique_ptr<FastIDKiller>
      makeAndFillAction(G4AtlasUserActions&) override final;

    private:

      Gaudi::Property<float> m_R{this, "R", 1150., "ID outer radius"};
      Gaudi::Property<float> m_Z{this, "Z", 3490., "ID maximum Z coordinate"};
      Gaudi::Property<float> m_energyCut{this, "energyCut", 100000000., "Energy threshold"};
      Gaudi::Property<bool> m_isDalek{this, "isDalek", false, "Use FastIDKiller as DALEK (Domain Actuated Low Energy Killer)"};

  }; // class FastIDKillerTool

} // namespace G4UA

#endif
