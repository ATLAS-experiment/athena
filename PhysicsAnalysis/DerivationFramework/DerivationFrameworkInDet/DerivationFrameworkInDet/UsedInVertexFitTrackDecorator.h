/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// UsedInVertexFitTrackDecorator.h
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_USEDINVERTEXFITTRACKDECORATOR_H
#define DERIVATIONFRAMEWORK_USEDINVERTEXFITTRACKDECORATOR_H

// Framework include(s):
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// Tool include(s):
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "InDetRecToolInterfaces/IInDetUsedInFitTrackDecoratorTool.h"

// STL include(s):
#include <string>

namespace DerivationFramework {

  class UsedInVertexFitTrackDecorator : public extends<AthAlgTool, IAugmentationTool>
  {
    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
  public:

    /// @name Constructor
    /// @{

    using base_class::base_class;

    /// @}

    /// @name Function(s) implementing the AthAlgTool and IAugmentationTool interfaces
    /// @{

    /// Function initialising the tool
    StatusCode initialize();

    /// Function finalizing the tool
    StatusCode finalize();

    /// Function decorating the inputs
    virtual StatusCode addBranches(const EventContext& ctx) const;

    /// @}

    ///////////////////////////////////////////////////////////////////
    // Private data:
    ///////////////////////////////////////////////////////////////////
   private:

    /// @name The properties that can be defined via the python job options
    /// @{

    /// ToolHandle for the IInDetUsedInFitTrackDecoratorTool
    ToolHandle<InDet::IInDetUsedInFitTrackDecoratorTool> m_decoTool{this, "UsedInFitDecoratorTool", "InDet::InDetUsedInFitTrackDecoratorTool/IDUsedInFitDecoratorTool"};

    /// @}

  }; // end: class UsedInVertexFitTrackDecorator
} // end: namespace DerivationFramework

#endif // end: DERIVATIONFRAMEWORK_USEDINVERTEXFITTRACKDECORATOR_H
