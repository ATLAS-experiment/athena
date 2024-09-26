/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataCommon/src/C_v1.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2014
 * @brief Class used for testing xAOD data reading/writing with packed containers.
 */


#include "DataModelTestDataCommon/versions/P_v1.h"
#include "xAODCore/AuxStoreAccessorMacros.h"


namespace DMTest {


AUXSTORE_PRIMITIVE_SETTER_AND_GETTER (P_v1, unsigned int,    pInt,  setPInt)
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER (P_v1, float,    pFloat,  setPFloat)

AUXSTORE_OBJECT_SETTER_AND_GETTER (P_v1, std::vector<int>,    pvInt,  setPVInt)
AUXSTORE_OBJECT_MOVE (P_v1, std::vector<int>,    pvInt,  setPVInt)

AUXSTORE_OBJECT_SETTER_AND_GETTER (P_v1, std::vector<float>,    pvFloat,  setPVFloat)
AUXSTORE_OBJECT_MOVE (P_v1, std::vector<float>,    pvFloat,  setPVFloat)


} // namespace DMTest
