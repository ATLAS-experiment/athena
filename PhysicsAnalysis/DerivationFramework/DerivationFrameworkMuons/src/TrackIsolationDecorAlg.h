/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRACKISOLATIONDECORALG_H_
#define TRACKISOLATIONDECORALG_H_

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteDecorHandleKey.h>
#include <StoreGate/ReadDecorHandleKeyArray.h>
#include <xAODPrimitives/IsolationType.h>
#include <xAODTracking/VertexContainer.h>

#include <RecoToolInterfaces/ITrackIsolationTool.h>
#include <RecoToolInterfaces/IsolationCommon.h>

namespace DerivationFramework {
class TrackIsolationDecorAlg : public AthReentrantAlgorithm {
public:
    /// Constructor with parameters:
    TrackIsolationDecorAlg(const std::string& name, ISvcLocator* pSvcLocator);

    /// Destructor:
    ~TrackIsolationDecorAlg();

    /// Athena algorithm's Hooks
    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

private:
    static bool isSame(const xAOD::IParticle* a, const xAOD::IParticle* b);
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_idTrkKey{this, "IdTrackCollection", "InDetTrackParticles",
                                                                "Collection of track particles making up the isolation cones"};
    
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_toDeorTrkKey{this, "TrackCollection", "InDetTrackParticles",
                                                              "Collection of track particles to decorate the isolation onto"};
    SG::ReadHandleKey<xAOD::VertexContainer> m_vtx_key{this, "VertexCollection", "PrimaryVertices"};

    Gaudi::Property<std::string> m_customName{this, "customName", "", "Custom appendix of the isolation variables"};

    Gaudi::Property<float> m_pt_min{this, "PtMin", 2.5 * Gaudi::Units::GeV, "Minimal track pt required to decorate the ID track"};

    /// Optional list of decorators to select only the good tracks for the isolation decoration. Only one decorator needs
    /// to pass to launch the isolation calculation
    Gaudi::Property<std::vector<std::string>> m_trkSel_Decors{this, "TrackSelections", {},
                                                "List of decorator names of which one needs to be true to run the isolation" };
    SG::ReadDecorHandleKeyArray<xAOD::TrackParticleContainer> m_trkSel_keys{this, "SelectionKeys", {},
                                                                            "Will be overwritten in initialize"};
    
    /// Now let's come to the WriteDecorHandleKeys
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_ptcone20_key{this, "IsoPtCone20", m_toDeorTrkKey, ""};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_ptcone30_key{this, "IsoPtCone30", m_toDeorTrkKey, ""};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_ptcone40_key{this, "IsoPtCone40", m_toDeorTrkKey, ""};

    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_ptvarcone20_key{this, "IsoPtVarCone20", m_toDeorTrkKey, ""};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_ptvarcone30_key{this, "IsoPtVarCone30", m_toDeorTrkKey, ""};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_ptvarcone40_key{this, "IsoPtVarCone40", m_toDeorTrkKey, ""};

    ToolHandle<xAOD::ITrackIsolationTool> m_isoTool{this, "IsolationTool", ""};

    std::vector<xAOD::Iso::IsolationType> m_trk_iso_types{xAOD::Iso::ptcone40, xAOD::Iso::ptcone30, xAOD::Iso::ptcone20};
    xAOD::TrackCorrection m_trk_corr{};
};
}
#endif /* TRACKISOLATIONDECORALG_H_ */
