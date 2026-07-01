/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
/// Rec::MuonPringingTool.h, (c) ATLAS Detector software        ///
///                                                             ///
/// This tool creates dump output of different detail to strings///
/// strings or files for an Analysis::Muon or a whole collection.///
///////////////////////////////////////////////////////////////////
#include "MuonPrintingTool.h"

#include "AthLinks/ElementLink.h"
#include "MuonSegment/MuonSegment.h"
#include "MuonSegment/MuonSegmentCombinationCollection.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkMaterialOnTrack/MaterialEffectsOnTrack.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackInfo.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"

/** constructor */
Rec::MuonPrintingTool::MuonPrintingTool(const std::string& type, const std::string& name, const IInterface* parent) :
    AthAlgTool(type, name, parent) {
    declareInterface<IMuonPrintingTool>(this);
}

/** destructor */
Rec::MuonPrintingTool::~MuonPrintingTool() {}

/** initialization */
StatusCode Rec::MuonPrintingTool::initialize() {
    ATH_CHECK(m_edmPrinter.retrieve());
    ATH_MSG_DEBUG("Retrieved " << m_edmPrinter);

    ATH_MSG_INFO("Initialize() successful in " << name());
    return StatusCode::SUCCESS;
}

/** end of the job - finalize */
StatusCode Rec::MuonPrintingTool::finalize() {
    ATH_MSG_DEBUG("Nothing to finalize.");
    return StatusCode::SUCCESS;
}

std::string Rec::MuonPrintingTool::print(const xAOD::TrackParticle& tp) const {
    std::ostringstream sout;
    sout << "  pt : " << tp.pt() << " eta : " << tp.eta() << " phi : " << tp.phi();
    return sout.str();
}

