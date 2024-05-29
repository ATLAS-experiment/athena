// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataWrite/versions/AllocTestAuxContainer_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Testing an xAOD object with a non-standard memory allocator.
 */


#ifndef DATAMODELTESTDATAWRITE_ALLOCTESTAUXCONTAINER_V1_H
#define DATAMODELTESTDATAWRITE_ALLOCTESTAUXCONTAINER_V1_H


#include "xAODCore/AuxContainerBase.h"
#include "AthenaKernel/BaseInfo.h"
#include <vector>
#include <memory>


namespace DMTest {


/**
 * @brief Testing an xAOD object with a non-standard memory allocator.
 *
 * This class does not use non-default allocators.
 * The identically-named class in DataModelTestDataRead is the same
 * except for using non-default allocators.  That will be used
 * for testing forwards/backwards compatibility.
 */
class AllocTestAuxContainer_v1
  : public xAOD::AuxContainerBase
{
public:
  AllocTestAuxContainer_v1();

  AUXVAR_DECL (int, atInt1);
  AUXVAR_DECL (int, atInt2);
};


} // namespace DMTest


SG_BASE (DMTest::AllocTestAuxContainer_v1, xAOD::AuxContainerBase);


#endif // not DATAMODELTESTDATAWRITE_ALLOCTESTAUXCONTAINER_V1_H
