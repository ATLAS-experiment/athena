/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataCommon/src/CAuxContainer_v1.xcx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2014
 * @brief Class used for testing xAOD data reading/writing.
 */


#include "DataModelTestDataCommon/versions/CAuxContainer_v1.h"
#include "AthContainersInterfaces/AuxDataOption.h"
#include <stdexcept>


#define CHECK_OPTION(ret)                       \
  do {                                          \
    if (!ret) {                                 \
      ATH_MSG_ERROR("setOption failed");        \
      return StatusCode::FAILURE;               \
    }                                           \
  } while(0)


namespace DMTest {


CAuxContainer_v1::CAuxContainer_v1()
  : xAOD::AuxContainerBase()
{
  if (!pInt.setOption (SG::AuxDataOption ("nbits", 17)) ||
      !pFloat.setOption (SG::AuxDataOption ("nbits", 17)) || 
      !pFloat.setOption (SG::AuxDataOption ("signed", 0)) ||
      !pFloat.setOption (SG::AuxDataOption ("nmantissa", 17)) ||
      !pFloat.setOption (SG::AuxDataOption ("scale", 10)) ||

      !pvInt.setOption (SG::AuxDataOption ("nbits", 13)) ||
      !pvFloat.setOption (SG::AuxDataOption ("nbits", 13)) ||
      !pvFloat.setOption (SG::AuxDataOption ("nmantissa", 12)) )
  {
    throw std::runtime_error ("Can't set packing options in CAuxContainer_v1");
  }
}


} // namespace DMTest
