// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/PLinksAuxContainer.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2023
 * @brief For testing packed links.
 */


#ifndef DATAMODELTESTDATACOMMON_PLINKSAUXCONTAINER_H
#define DATAMODELTESTDATACOMMON_PLINKSAUXCONTAINER_H


#include "DataModelTestDataCommon/versions/PLinksAuxContainer_v1.h"


namespace DMTest {


typedef PLinksAuxContainer_v1 PLinksAuxContainer;


} // namespace DMTest


#include "xAODCore/CLASS_DEF.h"
CLASS_DEF (DMTest::PLinksAuxContainer, 1309326410, 1)


#endif // not DATAMODELTESTDATACOMMON_PLINKSAUXCONTAINER_H
