/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGFPGATrackSimOBJECTS_MULTITRUTH_H
#define TRIGFPGATrackSimOBJECTS_MULTITRUTH_H

#include <TObject.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

// FPGATrackSimMultiTruth
// ================================================================
// code to match clusters and tracks to GEANT charge deposition information
// ================================================================
// 20-04-2009 Antonio Boveia (boveia@hep.uchicago.edu)
//
// class which represents the relative contributions of one or more
// truth particles (identified by a unique "barcode") to some
// measurement. for example, a single silicon pixel may record the
// summed ionization from two different charged particles passing
// through it during the same event. one particle may (and likely
// will) contribute more charge than another, so the class stores a
// weight for each barcode which can be used to incorporate this
// information.
//
// there are two ways of combining the information: the "add" function
// and the "maximize" function. "add" sums the weights of individual
// contributions, appropriate for combining the truth for individual
// hits to obtain the truth for a track. in this case, "best" then
// returns the barcode with the largest sum of weights. "maximize"
// simply remembers the barcode with the greatest weight, and "best"
// then returns that barcode.
//

class FPGATrackSimMultiTruth : public TObject {
public:

  typedef std::pair<long, long> Barcode; // = (event index, barcode)
  typedef float Weight;
  typedef std::map<Barcode, Weight> TruthMap;

  FPGATrackSimMultiTruth() = default;
  FPGATrackSimMultiTruth(const FPGATrackSimMultiTruth::Barcode& code, const FPGATrackSimMultiTruth::Weight weight = 1.) { m_truth[code] = weight; }
  virtual ~FPGATrackSimMultiTruth() = default;

  struct AddAccumulator {
    auto operator()(const FPGATrackSimMultiTruth& result, const FPGATrackSimMultiTruth& a) const { return result.add(a); }
  };

  struct MaxAccumulator {
    auto operator()(const FPGATrackSimMultiTruth& result, const FPGATrackSimMultiTruth& a) const { return result.maximize(a); }
  };

  auto begin() { return m_truth.begin(); }
  auto end() { return m_truth.end(); }
  auto begin() const { return m_truth.begin(); }
  auto end() const { return m_truth.end(); }

  bool isEmpty() const { return m_truth.empty(); }

  long best_barcode() const;

  void add(const FPGATrackSimMultiTruth::Barcode& code, const FPGATrackSimMultiTruth::Weight& weight);
  void add(const FPGATrackSimMultiTruth& rval);

  void maximize(const FPGATrackSimMultiTruth::Barcode& code, const FPGATrackSimMultiTruth::Weight& weight);
  void maximize(const FPGATrackSimMultiTruth& rval);

  void assign_equal_normalization();

  inline unsigned multiplicity() const { return m_truth.size(); }

  // Finds the best barcode and its normalized weight, returning them by reference.
  // Returns true on success.
  inline bool best(FPGATrackSimMultiTruth::Barcode& code, FPGATrackSimMultiTruth::Weight& weight) const
  {
    if (m_truth.empty()) return false;
    auto i = std::max_element(m_truth.begin(), m_truth.end(), TruthMapWeightLt());
    code = i->first;
    weight = total_weight() > 0. ? (i->second) / total_weight() : 0.;
    return true;
  }


private:

  struct TruthMapWeightAcc {
    auto operator()(const FPGATrackSimMultiTruth::Weight& result, const TruthMap::value_type& a) const { return result + a.second; }
  };

  struct TruthMapWeightLt {
    bool operator()(const TruthMap::value_type& a, const TruthMap::value_type& b) const {
      const bool a_info = (a.first.first != -1) && (a.first.second != -1);
      const bool b_info = (b.first.first != -1) && (b.first.second != -1);
      return a_info && b_info ? a.second < b.second : b_info;
    }
  };

  // add and mult for std::accumulate
  const FPGATrackSimMultiTruth add(const FPGATrackSimMultiTruth& rval) const;
  const FPGATrackSimMultiTruth maximize(const FPGATrackSimMultiTruth& rval) const;

  // matching probability definition and maximization logic
  inline FPGATrackSimMultiTruth::Weight total_weight() const { return std::accumulate(m_truth.begin(), m_truth.end(), 0., TruthMapWeightAcc()); }

  inline FPGATrackSimMultiTruth::Weight weight(const FPGATrackSimMultiTruth::Barcode& code) const {
    return m_truth.empty() || (m_truth.find(code) == m_truth.end()) ? 0. : ((m_truth.find(code))->second) / total_weight();
  }

  TruthMap m_truth;


  ClassDef(FPGATrackSimMultiTruth, 2) // this is a TObject to be stored in the FPGATrackSim ROOT output streams
};
std::ostream& operator<<(std::ostream& o, const FPGATrackSimMultiTruth& mt);


#endif // TRIGFPGATrackSimOBJECTS_MULTITRUTH_H
