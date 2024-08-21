/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonAlignmentData/MuonAlignmentErrorData.h"

void MuonAlignmentErrorData::setAlignmentErrorRules(std::vector<MuonAlignmentErrorRule>&& vec) {
    m_deviations = std::move(vec);
}

const std::vector<MuonAlignmentErrorData::MuonAlignmentErrorRule>& MuonAlignmentErrorData::getAlignmentErrorRules() const {
    return m_deviations;
}

void MuonAlignmentErrorData::setClobVersion(std::string clobVersion) {
    m_clobVersion = std::move(clobVersion);
}
const std::string& MuonAlignmentErrorData::getClobVersion() const {
    return m_clobVersion;
}

void MuonAlignmentErrorData::setHasNswHits(bool val) {
    m_hasNswHits = val;
}
bool MuonAlignmentErrorData::hasNswHits() const {
    return m_hasNswHits;
}
void MuonAlignmentErrorData::setMuonAlignmentErrorRuleCache(std::vector<MuonAlignmentErrorRuleCache>&& vec_new) {
    m_deviations_new = std::move(vec_new);
}

const std::vector<MuonAlignmentErrorData::MuonAlignmentErrorRuleCache>& MuonAlignmentErrorData::getMuonAlignmentErrorRuleCache() const {
    return m_deviations_new;
}

