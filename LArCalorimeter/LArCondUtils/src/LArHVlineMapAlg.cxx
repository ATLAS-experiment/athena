/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArHVlineMapAlg.h" 
#include "LArIdentifier/LArOnlineID.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloIdManager.h"

LArHVlineMapAlg::LArHVlineMapAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name,pSvcLocator),
  m_hvmapTool("LArHVMapTool",this)
{}

// intialize 
StatusCode LArHVlineMapAlg::initialize()
{
  ATH_CHECK(m_mapKey.initialize());
  ATH_CHECK(m_hvCablingKey.initialize());
  ATH_CHECK(m_caloDetDescrMgrKey.initialize());
  ATH_CHECK(detStore()->retrieve(m_caloHelper, "CaloCell_ID") );
  ATH_CHECK(detStore()->retrieve(m_hvlineHelper, "LArHVLineID") );

  const CaloIdManager *caloIdMgr;
  ATH_CHECK(detStore()->retrieve(caloIdMgr));

  return StatusCode::SUCCESS;
}

StatusCode LArHVlineMapAlg::execute(const EventContext& ctx ) const 
{
  SG::WriteCondHandle<LArHVNMap> writeHandle{m_mapKey,ctx};
  
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("Found valid write handle");
    return StatusCode::SUCCESS;
  }  

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloDetDescrMgrKey,ctx};
  const CaloDetDescrManager *calodetdescrmgr = *caloMgrHandle;
  if(!calodetdescrmgr) {
     ATH_MSG_ERROR("Why do not have CaloDetDescrManager ? "<<m_caloDetDescrMgrKey.fullKey());
     return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<LArHVIdMapping> hvCabling (m_hvCablingKey, ctx);
  if(!hvCabling.isValid()) {
     ATH_MSG_ERROR("Could not get HV mapping !");
     return StatusCode::FAILURE;
  } else {
     ATH_MSG_DEBUG("Got HV mapping !");
  }

  writeHandle.addDependency(hvCabling);

  unsigned hmax=m_hvlineHelper->hvlineHashMax();

  std::unique_ptr<LArHVNMap> hvmap=std::make_unique<LArHVNMap>(hmax,m_hvlineHelper);

  CaloCell_Base_ID ::id_iterator itb = m_caloHelper->cell_begin();
  CaloCell_Base_ID ::id_iterator ite = m_caloHelper->cell_end();

  for(; itb<ite; ++itb) {

     if(m_caloHelper->is_tile(*itb)) continue;
     std::vector<HWIdentifier> linevec;
     m_hvmapTool->GetHVLines(*itb, calodetdescrmgr, linevec);
     if(linevec.size()) {
        for(unsigned int i=0; i<linevec.size(); ++i) {
           unsigned lhash = m_hvlineHelper->hvlineHash(linevec[i]);
           hvmap->incHVline(lhash);
           ATH_MSG_VERBOSE("Adding cell: "<<itb->get_identifier32().get_compact() << " to a line: "<<linevec[i].get_identifier32().get_compact());
        }
     } else {
        ATH_MSG_WARNING("Why do not have HVlines for the cell: " << itb->get_identifier32().get_compact());
     }
  }

  if(writeHandle.record(std::move(hvmap)).isFailure()) {
     ATH_MSG_ERROR("Could not record LArHVNMap object with " << writeHandle.key());
     return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("recorded new " << writeHandle.key());

  return StatusCode::SUCCESS;

}

