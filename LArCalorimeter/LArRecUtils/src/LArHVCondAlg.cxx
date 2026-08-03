/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "./LArHVCondAlg.h" 
#include "LArRecConditions/LArHVPathologiesDb.h"
#include "CaloDetDescr/CaloDetectorElements.h"
#include "CaloGeoHelpers/CaloPhiRange.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "LArReadoutGeometry/EMBCell.h"
#include "LArHV/LArHVManager.h"
#include "LArHV/EMBHVManager.h"
#include "LArHV/EMECHVModule.h"
#include "LArHV/EMBHVElectrode.h"
#include "LArHV/EMBPresamplerHVManager.h"
#include "LArHV/EMBPresamplerHVModule.h"
#include "LArReadoutGeometry/EMECCell.h"
#include "LArHV/EMECHVManager.h"
#include "LArHV/EMECHVElectrode.h"
#include "LArHV/EMECPresamplerHVModule.h"
#include "LArReadoutGeometry/HECCell.h"
#include "LArHV/HECHVManager.h"
#include "LArHV/HECHVSubgap.h"
#include "LArReadoutGeometry/FCALTile.h"
#include "LArHV/FCALHVManager.h"
#include "LArHV/FCALHVLine.h"

#include "LArIdentifier/LArElectrodeID.h"
#include "LArIdentifier/LArHVLineID.h"
#include "LArIdentifier/LArOnlineID.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "AthenaKernel/IOVInfiniteRange.h"

#include "CoralBase/Blob.h"

#include <cmath> 
#include <cstdlib>
#include <set>
#include <sstream>

#define HV_NON_NOMINAL_TOLERANCE 10 // tolerance : 1V for HV
#define DEAD_HV_THRESHOLD 10 // HV <10 V="dead"
#define MAX_LAR_CELLS 182468

//initialize
StatusCode LArHVCondAlg::initialize(){
  ATH_CHECK( detStore()->retrieve (m_calocellID, "CaloCell_ID") );

  m_larem_id   = m_calocellID->em_idHelper();
  m_larhec_id   = m_calocellID->hec_idHelper();
  m_larfcal_id   = m_calocellID->fcal_idHelper();

  ATH_CHECK(detStore()->retrieve(m_electrodeID));
  ATH_CHECK(detStore()->retrieve(m_hvLineID));
  ATH_CHECK(detStore()->retrieve(m_onlineID));

  m_doR= m_doRProp && (m_useCurrentEMB || m_useCurrentFCAL1 || m_useCurrentOthers);

  if (m_doR) {
    ATH_MSG_INFO("Will use currents to correct voltage-drop at HV-resistors");
  } 
  else {
    ATH_MSG_INFO("Will NOT correct voltage-drop at HV-resistors");
  }

  // Read Handles
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_pathologiesKey.initialize (m_doHV || m_doAffectedHV));
  ATH_CHECK(m_DCSFolderKeys.initialize (m_doHV || m_doAffectedHV));
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_BFKey.initialize() );
  ATH_CHECK(m_hvMappingKey.initialize (m_doHV || m_doAffectedHV));
  ATH_CHECK(m_hvRKey.initialize(m_doR && (m_doHV || m_doAffectedHV)));
  ATH_CHECK(m_onlineHVScaleCorrKey.initialize(!m_outputHVScaleResidCorrKey.empty()));
  ATH_CHECK(m_caloMgrKey.initialize());
  // Write Handles

  ATH_CHECK(m_outputHVScaleResidCorrKey.initialize(!m_outputHVScaleResidCorrKey.empty()));
  ATH_CHECK(m_outputHVScaleFullCorrKey.initialize(!m_outputHVScaleFullCorrKey.empty()));
  ATH_CHECK(m_affectedKey.initialize());

  m_scaleTool=std::make_unique<LArHVScaleCorrTool>(m_calocellID,msg(),m_fixHVCorrStrings);

  ATH_CHECK(fixVoltageAndCurrent());

  ATH_MSG_DEBUG("Configured with doHV " << m_doHV << " doAffected " << m_doAffected 
                << " doAffectedHV " << m_doAffectedHV);

  return StatusCode::SUCCESS;
}


StatusCode LArHVCondAlg::execute()
{
  ATH_MSG_DEBUG("executing");
  const EventContext& ctx = Gaudi::Hive::currentContext();
  // Allow sharing this between the two calls.
  voltagePerLine_t voltagePerLine;
  if (m_doHV || m_doAffectedHV) {
    ATH_CHECK( makeHVScaleCorr (ctx, voltagePerLine) );
  }
  if (m_doAffected) {
    ATH_CHECK( makeAffectedRegionInfo (ctx, voltagePerLine) );
  }
  
  return StatusCode::SUCCESS;
}


StatusCode LArHVCondAlg::fixVoltageAndCurrent()
{
  
  std::set<unsigned> changes;
  const auto items = {std::make_pair(&m_fixHVStrings, &m_fixVoltagePerLine),
                      std::make_pair(&m_fixCurrentStrings, &m_fixCurrentPerLine)};
  for (auto [prop, store]: items) {
    store->clear();
    for (auto& p: prop->value()) {
      std::stringstream ss(p);
      unsigned hvline;
      float value;
      ss >> hvline >> value >> std::skipws;
      if (ss && ss.eof()) {
        changes.insert(hvline);
        if (auto empl = store->emplace(hvline, value); !empl.second) {
          ATH_MSG_WARNING("Setting multiple times HV and/or current for line " << hvline
                          << ", this will replace the previous value(s).");
          empl.first->second = value;
        }
        continue;
      }
      ATH_MSG_ERROR("Couldn't interpret HV or current setting: \"" << p << "\"");
      return StatusCode::FAILURE;
    }
  }
  if (changes.size() > 0) {
    ATH_MSG_INFO("Fixed values of voltage and/or current to be used instead of DCS readings are provided for " 
                << changes.size() << " HV line(s)");
  }
  return StatusCode::SUCCESS;
}


