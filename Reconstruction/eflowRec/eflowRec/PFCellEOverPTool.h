/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFCELLEOVERPTOOL_H
#define PFCELLEOVERPTOOL_H

#include "eflowRec/IEFlowCellEOverPTool.h"

class eflowBaseParameters;

/**
Class to store reference e/p mean and widths, as well as reference energy density radial profile fit parameters. The data is input to an eflowEEtaBinnedParameters object in the execute method. Stores data at the uncalibrated (EM) scale and is used by default. This inherits from IEFlowCellEOverPTool.
*/
class PFCellEOverPTool : public IEFlowCellEOverPTool {

 public:

  PFCellEOverPTool(const std::string& type,const std::string& name,const IInterface* parent);

  ~PFCellEOverPTool() {};

  StatusCode initialize();
  StatusCode fillBinnedParameters(eflowEEtaBinnedParameters *binnedParameters) const;
  StatusCode finalize() ;

 private:

  std::vector<double>  m_energyBinLowerBoundaries;
  std::vector<double> m_etaBinLowerBoundaries;
  std::vector<int> m_firstIntBinLowerBoundaries;
  std::vector<int> m_caloLayerBins;

  /** Location for e/p and cell ordering reference files */
  Gaudi::Property<std::string> m_referenceFileLocation{this,"referenceFileLocation","/home/markhodgkinson.linux/Releases/RLatest_eOvperP/athena/EOverPTools/EoverpNtupleAnalysis/ResultsDir/","Location for e/p and cell ordering reference files"};
};
#endif
