// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataCommon/versions/CVecWithData_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2015
 * @brief Test writing a container with metadata.
 */


#ifndef DATAMODELTESTDATACOMMON_CVECWITHDATA_V1_H
#define DATAMODELTESTDATACOMMON_CVECWITHDATA_V1_H


#include "DataModelTestDataCommon/versions/CVec_v1.h"


namespace DMTest {


class CVecWithData_v1
  : public CVec_v1
{
public:
  using CVec_v1::CVec_v1;
  int meta1 = 0;
};


} // namespace DMTest


#endif // not DATAMODELTESTDATACOMMON_CVECWITHDATA_V1_H
