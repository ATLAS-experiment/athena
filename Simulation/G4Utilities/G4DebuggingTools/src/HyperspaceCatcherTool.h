/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4DEBUGGINGTOOLS_G4UA__HYPERSPACECATCHERTOOL_H
#define G4DEBUGGINGTOOLS_G4UA__HYPERSPACECATCHERTOOL_H

#include "G4AtlasTools/UserActionToolBase.h"
#include "HyperspaceCatcher.h"

namespace G4UA
{

  /// Tool which creates and manages the HyperspaceCatcher user action.
  ///
  class HyperspaceCatcherTool : public UserActionToolBase<HyperspaceCatcher>
  {

  public:

    using UserActionToolBase<HyperspaceCatcher>::UserActionToolBase;

  protected:

    virtual std::unique_ptr<HyperspaceCatcher>
    makeAndFillAction(G4AtlasUserActions&) override final;

  private:

    Gaudi::Property<int> m_treatmentLevel{this, "TreatmentLevel", 0};
    Gaudi::Property<int> m_killAfter{this, "KillAfter", 0};

  }; // class HyperspaceCatcherTool

} // namespace G4UA

#endif
