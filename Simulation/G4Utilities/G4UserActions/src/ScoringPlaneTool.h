/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4USERACTIONS_G4UA__SCORINGPLANETOOL_H
#define G4USERACTIONS_G4UA__SCORINGPLANETOOL_H

// Infrastructure includes
#include "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "ScoringPlane.h"


namespace G4UA
{

  /// @class ScoringPlaneTool
  /// @brief A tool to manage ScoprinPlane actions
  ///
  /// creates one ScoprinPlane instance per thread
  /// @author Andrea Di Simone
  ///
  class ScoringPlaneTool : public UserActionToolBase<ScoringPlane>
  {

    public:

      using UserActionToolBase<ScoringPlane>::UserActionToolBase;

    protected:

      /// creates the action instances
      virtual std::unique_ptr<ScoringPlane>
      makeAndFillAction(G4AtlasUserActions&) override final;

    private:

      /// holds the python configuration
      Gaudi::Property<double> m_plane{this, "Plane", 22600};
      Gaudi::Property<int> m_pkill{this, "PKill", 0};
      Gaudi::Property<std::string> m_fname{this, "FName", "ufo.root"};

  }; // class ScoringPlaneTool

} // namespace G4UA

#endif
