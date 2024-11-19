/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArHVMapTool.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloDetDescr/CaloDetectorElements.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"

#include "LArHV/EMBHVElectrode.h"
#include "LArHV/EMBHVManager.h"
#include "LArHV/EMBPresamplerHVModule.h"
#include "LArHV/EMBPresamplerHVManager.h"
#include "LArHV/EMECHVElectrode.h"
#include "LArHV/EMECPresamplerHVModule.h"
#include "LArHV/HECHVSubgap.h"
#include "LArHV/FCALHVLine.h"
#include "LArHV/FCALHVManager.h"

LArHVMapTool::LArHVMapTool(const std::string& type, const std::string& name, const IInterface* parent)
  : AthAlgTool(type,name,parent)
{
}

StatusCode LArHVMapTool::initialize(){

  CHECK(detStore()->retrieve(m_caloIdMgr));

  m_larem_id   = m_caloIdMgr->getEM_ID();
  m_larhec_id   = m_caloIdMgr->getHEC_ID();
  m_larfcal_id   = m_caloIdMgr->getFCAL_ID();

  ATH_CHECK(m_caloDetDescrMgrKey.initialize());
  ATH_CHECK( m_hvCablingKey.initialize() );

  ATH_MSG_DEBUG("LArHVMapTool::initialize ...");

  const EventContext& ctx = Gaudi::Hive::currentContext();

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloDetDescrMgrKey,ctx};
  const CaloDetDescrManager *calodetdescrmgr = *caloMgrHandle;
  SG::ReadCondHandle<LArHVIdMapping> hvCabling (m_hvCablingKey, ctx);
  m_hvmapping=*hvCabling;
  if(!m_hvmapping) {
     ATH_MSG_ERROR("Could not get HV mapping !");
     return StatusCode::FAILURE;
  } else {
     ATH_MSG_DEBUG("Got HV mapping !");
  }

  IdentifierHash h=0;
  Identifier offId;
  m_larem_id->get_id(h,offId);
  std::vector<int> hvlineVec;
  GetHVLines(offId, calodetdescrmgr, hvlineVec);

  return StatusCode::SUCCESS;
}

void LArHVMapTool::GetHVLines(const Identifier& id, const CaloDetDescrManager *calodetdescrmgr, 
                              std::vector<int> &hvLineVec) const {
   return GetHVLinesCore(id, calodetdescrmgr, &hvLineVec, nullptr);
}

void LArHVMapTool::GetHVLines(const Identifier& id, const CaloDetDescrManager *calodetdescrmgr, 
                              std::vector<HWIdentifier> &hvLineId) const {
   return GetHVLinesCore(id, calodetdescrmgr, nullptr, &hvLineId);
}