StatusCode LArHVCondAlg::makeHVScaleCorr (const EventContext& ctx,
                                          voltagePerLine_t& voltagePerLine) const
{
  constexpr int RESIDUAL = 0, FULL = 1; 
  std::map<int, SG::WriteCondHandle<LArHVCorr>> writeHandle;
  for (int ct: {RESIDUAL, FULL}) {
    auto& key = (ct == FULL)? m_outputHVScaleFullCorrKey: m_outputHVScaleResidCorrKey;
    if (key.empty()) continue;
    auto itr = writeHandle.try_emplace(ct, key, ctx).first;
    if (itr->second.isValid()) {
      ATH_MSG_DEBUG("Found valid write handle for LArHVCorr, key " << key.key());
      writeHandle.erase(itr);
    }
  }
  if (writeHandle.empty()) return StatusCode::SUCCESS;
  const bool doFull = writeHandle.count(FULL), doResid = writeHandle.count(RESIDUAL);
  auto addDep = [&](auto& dep, const char* debugDescr=nullptr) {
    for (auto&[ct, wh]: writeHandle) {
      wh.addDependency(dep);
      if (!debugDescr) return;
      std::string dr;
      if constexpr(requires{dep.getRange();}) {
        dr = dep.getRange();
      } else {
        dr = dep;
      }
      ATH_MSG_DEBUG("Range of " << debugDescr << " " << dr
                    << ", intersection (corr. type " << ct << "): " << wh.getRange());
    }
  };

  //Start with infinite range and narrow it down
  {
    auto iov = IOVInfiniteRange::infiniteMixed();
    addDep(iov);
  }

  SG::ReadCondHandle<LArOnOffIdMapping> larCablingHdl(m_cablingKey, ctx);
  const LArOnOffIdMapping* cabling=*larCablingHdl;
  addDep(larCablingHdl, "cabling");

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey,ctx};
  const CaloDetDescrManager* calodetdescrmgr = *caloMgrHandle;
  addDep(caloMgrHandle);

  const ILArHVScaleCorr *onlHVCorr{nullptr};
  if(writeHandle.count(RESIDUAL)) {
    SG::ReadCondHandle<ILArHVScaleCorr> onlHVCorrHdl(m_onlineHVScaleCorrKey, ctx);
    onlHVCorr = *onlHVCorrHdl;
    writeHandle.at(RESIDUAL).addDependency(onlHVCorrHdl);
    ATH_MSG_DEBUG("Range of online HV correction  " << onlHVCorrHdl.getRange() << ", intersection: " << writeHandle.at(RESIDUAL).getRange());
  }

  SG::ReadCondHandle<LArHVIdMapping> mappingHdl{m_hvMappingKey, ctx};
  const LArHVIdMapping* hvCabling = *mappingHdl;
  addDep(mappingHdl);

  pathVec  hasPathologyEM;
  pathVec  hasPathologyHEC;
  pathVec  hasPathologyFCAL;
  hasPathologyEM.resize(m_larem_id->channel_hash_max());
  hasPathologyHEC.resize(m_larhec_id->channel_hash_max());
  hasPathologyFCAL.resize(m_larfcal_id->channel_hash_max());

  bool doPathology=true; 
  SG::ReadCondHandle<LArHVPathology> pHdl(m_pathologiesKey,ctx);
  const LArHVPathology* pathologyContainer = *pHdl;
  if(!pathologyContainer) {
    ATH_MSG_WARNING("Why do not have HV pathology object " << m_pathologiesKey.fullKey() << " ? Work without pathologies !!!");
    doPathology=false;
  }
 
  if(doPathology) {
    addDep(pHdl, "HV-Pathology");
    const std::vector<LArHVPathologiesDb::LArHVElectPathologyDb> &pathCont = pathologyContainer->getPathology();
    const size_t nPathologies=pathCont.size();
    if (m_nPathologies != nPathologies) {
      ATH_MSG_INFO( "Number of HV pathologies found " << nPathologies);
      m_nPathologies=nPathologies;
    }
    for(unsigned i=0; i<nPathologies; ++i) {
      LArHVPathologiesDb::LArHVElectPathologyDb electPath = pathCont[i];
      Identifier id(electPath.cellID);
      if (m_larem_id->is_lar_em(id)) {
        IdentifierHash idHash = m_larem_id->channel_hash(id);
        unsigned int index = (unsigned int)(idHash);
        if (index<hasPathologyEM.size()) {
          if(!hasPathologyEM[index].empty()) {
            if(hasPathologyEM[index].size()<static_cast<size_t>(abs(electPath.electInd+1)))
              hasPathologyEM[index].resize(electPath.electInd+1);
            hasPathologyEM[index][electPath.electInd]=electPath.pathologyType;
          } else {
            std::vector<unsigned short> svec;
            svec.resize(electPath.electInd+1);
            svec[electPath.electInd]=electPath.pathologyType;
            hasPathologyEM[index]=svec;
          }
        }
      }
      if (m_larhec_id->is_lar_hec(id)) {
        IdentifierHash idHash = m_larhec_id->channel_hash(id);
        unsigned int index = (unsigned int)(idHash);
        if (index<hasPathologyHEC.size()) {
          if(!hasPathologyHEC[index].empty()) {
            if(hasPathologyHEC[index].size()<static_cast<size_t>(abs(electPath.electInd+1)))
              hasPathologyHEC[index].resize(electPath.electInd+1);
            hasPathologyHEC[index][electPath.electInd]=electPath.pathologyType;
          } else {
            std::vector<unsigned short> svec;
            svec.resize(electPath.electInd+1);
            svec[electPath.electInd]=electPath.pathologyType;
            hasPathologyHEC[index]=svec;
          }
        }
      }
      if (m_larfcal_id->is_lar_fcal(id)) {
        IdentifierHash idHash = m_larfcal_id->channel_hash(id);
        unsigned int index = (unsigned int)(idHash);
        if (index<hasPathologyFCAL.size()) {
          if(!hasPathologyFCAL[index].empty()) {
            if(hasPathologyFCAL[index].size()<static_cast<size_t>(abs(electPath.electInd+1)))
              hasPathologyFCAL[index].resize(electPath.electInd+1);
            hasPathologyFCAL[index][electPath.electInd]=electPath.pathologyType;
          } else {
            std::vector<unsigned short> svec;
            svec.resize(electPath.electInd+1);
            svec[electPath.electInd]=electPath.pathologyType;
            hasPathologyFCAL[index]=svec;
          }
 
        }
      }
    } // Pathology containers
  }//doPathology

  const float* rValues{nullptr};
  if(m_doR) {
    SG::ReadCondHandle<AthenaAttributeList> readAttrHandle{m_hvRKey, ctx};
    const AthenaAttributeList* attr = *readAttrHandle;
    addDep(readAttrHandle);
    // store the conditions blob
    const coral::Blob& rBlob = (*attr)["ElectrodeRvalues"].data<coral::Blob>();
    if(rBlob.size()/sizeof(float) != m_electrodeID->electrodeHashMax()) {
      ATH_MSG_ERROR("Expected " << m_electrodeID->electrodeHashMax() << " R values, but got " << rBlob.size()/sizeof(float) << " aborting");
      return StatusCode::FAILURE;
    }
    rValues = static_cast<const float*>(rBlob.startingAddress());
  }

  ATH_CHECK( getVoltagePerLine (ctx, voltagePerLine, addDep) );

  voltagePerCell_t voltageVec(MAX_LAR_CELLS);
  ATH_CHECK(fillPathAndCellHV(calodetdescrmgr,voltageVec, hvCabling, voltagePerLine, 
                              pathologyContainer, hasPathologyEM, hasPathologyHEC, hasPathologyFCAL, rValues));

  auto clampCorr = [&](float value, const HWIdentifier hwid) {
     if (value < 0.9f) {
      if (value < 0.01f) {
        ATH_MSG_WARNING("Ignoring suspiciously small correction factor of " << value  
                        << " for channel " << m_onlineID->channel_name(hwid));
        value = 1.f;
      } else if (value < 0.4f) {
        ATH_MSG_WARNING("HV corr for channel " << m_onlineID->channel_name(hwid)
                                               << " = " << value);
      } else {
        ATH_MSG_DEBUG("HV corr for channel " << m_onlineID->channel_name(hwid)
                                             << " = " << value);
      }
    }
    return value;
  };
  std::vector<float> vScale[2];
  if (doResid) vScale[RESIDUAL].resize(MAX_LAR_CELLS, 1.f);
  if (doFull) vScale[FULL].resize(MAX_LAR_CELLS, 1.f);
  for (unsigned i=0;i<MAX_LAR_CELLS;++i) {
    IdentifierHash hash(i);
    const CaloDetDescrElement* dde = calodetdescrmgr->get_element(hash);
    const HWIdentifier hwid=cabling->createSignalChannelIDFromHash(hash);
    float scale=m_scaleTool->getHVScale(dde,voltageVec[i],msg());
    if (doResid) { // factor out the online correction
      float onlineCorr = onlHVCorr->HVScaleCorr(hwid);
      if (onlineCorr<=0.f || onlineCorr>=100.f) onlineCorr = 1.f;
      vScale[RESIDUAL][i] = clampCorr(scale/onlineCorr, hwid);
    }
    if (doFull) vScale[FULL][i] = clampCorr(scale, hwid);
  } //end loop over all cells
  for (auto& [ct, wh]: writeHandle) {
    auto hvCorr = std::make_unique<LArHVCorr>(std::move(vScale[ct]), cabling, m_calocellID);
    if (wh.record(std::move(hvCorr)).isFailure()) {
      ATH_MSG_ERROR("Could not record LArHVCorr object with " << wh.key()
                    << " with EventRange " << wh.getRange() << " into Conditions Store");
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("recorded new " << wh.key() << " with range " << wh.getRange() << " into Conditions Store");
  }
  return StatusCode::SUCCESS;
}


StatusCode LArHVCondAlg::makeAffectedRegionInfo (const EventContext& ctx,
                                                 voltagePerLine_t& voltagePerLine) const
{
  SG::WriteCondHandle<CaloAffectedRegionInfoVec> writeAffectedHandle{m_affectedKey, ctx};
  if (writeAffectedHandle.isValid()) {
    ATH_MSG_DEBUG("Found valid write LArAffectedRegions handle");
    return StatusCode::SUCCESS;
  }

  SG::ReadCondHandle<LArOnOffIdMapping> larCablingHdl(m_cablingKey, ctx);
  const LArOnOffIdMapping* cabling=*larCablingHdl;
  ATH_MSG_DEBUG("Range of cabling" << larCablingHdl.getRange() << ", intersection: " << writeAffectedHandle.getRange());
  writeAffectedHandle.addDependency(larCablingHdl);

  SG::ReadCondHandle<LArBadFebCont> readBFHandle{m_BFKey, ctx};
  const LArBadFebCont* bfCont = *readBFHandle;
  writeAffectedHandle.addDependency(readBFHandle);
  ATH_MSG_DEBUG("Range of BadFeb " << readBFHandle.getRange() << ", intersection: " << writeAffectedHandle.getRange());

  auto vAffected = std::make_unique<CaloAffectedRegionInfoVec>();
  if (m_doAffectedHV) {
    auto addDep = [&] (SG::ReadCondHandle<CondAttrListCollection>& h, const char* debugDescr) {
                    writeAffectedHandle.addDependency (h);
                    if (debugDescr) ATH_MSG_DEBUG("Range of " << debugDescr << " " << h.getRange()
                                                  << ", intersection: " << writeAffectedHandle.getRange());
                  };
    ATH_CHECK( getVoltagePerLine (ctx, voltagePerLine, addDep) );

    SG::ReadCondHandle<LArHVIdMapping> mappingHdl{m_hvMappingKey, ctx};
    const LArHVIdMapping* hvCabling = *mappingHdl;
    writeAffectedHandle.addDependency(mappingHdl);

    ATH_CHECK(searchNonNominalHV_EMB(vAffected.get(), hvCabling, voltagePerLine));
    ATH_CHECK(searchNonNominalHV_EMEC_OUTER(vAffected.get(), hvCabling, voltagePerLine));
    ATH_CHECK(searchNonNominalHV_EMEC_INNER(vAffected.get(), hvCabling, voltagePerLine));
    ATH_CHECK(searchNonNominalHV_HEC(vAffected.get(), hvCabling, voltagePerLine));
    ATH_CHECK(searchNonNominalHV_FCAL(vAffected.get(), hvCabling, voltagePerLine));
  }

  ATH_CHECK(updateMethod(ctx, vAffected.get(), bfCont, cabling));
  ATH_CHECK(writeAffectedHandle.record(std::move(vAffected)));
  ATH_MSG_INFO("recorded new " << writeAffectedHandle.key() << " with range " 
               <<  writeAffectedHandle.getRange()<< " into Conditions Store");

  return StatusCode::SUCCESS;
}


StatusCode LArHVCondAlg::getVoltagePerLine (const EventContext& ctx,
                                            voltagePerLine_t& voltagePerLine,
                                            const addDepFcn_t& addDep) const
{
  // Do this bit unconditionally, so that dependencies are propagated correctly.
  std::vector<const CondAttrListCollection*> attrvec;
  // get handles to DCS Database folders
  for (const auto& fldkey: m_DCSFolderKeys ) {
    SG::ReadCondHandle<CondAttrListCollection> dcsHdl(fldkey, ctx);
    const CondAttrListCollection* cattr = *dcsHdl;
    if(cattr) {
      ATH_MSG_DEBUG("Folder: "<<dcsHdl.key()<<" has size: "<<std::distance(cattr->begin(),cattr->end()));
      attrvec.push_back(cattr);
      addDep (dcsHdl, dcsHdl.key().c_str());
    } else {
      ATH_MSG_WARNING("Why do not have DCS folder " << fldkey.fullKey());
    }
  } // over DCS folders

  // But we can skip this if we've already done it.
  if (voltagePerLine.empty()) {
    ATH_CHECK(dcs2LineVoltage(voltagePerLine, attrvec));
  }

  return StatusCode::SUCCESS;
}


