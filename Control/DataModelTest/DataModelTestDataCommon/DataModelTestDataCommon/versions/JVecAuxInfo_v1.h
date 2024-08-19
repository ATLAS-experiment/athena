// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/JVecAuxInfo_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief For testing jagged vectors.
 */


#ifndef DATAMODELTESTDATACOMMON_JVECAUXINFO_V1_H
#define DATAMODELTESTDATACOMMON_JVECAUXINFO_V1_H


#include "DataModelTestDataCommon/CVec.h"
#include "xAODCore/JaggedVec.h"
#include "xAODCore/AuxInfoBase.h"
#include "AthLinks/ElementLink.h"
#include <string>


namespace DMTest {


/**
 * @brief For testing jagged vectors.
 */
class JVecAuxInfo_v1
  : public xAOD::AuxInfoBase
{
public:
  JVecAuxInfo_v1();


private:
  AUXVAR_JAGGEDVEC_DECL(float,             fvec);
  AUXVAR_JAGGEDVEC_DECL(int,               ivec);
  AUXVAR_JAGGEDVEC_DECL(std::string,       svec);
  AUXVAR_JAGGEDVEC_DECL(ElementLink<CVec>, lvec);
};


} // namespace DMTest


SG_BASE (DMTest::JVecAuxInfo_v1, xAOD::AuxInfoBase);


#endif // not DATAMODELTESTDATACOMMON_JVECAUXINFO_V1_H
