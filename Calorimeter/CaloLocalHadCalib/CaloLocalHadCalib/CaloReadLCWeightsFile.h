/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-
#ifndef CALOCALLOCALHADCALIB_CALOREADLCWEIGHTSFILE
#define CALOCALLOCALHADCALIB_CALOREADLCWEIGHTSFILE

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloConditions/CaloLocalHadCoeff.h"
#include <string>


class CaloReadLCWeightsFile : public AthAlgorithm
{
 public:
  CaloReadLCWeightsFile(const std::string & name, ISvcLocator * pSvcLocator);
  ~CaloReadLCWeightsFile();

  StatusCode initialize(); 
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();
  StatusCode initDataFromFile(const std::string& theLCWeightFileName,
                              CaloLocalHadCoeff& data);

 private:
  Gaudi::Property<std::string> m_key{this, "CorrectionKey", "HadWeights", "DetStore key of the output CaloLocalHadCoeff object"};
  Gaudi::Property<std::string> m_LCWeightFileName{this, "LCWeightFileName", "", "Name of the input LC weights ROOT file"};
};

#endif
