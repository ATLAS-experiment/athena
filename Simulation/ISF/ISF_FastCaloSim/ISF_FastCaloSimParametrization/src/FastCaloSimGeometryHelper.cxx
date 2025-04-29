/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FastCaloSimGeometryHelper.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "CaloDetDescrUtils/CaloDetDescrBuilder.h"
#include "AthenaKernel/getMessageSvc.h"

using namespace std;

/** Constructor **/
FastCaloSimGeometryHelper::FastCaloSimGeometryHelper(const std::string& t, const std::string& n, const IInterface* p) : base_class(t,n,p), CaloGeometry()
{
}

FastCaloSimGeometryHelper::~FastCaloSimGeometryHelper()
= default;

// Athena algtool's Hooks
StatusCode FastCaloSimGeometryHelper::initialize()
{
  ATH_MSG_INFO("Initializing FastCaloSimGeometryHelper");
  m_caloMgr = detStore()->tryConstRetrieve<CaloDetDescrManager>(caloMgrStaticKey);
  if(!m_caloMgr) {
    std::unique_ptr<CaloDetDescrManager> caloMgrPtr = buildCaloDetDescrNoAlign(serviceLocator()
									       , Athena::getMessageSvc());
    ATH_CHECK(detStore()->record(std::move(caloMgrPtr), caloMgrStaticKey));
    ATH_CHECK(detStore()->retrieve(m_caloMgr, caloMgrStaticKey));
  }
  LoadGeometryFromCaloDDM();  
  return StatusCode::SUCCESS;
}

StatusCode FastCaloSimGeometryHelper::finalize()
{
  ATH_MSG_INFO("Finalizing FastCaloSimGeometryHelper");

  return StatusCode::SUCCESS;
}

bool FastCaloSimGeometryHelper::LoadGeometryFromCaloDDM()
{
  ATH_MSG_INFO("Start LoadGeometryFromCaloDDM()");

  int jentry=0;
  for(CaloDetDescrManager::calo_element_const_iterator calo_iter=m_caloMgr->element_begin();calo_iter<m_caloMgr->element_end();++calo_iter)
  {
    const CaloDetDescrElement* pcell=*calo_iter;
    addcell(pcell);

    if(jentry%10000==0)
    {
      ATH_MSG_DEBUG("Load calo cell "<<jentry<<" : "<<pcell->getSampling()<<", "<<pcell->identify());
    }
    ++jentry;
  }

  bool ok=PostProcessGeometry();

  ATH_MSG_INFO("Done LoadGeometryFromCaloDDM()");

  return ok;
}
