/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonValidationPlots.h"
#include "AthContainers/ConstAccessor.h"

#include <utility>

#include "MuonHistUtils/MuonEnumDefs.h"


MuonValidationPlots::MuonValidationPlots(PlotBase* pParent, const std::string& sDir, std::set<int> wps,
                                         std::set<int> authors, bool isData, bool doBinnedResolutionPlots,
                                         bool doSeparateSAFMuons) :
    PlotBase(pParent, sDir),
    m_selectedWPs(wps.begin(),wps.end()),
    m_selectedAuthors(authors.begin(), authors.end()),
    m_truthSelections(2, ""),
    m_oTruthRelatedMuonPlots(nullptr),
    m_isData(isData),
    m_doSeparateSAFMuons(doSeparateSAFMuons) {
    if (!m_isData) {
        m_truthSelections[0] = "all";           // no selection on truth muons (minimum selection is |eta|<2.5, pt>5 GeV, defined in
                                                // MuonPhysValMonitoringTool::handleTruthMuon()
        m_truthSelections[1] = "MSAcceptance";  // truth muons in MS acceptance (at least 4 associated hits in the MS)

        // histogram classes for all muons
        for (const auto& truthSelection : m_truthSelections) {
            m_oTruthMuonPlots.emplace_back(std::make_unique<Muon::TruthMuonPlotOrganizer>(this, "truth/" + truthSelection));
        }
        m_oTruthRelatedMuonPlots = std::make_unique<Muon::TruthRelatedMuonPlotOrganizer>(this, "matched/AllMuons", doBinnedResolutionPlots);  
    }

    std::vector<int> allPlotCategories(0);
    std::vector<int> selectedPlotCategories(0);
    for (unsigned int i = 0; i < Muon::MAX_RECOPLOTCLASS; i++) {
        allPlotCategories.emplace_back(i);
        if (i != Muon::MUON_CHARGEPARAM) selectedPlotCategories.emplace_back(i);
    }

    // histogram classes for all muons
    m_oRecoMuonPlots = std::make_unique<Muon::RecoMuonPlotOrganizer>(this, "reco/AllMuons", allPlotCategories);

    // define a histogram class for each of the selected muon qualities
    for (unsigned int i = 0; i < m_selectedWPs.size(); i++) {
        auto sQuality = xAOD::Muon::toString(static_cast<xAOD::Muon::Quality>(m_selectedWPs[i]));
        m_oRecoMuonPlots_perQuality.emplace_back(std::make_unique<Muon::RecoMuonPlotOrganizer>(
            this, std::format("reco/{:}", sQuality), (sQuality == "Medium" || sQuality == "Tight") ? allPlotCategories : selectedPlotCategories));

        if (!m_isData) {
            bool doBinnedPlots = false;
            if (sQuality == "Medium") doBinnedPlots = true;
            m_oTruthRelatedMuonPlots_perQuality.emplace_back(
                std::make_unique<Muon::TruthRelatedMuonPlotOrganizer>(this, std::format("matched/{:}", sQuality), doBinnedPlots));
        }
    }

    // define a histogram class for each of the selected muon authors (+one inclusive for all authors)
    for (unsigned int i = 0; i < m_selectedAuthors.size(); i++) {
        const auto author = static_cast<xAOD::Muon::Author>(m_selectedAuthors[i]);
        std::string sAuthor{xAOD::Muon::toString(author)};
        if (sAuthor == "CaloTag") sAuthor = "CaloTagTight";
        m_oRecoMuonPlots_perAuthor.emplace_back(
            std::make_unique<Muon::RecoMuonPlotOrganizer>(this, "reco/" + sAuthor, (sAuthor == "MuidCo") ? allPlotCategories : selectedPlotCategories));
        if (!m_isData)
            m_oTruthRelatedMuonPlots_perAuthor.emplace_back(
                std::make_unique<Muon::TruthRelatedMuonPlotOrganizer>(this, "matched/" + sAuthor, doBinnedResolutionPlots));
    }

    // define histogram class for loose CaloTag and append to author plots, not very nice workaround though
    for (unsigned int i = 0; i < m_selectedAuthors.size(); i++) {
        if (static_cast<xAOD::Muon::Author>(m_selectedAuthors[i]) == xAOD::Muon::Author::CaloTag) {  // found CaloTag in list, also do CaloTagLoose
            m_oRecoMuonPlots_perAuthor.emplace_back(std::make_unique<Muon::RecoMuonPlotOrganizer>(this, "reco/CaloTagLoose", selectedPlotCategories));
            if (!m_isData)
                m_oTruthRelatedMuonPlots_perAuthor.emplace_back(
                    std::make_unique<Muon::TruthRelatedMuonPlotOrganizer>(this, "matched/CaloTagLoose", doBinnedResolutionPlots));
        }
    }

    // define histogram class for SiliconAssociatedForwardMuons
    if (m_doSeparateSAFMuons) {
        m_oRecoMuonPlots_SiAssocFwrdMu.emplace_back(std::make_unique<Muon::RecoMuonPlotOrganizer>(this, "reco/SiAssocForward", selectedPlotCategories));
        if (!m_isData)
            m_oTruthRelatedMuonPlots_SiAssocFwrdMu.emplace_back(
                std::make_unique<Muon::TruthRelatedMuonPlotOrganizer>(this, "matched/SiAssocForward", doBinnedResolutionPlots));
    }

}

MuonValidationPlots::~MuonValidationPlots() =default;

