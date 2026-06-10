/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArDigits2NtupleEB.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "Identifier/HWIdentifier.h"
#include "LArRawEvent/LArSCDigit.h"
#include "LArRawEvent/LArLATOMEHeaderContainer.h"
#include "LArRawEvent/LArRawSCContainer.h"

LArDigits2NtupleEB::LArDigits2NtupleEB(const std::string& name, ISvcLocator* pSvcLocator):
  LArCond2NtupleBaseEB(name, pSvcLocator),
  m_ipass(0),
  m_event(0)
{
  m_ntTitle = "LArDigits";
  m_ntpath  = "/NTUPLES/FILE1/LARDIGITS";
}


StatusCode LArDigits2NtupleEB::initialize()
{
  ATH_MSG_DEBUG( "in initialize" );

  ATH_MSG_DEBUG(" IS it SC?? " << m_isSC );

  ATH_CHECK( LArCond2NtupleBaseEB::initialize() );

  StatusCode sc = m_nt->addItem("IEvent",m_IEvent);
  if (sc!=StatusCode::SUCCESS) {
    ATH_MSG_ERROR( "addItem 'IEvent' failed" );
    return sc;
  }

  sc = m_nt->addItem("samples",m_SC,m_Nsamples,m_samples);
  if (sc!=StatusCode::SUCCESS) {
    ATH_MSG_ERROR( "addItem 'samples' failed" );
    return sc;
  }

  sc = m_nt->addItem("Nsamples",m_SC,m_ntNsamples,0,32);
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

  if(!m_isSC){
    sc = m_nt->addItem("ELVL1Id",m_SC,m_ELVL1Id);
    if (sc!=StatusCode::SUCCESS) {
       ATH_MSG_ERROR( "addItem 'ELVL1Id' failed" );
       return sc;
    }
  }

    // SC_ET RawSCContainer
  sc = m_nt->addItem("energyVec_ET", m_SC, m_Net, m_energyVec_ET);
  if (sc.isFailure()) {
	ATH_MSG_ERROR( "addItem 'energyVec_ET' failed" );
	return sc;
      }

  sc = m_nt->addItem("bcidVec_ET", m_SC, m_Net, m_bcidVec_ET);
  if (sc.isFailure()) {
	 ATH_MSG_ERROR( "addItem 'bcidVec_ET' failed" );
	 return sc;
      }

  sc = m_nt->addItem("saturVec_ET", m_SC, m_Net, m_saturVec_ET);
    if (sc.isFailure()) {
	 ATH_MSG_ERROR( "addItem 'saturVec_ET' failed" );
	 return sc;
      }

  sc = m_nt->addItem("energyVec_ET_ID", m_SC, m_Net, m_energyVec_ET_ID);
    if (sc.isFailure()) {
	 ATH_MSG_ERROR( "addItem 'energyVec_ET_ID' failed" );
	 return sc;
      }

  sc = m_nt->addItem("bcidVec_ET_ID", m_SC, m_Net, m_bcidVec_ET_ID);
    if (sc.isFailure()) {
	 ATH_MSG_ERROR( "addItem 'bcidVec_ET_ID' failed" );
	 return sc;
      }

  sc = m_nt->addItem("saturVec_ET_ID", m_SC, m_Net, m_saturVec_ET_ID);
    if (sc.isFailure()) {
	 ATH_MSG_ERROR( "addItem 'saturVec_ET_ID' failed" );
	 return sc;
      }

  ATH_CHECK(m_contKey.initialize() );
  ATH_CHECK(m_evtInfoKey.initialize() );
  ATH_CHECK(m_LArFebHeaderContainerKey.initialize(!m_isSC) );

  m_ipass	   = 0;
  m_event	   = 0;

  return StatusCode::SUCCESS;

}

