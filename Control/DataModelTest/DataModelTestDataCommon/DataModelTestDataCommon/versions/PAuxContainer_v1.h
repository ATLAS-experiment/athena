// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/versions/PAuxContainer_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2024
 * @brief Class used for testing xAOD data reading/writing with packed containers.
 */


#ifndef DATAMODELTESTDATACOMMON_PAUXCONTAINER_V1_H
#define DATAMODELTESTDATACOMMON_PAUXCONTAINER_V1_H


#include "xAODCore/AuxContainerBase.h"
#include "AthContainers/PackedContainer.h"
#include "AthenaKernel/BaseInfo.h"
#include <vector>


namespace DMTest {


/**
 * @brief Class used for testing xAOD data reading/writing with packed containers.
 */
class PAuxContainer_v1
  : public xAOD::AuxContainerBase
{
public:
  PAuxContainer_v1();

private:
  AUXVAR_PACKEDCONTAINER_DECL (unsigned int, pInt);
  AUXVAR_PACKEDCONTAINER_DECL (float, pFloat);
  AUXVAR_PACKEDCONTAINER_DECL (std::vector<int>, pvInt);
  AUXVAR_PACKEDCONTAINER_DECL (std::vector<float>, pvFloat);
};


} // namespace DMTest


SG_BASE (DMTest::PAuxContainer_v1, xAOD::AuxContainerBase);



#endif // not DATAMODELTESTDATACOMMON_PAUXCONTAINER_V1_H
