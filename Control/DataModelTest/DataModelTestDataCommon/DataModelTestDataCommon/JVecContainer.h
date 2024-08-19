// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/JVecContainer.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief For testing jagged vectors.
 */


#ifndef DATAMODELTESTDATACOMMON_JVECCONTAINER_H
#define DATAMODELTESTDATACOMMON_JVECCONTAINER_H


#include "DataModelTestDataCommon/versions/JVecContainer_v1.h"


namespace DMTest {


typedef JVecContainer_v1 JVecContainer;


} // namespace DMTest


#include "xAODCore/CLASS_DEF.h"
CLASS_DEF (DMTest::JVecContainer, 1256059583, 1)


#endif // not DATAMODELTESTDATACOMMON_JVECCONTAINER_H
