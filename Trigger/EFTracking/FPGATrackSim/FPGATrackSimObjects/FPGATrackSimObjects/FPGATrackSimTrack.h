/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGFPGATrackSimOBJECTS_FPGATrackSimTRACK_H
#define TRIGFPGATrackSimOBJECTS_FPGATrackSimTRACK_H

#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimMultiTruth.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include <vector>
#include <iosfwd>
#include <cmath>

#include "GeneratorObjects/HepMcParticleLink.h"
#include "TObject.h"

class FPGATrackSimTrack {

 public:

  FPGATrackSimTrack() = default;
  virtual ~FPGATrackSimTrack();

  TrackCorrType getTrackCorrType() const { return m_trackCorrType; }
  TrackStage getTrackStage() const { return m_trackStage; }
  bool getDoDeltaGPhis() const { return m_doDeltaGPhis; }
  int getBankID() const { return m_bankID; }
  int getPatternID() const { return m_patternID; }
  int getFirstSectorID() const { return m_firstSectorID; }
  int getSecondSectorID() const { return m_secondSectorID; }
  int getTrackID() const { return m_trackID; }
  float getParameter(int) const;
  float getHoughX() const { return m_houghX; }
  float getHoughY() const { return m_houghY; }
  float getQOverPt() const { return m_qoverpt; }
  float getPt() const { return m_qoverpt != 0 ? std::abs(1 / m_qoverpt) : 99999999.; }
  float getD0() const { return m_d0; }
  float getPhi() const { return m_phi; }
  float getZ0() const { return m_z0; }
  float getEta() const { return m_eta; }
  float getTheta() const { return 2*std::atan(std::exp(-m_eta)); }
  float getChi2() const { return m_chi2; }
  float getOrigChi2() const { return m_origchi2; }
  float getChi2ndof() const { return m_chi2 / (getNCoords() - m_nmissing - 5); }
  float getOrigChi2ndof() const { return m_origchi2 / (getNCoords() - m_nmissing - 5); }
  int getSubRegion() const { return m_subregion; }
  int getRegion() const { return m_region; }
  unsigned getHoughXBin() const { return m_xBin; }
  unsigned getHoughYBin() const { return m_yBin; }

  int   getNMissing() const { return m_nmissing; } // missing coordinates
  unsigned int getTypeMask() const { return m_typemask; }
  unsigned int getHitMap() const { return m_hitmap; } // coordinate mask!!
  //write a detmap
  int getNCoords() const;
  signed long getEventIndex() const { return m_eventindex; }
  HepMcParticleLink::barcode_type getBarcode() const { return m_barcode; }
  HepMcParticleLink::barcode_type getUniqueID() const { return m_uniqueID; }
  unsigned long barcode() const { return getBarcode(); }
  float getBarcodeFrac() const { return m_barcode_frac; }
  //Should be passed as const ref to avoid excessive copying.
  const std::vector <FPGATrackSimHit>& getFPGATrackSimHits() const { return m_hits; }
  std::vector<float> getCoords(unsigned ilayer) const;
  // helper function to calculate coordinates for the methods based in idealized detector geometry. See https://cds.cern.ch/record/2633242
  // in the delta global phis method, the coordinates are the ideal z, and the delta global phis.
  // the delta global phis are the difference between the global phi of the input hit transformed to the ideal detector layer (this is hitGPhi)
  // and the global phi of the Hough Transform "track" at the ideal layers radius (target_r, R') (this is expectedGPhi)
  // expectedGPhi = phi_0 (from HT) - R' * A*q/pT - ((R' * A*q/pT)^3) / 6
  // hitGPhi = GPhi (from hit) + (R - R') * A*q/pT + ((R * A*q/pT)^3) / 6
  std::vector<float> computeIdealCoords(unsigned ilayer) const;
  //Has some size protections
  float getEtaCoord(int ilayer) const;
  float getPhiCoord(int ilayer) const;

  // Retrieve the idealized radius for a given layer.
  double getIdealRadius(int ilayer) const { return m_idealRadii.at(ilayer); };

  bool isValidCand() const    { return m_isValidCand; }

  void setTrackCorrType(TrackCorrType v) { m_trackCorrType = v; }
  void setTrackStage(TrackStage v) { m_trackStage = v; }
  void setDoDeltaGPhis(bool v) { m_doDeltaGPhis = v; }
  void setBankID(int v) { m_bankID = v; }
  void setPatternID(int v) { m_patternID = v; }
  void setFirstSectorID(int v) { m_firstSectorID = v; }
  void setSecondSectorID(int v) { m_secondSectorID = v; }
  void setTrackID(int v) { m_trackID = v; }
  void setParameter(int, float);
  void setHoughX(float v) { m_houghX = v; }
  void setHoughY(float v) { m_houghY = v; }
  void setQOverPt(float v) { m_qoverpt = v; }
  void setD0(float v) { m_d0 = v; }
  void setPhi(float v, bool ForceRange = true);
  void setZ0(float v) { m_z0 = v; }
  void setEta(float v) { m_eta = v; }
  void setChi2(float v) { m_chi2 = v; }
  void setOrigChi2(float v) { m_origchi2 = v; }
  void setNMissing(int v) { m_nmissing = v; }
  void setTypeMask(unsigned int v) { m_typemask = v; }
  void setHitMap(unsigned int v) { m_hitmap = v; }
  void setEventIndex(const signed long& v) { m_eventindex = v; }
  void setBarcode(const HepMcParticleLink::barcode_type& v) { m_barcode = v; }
  void setUniqueID(const HepMcParticleLink::barcode_type& v) { m_uniqueID = v; }
  void setBarcodeFrac(const float& v) { m_barcode_frac = v; }

