/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4AtlasAlg_AthenaTrackingActionTool_H
#define G4AtlasAlg_AthenaTrackingActionTool_H

// G4Atlas includes
#include "G4AtlasInterfaces/IUserActionTool.h"
#include "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "AthenaTrackingAction.h"

namespace G4UA
{

  /// @class AthenaTrackingActionTool
  /// @brief Tool which manages the AthenaTrackingAction
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class AthenaTrackingActionTool : public UserActionToolBase<AthenaTrackingAction>
  {

    public:

      /// Standard constructor
      AthenaTrackingActionTool(const std::string& type, const std::string& name,
                               const IInterface* parent);

      /// Initialize the tool (just for debugging printout)
      virtual StatusCode initialize() override;

    protected:

      /// Setup the user action for current thread
      virtual std::unique_ptr<AthenaTrackingAction>
      makeAndFillAction(G4AtlasUserActions& actionLists) override final;

    private:

      /// The saving level for secondaries.
      int m_secondarySavingLevel;

  }; // class AthenaTrackingActionTool

} // namespace G4UA

#endif
