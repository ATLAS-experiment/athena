// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimBinUtil_H
#define FPGATrackSimBinUtil_H

/**
 * @file FPGATrackSimBinUtil.h
 * @author Elliot Lipeles
 * @date Sept 10th, 2024
 * @brief Binning Utilities for GenScanTool
 *
 * Declarations in this file (there are a series of small classes):
 *
 *
 *
 * Overview of stucture:
 *

 *
 * References:
 *
 */

#include <array>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"


namespace FPGATrackSimBinUtil {

//--------------------------------------------------------------------------------------------------
//
// These just makes ParSet and IdxSet types that have 5 double or unsigned int
// for the 5-track parameters. The point of the structs is that they are fixed
// to the right length and can be converted back and forth to std::vector
// through cast operators and constructors
//
//--------------------------------------------------------------------------------------------------
struct ParSet : public std::array<double, FPGATrackSimTrackPars::NPARS> {
  using array<double, FPGATrackSimTrackPars::NPARS>::array;
  ParSet(const std::vector<double> &val);
  operator const std::vector<double>() const;
};
struct IdxSet : public std::array<unsigned, FPGATrackSimTrackPars::NPARS> {
  using array<unsigned, FPGATrackSimTrackPars::NPARS>::array;
  IdxSet(const std::vector<unsigned> &val);
  operator const std::vector<unsigned>() const;
};

// invalid bin value, there is no way a true bin could be there
const IdxSet invalidBin{std::initializer_list<unsigned>({std::numeric_limits<unsigned>::max(),std::numeric_limits<unsigned>::max(),
  std::numeric_limits<unsigned>::max(),std::numeric_limits<unsigned>::max(),std::numeric_limits<unsigned>::max()})};


// Generic Utility for splitting in vector (e.g. idx or #bins 5-d vectors)
// into subvectors (e.g. idx for just the scan parameters). Technically, for
// a list of parameter indices (elems) gives the subvector of the invec with
// just those indices
std::vector<unsigned> subVec(const std::vector<unsigned> &elems,
                             const IdxSet &invec);

// Opposite of above subVec, this sets the subvector
void setIdxSubVec(IdxSet &idx, const std::vector<unsigned> &subvecelems,
                  const std::vector<unsigned> &subvecidx);

// Makes are set of parameters corresponding to the corners specified by
// scanpars of the bin specified by idx e.g. if scan pars is (pT,d0) then the
// set is (low pT,low d0), (low pT, high d0), (high pT,low d0), (high pT, high
// d0)
std::vector<IdxSet> makeVariationSet(const std::vector<unsigned> &scanpars,
                                     const IdxSet &idx);

// this is needed by the stream manager below for outputing std::vector<unsigned>
std::ostream &operator<<(std::ostream &os, const std::vector<unsigned> &idx);

// Class for writing const files formatted for firmware
struct StreamManager {
  StreamManager(const std::string &setname) : m_setname(setname) {}
  ~StreamManager();
  template <typename T> void writeVar(const std::string &var, T val) {
    auto emplace_result = m_map.try_emplace(
        var, m_setname + "_" + var + "_const.txt", std::ios_base::out);
    if (!emplace_result.second) {
      emplace_result.first->second << ",\n";
    }
    emplace_result.first->second << val;
  }

private:
  std::string m_setname;
  std::map<std::string, std::fstream> m_map;
};

// Stores hit plus the phi/etashift from the nominal bin center
struct StoredHit {
  StoredHit(const std::shared_ptr<const FPGATrackSimHit>& hit) : hitptr(hit), phiShift(0), etaShift(0), layer(-1) {}
  std::shared_ptr<const FPGATrackSimHit> hitptr;
  double phiShift; // shift in r-phi plane as quantified by BinDesc
  double etaShift;  // shift in r-z plane as  quantified by BinDesc
  unsigned layer;
  double rzrad() const;
  static const unsigned invalidLayer = std::numeric_limits<unsigned>::max();
};
std::ostream &operator<<(std::ostream &os, const StoredHit &hit);


//-------------------------------------------------------------------------------------------------------
//
// Geometry Helpers -- does basic helix calculations
//
//-------------------------------------------------------------------------------------------------------
class GeomHelpers
{
public:
    // This is the constant needed to relate hit phi to track phi due to curvature
    static constexpr double CurvatureConstant = fpgatracksim::A;

    // standard eta to theta calculation
    static double ThetaFromEta(double eta);

    // standard theta to eta calculation
    static double EtaFromTheta(double theta);

    // find the expected z position from the radius and track parameters
    static double zFromPars(double r, const FPGATrackSimTrackPars &pars);

    // find the expected z position from the radius and track parameters
    static double phiFromPars(double r, const FPGATrackSimTrackPars &pars);

    // find the track phi that would be consistent with the other track parameters and the hit (r,phi)
    static double parsToTrkPhi(const FPGATrackSimTrackPars &pars, FPGATrackSimHit const *hit);
};


}; // namespace FPGATrackSimBinUtil

#endif // FPGATrackSimBinUtil_H
