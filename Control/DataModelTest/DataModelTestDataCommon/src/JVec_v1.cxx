/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/src/JVec_v1.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief For testing jagged vectors.
 */


#include "DataModelTestDataCommon/versions/JVec_v1.h"
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "AthContainers/JaggedVec.h"


namespace DMTest {


AUXSTORE_OBJECT_SETTER_AND_GETTER(JVec_v1, SG::JaggedVecElt<int>, ivec, setIVec)
AUXSTORE_OBJECT_SETTER_AND_GETTER(JVec_v1, SG::JaggedVecElt<float>, fvec, setFVec)
AUXSTORE_OBJECT_SETTER_AND_GETTER(JVec_v1, SG::JaggedVecElt<std::string>, svec, setSVec)
AUXSTORE_OBJECT_SETTER_AND_GETTER(JVec_v1, SG::JaggedVecElt<ElementLink<CVec> >, lvec, setLVec)


} // namespace DMTest

