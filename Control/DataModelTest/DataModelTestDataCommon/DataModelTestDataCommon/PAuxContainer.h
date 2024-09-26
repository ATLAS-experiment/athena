// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/PAuxContainer.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2024
 * @brief Class used for testing xAOD data reading/writing with packed containers.
 */


#ifndef DATAMODELTESTDATACOMMON_PAUXCONTAINER_H
#define DATAMODELTESTDATACOMMON_PAUXCONTAINER_H


#include "DataModelTestDataCommon/versions/PAuxContainer_v1.h"


namespace DMTest {


using PAuxContainer = PAuxContainer_v1;


} // namespace DMTest


#include "xAODCore/CLASS_DEF.h"
CLASS_DEF (DMTest::PAuxContainer, 9753, 1)


#endif // not DATAMODELTESTDATACOMMON_PAUXCONTAINER_H
