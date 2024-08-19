// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/JVecAuxInfo.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief For testing jagged vectors.
 */


#ifndef DATAMODELTESTDATACOMMON_JVECAUXINFO_H
#define DATAMODELTESTDATACOMMON_JVECAUXINFO_H



#include "DataModelTestDataCommon/versions/JVecAuxInfo_v1.h"


namespace DMTest {


typedef JVecAuxInfo_v1 JVecAuxInfo;


} // namespace DMTest


#include "xAODCore/CLASS_DEF.h"
CLASS_DEF (DMTest::JVecAuxInfo, 114522814, 1)


#endif // not DATAMODELTESTDATACOMMON_JVECAUXINFO_H
