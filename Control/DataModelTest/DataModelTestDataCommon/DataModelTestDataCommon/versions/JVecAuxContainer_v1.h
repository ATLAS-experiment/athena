// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/versions/JVecAuxContainer_c1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief For testing jagged vectors.
 */


#ifndef DATAMODELTESTDATACOMMON_JVECAUXCONTAINER_C1_H
#define DATAMODELTESTDATACOMMON_JVECAUXCONTAINER_C1_H


#include "DataModelTestDataCommon/CVec.h"
#include "xAODCore/JaggedVec.h"
#include "xAODCore/AuxContainerBase.h"
#include "AthLinks/ElementLink.h"
#include <string>


namespace DMTest {


/**
 * @brief For testing jagged vectors.
 */
class JVecAuxContainer_v1
  : public xAOD::AuxContainerBase
{
public:
  JVecAuxContainer_v1();


private:
  AUXVAR_JAGGEDVEC_DECL(float,             fvec);
  AUXVAR_JAGGEDVEC_DECL(int,               ivec);
  AUXVAR_JAGGEDVEC_DECL(std::string,       svec);
  AUXVAR_JAGGEDVEC_DECL(ElementLink<CVec>, lvec);
};


} // namespace DMTest


SG_BASE (DMTest::JVecAuxContainer_v1, xAOD::AuxContainerBase);


#endif // not DATAMODELTESTDATACOMMON_JVECAUXCONTAINER_C1_H
