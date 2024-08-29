// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/PLinksContainer.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2023
 * @brief For testing packed links.
 */


#ifndef DATAMODELTESTDATACOMMON_PLINKSCONTAINER_H
#define DATAMODELTESTDATACOMMON_PLINKSCONTAINER_H


#include "DataModelTestDataCommon/versions/PLinksContainer_v1.h"


namespace DMTest {


typedef PLinksContainer_v1 PLinksContainer;


} // namespace DMTest


#include "xAODCore/CLASS_DEF.h"
CLASS_DEF (DMTest::PLinksContainer, 1252897697, 1)


#endif // not DATAMODELTESTDATACOMMON_PLINKSCONTAINER_H