StatusCode LArHVCondAlg::fillPathAndCellHV(const CaloDetDescrManager* calodetdescrmgr
             , voltagePerCell_t& hvdata
					   , const LArHVIdMapping* hvCabling
					   , const voltagePerLine_t& voltage
					   , const LArHVPathology& pathologies
					   , pathVec& hasPathologyEM
					   , pathVec& hasPathologyHEC
					   , pathVec& hasPathologyFCAL
					   , const float* rValues) const
{

  std::vector<unsigned int> listElec;

  const float uAkOhm = 1.e-3; // current is uA, rValues kOhm, result should be V


  // loop over all EM Identifiers
  for (auto id: m_larem_id->channel_ids()) {
    const IdentifierHash hash=m_calocellID->calo_cell_hash(id);
    voltageCell_t& v=hvdata[hash];
    if (abs(m_larem_id->barrel_ec(id))==1 && m_larem_id->sampling(id) > 0) { // LAr EMB
      unsigned int index = (unsigned int)(m_larem_id->channel_hash(id));
      bool hasPathology=false; 
      if (index<hasPathologyEM.size()) {
	if (!hasPathologyEM[index].empty()) {
	  hasPathology=true;
	  listElec = getElecList(id,pathologies);
	}
      }
      const EMBDetectorElement* embElement = dynamic_cast<const EMBDetectorElement*>(calodetdescrmgr->get_element(hash));
      if (!embElement) std::abort();
      const EMBCellConstLink cell = embElement->getEMBCell();
      unsigned int nelec = cell->getNumElectrodes();
      unsigned int ngap = 2*nelec;
      float wt = 1./ngap;
      for (unsigned int i=0;i<nelec;i++) {
	const EMBHVElectrode& electrode = cell->getElectrode(i);
	//   " " << electrode->getModule()->getEtaIndex() << " " << electrode->getModule()->getPhiIndex() << 
	//   " " << electrode->getModule()->getSectorIndex() << " " << electrode->getElectrodeIndex() << std::endl;
	for (unsigned int igap=0;igap<2;igap++) {
	  float hv=0;
	  float curr=0;
	  unsigned int hvline = electrode.hvLineNo(igap,hvCabling);
	  auto hvIt=voltage.find(hvline);
	  if(hvIt != voltage.end()) { //Found HV line
	    hv=hvIt->second.hv;
	    if(rValues &&  m_useCurrentEMB) { // modify the current record
	      curr=hvIt->second.curr;
	      const EMBHVModule &hvmod = electrode.getModule();
	      unsigned ridx = m_electrodeID->electrodeHash(m_electrodeID->ElectrodeId(0,
										      hvmod.getSideIndex(),
										      hvCabling->getCellModule(id),
										      hvmod.getPhiIndex(),
										      hvmod.getEtaIndex(),
										      igap,
										      electrode.getElectrodeIndex() ));
	      if(curr > 0.) curr *= uAkOhm * rValues[ridx]; else curr = 0.;
	      ATH_MSG_VERBOSE("channel. "<<std::hex<<id.get_identifier32()<<std::dec <<" hvline: "<<hvline<<" curr. " << curr << " R: "<<rValues[ridx]);
	    }//end if rValues
	    if (hasPathology) {
	      ATH_MSG_VERBOSE( "Has pathology for id: "<< m_larem_id->print_to_string(id)<<" "<<hasPathologyEM[index]);
	      msg(MSG::VERBOSE) << "Original hv: "<<hv<<" ";
	      for (unsigned int ii=0;ii<listElec.size();ii++) {
		if (listElec[ii]==(2*i+igap) && listElec[ii]<hasPathologyEM[index].size() && hasPathologyEM[index][listElec[ii]]) {
		  if(hasPathologyEM[index][listElec[ii]]&LArHVPathologyBits::MaskHV) {
		    hv=0.;
		    curr = 0.;
		  } else if(hasPathologyEM[index][listElec[ii]]&LArHVPathologyBits::MaskCurr) { 
		    curr = 0.;
		  } else {
		    hv=((hasPathologyEM[index][listElec[ii]]&LArHVPathologyBits::SetHVMask)>>4);
		    curr=0.;
		  }
		}
	      }
	      msg(MSG::VERBOSE) << "set hv: "<<hv<<endmsg;
	    }//end if has patology
	  
	  }//end got hv
	  else {
	    ATH_MSG_WARNING("Do not have hvline: "<<hvline<<" in LArHVData mapping ! Set voltage to 0 !");
	  }
	  addHV(v,hv-curr,wt);
	}//end loop over gaps
      }//end loop over electrodes
    } else if (abs(m_larem_id->barrel_ec(id))==1 && m_larem_id->sampling(id) == 0) { // EMBPS

      const EMBDetectorElement* embElement = dynamic_cast<const EMBDetectorElement*>(calodetdescrmgr->get_element(hash));
      if (!embElement) std::abort();
      const EMBCellConstLink cell = embElement->getEMBCell();
      const EMBPresamplerHVModule& hvmodule =  cell->getPresamplerHVModule ();
 
      float wt = 0.5;
      for (unsigned int igap=0;igap<2;igap++) {
	float hv=0;
	float curr=0;
	unsigned hvline = hvmodule.hvLineNo(igap,hvCabling);
	auto hvIt=voltage.find(hvline);
	if(hvIt != voltage.end()) {  //Found HV line
	  hv=hvIt->second.hv;
	  if(rValues && m_useCurrentOthers) { // modify the current record
	    curr=hvIt->second.curr;
	    unsigned ridx = m_electrodeID->electrodeHash(m_electrodeID->ElectrodeId(1,
										    hvmodule.getSideIndex(),
										    hvCabling->getCellModule(id),
										    0, // not used in EMBPS
										    hvmodule.getEtaIndex(),
										    igap,
										    0 // not used in EMBPS
										    ));
	    if(curr > 0.) curr *= uAkOhm * rValues[ridx]; else curr = 0;
	    ATH_MSG_VERBOSE("channel. "<<std::hex<<id.get_identifier32()<<std::dec <<" hvline: "<<hvline<<" curr. " << curr << " R: "<<rValues[ridx]);
	  }//end have rValue
	}//end have voltage
	else {
	  ATH_MSG_WARNING("Do not have hvline: "<<hvline<<" in LArHVData mapping ! Set voltage to 0 !");
	}
	addHV(v,hv-curr,wt);
      }//end loop over gaps
    } else if (abs(m_larem_id->barrel_ec(id))>1 && m_larem_id->sampling(id) > 0){ // LAr EMEC
      unsigned int index = (unsigned int)(m_larem_id->channel_hash(id));
      bool hasPathology=false;
      if (index<hasPathologyEM.size()) {
	if (!hasPathologyEM[index].empty()) {
	  hasPathology=true;
	  listElec = getElecList(id, pathologies);
	}
      }
 
      const EMECDetectorElement* emecElement = dynamic_cast<const EMECDetectorElement*>(calodetdescrmgr->get_element(hash));
      if (!emecElement) std::abort();
      const EMECCellConstLink cell = emecElement->getEMECCell();
      unsigned int nelec = cell->getNumElectrodes();
      unsigned int ngap = 2*nelec;
      float wt = 1./ngap;
      for (unsigned int i=0;i<nelec;i++) {
	const EMECHVElectrode& electrode = cell->getElectrode(i);
	for (unsigned int igap=0;igap<2;igap++) {
	float hv=0;
	float curr=0;
	unsigned  hvline = electrode.hvLineNo(igap,hvCabling);
	auto hvIt=voltage.find(hvline);
	if(hvIt != voltage.end()) { //Found HV line
	  hv=hvIt->second.hv;
	  if(rValues && m_useCurrentOthers) { // modify the current record
	    curr=hvIt->second.curr;
	    const EMECHVModule &hvmod = electrode.getModule();
	    unsigned ridx = m_electrodeID->electrodeHash(m_electrodeID->ElectrodeId(2,
										    hvmod.getSideIndex(),
										    hvCabling->getCellModule(id),
										    hvmod.getPhiIndex(),
										    hvmod.getEtaIndex(),
										    hvmod.getSectorIndex(),
										    electrode.getElectrodeIndex() ));
	    if(curr > 0.) curr *= uAkOhm * rValues[ridx]; else curr = 0.;
	    ATH_MSG_VERBOSE("channel. "<<std::hex<<id.get_identifier32()<<std::dec <<" hvline: "<<hvline<<" curr. " << curr << " R: "<<rValues[ridx]);
	  }
	  if (hasPathology) {
	    msg(MSG::VERBOSE) << "Has pathology for id: "<< m_larem_id->print_to_string(id)<<" "<<hasPathologyEM[index]<<endmsg;
	    for (unsigned int ii=0;ii<listElec.size();ii++) {
	      if (listElec[ii]==(2*i+igap) && listElec[ii]<hasPathologyEM[index].size() && hasPathologyEM[index][listElec[ii]]) {
		if(hasPathologyEM[index][listElec[ii]]&LArHVPathologyBits::MaskHV) {
		  hv=0.;
		  curr = 0.;
		} else if(hasPathologyEM[index][listElec[ii]]&LArHVPathologyBits::MaskCurr) { 
		  curr = 0.;
		} else {
		  hv=((hasPathologyEM[index][listElec[ii]]&0xFFF0)>>4);
		  curr=0.;
		}
	      }
	    }
	  }//end hasPatology
	}//end have voltage
	else {
	  ATH_MSG_WARNING("Do not have hvline: "<<hvline<<" in LArHVData mapping ! Set voltage to 0 !");
	}
	addHV(v,hv-curr,wt);
	}//end loop over gaps
      }//end loop over electrodes
 
    } else if (abs(m_larem_id->barrel_ec(id))>1 &&  m_larem_id->sampling(id)==0) { // EMECPS

      const EMECDetectorElement* emecElement = dynamic_cast<const EMECDetectorElement*>(calodetdescrmgr->get_element(hash));
      if (!emecElement) std::abort();
      const EMECCellConstLink cell = emecElement->getEMECCell();
      const EMECPresamplerHVModule& hvmodule = cell->getPresamplerHVModule ();
 
      double wt = 0.5; 
      for (unsigned int igap=0;igap<2;igap++) {
	float hv=0;
	float curr=0;
	unsigned int hvline = hvmodule.hvLineNo(igap,hvCabling);
	auto hvIt=voltage.find(hvline);
	if(hvIt != voltage.end()) { //Found HV line
	  hv=hvIt->second.hv;
	  if(rValues && m_useCurrentOthers) { // modify the current record
	    curr=hvIt->second.curr;
	    unsigned ridx = m_electrodeID->electrodeHash(m_electrodeID->ElectrodeId(3,
										    hvmodule.getSideIndex(),
										    hvCabling->getCellModule(id),
										    0, // not used in EMECPS
										    0,
										    igap,
										    0 // not used in EMECPS
										    ));
	    if(curr >0.) curr *= uAkOhm * rValues[ridx]; else curr=0.;
	    ATH_MSG_VERBOSE("channel. "<<std::hex<<id.get_identifier32()<<std::dec <<" hvline: "<<hvline<<" curr. " << curr << " R: "<<rValues[ridx]);
	  }//end if rValues
	   
	}//end have hv-value
	else {
	    ATH_MSG_WARNING("Do not have hvline: "<<hvline<<" in LArHVData mapping ! Set voltage to 0 !");
	}
	addHV(v,hv-curr,wt);
      }//end loop over gaps
    } else { // something wrong
      ATH_MSG_ERROR("This could not be, what happened with EM identifiers ?");
      return StatusCode::FAILURE;
    }
  } // end loop over EM-identifiers


  // LAr HEC
  for( auto id: m_larhec_id->channel_ids()) {
    const IdentifierHash hash=m_calocellID->calo_cell_hash(id);
    unsigned int index = (unsigned int)(m_larhec_id->channel_hash(id));
    bool hasPathology=false;
    if (index<hasPathologyHEC.size()) {
     if (!hasPathologyHEC[index].empty()) {
      hasPathology=true;
      listElec = getElecList(id, pathologies);
     }
    }
    const HECDetectorElement* hecElement = dynamic_cast<const HECDetectorElement*>(calodetdescrmgr->get_element(hash));
    if (!hecElement) std::abort();
    const HECCellConstLink cell = hecElement->getHECCell();
    unsigned int nsubgaps = cell->getNumSubgaps();
    float wt = 1./nsubgaps;
    voltageCell_t& v=hvdata[hash];
    for (unsigned int i=0;i<nsubgaps;i++) {
      float hv=0;
      float curr=0;
      const HECHVSubgap& subgap = cell->getSubgap(i);
      unsigned int hvline = subgap.hvLineNo(hvCabling);
      auto hvIt=voltage.find(hvline);
      if(hvIt != voltage.end()) { //Found HV line
	hv=hvIt->second.hv;
	if(rValues && m_useCurrentOthers) { // modify the current record
	  curr=hvIt->second.curr;
	  const HECHVModule &hvmod = subgap.getModule();
	  unsigned ridx = m_electrodeID->electrodeHash(m_electrodeID->ElectrodeId(4,
										  hvmod.getSideIndex(),
										  hvCabling->getCellModule(id),
										  0, // not used in HEC
										  hvmod.getSamplingIndex(),
										  subgap.getSubgapIndex(),
										  0 // not used in HEC
										  ));
	  if(curr > 0.) curr *= uAkOhm * rValues[ridx]; else curr = 0.;
	  ATH_MSG_VERBOSE("channel. "<<std::hex<<id.get_identifier32()<<std::dec <<" hvline: "<<hvline<<" cur. " << curr << " R: "<<rValues[ridx]);
	}
	if (hasPathology) {
	  msg(MSG::VERBOSE) << "Has pathology for id: "<< m_larhec_id->print_to_string(id)<<" "<<hasPathologyHEC[index]<<endmsg;
	  for (unsigned int ii=0;ii<listElec.size();ii++) {
	    if (listElec[ii]==i && listElec[ii]<hasPathologyHEC[index].size() && hasPathologyHEC[index][listElec[ii]]) {
	      if(hasPathologyHEC[index][listElec[ii]]&LArHVPathologyBits::MaskHV) {
		hv=0.;
		curr = 0.;
	      } else if(hasPathologyHEC[index][listElec[ii]]&LArHVPathologyBits::MaskCurr){
		curr = 0.;
	      } else {
		hv=((hasPathologyHEC[index][listElec[ii]]&LArHVPathologyBits::SetHVMask)>>4);
		curr=0.;
	      }
	    }
	  }
	}//end have pathology
      } //end have voltage
      else {
	ATH_MSG_WARNING("Do not have hvline: "<<hvline<<" in LArHVData mapping ! Set voltage to 0 !");
      }
      addHV(v,hv-curr,wt); 
    }//end loop over subgaps
  }//end loop over HEC-IDs


  for(auto id: m_larfcal_id->channel_ids()) { // LAr FCAL
    unsigned int index = (unsigned int)(m_larfcal_id->channel_hash(id));
    const IdentifierHash hash=m_calocellID->calo_cell_hash(id);
    bool hasPathology=false;
    if (index<hasPathologyFCAL.size()) {
      if (!hasPathologyFCAL[index].empty()) {
        hasPathology=true;
        listElec = getElecList(id, pathologies);
      }
    }

    const FCALDetectorElement* fcalElement = dynamic_cast<const FCALDetectorElement*>(calodetdescrmgr->get_element(hash));
    if (!fcalElement) std::abort();
    const FCALTile* tile = fcalElement->getFCALTile();
    unsigned int nlines = tile->getNumHVLines();
    unsigned int nlines_found=0;
    for (unsigned int i=0;i<nlines;i++) {
      const FCALHVLine* line = tile->getHVLine(i);
      if (line) nlines_found++;
    }
    if (nlines_found>0) {
      float wt = 1./nlines_found;
      voltageCell_t& v=hvdata[hash]; 
      for (unsigned int i=0;i<nlines;i++) {
	const FCALHVLine* line = tile->getHVLine(i);
	if (!line) continue;
	unsigned int hvline = line->hvLineNo(hvCabling);
	float hv=0;
	float curr=0;
	auto hvIt=voltage.find(hvline);
	if(hvIt != voltage.end()) { //Found HV line
	  hv=hvIt->second.hv;
	  bool useCurrent= (m_larfcal_id->module(id)==1 && m_useCurrentFCAL1) || (m_larfcal_id->module(id)!=1 && m_useCurrentOthers);
	  if(rValues && useCurrent) { // modify the current record
	    curr=hvIt->second.curr;
	    const FCALHVModule& hvmod = line->getModule();
	    unsigned ridx = m_electrodeID->electrodeHash(m_electrodeID->ElectrodeId(5,
										    hvmod.getSideIndex(),
										    hvCabling->getCellModule(id),
										    0, // not used in FCAL
										    hvmod.getSamplingIndex(),
										    hvmod.getSectorIndex(),
										    line->getLineIndex()
										    ));
	    if(curr > 0.) curr *= uAkOhm * rValues[ridx]; else curr = 0.;
	    ATH_MSG_VERBOSE("channel. "<<std::hex<<id.get_identifier32()<<std::dec <<" hvline: "<<hvline<<" curr." << curr << " R: "<<rValues[ridx]);
	  }
	  if (hasPathology) {
	    msg(MSG::VERBOSE) << "Has pathology for id: "<< m_larfcal_id->print_to_string(id)<<" "<<hasPathologyFCAL[index]<<endmsg;
	    for (unsigned int ii=0;ii<listElec.size();ii++) {
	      if (listElec[ii]==i && listElec[ii]<hasPathologyFCAL[index].size() && hasPathologyFCAL[index][listElec[ii]]) {
		if(hasPathologyFCAL[index][listElec[ii]]&LArHVPathologyBits::MaskHV){
		  hv=0.;
		  curr = 0.;
		} else if(hasPathologyFCAL[index][listElec[ii]]&LArHVPathologyBits::MaskCurr){
		  curr = 0.;
		} else {
		  hv=((hasPathologyFCAL[index][listElec[ii]]&0xFFF0)>>4);
		  curr=0.;
		}
	      }
	    }
	  }//end if have pathology
	}//end got voltage
		else {
	    ATH_MSG_WARNING("Do not have hvline: "<<hvline<<" in LArHVData mapping ! Set voltage to 0 !");
	}
	addHV(v,hv-curr,wt);

      }//end loop over lines
    }//end if found line
  }// end loop over fcal ids
  
  return StatusCode::SUCCESS; 
}

