/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataWrite/src/AllocTestAuxContainer_v1.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Testing an xAOD object with a non-standard memory allocator.
 */


#include "DataModelTestDataWrite/versions/AllocTestAuxContainer_v1.h"


namespace DMTest {


AllocTestAuxContainer_v1::AllocTestAuxContainer_v1()
  : xAOD::AuxContainerBase()
{
}


} // namespace DMTest
