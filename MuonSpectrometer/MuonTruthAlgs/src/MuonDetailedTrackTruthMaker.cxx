/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Algorithm producing truth info for PrepRawData, keeping all MC particles contributed to a PRD.
// A. Gaponenko, 2006

#include "MuonTruthAlgs/MuonDetailedTrackTruthMaker.h"

#include <iterator>
#include <vector>

StatusCode MuonDetailedTrackTruthMaker::initialize() {
    ATH_MSG_DEBUG("MuonDetailedTrackTruthMaker::initialize()");

    if (m_PRD_TruthNames.empty()) {
        ATH_MSG_FATAL("No PRD truth collections have been configured for processing");
        return StatusCode::FAILURE;
    }
    //----------------
    ATH_CHECK(m_truthTool.retrieve());
    ATH_MSG_DEBUG("Retrieved tool " << m_truthTool);

    m_detailedTrackTruthNames.resize(m_trackCollectionNames.size());
    for (unsigned int i = 0; i < m_trackCollectionNames.size(); i++) {
        m_detailedTrackTruthNames.at(i)=m_trackCollectionNames.at(i).key() + "DetailedTruth";;
        ATH_MSG_INFO("process " << m_trackCollectionNames.at(i).key() << " for detailed truth collection "
                                << m_detailedTrackTruthNames.at(i).key());
    }

    ATH_CHECK(m_trackCollectionNames.initialize());
    ATH_CHECK(m_PRD_TruthNames.initialize());
    ATH_CHECK(m_detailedTrackTruthNames.initialize());

    //----------------
    return StatusCode::SUCCESS;
}

// -----------------------------------------------------------------------------------------------------
StatusCode MuonDetailedTrackTruthMaker::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("MuonDetailedTrackTruthMaker::execute()");

    //----------------------------------------------------------------
    // Retrieve prep raw data truth
    std::vector<const PRD_MultiTruthCollection*> prdCollectionVector;
    for (const auto& truthKey : m_PRD_TruthNames){
        prdCollectionVector.emplace_back(nullptr);
        ATH_CHECK(SG::get(prdCollectionVector.back(), truthKey, ctx));
    }

    ATH_MSG_DEBUG("Loaded in total "<<prdCollectionVector.size()<<" prd truth collections");
    //----------------------------------------------------------------
    // Retrieve track collections

    int i = 0;
    for (const auto& trkKey : m_trackCollectionNames) {
        const TrackCollection* tcol{nullptr};
        ATH_CHECK(SG::get(tcol, trkKey, ctx));

        //----------------------------------------------------------------
        // Produce and store the output.

        SG::WriteHandle dttc(m_detailedTrackTruthNames.at(i), ctx);
        ATH_MSG_INFO("Write detailed collection "<<m_detailedTrackTruthNames.at(i).fullKey());
        ATH_CHECK(dttc.record(std::make_unique<DetailedTrackTruthCollection>()));
        dttc->setTrackCollection(tcol);
        m_truthTool->buildDetailedTrackTruth(dttc.ptr(), *tcol, prdCollectionVector, ctx);
        i++;
    }
    return StatusCode::SUCCESS;
}
