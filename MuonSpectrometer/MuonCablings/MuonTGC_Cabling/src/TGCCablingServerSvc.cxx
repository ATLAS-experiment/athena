/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_CHECK_FILE_THREAD_SAFETY;

#include "MuonTGC_Cabling/TGCCablingServerSvc.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/StatusCode.h"
#include "StoreGate/StoreGateSvc.h"

namespace Muon {

TGCCablingServerSvc::TGCCablingServerSvc(const std::string& name, ISvcLocator* sl) : 
AthService(name, sl) {}

StatusCode TGCCablingServerSvc::queryInterface(const InterfaceID& riid, void** ppvIF) {
    if( IID_TGCcablingServerSvc.versionMatch(riid) ) 
    { 
        *ppvIF = dynamic_cast<ITGCcablingServerSvc*>(this); 
    } else { 
        ATH_MSG_DEBUG ( name() << " cast to Service Interface" );
        return AthService::queryInterface(riid, ppvIF); 
    }
    
    addRef();
    return StatusCode::SUCCESS;
}

StatusCode TGCCablingServerSvc::giveCabling(const ITGCcablingSvc*& cabling) const {
    ATH_MSG_DEBUG ( "requesting instance of TGCcabling" );
    
    cabling = nullptr;
    
    if(m_atlas) {
        ATH_CHECK( service("MuonTGC_CablingSvc",cabling,true) );
    } else {
        ATH_CHECK( service("TGCcablingSvc",cabling,true) );
    }

    return StatusCode::SUCCESS;
}

bool TGCCablingServerSvc::isAtlas() const {
    return m_atlas;
}

}
