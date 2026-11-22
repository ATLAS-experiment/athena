/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4USERACTIONS_G4UA__RADLENGTHACTIONTOOL_H
#define G4USERACTIONS_G4UA__RADLENGTHACTIONTOOL_H

// Infrastructure includes
#include "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "RadLengthAction.h"


namespace G4UA{

  /// @class RadLengthActionTool
  /// @brief A tool to manage RadLengthAction actions
  ///
  /// creates one RadLengthAction instance per thread

  class RadLengthActionTool: public UserActionToolBase<RadLengthAction>
  {

  public:

    using UserActionToolBase<RadLengthAction>::UserActionToolBase;

  protected:

    /// creates the action instances
    virtual std::unique_ptr<RadLengthAction>
    makeAndFillAction(G4AtlasUserActions&) override final;

  private:

    /// holds the python configuration
    Gaudi::Property<int> m_volumeDepthLevel{this, "VolumeDepthLevel", 1, "Depth of volume, how many daughters should be taken into account"};

  }; // class RadLengthActionTool

} // namespace G4UA
#endif
