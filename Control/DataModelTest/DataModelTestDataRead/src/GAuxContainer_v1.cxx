/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataRead/src/GAuxContainer_v1.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2014
 * @brief Test for xAOD auto schema evolution.
 */


#include "DataModelTestDataRead/versions/GAuxContainer_v1.h"
#include "AthContainersInterfaces/AuxDataOption.h"
#include <stdexcept>


namespace DMTest {


GAuxContainer_v1::GAuxContainer_v1()
  : xAOD::AuxContainerBase()
{
  if (!anInt.setOption (SG::AuxDataOption ("nbits", 17)))
  {
    throw std::runtime_error ("Can't set packing options in GAuxContainer_v1");
  }
}


} // namespace DMTest
