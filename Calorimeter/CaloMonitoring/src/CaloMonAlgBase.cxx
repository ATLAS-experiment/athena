/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloMonAlgBase.h" 

CaloMonAlgBase::CaloMonAlgBase(const std::string& name, ISvcLocator* pSvcLocator) 
  :AthMonitorAlgorithm(name, pSvcLocator)
 {
  declareProperty("useBadLBTool", m_useBadLBTool=false);
  declareProperty("BadLBTool", m_BadLBTool);

  declareProperty("useReadyFilterTool",m_useReadyFilterTool=true);
  declareProperty("ReadyFilterTool",m_ReadyFilterTool);

  declareProperty("useLArCollisionFilterTool",m_useCollisionFilterTool=true);

  declareProperty("useLArNoisyAlg",m_useLArNoisyAlg=false);

  declareProperty("useBeamBackgroundRemoval",m_useBeamBackgroundRemoval=false);
}

StatusCode CaloMonAlgBase::initialize() {

  // retrieve AtlasReadyFilter tool
  if(m_useReadyFilterTool){
   StatusCode sc = m_ReadyFilterTool.retrieve();
   if(sc.isFailure()) {
     ATH_MSG_ERROR("Could Not Retrieve AtlasReadyFilterTool " << m_ReadyFilterTool);
     m_useReadyFilterTool = false;
     return sc;
   }
  }
  else {
    m_ReadyFilterTool.disable();
  }

  // retrieve BadLBFilter tool 
 if(m_useBadLBTool){
  StatusCode sc = m_BadLBTool.retrieve();
  if(sc.isFailure()){
    ATH_MSG_ERROR("Unable to retrieve the DQBadLBFilterTool");
    m_useBadLBTool = false;
    return sc;
  }
  ATH_MSG_INFO("DQBadLBFilterTool retrieved");
 }
 else {
   m_BadLBTool.disable();
 } 

 ATH_CHECK( m_LArCollisionTimeKey.initialize(m_useCollisionFilterTool) );
 ATH_CHECK( m_beamBackgroundKey.initialize(m_useBeamBackgroundRemoval) );
 ATH_CHECK( detStore()->retrieve (m_calo_id, "CaloCell_ID") );

 return AthMonitorAlgorithm::initialize();
}

