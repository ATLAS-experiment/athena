/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCALIBTEST_LARREADHADDMCOEFFFILE2
#define LARCALIBTEST_LARREADHADDMCOEFFFILE2

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloConditions/CaloHadDMCoeff2.h"

#include <vector>
#include <string>


class LArReadHadDMCoeffFile2 : public AthAlgorithm
{
  public:
    using AthAlgorithm::AthAlgorithm;
    ~LArReadHadDMCoeffFile2();

    StatusCode initialize(); 
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();
    StatusCode initDataFromFile(const std::string& hadDMCoeffFileName);
    int parse_dim(std::string &sLine, CaloHadDMCoeff2::HadDMDimension &dim);

  private:
    Gaudi::Property<std::string> m_key{this, "CorrectionKey", "HadDMCoeff"};
    Gaudi::Property<std::string> m_hadDMCoeffFileName{this, "HadDMCoeffFileName", ""};

    CaloHadDMCoeff2* m_data{nullptr};
};

#endif