void MuonValidationPlots::fillRecoMuonPlots(const xAOD::Muon& mu, float weight) {
    // fill hists for all muons
    m_oRecoMuonPlots->fill(mu, weight);
    // fill separate hists for each muon quality
    xAOD::Muon::Quality muqual = mu.quality();
    for (unsigned int i = 0; i < m_selectedWPs.size(); i++) {
        if (muqual <= (xAOD::Muon::Quality)m_selectedWPs[i]) { m_oRecoMuonPlots_perQuality[i]->fill(mu, weight); }
    }
    // fill separate hists for each author
    for (unsigned int i = 0; i < m_selectedAuthors.size(); i++) {
        auto author = static_cast<xAOD::Muon::Author>(m_selectedAuthors[i]);
        if (mu.isAuthor(author)) {
            if (author == (xAOD::Muon::Author::CaloTag)) {
                int ipar = 0;
                if (mu.parameter(ipar, xAOD::Muon::ParamDef::CaloMuonIDTag)) { ; }
                if (ipar < 11) continue;
            }

            // filter SiliconAssociatedForwardMuons
            if (mu.muonType() != xAOD::Muon::MuonType::SiliconAssociatedForwardMuon || !m_doSeparateSAFMuons)
                m_oRecoMuonPlots_perAuthor[i]->fill(mu, weight);
        }
    }
    // fill SiliconAssociatedForwardMuons
    for (unsigned int i = 0; i < m_oTruthRelatedMuonPlots_SiAssocFwrdMu.size(); i++) {
        if (mu.muonType() == xAOD::Muon::MuonType::SiliconAssociatedForwardMuon)
            m_oRecoMuonPlots_SiAssocFwrdMu[i]->fill(mu, weight);
    }
    // fill CaloTagLoose (one additional plot in plot list)
    unsigned int counter = m_selectedAuthors.size();
    if (counter + 1 == m_oRecoMuonPlots_perAuthor.size()) {
        if (mu.isAuthor(xAOD::Muon::Author::CaloTag)) m_oRecoMuonPlots_perAuthor[counter]->fill(mu, weight);
    }
}

void MuonValidationPlots::fillTruthMuonPlots(const xAOD::TruthParticle& truthMu, float weight) {
    m_oTruthMuonPlots[0]->fill(truthMu, weight);  // no selections
    if (isGoodTruthTrack(truthMu)) {  // in MS acceptance (minimum precision hits)
        m_oTruthMuonPlots[1]->fill(truthMu, weight);
    }
}

void MuonValidationPlots::fill(const xAOD::Muon& mu, float weight) { fillRecoMuonPlots(mu, weight); }

void MuonValidationPlots::fill(const xAOD::TruthParticle& truthMu, float weight) { fillTruthMuonPlots(truthMu, weight); }

void MuonValidationPlots::fill(const xAOD::TruthParticle* truthMu, 
                               const xAOD::Muon* mu, const xAOD::TrackParticleContainer* ,
                               float weight) {
    if (truthMu) fillTruthMuonPlots(*truthMu, weight);
    if (mu) fillRecoMuonPlots(*mu, weight);

    if ((mu) && (truthMu)) {
        // plots for all
        m_oTruthRelatedMuonPlots->fill(*truthMu, *mu, weight);
        // fill SiliconAssociatedForwardMuons
        for (unsigned int i = 0; i < m_oTruthRelatedMuonPlots_SiAssocFwrdMu.size(); i++) {
            if (mu->muonType() == xAOD::Muon::MuonType::SiliconAssociatedForwardMuon || !m_doSeparateSAFMuons)
                m_oTruthRelatedMuonPlots_SiAssocFwrdMu[i]->fill(*truthMu, *mu, weight);
        }

        // plots per quality
        xAOD::Muon::Quality muqual = mu->quality();
        for (unsigned int i = 0; i < m_selectedWPs.size(); i++) {
            if (muqual <= (xAOD::Muon::Quality)m_selectedWPs[i]) {
                m_oTruthRelatedMuonPlots_perQuality[i]->fill(*truthMu, *mu, weight);
            }
        }
        // plots per author
        for (unsigned int i = 0; i < m_selectedAuthors.size(); i++) {
            auto author = static_cast<xAOD::Muon::Author>(m_selectedAuthors[i]);
            if (mu->isAuthor(author)) {
                if (author == xAOD::Muon::Author::CaloTag) {
                    int ipar = 0;
                    if (mu->parameter(ipar, xAOD::Muon::ParamDef::CaloMuonIDTag)) { ; }
                    if (ipar < 11) continue;
                }
                // filter SilicionAssociatedForwardMuons
                if (mu->muonType() != xAOD::Muon::MuonType::SiliconAssociatedForwardMuon || !m_doSeparateSAFMuons)
                    m_oTruthRelatedMuonPlots_perAuthor[i]->fill(*truthMu, *mu,  weight);
            }
        }
        // fill CaloTagLoose (one additional plot in plot list)
        unsigned int counter = m_selectedAuthors.size();
        if (counter + 1 == m_oRecoMuonPlots_perAuthor.size()) {
            if (mu->isAuthor(xAOD::Muon::Author::CaloTag)) m_oTruthRelatedMuonPlots_perAuthor[counter]->fill(*truthMu, *mu, weight);
        }
    }
}



bool MuonValidationPlots::isGoodTruthTrack(const xAOD::TruthParticle& truthMu) {
    static const std::array<std::string,6> hitTypes{"innerSmallHits",  "innerLargeHits", "middleSmallHits",
                               "middleLargeHits", "outerSmallHits", "outerLargeHits"};  // MDT + CSC
    int minPrecHits = 5;

    int nPrecHits = 0;
    bool hasEnoughPrecHits = false;

    for (const auto& hitTypeItr : hitTypes) {
        SG::ConstAccessor<uint8_t> acc(hitTypeItr);
        nPrecHits += acc.withDefault (truthMu, 0);
        if (nPrecHits >= minPrecHits) {
            hasEnoughPrecHits = true;
            break;
        }
    }
    return (hasEnoughPrecHits);
}