void LArHVCondAlg::addHV(voltageCell_t& v , float hv, float wt) const {
  bool found=false;
  for (unsigned int i=0;i<v.size();i++) {
    if (std::fabs(hv-v[i].hv) <0.1) {
      found=true;  
      v[i].weight += wt;
      break;
    }
  }
  if (!found) {
    v.emplace_back(hv,wt);
  }     // not already in the list
}


std::vector<unsigned int> LArHVCondAlg::getElecList(const Identifier& id, const LArHVPathology& pathologyContainer) const
{
    std::vector<unsigned int> myList;
    myList.clear();
    for(unsigned i=0; i<pathologyContainer.getPathology().size(); ++i) {
      LArHVPathologiesDb::LArHVElectPathologyDb electPath = pathologyContainer.getPathology()[i];
      if (electPath.cellID == (unsigned int)(id.get_identifier32().get_compact())) {
          myList.push_back(electPath.electInd);
      }
    }
    return myList;

}



StatusCode LArHVCondAlg::dcs2LineVoltage(voltagePerLine_t& result, const std::vector<const CondAttrListCollection* >& fldvec) const {


  result.clear();

  ATH_MSG_DEBUG("Got "<<fldvec.size()<<" DCS HV folders");
  for(const auto *attrlist : fldvec) { // loop over all DCS folders
    CondAttrListCollection::const_iterator citr=attrlist->begin(); 
    CondAttrListCollection::const_iterator citr_e=attrlist->end();
    ATH_MSG_DEBUG("Length: "<<std::distance(citr,citr_e));
    for(;citr!=citr_e;++citr) {
      const unsigned chan=citr->first;
      ATH_MSG_VERBOSE("Got HV cool chan: "<< chan);
      const coral::Attribute& attr=((citr)->second)["R_VMEAS"];
      float voltage=-999;
      if (!attr.isNull()) voltage=attr.data<float>(); //Ignore NULL values
      const coral::Attribute& attrc=((citr)->second)["R_IMEAS"];
      float current=0.;
      if (!attrc.isNull()) current=attrc.data<float>(); //Ignore NULL values
      ATH_MSG_VERBOSE("read voltage: "<<voltage<<" and current: "<<current );
      auto empl=result.emplace(chan,DCS_t{voltage,current});
      if (!empl.second) {
	ATH_MSG_WARNING("DCS channel " << chan << " encountered twice!");
      }
    }//end loop over attributeListCollection
  }
  for (auto [chan, desired]: m_fixVoltagePerLine) {
    if (auto empl = result.emplace(chan, DCS_t{desired, 0.f}); !empl.second) {
      DCS_t& dcs = empl.first->second;
      ATH_MSG_DEBUG("Changing voltage for " << chan << " from " << dcs.hv  << " to " << desired);
      dcs.hv = desired;
    } else {
      ATH_MSG_WARNING("voltage set for channel " << chan << " unknown to DCS.");
    }
  }
  for (auto [chan, desired]: m_fixCurrentPerLine) {
    if (auto empl = result.emplace(chan, DCS_t{0.f, desired}); !empl.second) {
      DCS_t& dcs = empl.first->second;
      ATH_MSG_DEBUG("Changing current for " << chan << " from " << dcs.curr  << " to " << desired);
      dcs.curr = desired;
    } else {
      ATH_MSG_WARNING("current set for channel " << chan << " unknown to DCS and voltage remains 0?!");
    }
  }
  return StatusCode::SUCCESS;
}


