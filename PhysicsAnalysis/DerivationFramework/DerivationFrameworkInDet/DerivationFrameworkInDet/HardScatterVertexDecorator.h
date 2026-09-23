/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_HARDSCATTERVERTEXDECORATOR_H
#define DERIVATIONFRAMEWORK_HARDSCATTERVERTEXDECORATOR_H

// Framework include(s):
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "GaudiKernel/ToolHandle.h"

// EDM include(s):
#include "xAODTracking/VertexContainerFwd.h"
#include "xAODEventInfo/EventInfo.h"

// Tool include(s):
#include "InDetRecToolInterfaces/IInDetHardScatterSelectionTool.h"

namespace DerivationFramework {

  class HardScatterVertexDecorator : public AthReentrantAlgorithm
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
    virtual StatusCode initialize() override final;

    /// Function decorating the inputs
    virtual StatusCode execute(const EventContext& ctx) const override final;

    /// @}

    ///////////////////////////////////////////////////////////////////
    // Private data:
    ///////////////////////////////////////////////////////////////////
  private:

    /// @name The properties that can be defined via the python job options
    /// @{

    /// ReadHandleKey for the input vertices
    SG::ReadHandleKey<xAOD::VertexContainer> m_vtxContKey{this, "VertexContainerName", "PrimaryVertices",
                                                          "Name of the input vertex container"};

    /// ToolHandle for the IInDetHardScatterSelectionTool
    ToolHandle<InDet::IInDetHardScatterSelectionTool> m_vtxSelectTool{this, "HardScatterSelectionTool", "",
                                                                      "IInDetHardScatterSelectionTool for selecting the hardscatter vertex" };

    /// xAOD::EventInfo ReadHandleKey
    SG::ReadHandleKey<xAOD::EventInfo> m_evtInfoKey {this, "EventInfo", "EventInfo", "EventInfo key"};

    /// WriteDecorHandleKey for the output hardscatter decoration (applied to xAOD::EventInfo)
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_evtDecoKey{this, "HardScatterDecoName", m_evtInfoKey, "hardScatterVertexLink",
                                              "Name of the hardscatter vertex decoration (applied to xAOD::EventInfo)"};

    /// @}

  }; // end: class HardScatterVertexDecorator
} // end: namespace DerivationFramework

#endif // end: DERIVATIONFRAMEWORK_HARDSCATTERVERTEXDECORATOR_H
