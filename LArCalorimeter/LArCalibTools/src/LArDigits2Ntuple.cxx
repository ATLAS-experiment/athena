/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArDigits2Ntuple.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "Identifier/HWIdentifier.h"
#include "LArRawEvent/LArSCDigit.h"

LArDigits2Ntuple::LArDigits2Ntuple(const std::string& name, ISvcLocator* pSvcLocator):
  LArCond2NtupleBase(name, pSvcLocator),
  m_ipass(0),
  m_event(0)
{
  m_ntTitle = "LArDigits";
  m_ntpath  = "/NTUPLES/FILE1/LARDIGITS";  
}

LArDigits2Ntuple::~LArDigits2Ntuple() 
= default;


StatusCode LArDigits2Ntuple::initialize()
{
  ATH_MSG_DEBUG( "in initialize" ); 

  ATH_MSG_DEBUG(" IS it SC?? " << m_isSC );

  if( (m_contKey.key().size()) && (m_accContKey.key().size() ||  m_accCalibContKey.key().size()) ){
       ATH_MSG_FATAL("Could not run with both standard and acc. digits !!!");
       ATH_MSG_FATAL("m_contKey: "<<m_contKey.key()<<" m_accContKey: "<<m_accContKey.key()<<" m_accCalibContKey: "<<m_accCalibContKey.key());
       return StatusCode::FAILURE;
  }
  
  ATH_CHECK( LArCond2NtupleBase::initialize() );
  
  StatusCode sc = m_nt->addItem("IEvent",m_IEvent);
  if (sc!=StatusCode::SUCCESS) {
    ATH_MSG_ERROR( "addItem 'IEvent' failed" );
    return sc;
  }
  
  if(m_accContKey.key().size() || m_accCalibContKey.key().size()) {
     sc = m_nt->addItem("mean",m_Nsamples,m_mean);
     if (sc!=StatusCode::SUCCESS) {
       ATH_MSG_ERROR( "addItem 'mean' failed" );
       return sc;
     }
     sc = m_nt->addItem("RMS",m_Nsamples,m_RMS);
     if (sc!=StatusCode::SUCCESS) {
       ATH_MSG_ERROR( "addItem 'RMS' failed" );
       return sc;
     }
     if(m_accCalibContKey.key().size()) { 
       sc = m_nt->addItem("DAC",m_dac);
       if (sc!=StatusCode::SUCCESS) {
         ATH_MSG_ERROR( "addItem 'DAC' failed" );
         return sc;
       }
       sc = m_nt->addItem("delay",m_delay);
       if (sc!=StatusCode::SUCCESS) {
         ATH_MSG_ERROR( "addItem 'delay' failed" );
         return sc;
       }
       sc = m_nt->addItem("Pulsed",m_pulsed);
       if (sc!=StatusCode::SUCCESS) {
         ATH_MSG_ERROR( "addItem 'Pulsed' failed" );
         return sc;
       }
     }
  }
   
  if(m_contKey.key().size()) {
     sc = m_nt->addItem("samples",m_Nsamples,m_samples);
     if (sc!=StatusCode::SUCCESS) {
       ATH_MSG_ERROR( "addItem 'samples' failed" );
       return sc;
     }
  }

  sc = m_nt->addItem("Nsamples",m_ntNsamples,0,32);
  if (sc!=StatusCode::SUCCESS) {
    ATH_MSG_ERROR( "addItem 'Nsamples' failed" );
    return sc;
  }

  if(m_fillBCID){
    sc = m_nt->addItem("BCID",m_bcid);
    if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "addItem 'BCID' failed" );
      return sc;
    }
  }

  sc = m_nt->addItem("ELVL1Id",m_ELVL1Id);
  if (sc!=StatusCode::SUCCESS) {
     ATH_MSG_ERROR( "addItem 'ELVL1Id' failed" );
     return sc;
  }

  if(!m_isSC){
    sc	   = m_nt->addItem("Gain",m_gain,-1,3);
    if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "addItem 'Gain' failed" );
      return sc;
    }
  }

  if(m_fillLB){
    NTuplePtr nt(ntupleSvc(),m_ntpath+"Evnt");
    if (!nt) {
       nt=ntupleSvc()->book(m_ntpath+"Evnt",CLID_ColumnWiseTuple,m_ntTitle+"Evnt");
    }
    if (!nt){
       ATH_MSG_ERROR( "Booking of NTuple at "<< m_ntpath << " and name " << m_ntTitle << " failed" );
       return StatusCode::FAILURE;
    }

    m_evt_nt=nt;

    sc = m_evt_nt->addItem("IEvent",m_IEventEvt);
    if (sc!=StatusCode::SUCCESS) {
       ATH_MSG_ERROR( "addItem 'IEvent' failed" );
       return sc;
    }

    sc=m_evt_nt->addItem("LB",m_LB);
    if (sc.isFailure()) {
        ATH_MSG_ERROR( "addItem 'LB' failed" );
        return sc;
    }
  }
  ATH_CHECK(m_contKey.initialize(m_contKey.key().size()) );
  ATH_CHECK(m_accContKey.initialize(m_accContKey.key().size()) );
  ATH_CHECK(m_accCalibContKey.initialize(m_accCalibContKey.key().size()) );
  ATH_CHECK(m_LArFebHeaderContainerKey.initialize(!m_isSC) );

  m_ipass	   = 0;
  m_event	   = 0;
  
  return StatusCode::SUCCESS;
  
}

