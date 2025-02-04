/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/



#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimObjects/FPGATrackSimFunctions.h"
#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

// first stage only

// We need a destructor apparently?
FPGATrackSimTrack::~FPGATrackSimTrack() {}

std::vector<float> FPGATrackSimTrack::getCoords(unsigned ilayer) const
{
  std::vector<float> coords;
  if (ilayer >= m_hits.size())
    throw std::range_error("FPGATrackSimTrack::getCoords() out of bounds");

  if (m_trackCorrType == TrackCorrType::None)
  {
    coords.push_back(m_hits[ilayer].getEtaIndex());
    coords.push_back(m_hits[ilayer].getPhiIndex());
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
  if (m_hits[ilayer].getHitType() == HitType::spacepoint) {
    unsigned other_layer = (m_hits[ilayer].getSide() == 0) ? ilayer + 1 : ilayer - 1;
    target_r = (target_r + m_idealRadii[other_layer]) / 2.;
  }

  double hough_x =  getHoughX();
  double hough_y =  getHoughY();
  
  // Use the centralized computeIdealCoords function from FPGATrackSimFunctions
  std::vector<float> coords = ::computeIdealCoords(m_hits[ilayer], hough_x, hough_y, target_r,  m_doDeltaGPhis, m_trackCorrType);

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
  if (m_hits[ilayer].getHitType() == HitType::spacepoint && (m_hits[ilayer].getPhysLayer() % 2) == 1) {
    target_coord = 0;
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
  for (const auto& hit : m_hits) {
    nCoords += hit.getDim();
  }
  return nCoords;
}

//set a specific position in m_hits
void FPGATrackSimTrack::setFPGATrackSimHit(unsigned i, const FPGATrackSimHit& hit)
{
  if (m_hits.size() > i)
    m_hits[i] = hit;
  else
    throw std::range_error("FPGATrackSimTrack::setFPGATrackSimHit() out of bounds");
}

/** set the number of layers in the track. =0 is used to clear the track */
void FPGATrackSimTrack::setNLayers(int dim)
{
  if (m_hits.size() > 0) m_hits.clear();
  m_hits.resize(dim);
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
  mtv.reserve(m_hits.size());

  // don't loop over coordinates, since we only calculate truth *per hit* and not per coordinate, though hitmap is saved for coordinates, so be careful
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