std::string Rec::MuonPrintingTool::print(const xAOD::Muon& muon) const {
    std::ostringstream sout;

    sout << "#####   Muon,  pt : " << muon.pt() << " eta : " << muon.eta() << " phi : " << muon.phi() << " mass : " << muon.m()
         << " author " << muon.author() << " type : " << muon.muonType() << " secondary authors: ";
    for (int a = 0; a < Muon::MuonStationIndex::toInt(xAOD::Muon::Author::NumberOfMuonAuthors); ++a) {
        xAOD::Muon::Author author = static_cast<xAOD::Muon::Author>(a);
        if (author != muon.author() && muon.isAuthor(author)) sout << " " << a;
    }
    sout << std::endl;

    sout << " ParamDef available:" << std::endl;
    using enum xAOD::Muon::ParamDef;
    static constexpr std::array<xAOD::Muon::ParamDef, 24> printPars{
        spectrometerFieldIntegral,
        scatteringCurvatureSignificance,
        scatteringNeighbourSignificance,
        momentumBalanceSignificance,
        segmentDeltaEta,
        segmentDeltaPhi,
        segmentChi2OverDoF,
        t0, beta, annBarrel, annEndCap, innAngle, meanDeltaADCCountsMDT,
        CaloMuonScore,  EnergyLossSigma,  FSR_CandidateEnergy,
        ParamEnergyLossSigmaMinus, ParamEnergyLossSigmaPlus,
        midAngle, msInnerMatchChi2, msOuterMatchChi2
    };
    for (auto p : printPars) {
        float val{0.f};
        if (muon.parameter(val, p)){
            sout<<" "<<p<<": "<<val<<std::endl;
        }
    }
    int iVal{0};
    if (muon.parameter(iVal, msInnerMatchDOF)) sout << "  msInnerMatchDOF : " << msInnerMatchDOF << std::endl;
    if (muon.parameter(iVal, msOuterMatchDOF)) sout << "  msOuterMatchDOF : " << msOuterMatchDOF << std::endl;
    if (muon.parameter(iVal, CaloMuonIDTag)) sout << "  CaloMuonIDTag : " << CaloMuonIDTag << std::endl;
    sout << "  EnergyLossType : " << muon.energyLossType() << std::endl;

    uint8_t nprecisionLayers = 0;
    uint8_t nprecisionHoleLayers = 0;
    uint8_t nphiLayers = 0;
    uint8_t ntrigEtaLayers = 0;
    uint8_t nphiHoleLayers = 0;
    uint8_t ntrigEtaHoleLayers = 0;
    uint8_t mainSector = 0;
    uint8_t secondSector = 0;
   const xAOD::TrackParticle& tp = *muon.trackParticle(xAOD::Muon::TrackParticleType::Primary);
    tp.summaryValue(nprecisionLayers, xAOD::numberOfPrecisionLayers);
    tp.summaryValue(nprecisionHoleLayers, xAOD::numberOfPrecisionHoleLayers);
    tp.summaryValue(nphiLayers, xAOD::numberOfPhiLayers);
    tp.summaryValue(nphiHoleLayers, xAOD::numberOfPhiHoleLayers);
    tp.summaryValue(ntrigEtaLayers, xAOD::numberOfTriggerEtaLayers);
    tp.summaryValue(ntrigEtaHoleLayers, xAOD::numberOfTriggerEtaHoleLayers);
    
    if (!muon.summaryValue(mainSector, xAOD::primarySector)) mainSector = 0;
    if (!muon.summaryValue(secondSector, xAOD::secondarySector)) secondSector = 0;
    sout << " Station Layers: precision " << static_cast<int>(nprecisionLayers) << " holes " << static_cast<int>(nprecisionHoleLayers)
         << " phi " << static_cast<int>(nphiLayers) << " holes " << static_cast<int>(nphiHoleLayers) << " trigEta "
         << static_cast<int>(ntrigEtaLayers) << " holes " << static_cast<int>(ntrigEtaHoleLayers) << " main sector "
         << static_cast<int>(mainSector) << " secondary " << static_cast<int>(secondSector) << std::endl;

    bool printMeasurements = true;

    if (const xAOD::TrackParticle* cbtp = muon.trackParticle(xAOD::Muon::TrackParticleType::CombinedTrackParticle); cbtp != nullptr) {
        sout << " --- Combined Muon track ---  " << print(*cbtp);
        if (!cbtp->trackLink().isValid()) {
            sout << " No Track link";
            ATH_MSG_DEBUG("Combined track particle without Trk::Track");
        } else {
            const Trk::Track* cbtr = *cbtp->trackLink();
            if (cbtr) sout << std::endl << m_edmPrinter->printStations(*cbtr);
            if (cbtr && printMeasurements) sout << std::endl << m_edmPrinter->printMeasurements(*cbtr);
        }
        sout << std::endl;
    }

    if (const xAOD::TrackParticle* satp = muon.trackParticle(xAOD::Muon::TrackParticleType::ExtrapolatedMuonSpectrometerTrackParticle); satp != nullptr) {
        sout << " --- Extrapolated Muon track ---  " << print(*satp);
        if (!satp->trackLink().isValid()) {
            sout << " No Track link";
            ATH_MSG_DEBUG("Extrapolated track particle without Trk::Track");
        } else {
            const Trk::Track* satr = *satp->trackLink();
            if (satr) sout << std::endl << m_edmPrinter->printStations(*satr);
            if (satr && printMeasurements) sout << std::endl << m_edmPrinter->printMeasurements(*satr);
        }
        sout << std::endl;
    }

    if (const xAOD::TrackParticle* satp = muon.trackParticle(xAOD::Muon::TrackParticleType::MuonSpectrometerTrackParticle); satp != nullptr) {
        sout << " --- MuonSpectrometer track ---  " << print(*satp);
        if (!satp->trackLink().isValid()) {
            sout << " No Track link";
            ATH_MSG_DEBUG("SA track particle without Trk::Track");
        } else {
            const Trk::Track* satr = *satp->trackLink();
            if (satr) sout << std::endl << m_edmPrinter->printStations(*satr);
            if (satr && printMeasurements) sout << std::endl << m_edmPrinter->printMeasurements(*satr);
        }
        sout << std::endl;
    }
    return sout.str();
}

std::string Rec::MuonPrintingTool::print(const xAOD::MuonContainer& muons) const {
    std::ostringstream sout;
    sout << "Muon Container Size :" << muons.size() << std::endl;

    for (const auto *m : muons) { sout << print(*m); }
    return sout.str();
}