//=========================================================================================
StatusCode LArHVCondAlg::searchNonNominalHV_EMB(CaloAffectedRegionInfoVec *vAffected
						, const LArHVIdMapping* hvCabling
						, const voltagePerLine_t& voltage) const {  // deals with LAr HV, EMBarrel

  ATH_MSG_DEBUG(" start HV_EMB ");
  const LArHVManager *manager = nullptr;

  if (detStore()->retrieve(manager)==StatusCode::SUCCESS) {
    
    // accordion calorimeter
    float HVnominal = HV_nominal("EMB",0.);
    const EMBHVManager& hvManager_EMB=manager->getEMBHVManager();
    
    for (unsigned int iSide=hvManager_EMB.beginSideIndex();iSide<hvManager_EMB.endSideIndex();iSide++) { // loop over HV modules
      for (unsigned int iPhi=hvManager_EMB.beginPhiIndex();iPhi<hvManager_EMB.endPhiIndex();iPhi++) {
	for (unsigned int iSector=hvManager_EMB.beginSectorIndex();iSector<hvManager_EMB.endSectorIndex();iSector++) {
	  for (unsigned int iEta=hvManager_EMB.beginEtaIndex();iEta<hvManager_EMB.endEtaIndex();iEta++) { //0 to 7
	    const EMBHVModule& hvMod = hvManager_EMB.getHVModule(iSide,iEta,iPhi,iSector);

            float eta_min=hvMod.getEtaMin();
            float eta_max=hvMod.getEtaMax();

            ATH_MSG_VERBOSE("iSide,iPhi,iSector,iEta " << iSide << " " << iPhi << " " << iSector << " " << iEta);
	    float phi_min=+30.,phi_max=-30.;
	    
	    bool are_previous_HV_affected=false;
            bool are_previous_HV_dead=false;
	    for (unsigned int ielec=0;ielec<32;ielec++) { //use hvMod->getNumElectrodes when bug is corrected
	      const EMBHVElectrode& electrode = hvMod.getElectrode(ielec);

	      double hv[2]={0.,0.};
	      for (unsigned int iGap=0;iGap<2;iGap++) { // EMB : 2, TRY TO FIND AUTOMATICALLY NB OF GAPS
		unsigned int hvline = electrode.hvLineNo(iGap,hvCabling);
		auto hvIt=voltage.find(hvline);
		if(hvIt == voltage.end()) { 
                  ATH_MSG_WARNING("Do not have hvline: "<<hvline<<" in LArHVData ! Assuming missing DCS data");
                  continue;
		}
		hv[iGap]=hvIt->second.hv;
	      } //end for iGap

              ATH_MSG_VERBOSE(" electrode HV " << ielec << " " << electrode.getPhi() << " "<< hv[0] << " " << hv[1] );

	      //take decisions according to all the gaps HV :
              bool isDead=false;
              if (fabs(hv[0])<DEAD_HV_THRESHOLD && fabs(hv[1])<DEAD_HV_THRESHOLD) isDead=true;
              bool isAffected=false;
              if ( !isDead && ((fabs(hv[0]-HVnominal)>HV_NON_NOMINAL_TOLERANCE) || (fabs(hv[1]-HVnominal)>HV_NON_NOMINAL_TOLERANCE))) isAffected=true;
              ATH_MSG_VERBOSE(" dead/affected " << isDead << " " << isAffected);
          
              // end previous dead region
              if (are_previous_HV_dead && !isDead) {
                 are_previous_HV_dead=false;
                 ATH_MSG_VERBOSE(" -- end dead region " << eta_min << " " << eta_max << " " <<phi_min << " " << phi_max);
                 CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,1,3,CaloAffectedRegionInfo::HVdead);
                 vAffected->push_back(current_CaloAffectedRegionInfo);
              }

              // end previous affected region
              if (are_previous_HV_affected && !isAffected) {
                 are_previous_HV_affected=false;
                 ATH_MSG_VERBOSE(" -- end affected region " << eta_min << " " << eta_max << " " <<phi_min << " " << phi_max);
                 CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,1,3,CaloAffectedRegionInfo::HVaffected);
                 vAffected->push_back(current_CaloAffectedRegionInfo);
              }

              if (isDead) {
                 if (!are_previous_HV_dead) {
                  phi_min=CaloPhiRange::fix(electrode.getPhi()-1e-4);
                  phi_max=CaloPhiRange::fix(electrode.getPhi()+1e-4);
                  ATH_MSG_VERBOSE(" -- start dead region " << eta_min << " " << eta_max << " " << phi_min << " " <<phi_max);
                  are_previous_HV_dead = true;
                 }
                 else {
                  extendPhiRegion(electrode.getPhi(),phi_min,phi_max);
                  ATH_MSG_VERBOSE(" extend dead region " << phi_min << " " << phi_max);
                 }
              }

              if (isAffected) {
                 if (!are_previous_HV_affected) {
                  phi_min=CaloPhiRange::fix(electrode.getPhi()-1e-4);
                  phi_max=CaloPhiRange::fix(electrode.getPhi()+1e-4);
                  ATH_MSG_VERBOSE(" -- start affected region " << eta_min << " " << eta_max << " " << phi_min << " " <<phi_max);
                  are_previous_HV_affected = true;
                 }
                 else {
                  extendPhiRegion(electrode.getPhi(),phi_min,phi_max);
                  ATH_MSG_VERBOSE(" extend affected region " << phi_min << " " << phi_max);
                 }
              }

	    } // end for ielec
	    
	    if (are_previous_HV_affected) { //in case a non nominal exists, stores it if we finish the 32 electrodes (because else the are_previous_HV_affected will be reinitialized for the next 32 electrodes serie )
              ATH_MSG_VERBOSE("  -- finish affected region after electrode loop " << eta_min << " " << eta_max << " " << phi_min << " " <<phi_max);
	      CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,1,3,CaloAffectedRegionInfo::HVaffected);
	      vAffected->push_back(current_CaloAffectedRegionInfo);	      
	    }
            if (are_previous_HV_dead) {
              ATH_MSG_VERBOSE("  -- finish dead region after electrode loop " << eta_min << " " << eta_max << " " << phi_min << " " <<phi_max);
              CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,1,3,CaloAffectedRegionInfo::HVdead);
              vAffected->push_back(current_CaloAffectedRegionInfo);
            }
	  } // end for iEta
	} // end for iSector
      } // end for iPhi
    } //end for iSide

    // barrel presampler
    const EMBPresamplerHVManager& hvManager_EMBPS=manager->getEMBPresamplerHVManager();
    HVnominal = HV_nominal("EMBPS",0.);
    for (unsigned int iSide=hvManager_EMBPS.beginSideIndex();iSide<hvManager_EMBPS.endSideIndex();iSide++) { // loop over HV modules
      for (unsigned int iPhi=hvManager_EMBPS.beginPhiIndex();iPhi<hvManager_EMBPS.endPhiIndex();iPhi++) {
          for (unsigned int iEta=hvManager_EMBPS.beginEtaIndex();iEta<hvManager_EMBPS.endEtaIndex();iEta++) { //0 to 7
            const EMBPresamplerHVModule& hvMod = hvManager_EMBPS.getHVModule(iSide,iEta,iPhi);
            ATH_MSG_VERBOSE("iSide,iPhi,iEta " << iSide << " " << iPhi << " " << iEta);
            double hv[2];
            for (int iGap=0;iGap<2;iGap++) {
	      unsigned int hvline = hvMod.hvLineNo(iGap,hvCabling);
	      auto hvIt=voltage.find(hvline);
	      if(hvIt == voltage.end()) { 
		ATH_MSG_WARNING("Do not have hvline: "<<hvline<<" in LArHVData ! Assuming missing DCS data");
		continue;
	      }
	      hv[iGap]=hvIt->second.hv;
	    }
            float eta_min=hvMod.getEtaMin();
            float eta_max=hvMod.getEtaMax();
            float phi_min=CaloPhiRange::fix(hvMod.getPhiMin());
            float phi_max=CaloPhiRange::fix(hvMod.getPhiMax());
            ATH_MSG_VERBOSE("  HV " <<  hv[0] << " " << hv[1] << " " << "  etamin,etamax,phimin,phimax " << eta_min << " " << eta_max << " " << phi_min << " " << phi_max);

            //take decisions according to all the gaps HV :
            bool isDead=false;
            if (fabs(hv[0])<DEAD_HV_THRESHOLD && fabs(hv[1])<DEAD_HV_THRESHOLD) isDead=true;
            bool isAffected=false;
            if ( !isDead && ((fabs(hv[0]-HVnominal)>HV_NON_NOMINAL_TOLERANCE) || (fabs(hv[1]-HVnominal)>HV_NON_NOMINAL_TOLERANCE))) isAffected=true;
            ATH_MSG_VERBOSE(" dead/affected " << isDead << " " << isAffected );

            if (isDead) {
              CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,0,0,CaloAffectedRegionInfo::HVdead);
              vAffected->push_back(current_CaloAffectedRegionInfo);             
            }
            if (isAffected) {
              CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,0,0,CaloAffectedRegionInfo::HVaffected);
              vAffected->push_back(current_CaloAffectedRegionInfo);
            }
          }       // loop over iEta EMBPS
      }        // loop over iphi EMBPS
    }    // lop over EMBPS side
  } else {
     ATH_MSG_ERROR("Do not have EMB HV Manager !!! ");
     return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
//=========================================================================================
StatusCode LArHVCondAlg::searchNonNominalHV_EMEC_OUTER(CaloAffectedRegionInfoVec *vAffected
						       , const LArHVIdMapping* hvCabling
						       , const voltagePerLine_t& voltage) const { // deals with LAr HV, EM EndCap OUTER

  const LArHVManager *manager = nullptr;

  ATH_MSG_DEBUG(" start HV_EMEC_OUTER ");

  if (detStore()->retrieve(manager)==StatusCode::SUCCESS) {
      
    const EMECHVManager& hvManager_EMEC_OUT=manager->getEMECHVManager(EMECHVModule::OUTER);

    for (unsigned int iSide=hvManager_EMEC_OUT.beginSideIndex();iSide<hvManager_EMEC_OUT.endSideIndex();iSide++) { // loop over HV modules
      for (unsigned int iPhi=hvManager_EMEC_OUT.beginPhiIndex();iPhi<hvManager_EMEC_OUT.endPhiIndex();iPhi++) {
	for (unsigned int iSector=hvManager_EMEC_OUT.beginSectorIndex();iSector<hvManager_EMEC_OUT.endSectorIndex();iSector++) {
	  for (unsigned int iEta=hvManager_EMEC_OUT.beginEtaIndex();iEta<hvManager_EMEC_OUT.endEtaIndex();iEta++) {
	    const EMECHVModule& hvMod=hvManager_EMEC_OUT.getHVModule(iSide,iEta,iPhi,iSector);

            float etaMod = 0.5*(fabs(hvMod.getEtaMin())+fabs(hvMod.getEtaMax()));
            float HVnominal = HV_nominal("EMEC",etaMod);

            float eta_min=hvMod.getEtaMin();
            float eta_max=hvMod.getEtaMax();

            ATH_MSG_VERBOSE("iSide,iPhi,iSector,iEta " << iSide << " " << iPhi << " " << iSector << " " 
                          << iEta << " eta_min , eta_max " << eta_min << " " << eta_max );


	    float phi_min=+30.,phi_max=-30.;

	    bool are_previous_HV_affected=false;
            bool are_previous_HV_dead=false;
	    for (unsigned int ielec=0;ielec<hvMod.getNumElectrodes();ielec++) { //use hvMod->getNumElectrodes when bug is corrected
	      const EMECHVElectrode& electrode = hvMod.getElectrode(ielec);

	      double hv[2];
	      for (unsigned int iGap=0;iGap<2;iGap++) { //EMEC : 2 gaps, TRY TO FIND AUTOMATICALLY NB OF GAPS
		unsigned int hvline = electrode.hvLineNo(iGap,hvCabling);
		auto hvIt=voltage.find(hvline);
		if(hvIt == voltage.end()) { 
                  ATH_MSG_WARNING("Do not have hvline: "<<hvline<<" in LArHVData ! Assuming missing DCS data");
                  continue;
		}
		hv[iGap]=hvIt->second.hv;
	      } //end for iGap

	      //------------------
	      //take decisions according to all the gaps HV :
              bool isDead=false;
              if (fabs(hv[0])<DEAD_HV_THRESHOLD && fabs(hv[1])<DEAD_HV_THRESHOLD) isDead=true;
              bool isAffected=false;
              if ( !isDead && ((fabs(hv[0]-HVnominal)>HV_NON_NOMINAL_TOLERANCE) || (fabs(hv[1]-HVnominal)>HV_NON_NOMINAL_TOLERANCE))) isAffected=true;
              ATH_MSG_VERBOSE(" electrode HV " << ielec << " " << electrode.getPhi() << " " << hv[0] 
                            << " " << hv[1] << " " << " isDead/isAffected " << isDead << " " << isAffected );
          
              // end previous dead region
              if (are_previous_HV_dead && !isDead) {
                 are_previous_HV_dead=false;
                 ATH_MSG_VERBOSE(" -- end dead region " << eta_min << " " << eta_max << " " <<phi_min << " " << phi_max);
                 CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,5,7,CaloAffectedRegionInfo::HVdead);
                 vAffected->push_back(current_CaloAffectedRegionInfo);
              }

              // end previous affected region
              if (are_previous_HV_affected && !isAffected) {
                 are_previous_HV_affected=false;
                 ATH_MSG_VERBOSE(" -- end affected region " << eta_min << " " << eta_max << " " <<phi_min << " " << phi_max);
                 CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,5,7,CaloAffectedRegionInfo::HVaffected);
                 vAffected->push_back(current_CaloAffectedRegionInfo);
              }

              if (isDead) {
                 if (!are_previous_HV_dead) {
                  phi_min=CaloPhiRange::fix(electrode.getPhi()-1e-4);
                  phi_max=CaloPhiRange::fix(electrode.getPhi()+1e-4);
                  are_previous_HV_dead = true;
                  ATH_MSG_VERBOSE(" -- start dead region " << eta_min << " " << eta_max << " " << phi_min << " " <<phi_max);
                 }
                 else {
                  extendPhiRegion(electrode.getPhi(),phi_min,phi_max);
                  ATH_MSG_VERBOSE("  extend affected region " << phi_min << " " << phi_max);
                 }
              }

              if (isAffected) {
                 if (!are_previous_HV_affected) {
                  phi_min=CaloPhiRange::fix(electrode.getPhi()-1e-4);
                  phi_max=CaloPhiRange::fix(electrode.getPhi()+1e-4);
                  are_previous_HV_affected = true;
                  ATH_MSG_VERBOSE(" -- start affected region " << eta_min << " " << eta_max << " " << phi_min << " " <<phi_max);
                 }
                 else {
                  extendPhiRegion(electrode.getPhi(),phi_min,phi_max);
                  ATH_MSG_VERBOSE(" extend affected region " << phi_min << " " << phi_max);
                 }
              }

	    } // end for ielec
	    
	    if (are_previous_HV_affected) { 
              //in case a non nominal exists, stores it if we finish the 32 electrodes 
              //(because else the are_previous_HV_affected will be reinitialized 
              //for the next 32 electrodes serie )
              ATH_MSG_VERBOSE("   - finih affected region after electrode loop " << eta_min << " " << eta_max 
                            << " " << phi_max << " " <<phi_max);
	      CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,5,7,CaloAffectedRegionInfo::HVaffected);
	      vAffected->push_back(current_CaloAffectedRegionInfo);	      
	    }
            if (are_previous_HV_dead) {
              ATH_MSG_VERBOSE(" -- finish dead region after electrode loop " << eta_min << " " << eta_max << " " << phi_max << " " <<phi_max);
              CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,5,7,CaloAffectedRegionInfo::HVdead);
              vAffected->push_back(current_CaloAffectedRegionInfo);
            }
	  } // end for iEta
	} // end for iSector
      } // end for iPhi
    } //end for iSide

    // endcap presampler
    const EMECPresamplerHVManager& hvManager_EMECPS=manager->getEMECPresamplerHVManager();
    float HVnominal = HV_nominal("EMECPS",0.);
    for (unsigned int iSide=hvManager_EMECPS.beginSideIndex();iSide<hvManager_EMECPS.endSideIndex();iSide++) { // loop over HV modules
      for (unsigned int iPhi=hvManager_EMECPS.beginPhiIndex();iPhi<hvManager_EMECPS.endPhiIndex();iPhi++) {
            const EMECPresamplerHVModule& hvMod = hvManager_EMECPS.getHVModule(iSide,iPhi);
            double hv[2];
            for (int iGap=0;iGap<2;iGap++) {
	      unsigned int hvline = hvMod.hvLineNo(iGap,hvCabling);
	      auto hvIt=voltage.find(hvline);
		if(hvIt == voltage.end()) { 
                  ATH_MSG_WARNING("Do not have hvline: "<<hvline<<" in LArHVData ! Assuming missing DCS data");
                  continue;
		}
		hv[iGap]=hvIt->second.hv;
            }//end loop over gaps
            float eta_min=hvMod.getEtaMin(); 
            float eta_max=hvMod.getEtaMax();
            float phi_min=CaloPhiRange::fix(hvMod.getPhiMin());
            float phi_max=CaloPhiRange::fix(hvMod.getPhiMax());
            ATH_MSG_VERBOSE("iSide,iPhi" << iSide << " " << iPhi << "  HV " <<  hv[0] << " " << hv[1] << " "
                          << "  etamin,etamax,phimin,phimax " << eta_min << " " << eta_max << " " 
                          << phi_min << " " << phi_max);

            //take decisions according to all the gaps HV :
            bool isDead=false;
            if (fabs(hv[0])<DEAD_HV_THRESHOLD && fabs(hv[1])<DEAD_HV_THRESHOLD) isDead=true;
            bool isAffected=false;
            if ( !isDead && ((fabs(hv[0]-HVnominal)>HV_NON_NOMINAL_TOLERANCE) || (fabs(hv[1]-HVnominal)>HV_NON_NOMINAL_TOLERANCE))) isAffected=true;
            ATH_MSG_VERBOSE(" dead/affected " << isDead << " " << isAffected);

            if (isDead) {
              CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,4,4,CaloAffectedRegionInfo::HVdead);
              vAffected->push_back(current_CaloAffectedRegionInfo);             
            }
            if (isAffected) {
              CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,4,4,CaloAffectedRegionInfo::HVaffected);
              vAffected->push_back(current_CaloAffectedRegionInfo);
            }
      }        // loop over iphi EMECPS
    }    // lop over EMECPS side
  } else {
     ATH_MSG_ERROR("DO not have EMEC HV manager !");
     return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
//=========================================================================================
StatusCode LArHVCondAlg::searchNonNominalHV_EMEC_INNER(CaloAffectedRegionInfoVec *vAffected
						       , const LArHVIdMapping* hvCabling
						       , const voltagePerLine_t& voltage) const { // deals with LAr HV, EM EndCap INNER
  const LArHVManager *manager = nullptr;

  ATH_MSG_VERBOSE(" start loop over EMEC_INNER ");
  if (detStore()->retrieve(manager)==StatusCode::SUCCESS) {

    const EMECHVManager& hvManager_EMEC_IN=manager->getEMECHVManager(EMECHVModule::INNER);

    for (unsigned int iSide=hvManager_EMEC_IN.beginSideIndex();iSide<hvManager_EMEC_IN.endSideIndex();iSide++) { // loop over HV modules
      for (unsigned int iPhi=hvManager_EMEC_IN.beginPhiIndex();iPhi<hvManager_EMEC_IN.endPhiIndex();iPhi++) {
	for (unsigned int iSector=hvManager_EMEC_IN.beginSectorIndex();iSector<hvManager_EMEC_IN.endSectorIndex();iSector++) {
	  for (unsigned int iEta=hvManager_EMEC_IN.beginEtaIndex();iEta<hvManager_EMEC_IN.endEtaIndex();iEta++) {
	    const EMECHVModule& hvMod=hvManager_EMEC_IN.getHVModule(iSide,iEta,iPhi,iSector);

            float etaMod = 0.5*(fabs(hvMod.getEtaMin())+fabs(hvMod.getEtaMax()));
            float HVnominal = HV_nominal("EMEC",etaMod);

            float eta_min = hvMod.getEtaMin();
            float eta_max = hvMod.getEtaMax();

            ATH_MSG_VERBOSE("iSide,iPhi,iSector,iEta " << iSide << " " << iPhi << " " << iSector << " " 
                          << iEta << " eta_min , eta_max " << eta_min << " " << eta_max);

	    float phi_min=+30.,phi_max=-30.;

	    bool are_previous_HV_affected=0;
	    bool are_previous_HV_dead=0;
	    for (unsigned int ielec=0;ielec<hvMod.getNumElectrodes();ielec++) { 
	      const EMECHVElectrode& electrode = hvMod.getElectrode(ielec);

	      double hv[2];
	      for (unsigned int iGap=0;iGap<2;iGap++) { 
		unsigned int hvline = electrode.hvLineNo(iGap,hvCabling);
		auto hvIt=voltage.find(hvline);
		if(hvIt == voltage.end()) { 
                  ATH_MSG_WARNING("Do not have hvline: "<<hvline<<" in LArHVData ! Assuming missing DCS data");
                  continue;
		}
		hv[iGap]=hvIt->second.hv;
	      } //end for iGap

	      //------------------
	      //take decisions according to all the gaps HV :
              bool isDead=false;
              if (fabs(hv[0])<DEAD_HV_THRESHOLD && fabs(hv[1])<DEAD_HV_THRESHOLD) isDead=true;
              bool isAffected=false;
              if ( !isDead && ((fabs(hv[0]-HVnominal)>HV_NON_NOMINAL_TOLERANCE) || (fabs(hv[1]-HVnominal)>HV_NON_NOMINAL_TOLERANCE))) isAffected=true;
              ATH_MSG_VERBOSE(" electrode HV " << ielec << " " << electrode.getPhi() << " "
                            << hv[0] << " " << hv[1] << " " << " isDead, isAffected " 
                            << isDead << " " << isAffected);
          
              // end previous dead region
              if (are_previous_HV_dead && !isDead) {
                 are_previous_HV_dead=false;
                 ATH_MSG_VERBOSE(" -- end dead region " << eta_min << " " << eta_max << " " <<phi_min << " " << phi_max);
                 CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,5,7,CaloAffectedRegionInfo::HVdead);
                 vAffected->push_back(current_CaloAffectedRegionInfo);
              }

              // end previous affected region
              if (are_previous_HV_affected && !isAffected) {
                 are_previous_HV_affected=false;
                 ATH_MSG_VERBOSE(" -- end affected region " << eta_min << " " << eta_max << " " <<phi_min << " " << phi_max);
                 CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,5,7,CaloAffectedRegionInfo::HVaffected);
                 vAffected->push_back(current_CaloAffectedRegionInfo);
              }

              if (isDead) {
                 if (!are_previous_HV_dead) {
                  phi_min=CaloPhiRange::fix(electrode.getPhi()-1e-4);
                  phi_max=CaloPhiRange::fix(electrode.getPhi()+1e-4);
                  ATH_MSG_VERBOSE(" -- start dead region " << phi_min << " " << phi_max);
                  are_previous_HV_dead = true;
                 }
                 else {
                  extendPhiRegion(electrode.getPhi(),phi_min,phi_max);
                  ATH_MSG_VERBOSE("  extend dead region " << phi_min << " " << phi_max);
                 }
              }

              if (isAffected) {
                 if (!are_previous_HV_affected) {
                  phi_min=CaloPhiRange::fix(electrode.getPhi()-1e-4);
                  phi_max=CaloPhiRange::fix(electrode.getPhi()+1e-4);
                  are_previous_HV_affected = true;
                  ATH_MSG_VERBOSE(" -- start affected region " << phi_min << " " << phi_max);
                 }
                 else {
                   extendPhiRegion(electrode.getPhi(),phi_min,phi_max);
                   ATH_MSG_VERBOSE("  extend affected region " << phi_min << " " << phi_max);
                 }
              }

	    } // end for ielec
	    
	    if (are_previous_HV_affected) { 
              //in case a non nominal exists, stores it if we finish the 32 electrodes 
              //(because else the are_previous_HV_affected will be reinitialized 
              //for the next 32 electrodes serie )
              ATH_MSG_VERBOSE("   - finish affected region after electrode loop " << eta_min << " " 
                            << eta_max << " " << phi_max << " " <<phi_max);
	      CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,5,7,CaloAffectedRegionInfo::HVaffected);
	      vAffected->push_back(current_CaloAffectedRegionInfo);	      
	    }
            if (are_previous_HV_dead) {
              ATH_MSG_VERBOSE("    - end dead region after electrode loop " << eta_min << " " << eta_max << " " << phi_max << " " <<phi_max);
              CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,5,7,CaloAffectedRegionInfo::HVdead);
              vAffected->push_back(current_CaloAffectedRegionInfo);
            }
	  } // end for iEta
	} // end for iSector
      } // end for iPhi
    } //end for iSide
  } else {
     ATH_MSG_ERROR("DO not have EMEC HV manager ");
     return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
//=========================================================================================
StatusCode LArHVCondAlg::searchNonNominalHV_HEC(CaloAffectedRegionInfoVec *vAffected
						, const LArHVIdMapping* hvCabling
						, const voltagePerLine_t& voltage) const { // deals with LAr HV, HEC
  
  ATH_MSG_DEBUG(" in HEC ");
  const LArHVManager *manager = nullptr;
  float etamax_layer[4]={3.3,3.1,3.1,3.3};
  float etamin_layer[4]={1.5,1.5,1.6,1.7};
 
  float HVnominal = HV_nominal("HEC",0.);


  if (detStore()->retrieve(manager)==StatusCode::SUCCESS) {
  
    const HECHVManager& hvManager_HEC=manager->getHECHVManager();
    
    for (unsigned int iSide=hvManager_HEC.beginSideIndex();iSide<hvManager_HEC.endSideIndex();iSide++) { // loop over HV modules      
      for (unsigned int iPhi=hvManager_HEC.beginPhiIndex();iPhi<hvManager_HEC.endPhiIndex();iPhi++) {
	for (unsigned int iSampling=hvManager_HEC.beginSamplingIndex();iSampling<hvManager_HEC.endSamplingIndex();iSampling++) {
          float eta_min,eta_max;
          if (iSide==1) {
           eta_min = etamin_layer[iSampling];
           eta_max = etamax_layer[iSampling];
          } else {
           eta_min = -1.*etamax_layer[iSampling];
           eta_max = -1.*etamin_layer[iSampling];
         }

	  const HECHVModule& hvMod = hvManager_HEC.getHVModule(iSide,iPhi,iSampling);
          ATH_MSG_VERBOSE(" iSide,iPhi,iSampling " << iSide << " " << iPhi << " " << iSampling);

	  double hv[4] = {0}; // 4 subgaps in HEC
	  for (unsigned int iGap=0;iGap<hvMod.getNumSubgaps();iGap++) {
	    const HECHVSubgap& subgap=hvMod.getSubgap(iGap);
	    unsigned int hvline = subgap.hvLineNo(hvCabling);
	    auto hvIt=voltage.find(hvline);
	    if(hvIt == voltage.end()) { 
	      ATH_MSG_WARNING("Do not have hvline: "<<hvline<<" in LArHVData ! Assuming missing DCS data");
	      continue;
	    }
	    if(iGap<4) hv[iGap]=hvIt->second.hv;
	  }// end for iGap

	  //------------------
          //take decisions according to all the gaps HV :
          bool isDead=false;
          if (fabs(hv[0])<DEAD_HV_THRESHOLD && fabs(hv[1])<DEAD_HV_THRESHOLD  && fabs(hv[2])<DEAD_HV_THRESHOLD && fabs(hv[3])<DEAD_HV_THRESHOLD) isDead=true;
          bool isAffected=false;
          if ( !isDead && ((fabs(hv[0]-HVnominal)>HV_NON_NOMINAL_TOLERANCE) || (fabs(hv[1]-HVnominal)>HV_NON_NOMINAL_TOLERANCE) || 
                           (fabs(hv[2]-HVnominal)>HV_NON_NOMINAL_TOLERANCE) || (fabs(hv[3]-HVnominal)>HV_NON_NOMINAL_TOLERANCE)) ) isAffected=true;
          ATH_MSG_VERBOSE(" HV values " << hv[0] << " " << hv[1] << " " << hv[2] << " " << hv[3] << " " 
                        << " isDead/isAffected " << isDead << " " << isAffected);

          float phiMin = CaloPhiRange::fix(hvMod.getPhiMin());
          float phiMax = CaloPhiRange::fix(hvMod.getPhiMax());


	  if (isDead) { //stores it, DEAD means all hvs < threshold
            ATH_MSG_VERBOSE(" new dead region " << eta_min << " " << eta_max << " " << phiMin << " " << phiMax << " layer " << 8+iSampling);
	    CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phiMin,phiMax,8+iSampling,8+iSampling,CaloAffectedRegionInfo::HVdead);
	    vAffected->push_back(current_CaloAffectedRegionInfo);
	  }
          if (isAffected) {
            ATH_MSG_VERBOSE(" new affected region " << eta_min << " " << eta_max << " " << phiMin << " " << phiMax << " layer " << 8+iSampling);
            CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phiMin,phiMax,8+iSampling,8+iSampling,CaloAffectedRegionInfo::HVaffected);
            vAffected->push_back(current_CaloAffectedRegionInfo);
          }

	} //end for iSampling
      }//end for iPhi
    }//end for iSide
  } else {
     ATH_MSG_ERROR("Do not have HEC HV manager ");
     return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
//=========================================================================================
StatusCode LArHVCondAlg::searchNonNominalHV_FCAL(CaloAffectedRegionInfoVec *vAffected
						 , const LArHVIdMapping* hvCabling
						 , const voltagePerLine_t& voltage) const { // deals with LAr HV, FCAL

  ATH_MSG_DEBUG( " inFCAL ");
  const LArHVManager *manager = nullptr;
  if (detStore()->retrieve(manager)==StatusCode::SUCCESS) {  
    
    const FCALHVManager& hvManager_FCAL=manager->getFCALHVManager();
    
    for (unsigned int iSide=hvManager_FCAL.beginSideIndex();iSide<hvManager_FCAL.endSideIndex();iSide++) { // loop over HV modules
      float eta_min=3.1,eta_max=4.9;
      if (iSide==0) {
         eta_min=-4.9;
         eta_max=-3.1;
      }
      for (unsigned int iSampling=hvManager_FCAL.beginSamplingIndex();iSampling<hvManager_FCAL.endSamplingIndex();iSampling++) {
        float HVnominal = HV_nominal("FCAL",(float)(iSampling));
	for (unsigned int iSector=hvManager_FCAL.beginSectorIndex(iSampling);iSector<hvManager_FCAL.endSectorIndex(iSampling);iSector++) {

	  const FCALHVModule& hvMod = hvManager_FCAL.getHVModule(iSide,iSector,iSampling);
          ATH_MSG_VERBOSE(" FCAL HVModule side,sampling,sector " << iSide << " " << iSampling << " " 
                        << iSector << "   HV nominal " << HVnominal);
 
          float dphi=CaloPhiRange::twopi()/16;
          if (iSampling==1) dphi=CaloPhiRange::twopi()/8.;
          if (iSampling==2) dphi=CaloPhiRange::twopi()/4.;
          float phi_min = ((float)(iSector))*dphi;
          phi_min =   CaloPhiRange::fix(phi_min);
          float phi_max = CaloPhiRange::fix(dphi+phi_min);
        
          ATH_MSG_VERBOSE(" eta_min,eta_max,phi_min,phi_max " << eta_min << " " << eta_max << " " << phi_min 
                        << " " << phi_max << "   number of lines " << hvMod.getNumHVLines());
          float hv[4] = {0};
	  for (unsigned int iLine=0;iLine<hvMod.getNumHVLines();iLine++) {
	    const FCALHVLine& hvline = hvMod.getHVLine(iLine);
	    unsigned int ihvline = hvline.hvLineNo(hvCabling);
	    auto hvIt=voltage.find(ihvline);
	    if(hvIt == voltage.end()) { 
	      ATH_MSG_WARNING("Do not have hvline: "<<ihvline<<" in LArHVData ! Assuming missing DCS data");
	      continue;
	    }
	    if (iLine<4) hv[iLine]=hvIt->second.hv;
          }
	  //------------------
	  //take decisions according to all the gaps HV :
          bool isDead=false;
          if (fabs(hv[0]) < DEAD_HV_THRESHOLD && fabs(hv[1]) < DEAD_HV_THRESHOLD && fabs(hv[2]) < DEAD_HV_THRESHOLD && fabs(hv[3]) < DEAD_HV_THRESHOLD) isDead=true;
          bool isAffected=false;
          if ( !isDead && ((fabs(hv[0]-HVnominal)>HV_NON_NOMINAL_TOLERANCE) || (fabs(hv[1]-HVnominal)>HV_NON_NOMINAL_TOLERANCE) ||
                           (fabs(hv[2]-HVnominal)>HV_NON_NOMINAL_TOLERANCE) || (fabs(hv[3]-HVnominal)>HV_NON_NOMINAL_TOLERANCE)) ) isAffected=true;
          ATH_MSG_VERBOSE(" HV values " << hv[0] << " " << hv[1] << " " << hv[2] << " " << hv[3] <<  " " 
                        << " isDead/isAffected " << isDead << " " << isAffected);

	    
	  if (isAffected) { 
            ATH_MSG_VERBOSE(" -- store affected region ");
	    CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,21+iSampling,21+iSampling,CaloAffectedRegionInfo::HVaffected);
	    vAffected->push_back(current_CaloAffectedRegionInfo);	      
	  }
          if (isDead) { 
            ATH_MSG_VERBOSE(" -- store dead region ");
            CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,21+iSampling,21+iSampling,CaloAffectedRegionInfo::HVdead);
            vAffected->push_back(current_CaloAffectedRegionInfo);
          }
	} //end iSector
      } //end iSampling
    }// end iSide
  } else {
     ATH_MSG_ERROR("Do not have FCAL HV manager");
     return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
//=========================================================================================
StatusCode LArHVCondAlg::updateMethod(const EventContext& ctx,
                                      CaloAffectedRegionInfoVec *vAffected, const LArBadFebCont* bfCont, 
                                      const LArOnOffIdMapping* cabling) const { //store informations on the missing Febs w/ range of eta, phi, layer
  ATH_MSG_DEBUG ( "updateMethod()" );
  
  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey,ctx};
  const CaloDetDescrManager* calodetdescrmgr = *caloMgrHandle;

  for (const HWIdentifier febId : m_onlineID->feb_range()) {  
    bool IsMissingFeb=(bfCont->status(febId).deadAll() || bfCont->status(febId).deadReadout());
    
    if (IsMissingFeb) {       //flag for special treatment for FEB that has non contiguous eta regions, so we have to separate them
      bool is_normal=0; //FEB without discontinuity
      bool is_additive1=0; //FEB with a discontinuity
      
      int layer_min=+30,layer_max=-30;
      float eta_min=+30,eta_max=-30;
      float phi_min=+30,phi_max=-30;
      
      int layer_min_additive1=+30,layer_max_additive1=-30;
      float eta_min_additive1=+30,eta_max_additive1=-30;
      float phi_min_additive1=+30,phi_max_additive1=-30;
      
      int chans_per_feb = m_onlineID->channelInSlotMax(febId);
      
      for (int icha=0;icha<chans_per_feb;icha++) {   //loop on each channel of the relevant FEB
	HWIdentifier channelId=m_onlineID->channel_Id(febId,icha);

	if (cabling->isOnlineConnected(channelId)) {
	  Identifier offlineId=cabling->cnvToIdentifier(channelId);
	  const CaloDetDescrElement* caloddElement=calodetdescrmgr->get_element(offlineId);
	  
	  CaloCell_ID::CaloSample current_layer=caloddElement->getSampling(); // calo layer
	  float current_eta=caloddElement->eta();
	  float current_eta_low=caloddElement->eta()-0.5*caloddElement->deta();
	  float current_eta_high=caloddElement->eta()+0.5*caloddElement->deta();
	  float current_phi_low=caloddElement->phi()-0.5*caloddElement->dphi();
	  float current_phi_high=caloddElement->phi()+0.5*caloddElement->dphi();
	  
	  if (caloddElement->is_lar_em_barrel() && caloddElement->getLayer()==3 && fabs(current_eta)>0.79 && fabs(current_eta)<1.33) { //EMB, back sampling, slot 12 : special treatment : this FEB has non contiguous eta regions, so we have to separate them
	    is_additive1=1; //in order to add it to the summary only if it happens
	    
	    if (current_layer<layer_min_additive1)
	      layer_min_additive1=current_layer;
	    if (current_layer>layer_max_additive1)
	      layer_max_additive1=current_layer;
	    
	    if (current_eta_low<eta_min_additive1)
	      eta_min_additive1=current_eta_low;
	    if (current_eta_high>eta_max_additive1)
	      eta_max_additive1=current_eta_high;
	    
            extendPhiRegion(current_phi_low,phi_min_additive1,phi_max_additive1);
            extendPhiRegion(current_phi_high,phi_min_additive1,phi_max_additive1);
	    
	  }
	  else { //normal case
	    is_normal=1; // normal case
	    
	    if (current_layer<layer_min)
	      layer_min=current_layer;
	    if (current_layer>layer_max)
	      layer_max=current_layer;
	    
	    if (current_eta_low<eta_min)
	      eta_min=current_eta_low;
	    if (current_eta_high>eta_max)
	      eta_max=current_eta_high;
	    
            extendPhiRegion(current_phi_low,phi_min,phi_max);
            extendPhiRegion(current_phi_high,phi_min,phi_max);

	  }
	} //end of isOnlineConnected()
      } // end of loop on channels
      
      if (is_normal) {
	CaloAffectedRegionInfo current_CaloAffectedRegionInfo(eta_min,eta_max,phi_min,phi_max,layer_min,layer_max,CaloAffectedRegionInfo::missingReadout);
	vAffected->push_back(current_CaloAffectedRegionInfo);
      }
      
      if (is_additive1) {
	CaloAffectedRegionInfo current_additive1_CaloAffectedRegionInfo(eta_min_additive1,eta_max_additive1,phi_min_additive1,phi_max_additive1,layer_min_additive1,layer_max_additive1,CaloAffectedRegionInfo::missingReadout);
	vAffected->push_back(current_additive1_CaloAffectedRegionInfo);
      }
    } // end of isMissingFeb  
  } // end of loop on Febs  
  return StatusCode::SUCCESS;
}
//====================================================================================
float LArHVCondAlg::HV_nominal(const char *identification,const float myparameter) const
{
  if (strcmp(identification,"EMBPS")==0) 
    return 1200.;
  else if (strcmp(identification,"EMECPS")==0)
    return 1600.;
  else if (strcmp(identification,"EMB")==0)
    return 2000.;
  else if (strcmp(identification,"EMEC")==0) {
    if ( fabs(myparameter)<1.5 )
      return 2500.;
    else if (fabs(myparameter)<1.6)
      return 2300.;
    else if (fabs(myparameter)<1.8 )
      return 2100.;
    else if ( fabs(myparameter) < 2.0 )
      return 1700.;
    else if ( fabs(myparameter) < 2.1 )
      return 1500.;
    else if ( fabs(myparameter) < 2.3 )
      return 1250.;
    else if ( fabs(myparameter) < 2.5 )
      return 1000.;
    else if ( fabs(myparameter) < 2.8 )
      return 2300.;
    else return 1800.;
  }
  else if (strcmp(identification,"HEC")==0) {
    return 1800.;
  }
  else if (strcmp(identification,"FCAL")==0) {
    if (myparameter<0.5)
      return 250.;
    else if (myparameter<1.5)
      return 375.;
    else if (myparameter<2.5)
      return 500.;
  }

  return -1;
}
//=========================================================================================

void LArHVCondAlg::extendPhiRegion(float phi, float & phi_min, float & phi_max) const {

  static const float epsilon=1e-4;
  
  phi = CaloPhiRange::fix(phi);

  if (phi_min>10. || phi_max<-10.) {
     phi_min = CaloPhiRange::fix(phi-epsilon);
     phi_max = CaloPhiRange::fix(phi+epsilon);
     return;
  }

  bool isInRegion=false;
  if (phi_min<phi_max) {
     if (phi>phi_min && phi<phi_max) isInRegion=true;
  }
  else {
     if (phi>phi_min || phi<phi_max) isInRegion=true;
  }
  if (isInRegion) return;

  float dphi1 = CaloPhiRange::diff(phi,phi_min); 
  float dphi2 = CaloPhiRange::diff(phi,phi_max); 
  if (fabs(dphi1)<fabs(dphi2) )
     phi_min=phi;
  else
     phi_max=phi;
  
  return;

}
