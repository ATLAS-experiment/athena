/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  Xin Chen <xin.chen@cern.ch>
*/
#ifndef JPSIPLUSETACSINGLEVERTEX_H
#define JPSIPLUSETACSINGLEVERTEX_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include <AsgTools/PropertyWrapper.h>
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include "xAODTracking/VertexContainer.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "InDetConversionFinderTools/VertexPointEstimator.h"
#include <vector>

namespace Trk {
    class TrkVKalVrtFitter;
    class V0Tools;
    class ParticleDataTable;
}

class IBeamCondSvc;

namespace DerivationFramework {

  static const InterfaceID IID_JpsiPlusEtacSingleVertex("JpsiPlusEtacSingleVertex", 1, 0);

  class JpsiPlusEtacSingleVertex : virtual public AthAlgTool, public IAugmentationTool
  {
  public:
    static const InterfaceID& interfaceID() { return IID_JpsiPlusEtacSingleVertex;}
    JpsiPlusEtacSingleVertex(const std::string& t, const std::string& n, const IInterface* p);
    virtual ~JpsiPlusEtacSingleVertex() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode addBranches() const override;

  private:
    std::string m_outputKey;
    std::string m_vertexContainerKey;
    std::vector<std::string> m_vertexJpsiHypoNames;
    Gaudi::Property<std::string> m_VxPrimaryCandidateName{this, "VxPrimaryCandidateName", "PrimaryVertices", "Name of primary vertex container"};
    Gaudi::Property<std::string> m_trackContainerName{this, "TrackContainerName", "InDetTrackParticles", "Name of Inner Detector TrackParticles container"};
    Gaudi::Property<double> m_trkMinPt1{this, "TrackMinPtTrk1", 2500., "1st leading track pt from eta_c"};
    Gaudi::Property<double> m_trkMinPt2{this, "TrackMinPtTrk2", 2000., "2nd leading track pt from eta_c"};
    Gaudi::Property<double> m_trkMinPt3{this, "TrackMinPtTrk3", 1500., "3rd leading track pt from eta_c"};
    Gaudi::Property<double> m_trkMinPt4{this, "TrackMinPtTrk4", 1000., "4th leading track pt from eta_c"};
    double m_jpsiMassLower;
    double m_jpsiMassUpper;
    double m_rho1MassLower;
    double m_rho1MassUpper;
    double m_rho2MassLower;
    double m_rho2MassUpper;
    double m_etacMassLower;
    double m_etacMassUpper;
    double m_MassLower;
    double m_MassUpper;
    Gaudi::Property<double> m_vtx0Daug1MassHypo{this, "Vtx0Daug1MassHypo", -1., "mass of 1st daughter from vertex 0 of Jpsi"};
    Gaudi::Property<double> m_vtx0Daug2MassHypo{this, "Vtx0Daug2MassHypo", -1., "mass of 2nd daughter from vertex 0 of Jpsi"};
    Gaudi::Property<double> m_vtx1Daug1MassHypo{this, "Vtx1Daug1MassHypo", -1., "mass of 1st daughter from subvertex 1 of eta_c"};
    Gaudi::Property<double> m_vtx1Daug2MassHypo{this, "Vtx1Daug2MassHypo", -1., "mass of 2nd daughter from subvertex 1 of eta_c"};
    Gaudi::Property<double> m_vtx2Daug1MassHypo{this, "Vtx2Daug1MassHypo", -1., "mass of 1st daughter from subvertex 2 of eta_c"};
    Gaudi::Property<double> m_vtx2Daug2MassHypo{this, "Vtx2Daug2MassHypo", -1., "mass of 2nd daughter from subvertex 2 of eta_c"};
    Gaudi::Property<size_t> m_maxCandidates{this, "MaxCandidates", 1000, "Maximum number of eta_c candidates"};
    Gaudi::Property<bool> m_ptOrdering{this, "PtOrdering", true, "Order TQ candidates by pt"};
    Gaudi::Property<double> m_maxDR{this, "MaxDR", 0.6, "Maximum DeltaR between Jpsi and tracks from eta_c"};
    Gaudi::Property<double> m_mass_jpsi{this, "JpsiMass", -1., "mass of Jpsi"};
    Gaudi::Property<double> m_mass_rho1{this, "Rho1Mass", -1., "mass of 1st rho"};
    Gaudi::Property<double> m_mass_rho2{this, "Rho2Mass", -1., "mass of 2nd rho"};
    Gaudi::Property<double> m_mass_etac{this, "EtacMass", -1., "mass of eta_c"};
    bool   m_constrJpsi;
    bool   m_constrRho1;
    bool   m_constrRho2;
    bool   m_constrEtac;
    double m_chi2cut_jpsi;
    double m_chi2cut_rho;
    double m_chi2cut;

    ServiceHandle<IBeamCondSvc>                      m_beamSpotSvc;
    ToolHandle < Trk::TrkVKalVrtFitter >             m_iVertexFitter;
    ToolHandle < Analysis::PrimaryVertexRefitter >   m_pvRefitter;
    ToolHandle < Trk::V0Tools >                      m_V0Tools;
    ToolHandle < Trk::ITrackSelectorTool >           m_trkSelector;
    ToolHandle < InDet::VertexPointEstimator >       m_vertexEstimator;

    std::string m_refPVContainerName;
    Gaudi::Property<bool> m_refitPV{this, "RefitPV", true, "If refit PVs"};
    Gaudi::Property<std::string> m_hypoName{this, "HypothesisName", "TQ", "Hypothesis name of TQ vertex"};
    Gaudi::Property<int> m_PV_max{this, "MaxnPV", 100, "Maximum number of associated PVs"};
    Gaudi::Property<int> m_DoVertexType{this, "DoVertexType", 7, "Bitcode for types of associated PVs"};
    Gaudi::Property<size_t> m_PV_minNTracks{this, "MinNTracksInPV", 0, "Minimum number of tracks in PVs"};

    std::unique_ptr<xAOD::Vertex> fitTwoTracks(const xAOD::TrackParticle* track1, const xAOD::TrackParticle* track2) const;
    double DR(double eta1, double phi1, double eta2, double phi2) const;
  };
}

#endif
