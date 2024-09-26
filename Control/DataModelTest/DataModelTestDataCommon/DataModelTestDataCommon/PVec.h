// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/PVec.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2024
 * @brief Class used for testing xAOD data reading/writing with packed containers.
 */


#ifndef DATAMODELTESTDATACOMMON_PVEC_H
#define DATAMODELTESTDATACOMMON_PVEC_H


#include "DataModelTestDataCommon/versions/PVec_v1.h"


namespace DMTest {


using PVec = PVec_v1;


} // namespace DMTest


#include "xAODCore/CLASS_DEF.h"
CLASS_DEF (DMTest::PVec, 9752, 1)


#endif // not DATAMODELTESTDATACOMMON_PVEC_H
