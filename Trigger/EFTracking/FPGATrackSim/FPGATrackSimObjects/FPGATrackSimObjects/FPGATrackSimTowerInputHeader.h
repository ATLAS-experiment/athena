/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimOBJECT_FPGATrackSimTOWERINPUTHEADER_H
#define FPGATrackSimOBJECT_FPGATrackSimTOWERINPUTHEADER_H

#include <TObject.h>

#include <vector>
#include <iostream>

#include "FPGATrackSimObjects/FPGATrackSimTruthTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimOfflineTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"

class FPGATrackSimTowerInputHeader
{
public:

  FPGATrackSimTowerInputHeader() { reset(); }
  FPGATrackSimTowerInputHeader(int id) { reset(); m_id = id; }
  FPGATrackSimTowerInputHeader(int id, double eta, double phi, double deta = 0, double dphi = 0);

  virtual ~FPGATrackSimTowerInputHeader() = default;

  void reset();

  // set / get the eta/phi of the RoI
  int setID(int id) { return m_id = id; };
  double setEta(double e) { return m_Eta = e; }
  double setPhi(double p) { return m_Phi = p; }

  double setDeltaEta(double de) { return m_DeltaEta = de; }
  double setDeltaPhi(double dp) { return m_DeltaPhi = dp; }

  int    id()  const { return m_id; };
  double eta() const { return m_Eta; }
  double phi() const { return m_Phi; }

  double deltaEta() const { return m_DeltaEta; }
  double deltaPhi() const { return m_DeltaPhi; }


  //  handling hits
  const std::vector<FPGATrackSimHit>& hits()  const { return m_Hits; }
  int nHits() const { return m_Hits.size(); }
  void addHit(const FPGATrackSimHit& s) { m_Hits.push_back(s); }
  void clearHits() { m_Hits.clear(); }
  void reserveHits(size_t size) { m_Hits.reserve(size); }

private:

  int                      m_id;

  double                   m_Eta;       // eta of the tower
  double                   m_Phi;       // phi of the tower

  double                   m_DeltaEta;  // delta eta of the tower
  double                   m_DeltaPhi;  // delta phi of the tower

  std::vector<FPGATrackSimHit>      m_Hits; // variables related to the FPGATrackSimHit storage


  ClassDefNV(FPGATrackSimTowerInputHeader, 2)
};


std::ostream& operator<<(std::ostream&, const FPGATrackSimTowerInputHeader&);

#endif  // FPGATrackSimOBJECT_FPGATrackSimTOWERINPUTHEADER_H
