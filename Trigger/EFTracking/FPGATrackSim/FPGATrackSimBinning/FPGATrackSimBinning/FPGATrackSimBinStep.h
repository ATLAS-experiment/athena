// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimBinStep_H
#define FPGATrackSimBinStep_H

/**
 * @file FPGATrackSimBinStep.h
 * @author Elliot Lipeles
 * @date Sept 10th, 2024
 * @brief Binning Classes for BinStep
 *
 * Declarations in this file (there are a series of small classes):
 *      class FPGATrackSimBinStep
 *      
 * 
 * Overview of stucture:
 *    -- 
 * 

 * 
 * References:
 *
 */
#include "AthenaBaseComps/AthAlgTool.h"


#include "FPGATrackSimBinning/FPGATrackSimBinArray.h"
#include "FPGATrackSimBinning/FPGATrackSimBinUtil.h"
#include "FPGATrackSimBinning/IFPGATrackSimBinDesc.h"

#include "GaudiKernel/StatusCode.h"

#include <cmath>
#include <string>
#include <vector>
#include <array>

class FPGATrackSimBinTool;

//--------------------------------------------------------------------------------------------------
//
// Class to define the sub steps of the full binning
//
//--------------------------------------------------------------------------------------------------
class FPGATrackSimBinStep : virtual public AthAlgTool {
public:
  FPGATrackSimBinStep(const std::string & algname, const std::string & name,
                      const IInterface * ifc) : AthAlgTool(algname, name, ifc) {}

  virtual StatusCode initialize() override;
  StatusCode setRanges(FPGATrackSimBinStep* prev,const FPGATrackSimBinUtil::ParSet& parMin, const FPGATrackSimBinUtil::ParSet& parMax);

  // property of step
  const std::vector<unsigned> stepIdx(FPGATrackSimBinUtil::IdxSet idx) const; // index for only the pars used in this step
  const std::vector<unsigned> stepBins() const;   // bin sizes for only the pars used in this step
  const std::vector<unsigned>& stepPars() const {return m_pars;}  // parameters used for this step
  const std::vector<unsigned> nBins() const {return m_parBins;}   // bin sizes for only the pars used in this step
  const std::string stepName() const {return this->name().substr(this->name().find_last_of(".")+1);}
  unsigned stepNum() const {return m_stepNum;}

  // Calculation of bin boundaries
  double binCenter(unsigned par, unsigned bin) const { return m_parMin[par] + m_parStep[par] * (double(bin) + 0.5); }
  double binLowEdge(unsigned par, unsigned bin) const { return m_parMin[par] + m_parStep[par] * (double(bin)); }
  double binHighEdge(unsigned par, unsigned bin) const { return m_parMin[par] + m_parStep[par] * (double(bin) + 1.0);}
  double binWidth(unsigned par) const { return m_parStep[par]; }
  FPGATrackSimBinUtil::ParSet binLowEdge(const FPGATrackSimBinUtil::IdxSet &idx) const;
  FPGATrackSimBinUtil::ParSet binCenter(const FPGATrackSimBinUtil::IdxSet &idx) const;

  // get bin value for a specific parameter value
  unsigned binIdx(unsigned par, double val) const {
    // Guard against both underflow and overflow.
    if (val < m_parMin[par]) return 0;
    else if (val > m_parMax[par]) return unsigned(floor((m_parMax[par] - m_parMin[par]) / m_parStep[par])) - 1;
    else return unsigned(floor((val - m_parMin[par]) / m_parStep[par]));
  }

  // convert parset (the binning parameters) to a 5-d bin
  FPGATrackSimBinUtil::IdxSet binIdx(const FPGATrackSimBinUtil::ParSet &pars) const;

  // Convert to previous steps idx
  FPGATrackSimBinUtil::IdxSet convertToPrev(const FPGATrackSimBinUtil::IdxSet &cur) const;

  // Check if its the first step
  bool isFirstStep() const {return m_prev==0;}
  
  //--------------------------------------------------------------------------------------------------
  //
  //  Set which bins are valid
  //
  //--------------------------------------------------------------------------------------------------
  // Which bins are consistent with the (pT, eta, pho, d0, z0)
  // ranges computed from region definition defined in the eventselection
  // service or set by the layer map
  void initValidBins();
  void setValidBin(const std::vector<unsigned>& idx); // also sets SubBins
  void printValidBin() const; // dump an output to log for x-checks
  const FPGATrackSimBinArray<int>& validBinsFull() const { return m_validBinFull;}
  const FPGATrackSimBinArray<int>& validBinsLocal() const { return m_validBinLocal;}

private:
  Gaudi::Property<std::vector<unsigned>> m_parBinsConfig{this,"parBins",{},"Vector of number of bins for each parameter (expect 5)"};

  // pars used in this step
  std::vector<unsigned> m_pars{}; 

  // Array that has a true/false for if a bin is valid
  // some bins may not be valid because they correspond to (pT,eta,phi,d0,z0)
  // that are being targeted for reconstruction
  FPGATrackSimBinArray<int> m_validBinFull; // this is the full binning
  FPGATrackSimBinArray<int> m_validBinLocal; // this is for the pars used at this step

  // pointer to FPGATrackSimBinStep of previous step
  FPGATrackSimBinStep *m_prev{0};
  unsigned m_stepNum{}; // number of step
  
  // the bins for this step
  FPGATrackSimBinUtil::IdxSet m_parBins; // one means no binning this step
  FPGATrackSimBinUtil::ParSet m_parStep; // step size of each bin

  // reference to the full range defined in the "tool"
  FPGATrackSimBinUtil::ParSet m_parMin;
  FPGATrackSimBinUtil::ParSet m_parMax;
  
  friend FPGATrackSimBinTool;
};

#endif // FPGATrackSimBinStep_H
