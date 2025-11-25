/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Reco_4mu.h
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_Reco_4mu_H
#define DERIVATIONFRAMEWORK_Reco_4mu_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "FourMuonTool.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include "xAODBPhys/BPhysHelper.h"

/** forward declarations
 */
namespace Trk {
  class V0Tools;
}

namespace xAOD {
  class BPhysHypoHelper;
}

/** THE reconstruction tool
 */
namespace DerivationFramework {

  class Reco_4mu : public extends<AthAlgTool, IAugmentationTool> {
  public:
    Reco_4mu(const std::string& t, const std::string& n, const IInterface* p);

    virtual StatusCode initialize() override final;

    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    /** tools
     */
    void ProcessVertex(xAOD::BPhysHypoHelper&, xAOD::BPhysHelper::pv_type, std::vector<double> trackMasses) const;
    PublicToolHandle<Trk::V0Tools> m_v0Tools{this, "V0Tools", "Trk::V0Tools"};
    ToolHandle<DerivationFramework::FourMuonTool> m_fourMuonTool{this, "FourMuonTool", "DerivationFramework::FourMuonTool"};
    ToolHandle<Analysis::PrimaryVertexRefitter> m_pvRefitter{this, "PVRefitter", "Analysis::PrimaryVertexRefitter"};

    /** job options
     */
    SG::WriteHandleKey<xAOD::VertexContainer> m_pairName{this, "PairContainerName", "Pairs"};
    SG::WriteHandleKey<xAOD::VertexContainer> m_quadName{this, "QuadrupletContainerName", "Quadruplets"};
    SG::ReadHandleKey<xAOD::VertexContainer> m_pvContainerName{this, "PVContainerName", "PrimaryVertices"};
    SG::WriteHandleKey<xAOD::VertexContainer> m_refPVContainerName{this, "RefPVContainerName", "RefittedPrimaryVertices"};
    Gaudi::Property<bool> m_refitPV{this, "RefitPV", false};
    Gaudi::Property<int> m_PV_max{this, "MaxPVrefit", 1};
    Gaudi::Property<int> m_DoVertexType{this, "DoVertexType", 1};
  };
}

#endif // DERIVATIONFRAMEWORK_Reco_4mu_H
