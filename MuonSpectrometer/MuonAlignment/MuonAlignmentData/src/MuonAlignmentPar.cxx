/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonAlignmentData/MuonAlignmentPar.h>


bool operator<(const Identifier& a, const MuonAlignmentPar& b) { return a < b.identify(); }
bool operator<(const MuonAlignmentPar& a, const Identifier& b) { return a.identify() < b; }

void MuonAlignmentPar::setIdentifier(const Identifier& id) { m_id = id; }
const Identifier& MuonAlignmentPar::identify() const {return m_id; }
bool MuonAlignmentPar::operator<(const MuonAlignmentPar& other) const { return m_id < other.m_id; }
void MuonAlignmentPar::setAmdbId(const std::string& stName, int stEta, int stPhi, int stJob) {
    m_station = stName;
    m_eta = stEta;
    m_phi = stPhi;
    m_job = stJob;
}
int MuonAlignmentPar::AmdbJob() const { return m_job; }
int MuonAlignmentPar::AmdbEta() const{ return m_eta; }
int MuonAlignmentPar::AmdbPhi() const{ return m_phi;}
const std::string& MuonAlignmentPar::AmdbStation() const {return m_station;}
