// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/JVecAuxContainer.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief For testing jagged vectors.
 */


#ifndef DATAMODELTESTDATACOMMON_JVECAUXCONTAINER_H
#define DATAMODELTESTDATACOMMON_JVECAUXCONTAINER_H


#include "DataModelTestDataCommon/versions/JVecAuxContainer_v1.h"


namespace DMTest {


typedef JVecAuxContainer_v1 JVecAuxContainer;


} // namespace DMTest


#include "xAODCore/CLASS_DEF.h"
CLASS_DEF (DMTest::JVecAuxContainer, 31761279, 1)


#endif // not DATAMODELTESTDATACOMMON_JVECAUXCONTAINER_H
