// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/PLinksAuxInfo.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2024
 * @brief For testing packed links.
 */


#ifndef DATAMODELTESTDATACOMMON_PLINKSAUXINFO_H
#define DATAMODELTESTDATACOMMON_PLINKSAUXINFO_H



#include "DataModelTestDataCommon/versions/PLinksAuxInfo_v1.h"


namespace DMTest {


typedef PLinksAuxInfo_v1 PLinksAuxInfo;


} // namespace DMTest


#include "xAODCore/CLASS_DEF.h"
CLASS_DEF (DMTest::PLinksAuxInfo, 157272641, 1)


#endif // not DATAMODELTESTDATACOMMON_PLINKSAUXINFO_H
