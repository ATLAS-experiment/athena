// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/JVec.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief For testing jagged vectors.
 */


#ifndef DATAMODELTESTDATACOMMON_JVEC_H
#define DATAMODELTESTDATACOMMON_JVEC_H


#include "DataModelTestDataCommon/versions/JVec_v1.h"


namespace DMTest {


typedef JVec_v1 JVec;


} // namespace DMTest


#include "xAODCore/CLASS_DEF.h"
CLASS_DEF (DMTest::JVec, 189265613, 1)


#endif // not DATAMODELTESTDATACOMMON_JVEC_H
