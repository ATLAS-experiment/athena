/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4USERACTIONS_G4UA__HITWRAPPERTOOL_H
#define G4USERACTIONS_G4UA__HITWRAPPERTOOL_H

// Infrastructure includes
#include  "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "HitWrapper.h"

namespace G4UA
{

  /// @brief A tool which manages the HitWrapper user action.
  class HitWrapperTool : public UserActionToolBase<HitWrapper>
  {

    public:

      using UserActionToolBase<HitWrapper>::UserActionToolBase;

    protected:

      /// Create the action for the current thread
      virtual std::unique_ptr<HitWrapper>
      makeAndFillAction(G4AtlasUserActions&) override final;

    private:

      Gaudi::Property<float> m_time{this, "Time", 25.};

  }; // class HitWrapperTool

} // namespace G4UA

#endif
