/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/* Takashi Kubota - June 30, 2008 */
#ifndef TGCCOINDATACNV_P2_H
#define TGCCOINDATACNV_P2_H

//-----------------------------------------------------------------------------
//
// file:   TgcCoinDataCnv_p2.h
//
//-----------------------------------------------------------------------------

#include "MuonTrigCoinData/TgcCoinData.h"
#include "TgcCoinData_p2.h"

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

#include "TrkEventTPCnv/TrkEventPrimitives/LocalPositionCnv_p1.h"
#include "TrkEventTPCnv/TrkEventPrimitives/ErrorMatrixCnv_p1.h"

class MsgStream;

class TgcCoinDataCnv_p2
    : public T_AthenaPoolTPPolyCnvBase< Muon::TgcCoinData, Muon::TgcCoinData, Muon::TgcCoinData_p2 >
{
public:
 TgcCoinDataCnv_p2() = default;

    void persToTrans( const Muon::TgcCoinData_p2 *persObj,
        Muon::TgcCoinData    *transObj,
        MsgStream                &log );
    void transToPers( const Muon::TgcCoinData    *transObj,
        Muon::TgcCoinData_p2 *persObj,
        MsgStream                &log );

 protected:        
    LocalPositionCnv_p1   *m_localPosCnv{nullptr};
    ErrorMatrixCnv_p1     *m_errorMxCnv{nullptr};
    unsigned int          m_numErrsPrinted{0};
};

#endif 
