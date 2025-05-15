/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSegmentPerformanceAlg.h"

#include "MuonStationIndex/MuonStationIndex.h"
#include "xAODMuon/MuonSegment.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "AthContainers/ConstAccessor.h"

using namespace Muon::MuonStationIndex;
MuonSegmentPerformanceAlg::MuonSegmentPerformanceAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthAlgorithm(name, pSvcLocator),
    m_writeToFile(false),   
    m_nevents(0) {

    declareProperty("writeToFile", m_writeToFile = false);
    declareProperty("FileName", m_fileName = "MuonSegmentPerformanceAlg.txt");
}

StatusCode MuonSegmentPerformanceAlg::initialize() {
    // initialize cuts, please make sure the number of bins and the sizes of the cuts + string are always the same
    unsigned int nbins = 3;
    m_nevents = 0;
    m_ntruth.resize(nbins);
    m_nfound.resize(nbins);
    m_nfake.resize(nbins);
    m_nhitCuts = {3, 4, 5};
    m_hitCutString = {"n==3 ", "n==4 ", "n>=5 "};

    ATH_CHECK(m_segmentKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_truthSegmentKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_truthSegmenLinkKey.initialize(SG::AllowEmpty));

    return StatusCode::SUCCESS;
}

StatusCode MuonSegmentPerformanceAlg::execute() {
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    const xAOD::MuonSegmentContainer* segments = nullptr;
    const xAOD::MuonSegmentContainer* truthSegments = nullptr;
    ATH_CHECK(SG::get(segments, m_segmentKey, ctx));
    ATH_CHECK(SG::get(truthSegments, m_truthSegmentKey, ctx));
    if (!segments || !truthSegments) {
        return StatusCode::SUCCESS;
    }
    std::set<const xAOD::MuonSegment*> matchedSegments;
    ++m_nevents;

    bool missedSegment = false;
    for (const auto seg : *truthSegments) {
        ChIndex chIndex = seg->chamberIndex();
        if (chIndex == ChIndex::ChUnknown || chIndex == ChIndex::ChIndexMax) {
            continue;
        }
        unsigned int index = 0;
        if (seg->nPrecisionHits() < 3) continue;
        while ((index < m_nhitCuts.size() - 1) && seg->nPrecisionHits() > m_nhitCuts[index] ) ++index;

        ++m_ntruth[index][toInt(chIndex)];

        if (index == 2 && (chIndex == ChIndex::CSS || chIndex == ChIndex::CSL) ){
            ATH_MSG_WARNING(chName(chIndex)<<"  with more than 4 layers ");
        }
        static const SG::ConstAccessor<ElementLink<xAOD::MuonSegmentContainer> >
          recoSegmentLinkAcc("recoSegmentLink");
        const ElementLink<xAOD::MuonSegmentContainer>& recoLink = recoSegmentLinkAcc(*seg);
        if (recoLink.isValid()) {
            ++m_nfound[index][toInt(chIndex)];
            matchedSegments.insert(*recoLink);
        } else {
            ATH_MSG_DEBUG(" Missing segment in sector "
                          << seg->sector() << "  " << chName(chIndex)
                          << " eta " << seg->etaIndex() << " nprec " << seg->nPrecisionHits() << " nphi " << seg->nPhiLayers()
                          << " nTrigEta " << seg->nTrigEtaLayers());
            missedSegment = true;
        }
    }
    if (missedSegment) ATH_MSG_DEBUG(" Dump Fake segments ");

    for (const auto seg : *segments) {
        if (matchedSegments.count(seg)) continue;
        ChIndex chIndex = seg->chamberIndex();
        if (chIndex == ChIndex::ChUnknown || chIndex == ChIndex::ChIndexMax) {
            ATH_MSG_WARNING("bad index ");
            continue;
        }

        unsigned int index = 0;
        if (seg->nPrecisionHits() < 3) continue;
        while ((index < m_nhitCuts.size() - 1) and (seg->nPrecisionHits() > m_nhitCuts[index]) ) ++index;
        if (missedSegment)
            ATH_MSG_DEBUG(" Fake segment in sector "
                          << seg->sector() << "  " << chName(chIndex)
                          << " eta " << seg->etaIndex() << " nprec " << seg->nPrecisionHits() << " nphi " << seg->nPhiLayers()
                          << " nTrigEta " << seg->nTrigEtaLayers());

        ++m_nfake[index][toInt(chIndex)];
    }

    return StatusCode::SUCCESS;
}

