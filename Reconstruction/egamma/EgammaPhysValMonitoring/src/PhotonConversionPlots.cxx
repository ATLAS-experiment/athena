/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PhotonConversionPlots.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODEgamma/PhotonxAODHelpers.h"

namespace Egamma {
    PhotonConversionPlots::PhotonConversionPlots(PlotBase* pParent, const std::string& sDir, const std::string& sParticleType):
        PlotBase(pParent, sDir),
        m_sParticleType(sParticleType),
        m_nVtx(nullptr),
        m_convR(nullptr),
        m_convZ(nullptr),
        m_convRvsEta(nullptr),
        m_convRvsType(nullptr),
        m_convType(nullptr),
        m_convDeltaEta(nullptr),
        m_convDeltaPhi(nullptr)
    {}

    void PhotonConversionPlots::initializePlots() {
        m_nVtx = Book1D("nVtx", "Number of vertices; Number of conversion vertices; Entries", 10, 0., 10);
        m_convR = Book1D("convR", "Conversion radius; Conversion radius (mm); Entries", 100, 0., 1000);
        m_convZ = Book1D("convZ", "Conversion z; Conversion z (mm); Entries", 100, -1000., 1000);
        m_convRvsEta = Book2D("convRvsEta", "Conversion radius vs #eta; #eta; Conversion radius (mm)", 1000, -5., 5., 100, 0., 1000);
        m_convRvsType = Book2D("convRvsType", "Conversion radius vs type; Type; Conversion radius (mm)", 10, 0., 10., 100, 0., 1000);
        m_convType = Book1D("convType", "Conversion type; Conversion type; Entries", 10, 0., 10);
        m_convDeltaEta = Book1D("convDeltaEta", "Conversion #Delta#eta; Conversion #Delta#eta; Entries", 250, -0.5, 0.5);
        m_convDeltaPhi = Book1D("convDeltaPhi", "Conversion #Delta#phi; Conversion #Delta#phi; Entries", 250, -0.5, 0.5);
    }

    void PhotonConversionPlots::fill(const xAOD::Photon& photon, const xAOD::EventInfo& eventInfo) {
        const float weight = eventInfo.beamSpotWeight();

        m_nVtx->Fill(photon.nVertices(), weight);
        const auto* vertex = photon.vertex();

        const float vtxRad = xAOD::EgammaHelpers::conversionRadius(&photon);
        m_convR->Fill(vtxRad, weight);
        m_convZ->Fill(vertex ? vertex->position().z() : -9999., weight);
        m_convRvsEta->Fill(photon.eta(), vtxRad, weight);
        m_convType->Fill(xAOD::EgammaHelpers::conversionType(&photon));
        m_convRvsType->Fill(xAOD::EgammaHelpers::conversionType(&photon), vtxRad, weight);

        float cnvDeltaEta1;
        float cnvDeltaPhi1;
        photon.vertexCaloMatchValue(cnvDeltaEta1, xAOD::EgammaParameters::convMatchDeltaEta1);
        photon.vertexCaloMatchValue(cnvDeltaPhi1, xAOD::EgammaParameters::convMatchDeltaPhi1);
        m_convDeltaEta->Fill(cnvDeltaEta1, weight); 
        m_convDeltaPhi->Fill(cnvDeltaPhi1, weight);

    }
}