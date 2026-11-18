/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCALIBTEST_LARREADHADDMCOEFFFILE
#define LARCALIBTEST_LARREADHADDMCOEFFFILE

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloConditions/CaloHadDMCoeff.h"

#include <vector>
#include <string>


class LArReadHadDMCoeffFile : public AthAlgorithm
{
 public:
  using AthAlgorithm::AthAlgorithm;
  ~LArReadHadDMCoeffFile();

  StatusCode initialize(); 
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();
  StatusCode initDataFromFile(const std::string& hadDMCoeffFileName);

 private:
  Gaudi::Property<std::string> m_key{this, "CorrectionKey", "HadDMCoeff"};
  Gaudi::Property<std::string> m_hadDMCoeffFileName{this, "HadDMCoeffFileName", ""};

  CaloHadDMCoeff* m_data{nullptr};

};

#endif