void LArHVMapTool::GetHVLinesCore(const Identifier& id, const CaloDetDescrManager *calodetdescrmgr,
                              std::vector<int> *hvLineVec, std::vector<HWIdentifier> *hvLineId) const {
  std::set<int> hv;
  std::set<HWIdentifier> hvId;
  if(hvLineVec) hvLineVec->clear();
  if(hvLineId) hvLineId->clear();

  // LAr EMB
  if (m_larem_id->is_lar_em(id) && m_larem_id->sampling(id)>0) {
    if (abs(m_larem_id->barrel_ec(id))==1) {
       const EMBDetectorElement* embElement = dynamic_cast<const EMBDetectorElement*>(calodetdescrmgr->get_element(id));
       if (embElement) {
          const EMBCellConstLink &cell = embElement->getEMBCell();
          unsigned int nelec = cell->getNumElectrodes();
          for (unsigned int i=0;i<nelec;i++) {
             const EMBHVElectrode &electrode = cell->getElectrode(i);
             for (unsigned int igap=0;igap<2;igap++) {
                HWIdentifier hvline;
                hv.insert(electrode.getModule().getManager().hvLineNo(electrode,igap, m_hvmapping, &hvline));
                hvId.insert(hvline);
             }
          }
       } else {
          ATH_MSG_ERROR( "Failed d'cast to EMBDetectorElement" );
       }
    } else { // LAr EMEC
      const EMECDetectorElement* emecElement = dynamic_cast<const EMECDetectorElement*>(calodetdescrmgr->get_element(id));
      if (emecElement) {
         const EMECCellConstLink cell = emecElement->getEMECCell();
         unsigned int nelec = cell->getNumElectrodes();
         for (unsigned int i=0;i<nelec;i++) {
            const EMECHVElectrode &electrode = cell->getElectrode(i);
            for (unsigned int igap=0;igap<2;igap++) {
               HWIdentifier hvline;
               hv.insert(electrode.getModule().getManager().hvLineNo( electrode, igap, m_hvmapping, &hvline));
               hvId.insert(hvline);
            }
         }
      } else {
        ATH_MSG_ERROR( "Failed d'cast to EMECDetectorElement" );
      }
    }
  } else if (m_larhec_id->is_lar_hec(id)) { // LAr HEC
    const HECDetectorElement* hecElement = dynamic_cast<const HECDetectorElement*>(calodetdescrmgr->get_element(id));
    if (hecElement) {
      const HECCellConstLink cell = hecElement->getHECCell();
      unsigned int nsubgaps = cell->getNumSubgaps();
      for (unsigned int igap=0;igap<nsubgaps;igap++) {
        const HECHVSubgap &subgap = cell->getSubgap(igap);
        HWIdentifier hvline;
        hv.insert(subgap.getModule().getManager().hvLineNo(subgap,m_hvmapping,&hvline));
        hvId.insert(hvline);
      }
    } else {
      ATH_MSG_ERROR( "Failed d'cast to HECDetectorElement" );
    }
  } else if (m_larfcal_id->is_lar_fcal(id)) { // LAr FCAL
    const FCALDetectorElement* fcalElement = dynamic_cast<const FCALDetectorElement*>(calodetdescrmgr->get_element(id));
    if (fcalElement) {
      const FCALTile* tile = fcalElement->getFCALTile();
      unsigned int nlines = tile->getNumHVLines();
      unsigned int nlines_found=0;
      for (unsigned int i=0;i<nlines;i++) {
          const FCALHVLine *line = tile->getHVLine(i);
          if (line) nlines_found++;
      }
      if ( nlines_found>0 ) {
         for (unsigned int i=0;i<nlines;i++) {
            const FCALHVLine *line = tile->getHVLine(i);
            if (!line) continue;
            HWIdentifier hvline;
            hv.insert(line->getModule().getManager().hvLineNo(*line,m_hvmapping,&hvline));
            hvId.insert(hvline);
         }
      }
    } else {
      ATH_MSG_ERROR( "Failed d'cast to FCALDetectorElement" );
    }
 
  } else if (m_larem_id->is_lar_em(id) && m_larem_id->sampling(id)==0) { // Presamplers
    if (abs(m_larem_id->barrel_ec(id))==1) {
      const EMBDetectorElement* embElement = dynamic_cast<const EMBDetectorElement*>(calodetdescrmgr->get_element(id));
      if (embElement) {
         const EMBCellConstLink cell = embElement->getEMBCell();
         const EMBPresamplerHVModule &hvmodule = cell->getPresamplerHVModule();
         for (unsigned int igap=0;igap<2;igap++) {
            HWIdentifier hvline;
            hv.insert(hvmodule.getManager().hvLineNo(hvmodule,igap,m_hvmapping,&hvline));
            hvId.insert(hvline);
         }
      } else {
         ATH_MSG_ERROR( "Failed d'cast to EMBDetectorElement (for presampler)" );
      }
    } else {
      const EMECDetectorElement* emecElement = dynamic_cast<const EMECDetectorElement*>(calodetdescrmgr->get_element(id));
      if (emecElement) {
         const EMECCellConstLink cell = emecElement->getEMECCell();
         const EMECPresamplerHVModule &hvmodule = cell->getPresamplerHVModule ();
         HWIdentifier hvline;
         hv.insert(hvmodule.getManager().hvLineNo(hvmodule, m_hvmapping, &hvline));
         hvId.insert(hvline);
      } else {
         ATH_MSG_ERROR( "Failed d'cast to EMECDetectorElement (for presampler)" );
      }
    }
  }

  ATH_MSG_VERBOSE("Found "<<hvId.size()<<" hvIds for id: "<<id.get_identifier32().get_compact());

  if(hvLineVec) {
     for (std::set<int>::iterator i=hv.begin();i!=hv.end();++i) hvLineVec->push_back(*i);
  }
  if(hvLineId) {
     for (std::set<HWIdentifier>::iterator i=hvId.begin();i!=hvId.end();++i) hvLineId->push_back(*i);
  }
  return;
}
