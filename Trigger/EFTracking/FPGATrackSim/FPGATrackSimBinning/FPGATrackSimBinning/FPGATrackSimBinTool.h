// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimBinTool_H
#define FPGATrackSimBinTool_H

/**
 * @file FPGATrackSimBinTool.h
 * @author Elliot Lipeles
 * @date Sept 10th, 2024
 * @brief Binning Classes for BinTool
 *
 * Declarations in this file (there are a series of small classes):
 *      class FPGATrackSimBinTool
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

#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"

#include "FPGATrackSimBinUtil.h"
#include "IFPGATrackSimBinDesc.h"
#include "FPGATrackSimBinStep.h"

#include <GaudiKernel/GaudiHandle.h>
#include <cmath>
#include <string>
#include <vector>

// Use IdxSet and ParSet from FPGATrackSimUtil
using FPGATrackSimBinUtil::ParSet;
using FPGATrackSimBinUtil::IdxSet;

//-------------------------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------------------------
class FPGATrackSimBinTool : virtual public AthAlgTool {
public:
  friend FPGATrackSimBinStep;

  FPGATrackSimBinTool(const std::string &algname, const std::string &name, const IInterface *ifc);
  
  virtual StatusCode initialize() override;

  //--------------------------------------------------------------------------------------------------
  //
  //  Access to FPGATrackSimBinSteps and FPGATrackSimBinDesc
  //
  //--------------------------------------------------------------------------------------------------
  const IFPGATrackSimBinDesc* binDesc() const {return m_binDesc.get();}
  const ToolHandleArray<FPGATrackSimBinStep>& steps() const { return m_steps;}
  FPGATrackSimBinStep* lastStep() { return (--m_steps.end())->get(); }
  const FPGATrackSimBinStep* lastStep() const { return (--m_steps.end())->get(); }
  const std::vector<std::string>& stepNames() const {return m_stepNames;}

  //--------------------------------------------------------------------------------------------------
  //
  //  Bin Boundaries and conversions from value to bin
  //
  //--------------------------------------------------------------------------------------------------

  // center of whole region
  ParSet center() const;

  // range of whole region
  double parRange(unsigned par) const { return m_parMax[par]-m_parMin[par];}
  double parCenter(unsigned par) const { return (m_parMax[par]+m_parMin[par])/2.0;}
  double parMin(unsigned par) const { return m_parMin[par];}
  double parMax(unsigned par) const { return m_parMax[par];}
  
  // Getters for the entire ranges
  const ParSet& parMin() const { return m_parMin;}
  const ParSet& parMax() const { return m_parMax;}


  // check if 1-d or 5-d parameter is within the range of the binning
  bool inRange(unsigned par, double val) const { return ((val < m_parMax[par]) && (val > m_parMin[par])); }
  bool inRange(const ParSet &pars) const;
  
  //--------------------------------------------------------------------------------------------------
  //
  //  Set which bins are valid
  //
  //--------------------------------------------------------------------------------------------------
  void initValidBins();
  void computeValidBins(const IFPGATrackSimEventSelectionSvc* evtSel);
  void setValidBin(const std::vector<unsigned>& idx); // also sets SubBins
  void printValidBin() const; // dump an output to log for x-checks

private:
  //--------------------------------------------------------------------------------------------------
  //
  // Python Configurables
  //
  //--------------------------------------------------------------------------------------------------
  Gaudi::Property<double> m_d0FractionalPadding{this, "d0FractionalPadding", {}, "Fractional padding used when calculating the valid range of bins"};
  Gaudi::Property<double> m_z0FractionalPadding{this, "z0FractionalPadding", {}, "Fractional padding used when calculating the valid range of bins"};
  Gaudi::Property<double> m_etaFractionalPadding{this, "etaFractionalPadding", {}, "Fractional padding used when calculating the valid range of bins"};
  Gaudi::Property<double> m_phiFractionalPadding{this, "phiFractionalPadding", {}, "Fractional padding used when calculating the valid range of bins"};
  Gaudi::Property<double> m_qOverPtFractionalPadding{this, "qOverPtFractionalPadding", {}, "Fractional padding used when calculating the valid range of bins"};
  Gaudi::Property<std::vector<double>> m_parMinConfig{this, "parMin", {}, "Vector of minimum bounds of parameters (expect 5"};
  Gaudi::Property<std::vector<double>> m_parMaxConfig{this, "parMax", {}, "Vector of maximum bounds of parameters (expect 5"};

  ToolHandleArray<FPGATrackSimBinStep> m_steps{this, "Steps", {}, "Array of FPGATrackSimBinStep: describes which parameters are binned at each step"};
  ToolHandle<IFPGATrackSimBinDesc> m_binDesc{this, "BinDesc", "FPGATrackSimBinDescBase", "FPGATrackSimBinDescBase: describes binning track parameters"};
  
  //
  // Internal data
  //

  // These indicate the range of the full binning
  ParSet m_parMin{};
  ParSet m_parMax{};

  // A list of the step names for convienience
  std::vector<std::string> m_stepNames;
};

#endif // FPGATrackSimBinTool_H