StatusCode CaloMonAlgBase::checkFilters(bool &ifPass, bool &passBeamBackgroundRemoval, const std::string &MonGroupName, const EventContext &ctx) const{

  ATH_MSG_DEBUG("CaloMonAlgBase:checkFilters() starts "<<MonGroupName);

  auto evtbin = Monitored::Scalar<int>("EvtBin",1);
  fill(MonGroupName,evtbin);

  ifPass = 0;   
  if (m_useReadyFilterTool) {
    if (m_ReadyFilterTool->accept()) {
      ifPass = 1;
      evtbin = 2;
      fill(MonGroupName,evtbin); //All events with ATLAS Ready
    }
  }
  else{
    evtbin = 2;
    fill(MonGroupName,evtbin); //ATLAS ready not activated
    ifPass = 1;
  }

  ATH_MSG_DEBUG("CaloMonAlgBase::checkFilters()  ATLAS Ready done");

  SG::ReadHandle<xAOD::EventInfo> eventInfo{GetEventInfo(ctx)};

  if (m_useBadLBTool) {
    if (m_BadLBTool->accept()) { 
      ifPass = ifPass && 1;
      if(ifPass) {evtbin=3; fill(MonGroupName,evtbin);} //All events with ATLAS Ready and Good LB
    }
    else {
      ifPass = 0;
    }
  }
  else{
    if(ifPass) {evtbin=3; fill(MonGroupName,evtbin);} 
  }

  ATH_MSG_DEBUG("CaloMonAlgBase::checkFilters() BadLBTool  done");

  // Filter the events identfied as collision by the LAr system
  // Useful in CosmicCalo to reject collision candidates
  if (m_useCollisionFilterTool){
    SG::ReadHandle<LArCollisionTime> larTime{m_LArCollisionTimeKey,ctx};
    if(!larTime.isValid()){
      ATH_MSG_WARNING("Unable to retrieve LArCollisionTime event store");
      if(ifPass) {evtbin=4; fill(MonGroupName,evtbin);} 
    }
    else {
      if (larTime->timeC()!=0 && larTime->timeA()!=0 && std::fabs(larTime->timeC() - larTime->timeA())<10)  {
	ifPass = 0;
      }
      else { 
	ifPass = ifPass && 1;
	if(ifPass) {evtbin=4; fill(MonGroupName,evtbin);} //All events with ATLAS Ready and Good LB and Good LAr collision time
      }
    }
  }
  else{
    if(ifPass) {evtbin=4; fill(MonGroupName,evtbin);} 
  }
  ATH_MSG_DEBUG("CaloMonAlgBase::checkFilters() CollisionFilterTool  done");


  passBeamBackgroundRemoval=true;
  if(m_useBeamBackgroundRemoval){
    SG::ReadHandle<BeamBackgroundData> beamBackgroundData{m_beamBackgroundKey, ctx};
    if(!beamBackgroundData.isValid()){
      ATH_MSG_WARNING("Unable to retrieve BeamBackgroundData");
    }
    else {
      if( beamBackgroundData->GetNumSegment() > 0 ) {
        passBeamBackgroundRemoval = false;
	ifPass = 0;
        ATH_MSG_DEBUG("Identified background event");
      }
      else {
        passBeamBackgroundRemoval = true;
        ifPass = ifPass && 1;
        if(ifPass){evtbin=5; fill(MonGroupName,evtbin);}  //All events with ATLAS Ready and Good LB and Good LAr collision time and not Beam Background
      }
    }
  }
  else {  // Do not use BeamBackgroundRemoval
    ifPass = ifPass && 1;
    if(ifPass) {evtbin=5; fill(MonGroupName,evtbin);}  //All events with ATLAS Ready and Good LB and Good LAr collision time and not Beam Background
  
  }
  ATH_MSG_DEBUG("CaloMonAlgBase::checkFilters() m_useBeamBackgroundRemoval  done");

  std::string TheTrigger;
  if ( m_vTrigChainNames.empty()) {  
    TheTrigger="NoTrigSel";
    ifPass = ifPass && 1; // No Trigger Filter check
    if(ifPass) {evtbin=6; fill(MonGroupName,evtbin);} //All events with ATLAS Ready and Good LB and Good LAr collision time and not Beam Background and Trigger Filter pass
  }
  else {
    TheTrigger = m_triggerChainString; // Trigger Filter not implemented ++ FIXME ==
    if(ifPass) {evtbin=6; fill(MonGroupName,evtbin);}  
  }

  if(m_useLArNoisyAlg && (eventInfo->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error)) {
    ifPass = 0;
  }
  else {
    ifPass = ifPass && 1;
    if(ifPass) {evtbin=7; fill(MonGroupName,evtbin);} //All events with ATLAS Ready and Good LB and Good LAr collision time and not Beam Background and Trigger Filter pass and no Lar Error
  }

  ATH_MSG_DEBUG("CaloMonAlgBase::checkFilters() is done");
  return StatusCode::SUCCESS;
}



void CaloMonAlgBase::getHistoCoordinates(const CaloDetDescrElement* dde, float& celleta, float& cellphi, unsigned& iLyr, unsigned& iLyrNS) const {
  
  celleta=dde->eta_raw();
  cellphi=dde->phi_raw();
  
  int calosample=dde->getSampling();
  if (dde->is_lar_em_endcap_inner()) calosample-=1; //Here, we consider the two layers of the EMEC IW as EME1 and EME2 instad of layer 2 and 3
  iLyrNS=m_caloSamplingToLyrNS.at(calosample); //will throw if out of bounds
  if ((iLyrNS==EMB1NS || iLyrNS==EMB2NS) && m_calo_id->region(dde->identify())==1) {
    //We are in the awkward region 1 of the EM Barrel
    //Looking at the image at http://atlas.web.cern.ch/Atlas/GROUPS/LIQARGEXT/TDR/figures6/figure6-17.eps
    //may be useful to understand what's going on here. 

    //In brief: Region 1, layer 1 is closer related ot the middle compartment (aka layer 2)
    //          and region 1, layer 2 closer related to the back compartment (aka layer 3)
    iLyrNS+=1;

    //Layer 2: 0<eta<1.4 + 1.4<eta<1.475, deta = 0.025. 3 rows of cells from region 1
    //Layer 3: 0<eta<1.35 (deta=0.050) +  1.4<eta<1.475 (deta = 0.075).  1 row of cell from region 1 with different dEta
  }
  
  const unsigned side=(celleta>0) ? 0 : 1; //Value >0 means A-side
  iLyr=iLyrNS*2+side;  
  }