// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/versions/P_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2024
 * @brief Class used for testing xAOD data reading/writing with packed containers.
 */


#ifndef DATAMODELTESTDATACOMMON_P_V1_H
#define DATAMODELTESTDATACOMMON_P_V1_H


#include "AthContainers/AuxElement.h"
#include "AthenaKernel/BaseInfo.h"


namespace DMTest {


/**
 * @brief Class used for testing xAOD data reading/writing with packed containers.
 */
class P_v1
  : public SG::AuxElement
{
public:
  unsigned int pInt() const;
  void setPInt (unsigned int);

  float pFloat() const;
  void setPFloat (float);

  const std::vector<int>& pvInt() const;
  void setPVInt (const std::vector<int>&);
  void setPVInt (std::vector<int>&&);

  const std::vector<float>& pvFloat() const;
  void setPVFloat (const std::vector<float>&);
  void setPVFloat (std::vector<float>&&);
};


} // namespace DMTest


SG_BASE (DMTest::P_v1, SG::AuxElement);


#endif // not DATAMODELTESTDATACOMMON_P_V1_H
