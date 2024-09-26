// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/P.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2024
 * @brief Class used for testing xAOD data reading/writing with packed containers.
 */


#ifndef DATAMODELTESTDATACOMMON_P_H
#define DATAMODELTESTDATACOMMON_P_H


#include "DataModelTestDataCommon/versions/P_v1.h"


namespace DMTest {


using P = P_v1;


} // namespace DMTest


#include "xAODCore/CLASS_DEF.h"
CLASS_DEF (DMTest::P, 9751, 1)


#endif // not DATAMODELTESTDATACOMMON_P_H
