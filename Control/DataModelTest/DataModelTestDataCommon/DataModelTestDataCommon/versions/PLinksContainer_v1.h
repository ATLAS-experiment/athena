// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/versions/PLinksContainer_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2023
 * @brief For testing packed links.
 */


#ifndef DATAMODELTESTDATACOMMON_PLINKSCONTAINER_V1_H
#define DATAMODELTESTDATACOMMON_PLINKSCONTAINER_V1_H


#include "DataModelTestDataCommon/versions/PLinks_v1.h"
#include "AthContainers/DataVector.h"


namespace DMTest {


typedef DataVector<PLinks_v1> PLinksContainer_v1;


} // namespace DMTest


#endif // not DATAMODELTESTDATACOMMON_PLINKSCONTAINER_V1_H
