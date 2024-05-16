// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/versions/CLinksAuxContainer_v1.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2019
 * @brief For testing thinning of xAOD objects.
 */


#ifndef DATAMODELTESTDATACOMMON_CLINKSAUXCONTAINER_V1_H
#define DATAMODELTESTDATACOMMON_CLINKSAUXCONTAINER_V1_H


#include "DataModelTestDataCommon/CVec.h"
#include "AthLinks/ElementLink.h"
#include "xAODCore/AuxContainerBase.h"


namespace DMTest {


/**
 * @brief For testing thinning of xAOD objects.
 *
 * A CLinks object holds both an ElementLink and a vector<ElementLink>
 * to CVec objects.
 */
class CLinksAuxContainer_v1
  : public xAOD::AuxContainerBase
{
public:
  CLinksAuxContainer_v1();


private:
  AUXVAR_DECL (ElementLink<CVec>, link);
  AUXVAR_DECL (std::vector<ElementLink<CVec> >, links);
};


} // namespace DMTest


SG_BASE (DMTest::CLinksAuxContainer_v1, xAOD::AuxContainerBase);


#endif // not DATAMODELTESTDATACOMMON_CLINKSAUXCONTAINER_V1_H
