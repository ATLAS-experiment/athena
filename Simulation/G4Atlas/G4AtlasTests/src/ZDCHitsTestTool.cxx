/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ZDCHitsTestTool.h"

#include "ZDC_SimEvent/ZDC_SimFiberHit.h"
#include "ZDC_SimEvent/ZDC_SimFiberHit_Collection.h"


#include <TH2D.h>
#include <TH1.h>
#include <TProfile.h>

ZDCHitsTestTool::ZDCHitsTestTool(const std::string& type, const std::string& name, const IInterface* parent)
    : SimTestToolBase(type, name, parent)
{  
  for(int side : {0,1}){
    for(int module = 0; module < 4; module++){
      m_zdc[side][module] = nullptr;
    }
    for(int channel = 0; channel < 16; channel++){
      m_rpd[side][channel] = nullptr;
    }
  }
}

StatusCode ZDCHitsTestTool::initialize()
{
  ATH_CHECK (detStore()->retrieve( m_ZdcID ) );

  m_path+="ZDC/";

  for(int side : {0,1}){
    std::string sideStr = (side == 0) ? "A" : "C";
    //ZDCs
    for(int module = 0; module < 4; module++){
      _TH1D(m_zdc[side][module],Form("zdc_side%c_%d", std::tolower(sideStr[0]), module),100,0.,1.e6);
      _SET_TITLE(m_zdc[side][module], Form("Cherenkov photons in ZDC - Side %s - Module %d", sideStr.c_str(), module),"n_{gamma}","counts");
    }
    //RPDs
    for(int channel = 0; channel < 16; channel++){
      _TH1D(m_rpd[side][channel],Form("rpd_side%c_%d", std::tolower(sideStr[0]), channel),100,0.,1.e4);
      _SET_TITLE(m_rpd[side][channel], Form("Cherenkov photons in RPD - Side %s - Channel %d", sideStr.c_str(), channel),"n_{gamma}","counts");
    }
  }
  
  return StatusCode::SUCCESS;
}

StatusCode ZDCHitsTestTool::processEvent() {

  const ZDC_SimFiberHit_Collection* iter;
  CHECK( evtStore()->retrieve(iter,"ZDC_SimFiberHit_Collection") );

  ZDC_SimFiberHit_ConstIterator it;
  for (it=(*iter).begin(); it != (*iter).end(); ++it) {
    ZDC_SimFiberHit ghit(*it);

    Identifier id = ghit.getID( );
    int side = (m_ZdcID->side(id) == -1) ? 0 : 1;
    int module = m_ZdcID->module(id);
    int channel = m_ZdcID->channel(id);

    if(module < 4){//ZDC
      m_zdc[side][module]->Fill(ghit.getNPhotons());

    }else if(module == 4){//RPD
      m_rpd[side][channel]->Fill(ghit.getNPhotons());

    }else{//Undefined
      ATH_MSG_ERROR("ZDCHitsTestTool::processEvent Hit detected in an undefined module. Module# " << module);
    }
  }

  return StatusCode::SUCCESS;
}
