// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataCommon/versions/CAuxContainer_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2014
 * @brief Class used for testing xAOD data reading/writing.
 */


#ifndef DATAMODELTESTDATACOMMON_CAUXCONTAINER_V1_H
#define DATAMODELTESTDATACOMMON_CAUXCONTAINER_V1_H


#include "xAODCore/AuxContainerBase.h"
#include "AthContainers/PackedContainer.h"
#include "AthenaKernel/BaseInfo.h"
#include <vector>


namespace DMTest {


class CAuxContainer_v1
  : public xAOD::AuxContainerBase
{
public:
  CAuxContainer_v1();

private:
  AUXVAR_DECL (int, anInt);
  AUXVAR_DECL (float, aFloat);
  AUXVAR_PACKEDCONTAINER_DECL (unsigned int, pInt);
  AUXVAR_PACKEDCONTAINER_DECL (float, pFloat);
  AUXVAR_PACKEDCONTAINER_DECL (std::vector<int>, pvInt);
  AUXVAR_PACKEDCONTAINER_DECL (std::vector<float>, pvFloat);
};


} // namespace DMTest


SG_BASE (DMTest::CAuxContainer_v1, xAOD::AuxContainerBase);


#endif // not DATAMODELTESTDATACOMMON_CAUXCONTAINER_V1_H
