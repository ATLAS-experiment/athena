/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-
#ifndef CALOLOCALHADCALIB_CALOREADLCOUTOFCLUSTERFILE
#define CALOLOCALHADCALIB_CALOREADLCOUTOFCLUSTERFILE

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloConditions/CaloLocalHadCoeff.h"
#include <string>


class CaloReadLCOutOfClusterFile : public AthAlgorithm
{
 public:
  CaloReadLCOutOfClusterFile(const std::string & name, ISvcLocator * pSvcLocator);
  ~CaloReadLCOutOfClusterFile();

  StatusCode initialize(); 
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();
  StatusCode initDataFromFile(const std::string& theLCOutOfClusterFileName,
                              CaloLocalHadCoeff& data);

 private:
  Gaudi::Property<std::string> m_key{this, "CorrectionKey", "OOC", "DetStore key of the output CaloLocalHadCoeff object"};
  Gaudi::Property<std::string> m_LCOutOfClusterFileName{this, "LCOutOfClusterFileName", "", "Name of the input LC out-of-cluster ROOT file"};
};

#endif