StatusCode LArDigits2Ntuple::execute()
{

  const EventContext& ctx = Gaudi::Hive::currentContext();
  if(!m_contKey.key().size() && !m_accContKey.key().size() && !m_accCalibContKey.key().size()) return StatusCode::SUCCESS;

  StatusCode	sc;
  
  ATH_MSG_DEBUG( "LArDigits2Ntuple in execute" ); 
  m_event++;
  unsigned long long thisevent;
  unsigned long	thisbcid	   = 0;
  unsigned long	thisELVL1Id	   = 0;

  thisevent	   = ctx.eventID().event_number();

  // Get BCID from FEB header
  if ( !m_isSC ){ // we are not processing SC data, Feb header could be accessed
    SG::ReadHandle<LArFebHeaderContainer> hdrCont(m_LArFebHeaderContainerKey, ctx);
    if (! hdrCont.isValid()) {
      ATH_MSG_WARNING( "No LArFEB container found in TDS" );
    }
    else {
      ATH_MSG_DEBUG( "LArFEB container found");
      if(m_fillBCID) thisbcid	   = (*hdrCont->begin())->BCId() ;
      ATH_MSG_DEBUG( "BCID FROM FEB HEADER " << thisbcid );
      thisELVL1Id   = (*hdrCont->begin())->ELVL1Id();
      ATH_MSG_DEBUG( "NSAMPLES FROM FEB HEADER " << (*hdrCont->begin())->NbSamples() );
    }
  }else{
    // This should be used for main readout later, once TDAQ fill event headers also in calib. runs properly
    thisbcid	   = ctx.eventID().bunch_crossing_id();
  }

  if( m_contKey.key().size() ) { // fill from standard digits
    SG::ReadHandle<LArDigitContainer> hdlDigit(m_contKey, ctx);
    if(!hdlDigit.isValid()) {
      ATH_MSG_WARNING( "Unable to retrieve LArDigitContainer with key " << m_contKey << " from DetectorStore. " );
      return StatusCode::SUCCESS;
    } else
      ATH_MSG_DEBUG( "Got LArDigitContainer with key " << m_contKey.key() );
 
    const LArDigitContainer DigitContainer   = *hdlDigit;
 
    if(!hdlDigit.cptr()) {
       ATH_MSG_WARNING( "No digits in this event ?");
       return StatusCode::SUCCESS;
    }
 
    for( const LArDigit *digi : DigitContainer ){
 
      if(m_fillBCID) m_bcid	= thisbcid; 
      m_ELVL1Id	   = thisELVL1Id; 
      m_IEvent	   = thisevent;
 
      unsigned int trueMaxSample	   = digi->nsamples();
 
      if (!m_isSC){
        m_gain	   = digi->gain();
        if(m_gain < CaloGain::INVALIDGAIN || m_gain > CaloGain::LARNGAIN) m_gain  = CaloGain::LARNGAIN;
      }
      if(trueMaxSample>m_Nsamples){
        if(!m_ipass){
          ATH_MSG_WARNING( "The number of digi samples in data is larger than the one specified by JO: " << trueMaxSample << " > " << m_Nsamples << " --> only " << m_Nsamples << " will be available in the ntuple " );
          m_ipass   = 1;
        }
        trueMaxSample   = m_Nsamples;
      }
      else if(trueMaxSample<m_Nsamples){
        if(!m_ipass){
          ATH_MSG_WARNING( "The number of digi samples in data is lower than the one specified by JO: " << trueMaxSample << " > " << m_Nsamples << " --> only " << trueMaxSample << " will be available in the ntuple " );
          m_ipass   = 1;
        }
      }
      m_ntNsamples   = trueMaxSample;
      ATH_MSG_DEBUG( "The number of digi samples in data "<< m_Nsamples  );
 
      fillFromIdentifier(digi->hardwareID());      
 
      if(m_isSC && m_fillEMB && m_barrel_ec !=0) continue;
      if(m_isSC && m_fillEndcap && m_barrel_ec !=1) continue;
 
      if(m_FTlist.size() > 0) {	// should do a selection
        if(std::find(std::begin(m_FTlist), std::end(m_FTlist), m_FT)  == std::end(m_FTlist)) {	// is our FT in list ?
          continue;
        }
      }
 
      if(m_Slotlist.size() > 0) {	// should do a selection
        if(std::find(std::begin(m_Slotlist), std::end(m_Slotlist), m_slot)  == std::end(m_Slotlist)) {	// is our slot in list ?
          continue;
        }
      }

      if(m_Sidelist.size() > 0) {	// should do a selection
        if(std::find(std::begin(m_Sidelist), std::end(m_Sidelist), m_pos_neg)  == std::end(m_Sidelist)) {	// is our side in list ?
          continue;
        }
      }

      if(m_BElist.size() > 0) {	// should do a selection
        if(std::find(std::begin(m_BElist), std::end(m_BElist), m_barrel_ec)  == std::end(m_BElist)) {	// is our BE in list ?
          continue;
        }
      }

      for(unsigned i =	0; i<trueMaxSample;++i) m_samples[i]	   = digi->samples().at(i);
 
 
      ATH_CHECK( ntupleSvc()->writeRecord(m_nt) );
    }// over cells 
  }// standard digits
   
  if( m_accCalibContKey.key().size() ) { // fill from acc. calib digits
    SG::ReadHandle<LArAccumulatedCalibDigitContainer> hdlDigit(m_accCalibContKey, ctx);
    if(!hdlDigit.isValid()) {
      ATH_MSG_WARNING( "Unable to retrieve LArAccumulatedCalibDigitContainer with key " << m_accCalibContKey << " from DetectorStore. " );
      return StatusCode::SUCCESS;
    } else
      ATH_MSG_DEBUG( "Got LArAccumulatedCalibDigitContainer with key " << m_accCalibContKey.key() );
 
    const LArAccumulatedCalibDigitContainer DigitContainer   = *hdlDigit;
 
    if(!hdlDigit.cptr()) {
       ATH_MSG_WARNING( "No digits in this event ?");
       return StatusCode::SUCCESS;
    }
 
    for( const LArAccumulatedCalibDigit *digi : DigitContainer ){
 
      if(m_fillBCID) m_bcid	= thisbcid; 
      m_ELVL1Id	   = thisELVL1Id; 
      m_IEvent	   = thisevent;
 
      unsigned int trueMaxSample	   = digi->nsamples();
 
      if (!m_isSC){
        m_gain	   = digi->gain();
        if(m_gain < CaloGain::INVALIDGAIN || m_gain > CaloGain::LARNGAIN) m_gain  = CaloGain::LARNGAIN;
      }
      if(trueMaxSample>m_Nsamples){
        if(!m_ipass){
          ATH_MSG_WARNING( "The number of digi samples in data is larger than the one specified by JO: " << trueMaxSample << " > " << m_Nsamples << " --> only " << m_Nsamples << " will be available in the ntuple " );
          m_ipass   = 1;
        }
        trueMaxSample   = m_Nsamples;
      }
      else if(trueMaxSample<m_Nsamples){
        if(!m_ipass){
          ATH_MSG_WARNING( "The number of digi samples in data is lower than the one specified by JO: " << trueMaxSample << " > " << m_Nsamples << " --> only " << trueMaxSample << " will be available in the ntuple " );
          m_ipass   = 1;
        }
      }
      m_ntNsamples   = trueMaxSample;
      ATH_MSG_DEBUG( "The number of acc. calib digi samples in data "<< m_Nsamples  );
 
      fillFromIdentifier(digi->hardwareID());      
 
      if(m_isSC && m_fillEMB && m_barrel_ec !=0) continue;
      if(m_isSC && m_fillEndcap && m_barrel_ec !=1) continue;
 
      if(m_FTlist.size() > 0) {	// should do a selection
        if(std::find(std::begin(m_FTlist), std::end(m_FTlist), m_FT)  == std::end(m_FTlist)) {	// is our FT in list ?
          continue;
        }
      }
 
      if(m_Slotlist.size() > 0) {	// should do a selection
        if(std::find(std::begin(m_Slotlist), std::end(m_Slotlist), m_slot)  == std::end(m_Slotlist)) {	// is our slot in list ?
          continue;
        }
      }
      for(unsigned i =	0; i<trueMaxSample;++i) {
         m_mean[i] = digi->mean(i);
         m_RMS[i] = digi->RMS(i);
      }
      //m_nTriggers = digi->nTriggers();
      m_dac = digi->DAC();
      m_delay = digi->delay();
      m_pulsed = digi->getIsPulsedInt();
 
 
      ATH_CHECK( ntupleSvc()->writeRecord(m_nt) );
    }// over cells 
  }// acc calib. digits

  if( m_accContKey.key().size() ) { // fill from acc digits
    SG::ReadHandle<LArAccumulatedDigitContainer> hdlDigit(m_accContKey, ctx);
    if(!hdlDigit.isValid()) {
      ATH_MSG_WARNING( "Unable to retrieve LArAccumulatedDigitContainer with key " << m_accContKey << " from DetectorStore. " );
      return StatusCode::SUCCESS;
    } else
      ATH_MSG_DEBUG( "Got LArAccumulatedDigitContainer with key " << m_accContKey.key() );
 
    const LArAccumulatedDigitContainer DigitContainer = *hdlDigit;
 
    if(!hdlDigit.cptr()) {
       ATH_MSG_WARNING( "No digits in this event ?");
       return StatusCode::SUCCESS;
    }
 
    for( const LArAccumulatedDigit *digi : DigitContainer ){
 
      if(m_fillBCID) m_bcid = thisbcid; 
      m_ELVL1Id = thisELVL1Id; 
      m_IEvent = thisevent;
 
      unsigned int trueMaxSample = digi->nsample();
 
      if (!m_isSC){
        m_gain = digi->gain();
        if(m_gain < CaloGain::INVALIDGAIN || m_gain > CaloGain::LARNGAIN) m_gain  = CaloGain::LARNGAIN;
      }

      if(trueMaxSample>m_Nsamples){
        if(!m_ipass){
          ATH_MSG_WARNING( "The number of digi samples in data is larger than the one specified by JO: " << trueMaxSample << " > " << m_Nsamples << " --> only " << m_Nsamples << " will be available in the ntuple " );
          m_ipass   = 1;
        }
        trueMaxSample   = m_Nsamples;
      } else if(trueMaxSample<m_Nsamples){
        if(!m_ipass){
          ATH_MSG_WARNING( "The number of digi samples in data is lower than the one specified by JO: " << trueMaxSample << " > " << m_Nsamples << " --> only " << trueMaxSample << " will be available in the ntuple " );
          m_ipass   = 1;
        }
      }
      m_ntNsamples   = trueMaxSample;
      ATH_MSG_DEBUG( "The number of acc. digi samples in data "<< m_Nsamples  );
 
      fillFromIdentifier(digi->hardwareID());      
 
      if(m_isSC && m_fillEMB && m_barrel_ec !=0) continue;
      if(m_isSC && m_fillEndcap && m_barrel_ec !=1) continue;
 
      if(m_FTlist.size() > 0) {	// should do a selection
        if(std::find(std::begin(m_FTlist), std::end(m_FTlist), m_FT)  == std::end(m_FTlist)) {	// is our FT in list ?
          continue;
        }
      }
 
      if(m_Slotlist.size() > 0) {	// should do a selection
        if(std::find(std::begin(m_Slotlist), std::end(m_Slotlist), m_slot)  == std::end(m_Slotlist)) {	// is our slot in list ?
          continue;
        }
      }
      for(unsigned i =	0; i<trueMaxSample;++i) {
         m_mean[i] = digi->mean(i);
         m_RMS[i] = digi->RMS(i);
      }
 
      ATH_CHECK( ntupleSvc()->writeRecord(m_nt) );
    }// over cells 
  }// acc digits

  if(m_fillLB) {
     m_IEventEvt   = thisevent;
     m_LB       = ctx.eventID().lumi_block();

     sc   = ntupleSvc()->writeRecord(m_evt_nt);
     if (sc != StatusCode::SUCCESS) {
       ATH_MSG_ERROR( "writeRecord failed" );
       return sc;
     }
  }

  ATH_MSG_DEBUG( "LArDigits2Ntuple has finished." );
  return StatusCode::SUCCESS;
}// end finalize-method.
