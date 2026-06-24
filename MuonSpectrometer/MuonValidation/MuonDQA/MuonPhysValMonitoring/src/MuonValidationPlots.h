/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONPHYSVALMONITORING_MUONVALIDATIONPLOTS_H
#define MUONPHYSVALMONITORING_MUONVALIDATIONPLOTS_H

#include "MuonHistUtils/RecoMuonPlotOrganizer.h"
#include "MuonHistUtils/TruthMuonPlotOrganizer.h"
#include "MuonHistUtils/TruthRelatedMuonPlotOrganizer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODMuon/Muon.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTruth/TruthParticle.h"

class MuonValidationPlots : public PlotBase {
public:
    MuonValidationPlots(PlotBase* pParent, const std::string& sDir, std::set<int> wps, std::set<int> authors, bool isData,
                        bool doBinnedResolutionPlots, bool doSplitSAFMuons);

    virtual ~MuonValidationPlots();
    void fill(const xAOD::Muon& mu, float weight = 1.0);
    bool isGoodTruthTrack(const xAOD::TruthParticle& truthMu);
    void fill(const xAOD::TruthParticle& truthMu, float weight = 1.0);
    void fill(const xAOD::TruthParticle* truthMu, const xAOD::Muon* mu, const xAOD::TrackParticleContainer* MSTracks, float weight = 1.0);
 
    std::vector<int> m_selectedWPs;
    std::vector<unsigned int> m_selectedAuthors;
    std::vector<std::string> m_truthSelections;

    std::unique_ptr<Muon::RecoMuonPlotOrganizer> m_oRecoMuonPlots;
    std::unique_ptr<Muon::TruthRelatedMuonPlotOrganizer> m_oTruthRelatedMuonPlots;
    std::vector<std::unique_ptr<Muon::RecoMuonPlotOrganizer>> m_oRecoMuonPlots_perQuality;
    std::vector<std::unique_ptr<Muon::RecoMuonPlotOrganizer>> m_oRecoMuonPlots_perAuthor;
    std::vector<std::unique_ptr<Muon::TruthRelatedMuonPlotOrganizer>> m_oTruthRelatedMuonPlots_perQuality;
    std::vector<std::unique_ptr<Muon::TruthRelatedMuonPlotOrganizer>> m_oTruthRelatedMuonPlots_perAuthor;
    std::vector<std::unique_ptr<Muon::TruthMuonPlotOrganizer>> m_oTruthMuonPlots;

    std::vector<std::unique_ptr<Muon::TruthRelatedMuonPlotOrganizer>> m_oTruthRelatedMuonPlots_SiAssocFwrdMu;
    std::vector<std::unique_ptr<Muon::RecoMuonPlotOrganizer>> m_oRecoMuonPlots_SiAssocFwrdMu;

private:
    void fillRecoMuonPlots(const xAOD::Muon& mu, float weight = 1.0);
    void fillTruthMuonPlots(const xAOD::TruthParticle& truthMu, float weight = 1.0);

    bool m_isData;
    bool m_doSeparateSAFMuons;
};

#endif
