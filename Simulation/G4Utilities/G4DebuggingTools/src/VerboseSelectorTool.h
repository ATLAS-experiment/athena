/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4DEBUGGINGTOOLS_G4UA__VERBOSESELECTORTOOL_H
#define G4DEBUGGINGTOOLS_G4UA__VERBOSESELECTORTOOL_H

#include "G4AtlasTools/UserActionToolBase.h"
#include "VerboseSelector.h"

namespace G4UA
{

  class VerboseSelectorTool : public UserActionToolBase<VerboseSelector>
  {

  public:
    using UserActionToolBase<VerboseSelector>::UserActionToolBase;
  protected:
    virtual std::unique_ptr<VerboseSelector>
    makeAndFillAction(G4AtlasUserActions&) override final;
  private:
    Gaudi::Property<int> m_targetEvent{this, "TargetEvent", -1};
    Gaudi::Property<int> m_targetTrack{this, "TargetTrack", -1};
    Gaudi::Property<int> m_verboseLevel{this, "VerboseLevel", 0};
    Gaudi::Property<int> m_verb{this, "Continue", 1};
    // properties for area check at step time
    Gaudi::Property<double> m_Xmin{this, "Xmin", 0};
    Gaudi::Property<double> m_Xmax{this, "Xmax", 0};
    Gaudi::Property<double> m_Ymin{this, "Ymin", 0};
    Gaudi::Property<double> m_Ymax{this, "Ymax", 0};
    Gaudi::Property<double> m_Zmin{this, "Zmin", 0};
    Gaudi::Property<double> m_Zmax{this, "Zmax", 0};
    // target pdgIDs of tracks to printout
    Gaudi::Property<std::vector<double>> m_targetPdgIDs{this, "TargetPdgIDs", {}};
  }; // class VerboseSelectorTool

} // namespace G4UA

#endif