StatusCode LArDigits2NtupleEB::execute(const EventContext& ctx)
{

  StatusCode	sc;

  ATH_MSG_DEBUG( "LArDigits2NtupleEB in execute" );
  m_event++;
  unsigned long long thisevent;
  unsigned long	thisbcid	   = 0;
  unsigned long	thisELVL1Id	   = 0;

  SG::ReadHandle<xAOD::EventInfo>evt (m_evtInfoKey, ctx);
  thisevent	   = evt->eventNumber();

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
    thisbcid	   = evt->bcid();
  }

  m_contKey="SC";
  SG::ReadHandle<LArDigitContainer> hdlDigit(m_contKey, ctx);

  if(!hdlDigit.isValid()) {
    ATH_MSG_WARNING( "Unable to retrieve LArDigitContainer with key " << m_contKey << " from DetectorStore. " );
    return StatusCode::SUCCESS;
  } else
    ATH_MSG_DEBUG( "Got LArDigitContainer with key " << m_contKey.key() );

  const LArDigitContainer DigitContainer   = *hdlDigit;
  const LArRawSCContainer*	etcontainer	   = nullptr;
  const LArRawSCContainer*	etcontainer_next	   = nullptr;

  unsigned cellCounter=0;

  if(m_fillBCID) m_bcid	   = thisbcid;
  m_IEvent                 = thisevent;


  sc = evtStore()->retrieve(etcontainer,"SC_ET");
  if (sc.isFailure()) { ATH_MSG_WARNING( "Unable to retrieve LArRawSCContainer with key SC_ET from DetectorStore. " );}
  else ATH_MSG_DEBUG( "Got LArRawSCContainer with key SC_ET " );

  sc = evtStore()->retrieve(etcontainer_next,"SC_ET_ID");
  if (sc.isFailure()) { ATH_MSG_WARNING( "Unable to retrieve LArRawSCContainer with key SC_ET_ID from DetectorStore. " );}
  else ATH_MSG_DEBUG( "Got LArRawSCContainer with key SC_ET_ID" );



 for( const LArDigit *digi : DigitContainer ){

    unsigned int trueMaxSample	   = digi->nsamples();
    m_ntNsamples[cellCounter]   = trueMaxSample;

    if (!m_isSC){
      m_gain[cellCounter]	   = digi->gain();
      m_ELVL1Id[cellCounter]	   = thisELVL1Id;
      if(m_gain[cellCounter] < CaloGain::INVALIDGAIN || m_gain[cellCounter] > CaloGain::LARNGAIN) m_gain[cellCounter]  = CaloGain::LARNGAIN;
    }
    if(trueMaxSample>m_Nsamples){
      if(!m_ipass){
        ATH_MSG_WARNING( "The number of samples in data is larger than the one specified by JO: " << trueMaxSample << " > " << m_Nsamples << " --> only " << m_Nsamples << " will be available in the ntuple " );
        m_ipass   = 1;
      }
      trueMaxSample   = m_Nsamples;
    }

    fillFromIdentifier(digi->hardwareID(), cellCounter);


    for(unsigned i =	0; i<trueMaxSample;++i) m_samples[cellCounter][i]	   = digi->samples().at(i);

     if( etcontainer ){
      const LArRawSC*rawSC   = etcontainer->at(cellCounter);

      unsigned int truenet = m_Net;

      if(truenet > rawSC->bcids().size()) truenet=rawSC->bcids().size();
      for( unsigned i=0; i<truenet;++i){	// just use the vector directly?
	    m_bcidVec_ET[cellCounter][i]	   = rawSC->bcids().at(i);
      }

      if(truenet > rawSC->energies().size()) truenet=rawSC->energies().size();
      for( unsigned i=0; i<truenet;++i){	// just use the vector directly?
	     m_energyVec_ET[cellCounter][i]	   = rawSC->energies().at(i);

      }

      if(truenet > rawSC->satur().size()) truenet=rawSC->satur().size();

      for( unsigned i = 0; i<truenet;++i){	// just use the vector directly?
	    m_saturVec_ET[cellCounter][i]	   = rawSC->satur().at(i);
       }

      m_Net=truenet;
    }

     if( etcontainer_next ){
      const LArRawSC*rawSC   = etcontainer_next->at(cellCounter);

      for( unsigned i=0; i<rawSC->bcids().size();++i){	// just use the vector directly?
	     m_bcidVec_ET_ID[cellCounter][i]	   = rawSC->bcids()[i];
      }
      for( unsigned i=0; i<rawSC->energies().size();++i){	// just use the vector directly?
	     m_energyVec_ET_ID[cellCounter][i]	   = rawSC->energies()[i];
      }
      for( unsigned i = 0; i<rawSC->satur().size();++i){	// just use the vector directly? ?
	     m_saturVec_ET_ID[cellCounter][i]	   = rawSC->satur()[i];
      }
    }

    cellCounter++;
  }// over cells


  ATH_CHECK( ntupleSvc()->writeRecord(m_nt) );

  ATH_MSG_DEBUG( "LArDigits2NtupleEB has finished, filled"<< cellCounter << " cells" );
  return StatusCode::SUCCESS;
}// end finalize-method.
