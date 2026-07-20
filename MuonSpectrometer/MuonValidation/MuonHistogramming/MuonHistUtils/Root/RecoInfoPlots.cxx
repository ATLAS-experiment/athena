/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHistUtils/RecoInfoPlots.h"

#include "AthContainers/ConstAccessor.h"
#include "MuonHistUtils/MuonEnumDefs.h"

namespace Muon {

RecoInfoPlots::RecoInfoPlots(PlotBase* pParent, const std::string& sDir)
    : PlotBase(pParent, sDir),
      m_oTrkRecoInfoPlots(this, "", "IDTrk"),
      m_oMSTrkRecoInfoPlots(this, "", "MSTrk"),
      m_oRecoInfoPlots(this, "") {}

void RecoInfoPlots::initializePlots() {
    constexpr auto nAuthors = static_cast<int>(xAOD::Muon::Author::NumberOfMuonAuthors);
    author = Book1D("author", "author;primary author;Entries",
                    nAuthors, -0.5, nAuthors - 0.5);
    all_authors = Book1D("AllAuthors", "AllAuthors;all authors;Entries",
                         nAuthors, -0.5, nAuthors - 0.5);
    muonType = Book1D("muonType", "muonType;muonType;Entries", 6, -0.5, 5.5);
    quality = Book1D("quality", "quality;quality;Entries", 4, -0.5, 3.5);
    quality_cutflow = Book1D("quality_cutflow",
                             "quality cut flow;quality;Entries", 4, -0.5, 3.5);

    // set labels
    for (int i = 1; i <= author->GetNbinsX(); i++) {
        author->GetXaxis()->SetBinLabel(i, xAOD::Muon::toString(static_cast<xAOD::Muon::Author>(author->GetBinCenter(i))).data() );
        all_authors->GetXaxis()->SetBinLabel(i, xAOD::Muon::toString(static_cast<xAOD::Muon::Author>(author->GetBinCenter(i))).data());
    }
    for (int i = 1; i <= muonType->GetNbinsX(); i++) {
        muonType->GetXaxis()->SetBinLabel( i, xAOD::Muon::toString(static_cast<xAOD::Muon::MuonType>(muonType->GetBinCenter(i))).data());
    }

    for (int i = 1; i <= quality->GetNbinsX(); i++) {
        int iQuality = quality->GetBinCenter(i);
        const auto sQuality = xAOD::Muon::toString(static_cast<xAOD::Muon::MuonType>(iQuality));
           
        quality->GetXaxis()->SetBinLabel(i, sQuality.data());
        quality_cutflow->GetXaxis()->SetBinLabel(quality->GetNbinsX() - i + 1,  sQuality.data());
    }
}

void RecoInfoPlots::fill(const xAOD::Muon& mu, float weight) {

    const xAOD::TrackParticle* primaryTrk = mu.trackParticle(xAOD::Muon::TrackParticleType::Primary);
    m_oRecoInfoPlots.fill(*primaryTrk, weight);
   
    const xAOD::TrackParticle* inDetTrk =  mu.trackParticle(xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle);
    if (inDetTrk) {
        m_oTrkRecoInfoPlots.fill(*inDetTrk, weight);
    }

    const xAOD::TrackParticle* msExtrapTrk =  mu.trackParticle(xAOD::Muon::TrackParticleType::ExtrapolatedMuonSpectrometerTrackParticle);
    if (!msExtrapTrk) {
        msExtrapTrk =  mu.trackParticle(xAOD::Muon::TrackParticleType::MuonSpectrometerTrackParticle);
    }
    if (msExtrapTrk) {
        m_oMSTrkRecoInfoPlots.fill(*msExtrapTrk, weight);
    }

    author->Fill(static_cast<int>(mu.author()), weight);
    for (int i = 1; i <= all_authors->GetNbinsX(); ++i) {
        if (mu.allAuthors() & (1 << (i-1))) {
            all_authors->Fill(i, weight);
        }
    }
    muonType->Fill(static_cast<int>(mu.muonType()), weight);

    xAOD::Muon::Quality muqual = mu.quality();
    quality->Fill(static_cast<int>(muqual), weight);
    for (int i = 1; i < quality_cutflow->GetNbinsX(); i++) {
        if (static_cast<int>(muqual) <= i){
            quality_cutflow->Fill(i, weight);
        }
    }
}

}  // namespace Muon
