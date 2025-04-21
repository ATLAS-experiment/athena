/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MuonTPExtrapolationAlg_H
#define MuonTPExtrapolationAlg_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "xAODMuon/MuonContainer.h"

namespace DerivationFramework{
class MuonTPExtrapolationAlg : public AthReentrantAlgorithm {
public:
    MuonTPExtrapolationAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MuonTPExtrapolationAlg() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
    /// run the extrapolation - only available in full athena
    std::unique_ptr<Trk::TrackParameters> extrapolateToTriggerPivotPlane(const EventContext& ctx,
                                                                         const xAOD::TrackParticle& track) const;

    // these define the surfaces that we extrapolate to.
    // We approximate the pivot plane in the form of a cylinder surface and two disks
    ToolHandle<Trk::IExtrapolator> m_extrapolator{this, "Extrapolator", ""};

    Gaudi::Property<float> m_endcapPivotPlaneZ{this, "EndcapPivotPlaneZ", 15525., "z position of pivot plane in endcap region"};
    Gaudi::Property<float> m_endcapPivotPlaneMinimumRadius{this, "EndcapPivotPlaneMinimumRadius", 0.,
                                                           "minimum radius of pivot plane in endcap region"};
    Gaudi::Property<float> m_endcapPivotPlaneMaximumRadius{this, "EndcapPivotPlaneMaximumRadius", 11977.,
                                                           "maximum radius of pivot plane in endcap region"};
    Gaudi::Property<float> m_barrelPivotPlaneRadius{this, "BarrelPivotPlaneRadius", 8000., "adius of pivot plane in barrel region"};
    Gaudi::Property<float> m_barrelPivotPlaneHalfLength{this, "BarrelPivotPlaneHalfLength", 9700,
                                                        "half length of pivot plane in barrel region"};

    /// Particle container to decorate the Pivot plane coordinates to
    SG::ReadHandleKey<xAOD::IParticleContainer> m_partKey{this, "ContainerKey", "RandomParticle"};

    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_extEtaKey{this, "Key_ExtrTP_Eta",  m_partKey, "EtaTriggerPivot"};
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_extPhiKey{this, "Key_ExtrTP_Phi",  m_partKey, "PhiTriggerPivot"};
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_extStatKey{this, "Key_ExtrTP_Status",  m_partKey, "DecoratedPivotEtaPhi"};

    /// Optional list of decorators to select only the good tracks for the isolation decoration. Only one decorator needs
    /// to pass to launch the isolation calculation
    Gaudi::Property<float> m_ptMin{this, "PtMin", 2.5 * Gaudi::Units::GeV, 
                                  "Minimal track pt required to decorate the ID track"};
    Gaudi::Property<std::vector<std::string>> m_trkSelDecors{this, "TrackSelections", {},
                                                  "List of decorator names of which one needs to be true to run the isolation" };
    
    SG::ReadDecorHandleKeyArray<xAOD::IParticleContainer> m_trkSelKeys{this, "SelectionKeys", {}, "Will be overwritten in initialize"};


};
}
#endif
