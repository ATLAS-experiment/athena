/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/



#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimObjects/FPGATrackSimFunctions.h"
#include <iostream>
#include <iomanip>
using namespace std;

// first stage only

// We need a destructor apparently?
FPGATrackSimTrack::~FPGATrackSimTrack() {}

std::vector<float> FPGATrackSimTrack::getCoords(unsigned ilayer) const
{
  std::vector<float> coords;
  if (ilayer >= m_hit_ptrs.size())
    throw std::range_error("FPGATrackSimTrack::getCoords() out of bounds");
  if (!m_hit_ptrs[ilayer])
    throw std::range_error("FPGATrackSimTrack::getCoords() null pointer at index " + std::to_string(ilayer));

  const auto& hit = *m_hit_ptrs[ilayer];

  if (m_trackCorrType == TrackCorrType::None)
  {
    coords.push_back(hit.getEtaIndex());
    coords.push_back(hit.getPhiIndex());
  }
  else
  {
    coords = computeIdealCoords(ilayer);
  }

  return coords;
}

std::vector<float> FPGATrackSimTrack::computeIdealCoords(unsigned ilayer) const
{
  
  double target_r = m_idealRadii[ilayer];
  if (ilayer >= m_hit_ptrs.size() || !m_hit_ptrs[ilayer])
    throw std::range_error("FPGATrackSimTrack::computeIdealCoords() invalid hit access at index " + std::to_string(ilayer));
  const auto& hit = *m_hit_ptrs[ilayer];
  if (hit.getHitType() == HitType::spacepoint) {
    unsigned other_layer = (hit.getSide() == 0) ? ilayer + 1 : ilayer - 1;
    target_r = (target_r + m_idealRadii[other_layer]) / 2.;
  }

  double hough_x =  getHoughX();
  double hough_y =  getHoughY();
  
  // Use the centralized computeIdealCoords function from FPGATrackSimFunctions
  std::vector<float> coords = ::computeIdealCoords(hit, hough_x, hough_y, target_r,  m_doDeltaGPhis, m_trackCorrType);

  return coords;
}

float FPGATrackSimTrack::getEtaCoord(int ilayer) const {
  auto coords = getCoords(ilayer);
  if (coords.size() > 0) {
    return coords.at(0);
  }
  else {
    throw std::range_error("FPGATrackSimTrack::getCoord(layer,coord) out of bounds");
  }
}

float FPGATrackSimTrack::getPhiCoord(int ilayer) const {
  auto coords = getCoords(ilayer);
  // If this is a spacepoint, and if this is the "outer" hit on a strip module
  // (side = 1) then we actually return the z/eta coord.
  // Since spacepoints are duplicated, this avoids using the same phi coord
  // twice and alsp avoids having to teach the code that strip spacepoints are
  // "2D" hits despite being in the strips, which everything assumes is 1D.
  // This makes it easy to mix and match spacepoints with strip hits that aren't
  // spacepoints (since the number of strip layers is held fixed).
  unsigned target_coord = 1;
  if (!m_hit_ptrs.empty()) {
    const auto& hit_ptr = m_hit_ptrs.at(ilayer);
    if (hit_ptr && hit_ptr->getHitType() == HitType::spacepoint && (hit_ptr->getPhysLayer() % 2) == 1) {
      target_coord = 0;
    }
  } else if (!m_hits.empty()) {
    if (m_hits.at(ilayer).getHitType() == HitType::spacepoint && (m_hits.at(ilayer).getPhysLayer() % 2) == 1) {
      target_coord = 0;
    }
  }

  if (coords.size() > target_coord) {
    return coords.at(target_coord);
  }
  else {
    throw std::range_error("FPGATrackSimTrack::getCoord(layer,coord) out of bounds");
  }
}

int FPGATrackSimTrack::getNCoords() const {
  int nCoords = 0;
  if (!m_hit_ptrs.empty()) {
    for (const auto& hit : m_hit_ptrs) {
      if (hit) nCoords += hit->getDim();
    }
  }
  else {
    for (const auto& hit : m_hits) {
      nCoords += hit.getDim();
    }
  }
  return nCoords;
}

// Set a specific transient hit (shared_ptr only).
// Caller is responsible for creating the shared_ptr:
// - For owned/synthetic hits: pass std::make_shared<FPGATrackSimHit>(...)
// - For SG hits: pass std::shared_ptr<const FPGATrackSimHit>(ptr, [](auto*){})
// This only modifies m_hit_ptrs; use persistifyHits() to copy to m_hits.
void FPGATrackSimTrack::setFPGATrackSimHit(unsigned i, std::shared_ptr<const FPGATrackSimHit> hit)
{
  if (m_hit_ptrs.size() <= i) m_hit_ptrs.resize(i+1);
  m_hit_ptrs[i] = std::move(hit);
}

/** set the number of layers in the track. =0 is used to clear the track */
void FPGATrackSimTrack::setNLayers(int dim)
{
  // Pre-fill with dummy hits to ensure dense vector model.
  // This guarantees that all layers 0 to dim-1 have entries (no sparse nulls).
  // setFPGATrackSimHit() will replace these dummies with real hits as needed.
  m_hit_ptrs.clear();
  m_hit_ptrs.reserve(dim);
  for (int i = 0; i < dim; i++) {
    FPGATrackSimHit dummy;
    dummy.setLayer(i);
    dummy.setSection(0);
    m_hit_ptrs.push_back(std::make_shared<FPGATrackSimHit>(dummy));
  }
}


