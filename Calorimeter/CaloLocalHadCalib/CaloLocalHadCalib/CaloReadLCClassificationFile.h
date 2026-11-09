/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-
#ifndef CALOLOCALHADCALIB_CALOREADLCCLASSIFICATIONFILE
#define CALOLOCALHADCALIB_CALOREADLCCLASSIFICATIONFILE

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloConditions/CaloLocalHadCoeff.h"
#include <string>


class CaloReadLCClassificationFile : public AthAlgorithm
{
 public:
  CaloReadLCClassificationFile(const std::string & name, ISvcLocator * pSvcLocator);
  ~CaloReadLCClassificationFile();

  StatusCode initialize(); 
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();
  StatusCode initDataFromFile(const std::string& theLCClassificationFileName,
                              CaloLocalHadCoeff& data);

 private:
  Gaudi::Property<std::string> m_key{this, "ClassificationKey", "EMFrac", "DetStore key of the output CaloLocalHadCoeff object"};
  Gaudi::Property<std::string> m_LCClassificationFileName{this, "LCClassificationFileName", "", "Name of the input LC classification ROOT file"};
};

#endif
