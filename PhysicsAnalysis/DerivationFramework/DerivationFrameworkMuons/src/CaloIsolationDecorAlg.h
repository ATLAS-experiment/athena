/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef DERIVATIONFRAMEWORKMUONS_CALOISOLATIONDECORALG_H_
#define DERIVATIONFRAMEWORKMUONS_CALOISOLATIONDECORALG_H_

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteDecorHandleKey.h>
#include <xAODPrimitives/IsolationType.h>
#include <xAODTracking/TrackParticleContainer.h>
#include <StoreGate/ReadDecorHandleKeyArray.h>
#include <RecoToolInterfaces/ICaloTopoClusterIsolationTool.h>

/// Algorithm to decorate the calorimeter isolation variables to the track particles
namespace DerivationFramework {
class CaloIsolationDecorAlg : public AthReentrantAlgorithm {
public:
    /// Constructor with parameters:
    CaloIsolationDecorAlg(const std::string& name, ISvcLocator* pSvcLocator);

    /// Destructor:
    ~CaloIsolationDecorAlg() = default;

    /// Athena algorithm's Hooks
    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

private:
    /// Athena configured tools
    ToolHandle<xAOD::ICaloTopoClusterIsolationTool> m_isoTool{this, "IsolationTool", ""};
    xAOD::CaloCorrection m_calo_corr;

    /// track collection to decorate
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trk_key{this, "TrackCollection", "InDetTrackParticles"};

    /// pt threshold to apply
    Gaudi::Property<float> m_pt_min{this, "PtMin", 3. * Gaudi::Units::GeV};

    Gaudi::Property<std::string> m_customName{this, "customName", ""};

    /// Optional list of decorators to select only the good tracks for the isolation decoration. Only one decorator needs
    /// to pass to launch the isolation calculation
    Gaudi::Property<std::vector<std::string>> m_trkSel_Decors{this, "TrackSelections", {},
                                                  "List of decorator names of which one needs to be true to run the isolation" };
    SG::ReadDecorHandleKeyArray<xAOD::TrackParticleContainer> m_trkSel_keys{this, "SelectionKeys", {},
                                                                           "Will be overwritten in initialize"};

    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_topocone20_key{this, "TopoCone20Key", m_trk_key, "",
                                                                           "Will be overwritten in initialize"};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_topocone30_key{this, "TopoCone30Key", m_trk_key, "",
                                                                           "Will be overwritten in initialize"};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_topocone40_key{this, "TopoCone40Key", m_trk_key, "",
                                                                           "Will be overwritten in initialize"};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_corr_key{this, "CorrDecorKey", m_trk_key, "",
                                                                     "Will be overwritten in initialize"};

    std::vector<xAOD::Iso::IsolationType> m_calo_isos{xAOD::Iso::topoetcone40, xAOD::Iso::topoetcone30, xAOD::Iso::topoetcone20};
};
}
#endif /* CaloIsolationDecorAlg_H_ */
