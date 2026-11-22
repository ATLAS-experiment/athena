/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4USERACTIONS_G4UA_LOOPERKILLERTOOL_H
#define G4USERACTIONS_G4UA_LOOPERKILLERTOOL_H

// Infrastructure includes
#include "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "LooperKiller.h"

namespace G4UA
{

  /// @class LooperKillerTool
  /// @brief Tool which manages the LooperKiller action.
  ///
  /// Create the LooperKiller for each worker thread
  ///
  /// @author Andrea Di Simone
  ///
  class LooperKillerTool : public UserActionToolBase<LooperKiller>
  {

    public:

      using UserActionToolBase<LooperKiller>::UserActionToolBase;

      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;

    protected:

      /// Create action for this thread
      virtual std::unique_ptr<LooperKiller>
      makeAndFillAction(G4AtlasUserActions&) override final;

    private:

      /// Configuration parameters
      Gaudi::Property<int> m_maxSteps{this, "MaxSteps", 1000000};
      Gaudi::Property<int> m_printSteps{this, "PrintSteps", 100};
      Gaudi::Property<int> m_verboseLevel{this, "VerboseLevel", 1};
      Gaudi::Property<bool> m_abortEvent{this, "AbortEvent", true};
      Gaudi::Property<bool> m_setError{this, "SetError", false};
      Gaudi::Property<bool> m_bsmOnly{this, "BSM_Only", false};

  }; // class LooperKillerTool

} // namespace G4UA

#endif