std::string MuonSegmentPerformanceAlg::printRatio(const std::string& prefix, unsigned int begin, unsigned int end, 
                                                  const counter_t& reco, const counter_t& truth) const {
    std::ostringstream sout;
    unsigned int width = 9;
    unsigned int precision = 3;
    sout << std::endl << prefix;
    for (unsigned int i = begin; i < end; ++i) {
        sout << std::setw(width) << std::setprecision(precision);
        if (truth[i] == 0)
            sout << " ";
        else
            sout << static_cast<double>(reco[i]) / static_cast<double>(truth[i]);
    }
    sout << std::endl
         << " #Events        "
         << " ";
    for (unsigned int i = begin; i < end; ++i) {
        sout << std::setw(width) << std::setprecision(precision);
        if (truth[i] == 0)
            sout << " ";
        else
            sout << static_cast<double>(truth[i]);
    }
    return sout.str();
}
std::string MuonSegmentPerformanceAlg::printRatio(const std::string& prefix, unsigned int begin, unsigned int end,
                                                  const counter_t& reco) const {
    std::ostringstream sout;
    unsigned int width = 9;
    unsigned int precision = 3;
    sout << std::endl << prefix;
    for (unsigned int i = begin; i < end; ++i) {
        sout << std::setw(width) << std::setprecision(precision);
        if (m_nevents == 0)
            sout << " ";
        else
            sout << static_cast<double>(reco[i]) / static_cast<double>(m_nevents);
    }
    return sout.str();
}

StatusCode MuonSegmentPerformanceAlg::finalize() {
    std::ofstream fileOutput;
    std::string outfile = "muonPerformance_segments.txt";
    fileOutput.open(outfile.c_str(), std::ios::trunc);
    std::ostringstream sout;
    sout.precision(4);

    unsigned int width = 9;
    sout << "Segment finding efficiencies barrel" << std::endl;
    sout << " Chambers        ";
    std::string prefix_eff = " Efficiency ";
    std::string prefix_fake = " Fake rate  ";
    unsigned end{0};
    for (unsigned int i = 0; i < s_chIdxMax; ++i){
        if (!isBarrel(static_cast<ChIndex>(i))) {
            break;
        }
        sout << std::setw(width) << chName(static_cast<ChIndex>(i));
        ++end;
    }
    for (unsigned int j = 0; j < end; ++j) {
        sout << printRatio(prefix_eff + m_hitCutString[j], 0, end, m_nfound[j], m_ntruth[j]);
    }
    sout << std::endl;
    for (unsigned int j = 0; j < end; ++j) {
        sout << printRatio(prefix_fake + m_hitCutString[j], 0, end, m_nfake[j]);
    }
    sout << std::endl;

    sout << "Segment finding efficiencies endcaps" << std::endl;
    sout << " Chambers        ";
    for (unsigned int i = end; i < s_chIdxMax; ++i)
        sout << std::setw(width) << chName(static_cast<ChIndex>(i));
    for (unsigned int j = 0; j < m_nfound.size(); ++j) {
        sout << printRatio(prefix_eff + m_hitCutString[j], end, s_chIdxMax, m_nfound[j],
                           m_ntruth[j]);
    }
    sout << std::endl;
    for (unsigned int j = 0; j < m_nfound.size(); ++j) {
        sout << printRatio(prefix_fake + m_hitCutString[j], end, s_chIdxMax, m_nfake[j]);
    }
    sout << std::endl;
    fileOutput << sout.str() << std::endl;
    fileOutput.close();
    return StatusCode::SUCCESS;
}

