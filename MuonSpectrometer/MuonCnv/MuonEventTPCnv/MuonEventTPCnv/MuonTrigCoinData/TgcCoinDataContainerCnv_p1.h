/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/* Takashi Kubota - June 30, 2008 */
#ifndef TGCCOINDATACONTAINERCNV_P1_H
#define TGCCOINDATACONTAINERCNV_P1_H

//-----------------------------------------------------------------------------
//
// file:   TgcCoinDataContainerCnv_p1.h
//
//-----------------------------------------------------------------------------

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

#include "MuonTrigCoinData/TgcCoinDataContainer.h"
#include "MuonEventTPCnv/MuonTrigCoinData/MuonCoinDataContainer_p1.h"
#include "TrkEventCnvTools/IEventCnvSuperTool.h"

namespace MuonGM{ class MuonDetectorManager;}

class MsgStream;
class TgcIdHelper;

namespace Muon{
    class TgcCoinDataContainerCnv_p1 : public T_AthenaPoolTPCnvBase< Muon::TgcCoinDataContainer, Muon::MuonCoinDataContainer_p1 >

    {
    public:
        typedef Muon::MuonCoinDataContainer_p1 PERS; 
        typedef Muon::TgcCoinDataContainer TRANS;
        TgcCoinDataContainerCnv_p1(): m_TgcId(0), m_isInitialized(0) {}
        virtual void persToTrans(const PERS* persCont, TRANS* transCont, MsgStream &log); 
        virtual void transToPers(const TRANS* transCont, PERS* persCont, MsgStream &log);
        virtual Muon::TgcCoinDataContainer* createTransient(const Muon::MuonCoinDataContainer_p1* persObj, MsgStream& log);
    private:
        StatusCode initialize(MsgStream &log);

        const TgcIdHelper *m_TgcId;
        ToolHandle  < Trk::IEventCnvSuperTool >   m_eventCnvTool{"Trk::EventCnvSuperTool/EventCnvSuperTool"}; 
        const MuonGM::TgcReadoutElement* getReadOutElement(const Identifier& id ) const;
        bool m_isInitialized;
    };

}
#endif 

