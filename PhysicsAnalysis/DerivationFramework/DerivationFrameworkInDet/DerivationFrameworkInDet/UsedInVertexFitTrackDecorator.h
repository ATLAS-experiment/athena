/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// UsedInVertexFitTrackDecorator.h
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_USEDINVERTEXFITTRACKDECORATOR_H
#define DERIVATIONFRAMEWORK_USEDINVERTEXFITTRACKDECORATOR_H

// Framework include(s):
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

// Tool include(s):
#include "InDetRecToolInterfaces/IInDetUsedInFitTrackDecoratorTool.h"

// STL include(s):
#include <string>

namespace DerivationFramework {

  class UsedInVertexFitTrackDecorator : public AthReentrantAlgorithm
  {
    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
  public:

    /// @name Constructor
    /// @{

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    /// @}

    /// @name Function(s) implementing the AthAlgTool and IAugmentationTool interfaces
    /// @{

    /// Function initialising the tool
    StatusCode initialize();

    /// Function finalizing the tool
    StatusCode finalize();

    /// Function decorating the inputs
    virtual StatusCode execute(const EventContext& ctx) const;

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
