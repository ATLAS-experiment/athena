// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/versions/PLinksAuxContainer_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2023
 * @brief For testing packed links.
 */


#ifndef DATAMODELTESTDATACOMMON_PLINKSAUXCONTAINER_V1_H
#define DATAMODELTESTDATACOMMON_PLINKSAUXCONTAINER_V1_H


#include "DataModelTestDataCommon/CVec.h"
#include "xAODCore/PackedLink.h"
#include "xAODCore/AuxContainerBase.h"


namespace DMTest {


/**
 * @brief For testing packed links.
 */
class PLinksAuxContainer_v1
  : public xAOD::AuxContainerBase
{
public:
  PLinksAuxContainer_v1();


private:
  AUXVAR_PACKEDLINK_DECL(CVec, plink);
  AUXVAR_PACKEDLINKVEC_DECL(std::vector, CVec, vlinks);
};


} // namespace DMTest


SG_BASE (DMTest::PLinksAuxContainer_v1, xAOD::AuxContainerBase);


#endif // not DATAMODELTESTDATACOMMON_PLINKSAUXCONTAINER_V1_H
