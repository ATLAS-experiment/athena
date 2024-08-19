// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/JVecContainer_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief For testing jagged vectors.
 */


#ifndef DATAMODELTESTDATACOMMON_JVECCONTAINER_V1_H
#define DATAMODELTESTDATACOMMON_JVECCONTAINER_V1_H


#include "DataModelTestDataCommon/versions/JVec_v1.h"
#include "AthContainers/DataVector.h"


namespace DMTest {


typedef DataVector<JVec_v1> JVecContainer_v1;


} // namespace DMTest


#endif // not DATAMODELTESTDATACOMMON_JVECCONTAINER_V1_H
