// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/PLinks.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2023
 * @brief For testing packed links.
 */


#ifndef DATAMODELTESTDATACOMMON_PLINKS_H
#define DATAMODELTESTDATACOMMON_PLINKS_H


#include "DataModelTestDataCommon/versions/PLinks_v1.h"


namespace DMTest {


typedef PLinks_v1 PLinks;


} // namespace DMTest


#include "xAODCore/CLASS_DEF.h"
CLASS_DEF (DMTest::PLinks, 140140683, 1)


#endif // not DATAMODELTESTDATACOMMON_PLINKS_H
