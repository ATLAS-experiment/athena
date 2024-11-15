/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file LArConditionsTestAlg.h
 *
 * @brief This file contains an algorithm for testing lar conditions
 * data access
 *
 * @author RD Schaffer  <R.D.Schaffer@cern.ch>
 * @author Hong Ma      <hma@bnl.gov>
 *
 * $Id: LArConditionsTestAlg.h,v 1.7 2009-01-15 15:06:50 gunal Exp $ */
#ifndef LARIOV_LARCONDITIONSTESTALG_H
#define LARIOV_LARCONDITIONSTESTALG_H

/**
 * @class LArConditionsTestAlg
 *
 * @brief Athena algorithm used for testing LAr conditions data access
 *
**/

#include "AthenaBaseComps/AthAlgorithm.h" 
#include "CxxUtils/checker_macros.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRecConditions/LArCalibLineMapping.h"
#include "LArRecConditions/LArFebRodMapping.h"
#include "LArConditionsTest/LArRampPTmp.h" 


class LArOnlineID; 
class GenericDbTable; 
class LArRampMC;

class ATLAS_NOT_THREAD_SAFE LArConditionsTestAlg : public AthAlgorithm
{

public:

    LArConditionsTestAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~LArConditionsTestAlg();

    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

private:

    StatusCode createCompareObjects();
    StatusCode testCondObjects();
    StatusCode testEachCondObject ATLAS_NOT_THREAD_SAFE (const LArRampMC* ramps);
    StatusCode testChannelSet();
    StatusCode testDbObjectRead() ;

    // Cache of compare data
    std::vector<LArRampPTmp>          m_rampCache;
    std::vector<LArRampPTmp>          m_rampCorrections;

    const LArOnlineID* m_onlineID; 
    BooleanProperty    m_testCondObjs;
    BooleanProperty    m_readCondObjs;
    BooleanProperty    m_writeCondObjs;
    BooleanProperty    m_writeCorrections;
    BooleanProperty    m_applyCorrections;
    BooleanProperty    m_testReadDB; 
    BooleanProperty    m_TB; 
    int                m_tbin ;
};
#endif // LARIOV_LARCONDITIONSTESTALG_H

