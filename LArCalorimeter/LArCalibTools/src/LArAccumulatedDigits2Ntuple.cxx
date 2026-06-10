/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArAccumulatedDigits2Ntuple.h"
#include "LArRawEvent/LArAccumulatedDigitContainer.h"

//#include "GaudiKernel/ToolHandle.h"

LArAccumulatedDigits2Ntuple::LArAccumulatedDigits2Ntuple(const std::string& name, ISvcLocator* pSvcLocator):
  LArCond2NtupleBase(name, pSvcLocator),
  m_ipass(0),
  m_event(0)
{
  m_ntTitle="AccumulatedDigits";
  m_ntpath="/NTUPLES/FILE1/ACCUMULATEDDIGITS"+m_contKey.key();

}

LArAccumulatedDigits2Ntuple::~LArAccumulatedDigits2Ntuple() 
= default;


StatusCode LArAccumulatedDigits2Ntuple::initialize()
{
   ATH_MSG_INFO( "in initialize" ); 

   StatusCode sc=LArCond2NtupleBase::initialize();
   if (sc!=StatusCode::SUCCESS) {
     ATH_MSG_ERROR( "Base init failed" );
     return StatusCode::FAILURE;
   }



  sc=m_nt->addItem("IEvent",m_IEvent);
  if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "addItem 'IEvent' failed" );
      return sc;
    }

  sc=m_nt->addItem("EventNum",m_EventNum);
  if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "addItem 'EventNum' failed" );
      return sc;
    }

  sc=m_nt->addItem("Ntrigger",m_Ntrigger,0,500); 
  if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "addItem 'Ntrigger' failed" );
      return sc;
    }
  
  sc=m_nt->addItem("Nsamples",m_ntNsamples,0,32);
  if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "addItem 'Nsamples' failed" );
      return sc;
    }

  sc=m_nt->addItem("Sum",m_Nsamples,m_sum);
  if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "addItem 'sum' failed" );
      return sc;
    }
  
  sc=m_nt->addItem("Sumsq",m_Nsamples,m_sumsq);
  if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "addItem 'sumsq' failed" );
      return sc;
    }

  sc=m_nt->addItem("Mean",m_mean);
  if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "addItem 'mean' failed" );
      return sc;
    }

  sc=m_nt->addItem("RMS",m_rms);
  if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "addItem 'rms' failed" );
      return sc;
    }

  sc=m_nt->addItem("covr",m_Nsamples-1,m_covr);
  if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "addItem 'covr' failed" );
      return sc;
    }

  ATH_CHECK( m_contKey.initialize(!m_contKey.key().empty()) );

  m_ipass = 0;
  m_event=0;

  return StatusCode::SUCCESS;

}

StatusCode LArAccumulatedDigits2Ntuple::execute(const EventContext& ctx)
{

  ATH_MSG_DEBUG( "in execute" ); 
  StatusCode sc;
  
  if (m_contKey.key().empty()) return StatusCode::SUCCESS;


  m_event++;
  
  const LArAccumulatedDigitContainer* accuDigitContainer = nullptr;
  SG::ReadHandle<LArAccumulatedDigitContainer> Hdl{m_contKey, ctx};
  if (!Hdl.isValid()) {
     ATH_MSG_WARNING( "Unable to retrieve LArAccumulatedDigitContainer with key " << m_contKey.key() << " from DetectorStore. " );
     return StatusCode::SUCCESS;
  } else {
     ATH_MSG_DEBUG( "Got LArAccumulatedDigitContainer with key " << m_contKey.key() );
     accuDigitContainer = Hdl.cptr();
  }
 
 if (accuDigitContainer) { 
   
   if(accuDigitContainer->empty()) {
     ATH_MSG_DEBUG( "LArAccumulatedDigitContainer with key=" << m_contKey << " is empty " );
     return StatusCode::SUCCESS;
   }else{
     ATH_MSG_DEBUG( "LArAccumulatedDigitContainer with key=" << m_contKey << " has " <<accuDigitContainer->size() << " entries" );
   }

   for (const LArAccumulatedDigit* digit : *accuDigitContainer) {

     m_IEvent = m_event;
     m_EventNum = ctx.eventID().event_number(); 
     m_Ntrigger = digit->nTrigger();
     unsigned int trueMaxSample = digit->nsample();
     m_ntNsamples = trueMaxSample;

     if(trueMaxSample>m_Nsamples){
       if(!m_ipass){
	 ATH_MSG_WARNING( "The number of samples in data is larger than the one specified by JO: " << trueMaxSample << " > " << m_Nsamples << " --> only " << m_Nsamples << " will be available in the ntuple " );
	 m_ipass=1;
       }
       trueMaxSample = m_Nsamples;
     }

     m_mean = digit->mean();
     m_rms  = digit->RMS();
     const std::vector<uint64_t> sampleSquare = digit->sampleSquare();
     const std::vector<uint64_t> sampleSum    = digit->sampleSum();
     for(unsigned i=0;i<trueMaxSample;i++) {
       m_sumsq[i] = sampleSquare[i];
       m_sum[i]   = sampleSum[i];
     }
     std::vector<float> cov;
     digit->getCov(cov,m_normalize);
     for(unsigned i=0;i<trueMaxSample-1;i++) {
       m_covr[i] = cov[i];
     }

     fillFromIdentifier(digit->hardwareID());      
     sc=ntupleSvc()->writeRecord(m_nt);
     if (sc!=StatusCode::SUCCESS) {
       ATH_MSG_ERROR( "writeRecord failed" );
       return sc;
     }
   } 
 } 
 return StatusCode::SUCCESS;
}// end execute method.
