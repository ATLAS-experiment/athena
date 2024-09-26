// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/versions/PVec_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2024
 * @brief Class used for testing xAOD data reading/writing with packed containers.
 */


#ifndef DATAMODELTESTDATACOMMON_PVEC_V1_H
#define DATAMODELTESTDATACOMMON_PVEC_V1_H


#include "DataModelTestDataCommon/versions/P_v1.h"
#include "AthContainers/DataVector.h"


namespace DMTest {


using PVec_v1 = DataVector<P_v1>;


} // namespace DMTest


#endif // not DATAMODELTESTDATACOMMON_PVEC_V1_H