  void setSubRegion(unsigned v) { m_subregion = v; }
  void setRegion(unsigned v) { m_region = v; }
  void setHoughXBin(unsigned v) { m_xBin = v; }
  void setHoughYBin(unsigned v) { m_yBin = v;}

  void setValidCand(bool v)   { m_isValidCand = v; }
  void setIdealRadii(const std::vector<double>& v) { m_idealRadii = v; }

  // Bin ID, if using FPGATrackSim binning.
  void setBinIdx(std::vector<unsigned> x) { m_binIdx = std::move(x); }
  const std::vector<unsigned>& getBinIdx() const { return m_binIdx; }

  void calculateTruth(); // this will calculate the above quantities based on the hits
  void setNLayers(int); //Reset/resize the track hits vector
  void setFPGATrackSimHit(unsigned i, const FPGATrackSimHit& hit);
  void setPars(FPGATrackSimTrackPars const& pars)
  {
    setQOverPt(pars.qOverPt);
    setPhi(pars.phi, false);
    setEta(pars.eta);
    setD0(pars.d0);
    setZ0(pars.z0);
  }

  FPGATrackSimTrackPars getPars() const {
    FPGATrackSimTrackPars pars;
    pars.qOverPt = getQOverPt();
    pars.eta = getEta();
    pars.phi = getPhi();
    pars.d0 = getD0();
    pars.z0 = getZ0();

    return pars;
  }

  // Functions for overlap removal
  unsigned int passedOR() const { return m_ORcode; }
  void setPassedOR(unsigned int);

  friend std::ostream& operator<<(std::ostream&, const FPGATrackSimTrack&);

 private:

  TrackCorrType m_trackCorrType = TrackCorrType::None; // type of correction to make for track coordinates
  TrackStage m_trackStage = TrackStage::FIRST; // Is this a 1st stage or second stage track?
  bool m_doDeltaGPhis = false; // if it uses the delta phis method for fitting

  int m_bankID = -1; // Bank ID of the road related to this track
  int m_patternID = 0; // TODO add documentation
  int m_firstSectorID = -1; // Sector identifier in the first stage tracking
  int m_secondSectorID = -1; // Sector identifier in thesecond stage tracking
  int m_trackID = -1; // Unique track ID in this bank

  int m_IdealGeoCorr = 0; // 

  float m_houghX = 0.0F; // phi0 from FPGATrackSimRoad_Hough
  float m_houghY = 0.0F; // QOverPt from FPGATrackSimRoad_Hough
  float m_qoverpt = 0.0F; // charge over pT
  float m_d0 = 0.0F; // impact paramter in the ATLAS reference system
  float m_phi = 0.0F; // phi of the track
  float m_z0 = 0.0F; // z0 in standard ATLAS reference system
  float m_eta = 0.0F; // eta of the track
  float m_chi2 = 0.0F; // chi2 of the track
  float m_origchi2 = 0.0F; // In the case of majority recovery, this is the chi2 of

  //TODO: Switch to matchedhits mask
  unsigned int m_nmissing = 0; // number of missing coordinates
  unsigned int m_typemask = 0; // set on in bits related to the step recovery were used, ex.: 0 no recovery, 01, rec 1st step, 11, recovery in the 1st and the 2nd stage
  unsigned int m_hitmap = 0;

  // Subregion (aka slice) number of the corresponding road.
  int m_subregion = 0;
  //Setting region for the bfield to work
  int m_region = 0;
  // Hough x, y bin numbers (needed for recreation of roads in extrapolation)
  unsigned m_xBin = 0;
  unsigned m_yBin = 0;

  std::vector<FPGATrackSimHit> m_hits; //[m_nlayers] hits associated to the track

  // bin ID. Just store this as a vector<unsigned>.
  std::vector<unsigned> m_binIdx;

  signed long m_eventindex = -1; // matched particle event index
  HepMcParticleLink::barcode_type m_barcode = std::numeric_limits<HepMcParticleLink::barcode_type>::max(); // matched geant particle barcode
  HepMcParticleLink::barcode_type m_uniqueID = std::numeric_limits<HepMcParticleLink::barcode_type>::max();
  // HepMcParticleLink::barcode_type m_uniqueID = std::numeric_limits<HepMcParticleLink::barcode_type>::max();
  float m_barcode_frac = 0.0F; // largest "matching fraction" with any "good"
  // geant particle, corresponding to the
  // particle with m_barcode

  // Is this a valid track candidate for fitting?
  // At the moment, this is only *not* true if the track candidate has an illegal mix of duplicated spacepoints.
  bool m_isValidCand = true;

  // Idealized radii used for the fit for this track. Unclear if it makes sense to store here,
  // but the information is loaded into the region map class at runtime and needs to be propagated
  // over to the track object (where the coordinate transform _currently_ lives) somehow.
  std::vector<double> m_idealRadii;

  // Overlap removal member
  // There is currently only one algorithm
  unsigned int m_ORcode = 1; // Each digit should represent pass/fail(1/0) result from a specific OR algorithm

  ClassDefNV(FPGATrackSimTrack, 7)
};

#endif // TRIGFPGATrackSimOBJECTS_FPGATrackSimTRACK_H
