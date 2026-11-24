/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4DEBUGGINGTOOLS_G4UA__G4ATLANTISDUMPERTOOL_H
#define G4DEBUGGINGTOOLS_G4UA__G4ATLANTISDUMPERTOOL_H

#include "G4AtlasTools/UserActionToolBase.h"
#include "G4AtlantisDumper.h"

namespace G4UA
{

  class G4AtlantisDumperTool : public UserActionToolBase<G4AtlantisDumper>
  {

  public:

    using UserActionToolBase<G4AtlantisDumper>::UserActionToolBase;

  protected:

    virtual std::unique_ptr<G4AtlantisDumper>
    makeAndFillAction(G4AtlasUserActions&) override final;

  private:

    Gaudi::Property<double> m_tedep_cut{this, "Edep", 0.1};
    Gaudi::Property<double> m_length_cut{this, "Length", 1.0};
    Gaudi::Property<double> m_p_cut{this, "P", 10.0};
    Gaudi::Property<double> m_ke_cut{this, "KE", 10.0};

  }; // class G4AtlantisDumperTool

} // namespace G4UA

#endif
