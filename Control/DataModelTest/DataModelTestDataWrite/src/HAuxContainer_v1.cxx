/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataWrite/src/HAuxContainer_v1.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2016
 * @brief Test for xAOD schema evolution.
 */


#include "DataModelTestDataWrite/versions/HAuxContainer_v1.h"
#include "AthContainersInterfaces/AuxDataOption.h"
#include <stdexcept>


namespace DMTest {


HAuxContainer_v1::HAuxContainer_v1()
  : xAOD::AuxContainerBase()
{
}


} // namespace DMTest
