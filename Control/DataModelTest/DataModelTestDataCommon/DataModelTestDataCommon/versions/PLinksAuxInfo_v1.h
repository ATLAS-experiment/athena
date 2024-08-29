// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/versions/PLinksAuxInfo_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2024
 * @brief For testing packed links.
 */


#ifndef DATAMODELTESTDATACOMMON_PLINKSAUXINFO_V1_H
#define DATAMODELTESTDATACOMMON_PLINKSAUXINFO_V1_H


#include "DataModelTestDataCommon/CVec.h"
#include "xAODCore/PackedLink.h"
#include "xAODCore/AuxInfoBase.h"


namespace DMTest {


/**
 * @brief For testing packed links.
 */
class PLinksAuxInfo_v1
  : public xAOD::AuxInfoBase
{
public:
  PLinksAuxInfo_v1();


private:
  AUXVAR_PACKEDLINK_DECL(CVec, plink);
  AUXVAR_PACKEDLINKVEC_DECL(std::vector, CVec, vlinks);
};


} // namespace DMTest


SG_BASE (DMTest::PLinksAuxInfo_v1, xAOD::AuxInfoBase);


#endif // not DATAMODELTESTDATACOMMON_PLINKSAUXINFO_V1_H
