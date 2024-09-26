/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataCommon/src/PAuxContainer_v1.xcx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2024
 * @brief Class used for testing xAOD data reading/writing with packed containers.
 */


#include "DataModelTestDataCommon/versions/PAuxContainer_v1.h"
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


PAuxContainer_v1::PAuxContainer_v1()
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
    throw std::runtime_error ("Can't set packing options in PAuxContainer_v1");
  }
}


} // namespace DMTest