// if ForceRange==true, then phi = [-pi..pi)
void FPGATrackSimTrack::setPhi(float phi, bool ForceRange) {
  if (ForceRange) {
    // when phi is ridiculously large, there is no point in adjusting it
    if (std::abs(phi) > 100) {
      if (m_chi2 < 100) { // this is a BAD track, so fail it if chi2 hasn't done so already
        m_chi2 += 100; // we want to fail this event anyway
      }
    }
    else {
      while (phi >= M_PI) phi -= (2. * M_PI);
      while (phi < -M_PI) phi += (2. * M_PI);
    }
  }
  m_phi = phi;
}

float FPGATrackSimTrack::getParameter(int ipar) const
{
  switch (ipar) {
  case 0:
    return m_qoverpt;
    break;
  case 1:
    return m_d0;
    break;
  case 2:
    return m_phi;
    break;
  case 3:
    return m_z0;
    break;
  case 4:
    return m_eta;
    break;
  }

  return 0.;
}


void  FPGATrackSimTrack::setParameter(int ipar, float val)
{
  switch (ipar) {
  case 0:
    m_qoverpt = val;
    break;
  case 1:
    m_d0 = val;
    break;
  case 2:
    m_phi = val;
    break;
  case 3:
    m_z0 = val;
    break;
  case 4:
    m_eta = val;
    break;
  }
}


ostream& operator<<(ostream& out, const FPGATrackSimTrack& track)
{

  out << "TRACK: ID=" << std::left << setw(8) << track.m_trackID;
  out << " SECTOR1=" << std::left << setw(8) << track.m_firstSectorID;
  out << " BANK=" << std::left << setw(8) << track.m_bankID;
  out << " BARCODE=" << std::left << setw(6) << track.m_barcode;
  out << " BARCODE_F=" << std::left << setw(9) << track.m_barcode_frac;
  out << " EVENT=" << std::left << setw(6) << track.m_eventindex;
  out << " HITMAP=" << std::left << setw(8) << track.getHitMap();
  out << " TYPE=" << std::left << setw(3) << track.m_typemask;
  out << " NMISS=" << std::left << setw(3) << track.getNMissing();
  out << "\n";
  streamsize oldprec = out.precision();
  out.precision(4);
  out << "    PHI=" << std::left << setw(10) << track.m_phi;
  out.setf(ios_base::scientific);
  out.precision(2);
  out << " Q/PT=" << std::left << setw(10) << track.m_qoverpt;
  out.unsetf(ios_base::scientific);
  out.precision(4);
  out << " d0=" << std::left << setw(10) << track.m_d0;
  out << " ETA=" << std::left << setw(10) << track.m_eta;
  out << " z0=" << std::left << setw(10) << track.m_z0;
  out << " Chi2=" << std::left << setw(12) << track.m_chi2;
  out << " OChi2=" << std::left << setw(12) << track.m_origchi2;

  out << endl;
  out.precision(oldprec);

  out << endl;

  // print the hits
  int iter = 0;
  for (const auto& hit : track.m_hits) {
    out << "Hit " << iter << ": " << hit << "\n";
    iter++;
  }

  return out;
}


void FPGATrackSimTrack::calculateTruth()
{
  vector<FPGATrackSimMultiTruth> mtv;
  mtv.reserve(m_hit_ptrs.size());

  // don't loop over coordinates, since we only calculate truth *per hit* and not per coordinate, though hitmap is saved for coordinates, so be careful
  if (!m_hit_ptrs.empty()) {
    for (const auto& thishit : m_hit_ptrs)
    {
      if (!thishit) throw std::runtime_error("Null hit pointer in FPGATrackSimTrack::calculateTruth()");
      if (thishit->isReal())
      {
        FPGATrackSimMultiTruth this_mt(thishit->getTruth());
        this_mt.assign_equal_normalization();
        if (thishit->isPixel())
          for ( auto& x : this_mt)
            x.second *= 2;
        mtv.push_back(this_mt);
      }
    }
  }
  else {
    for (const auto& thishit : m_hits)
    {
      if (thishit.isReal())
      {
        FPGATrackSimMultiTruth this_mt(thishit.getTruth());
        this_mt.assign_equal_normalization();
        if (thishit.isPixel())
          for ( auto& x : this_mt)
            x.second *= 2;
        mtv.push_back(this_mt);
      }
    }
  }

  // compute the best geant match, the barcode with the largest number of hits contributing to the track.
  // frac is then the fraction of the total number of hits on the track attributed to the barcode.
  FPGATrackSimMultiTruth mt(std::accumulate(mtv.begin(), mtv.end(), FPGATrackSimMultiTruth(), FPGATrackSimMultiTruth::AddAccumulator()));
  FPGATrackSimMultiTruth::Barcode tbarcode;
  FPGATrackSimMultiTruth::Weight tfrac;
  const bool ok = mt.best(tbarcode, tfrac);
  if (ok)
  {
    setEventIndex(tbarcode.first);
    setBarcode(tbarcode.second);
    setBarcodeFrac(tfrac);
  }
  else
  {
    setEventIndex(-1);
    setBarcode(-1);
    setBarcodeFrac(0);
  }
}

void FPGATrackSimTrack::setPassedOR(unsigned int code)
{
  m_ORcode = code;
}


layer_bitmask_t FPGATrackSimTrack::getHitMask() const {
  unsigned retv =0;
  for (unsigned lyr = 0; lyr < m_hit_ptrs.size(); lyr++)
  {
    if (m_hit_ptrs[lyr] && m_hit_ptrs[lyr]->isReal()) retv|=(1<< lyr);
  }
  return retv;
}
