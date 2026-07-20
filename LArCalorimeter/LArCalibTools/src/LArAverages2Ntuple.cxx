/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArAverages2Ntuple.h"

#include "CaloIdentifier/CaloCell_ID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"

LArAverages2Ntuple::LArAverages2Ntuple(const std::string& name, ISvcLocator* pSvcLocator): 
  LArCond2NtupleBase(name, pSvcLocator),m_event(0),m_pass(false)
{ }

StatusCode LArAverages2Ntuple::initialize()
{
  ATH_MSG_INFO ( "in initialize" );

  StatusCode sc;
  if ( m_isSC ){
    const LArOnline_SuperCellID* ll;
    sc = detStore()->retrieve(ll, "LArOnline_SuperCellID");
    if (sc.isFailure()) {
      ATH_MSG_ERROR( "Could not get LArOnlineID helper !" );
      return StatusCode::FAILURE;
    }
    else {
      m_onlineHelper = dynamic_cast<const LArOnlineID_Base*>(ll);
      ATH_MSG_DEBUG("Found the LArOnlineID helper");
    }
  } else { // m_isSC
    const LArOnlineID* ll;
    sc = detStore()->retrieve(ll, "LArOnlineID");
    if (sc.isFailure()) {
      ATH_MSG_ERROR( "Could not get LArOnlineID helper !" );
      return StatusCode::FAILURE;
    }
    else {
      m_onlineHelper = dynamic_cast<const LArOnlineID_Base*>(ll);
      ATH_MSG_DEBUG(" Found the LArOnlineID helper. ");
    }
  }
  m_ntName = "AVERAGES";
  m_ntTitle="Averages";
  m_ntpath=std::string("/NTUPLES/FILE1/")+m_ntName+m_contKey.key();

  ATH_CHECK(LArCond2NtupleBase::initialize());

  ATH_CHECK( m_nt->addItem("IEvent",m_IEvent) );
  ATH_CHECK( m_nt->addItem("EventNum",m_EventNum) );
  ATH_CHECK( m_nt->addItem("DAC",m_DAC,0,65535) );
  ATH_CHECK( m_nt->addItem("isPulsed",m_isPulsed,0,1) );
  ATH_CHECK( m_nt->addItem("delay",m_delay,0,240) );
  ATH_CHECK( m_nt->addItem("Ntrigger",m_Ntrigger,0,500) );
  ATH_CHECK( m_nt->addItem("Nsamples",m_ntNsamples,0,32) );
  ATH_CHECK( m_nt->addItem("Nsteps",m_Nsteps,0,50) );
  ATH_CHECK( m_nt->addItem("StepIndex",m_StepIndex,0,100) );

  static const int maxSamples = m_Nsamples;
  ATH_CHECK( m_nt->addItem("Sum",maxSamples,m_Sum) );
  ATH_CHECK( m_nt->addItem("SumSq",maxSamples,m_SumSq) );
  ATH_CHECK( m_nt->addItem("Mean",maxSamples,m_Mean) );
  ATH_CHECK( m_nt->addItem("RMS",maxSamples,m_RMS) );

  ATH_CHECK( m_contKey.initialize( !m_contKey.key().empty() ));
  return StatusCode::SUCCESS;

}

StatusCode LArAverages2Ntuple::execute(const EventContext& ctx)
{
  ATH_MSG_DEBUG ( "in execute" );
  

  const LArAccumulatedCalibDigitContainer* accuDigitContainer = nullptr;
  SG::ReadHandle<LArAccumulatedCalibDigitContainer> Hdl{m_contKey, ctx};
  if (!Hdl.isValid()) {
    ATH_MSG_WARNING ( "Unable to retrieve LArAccumulatedCalibDigitContainer with key " << m_contKey << " from DetectorStore. " );
    return StatusCode::SUCCESS;
  } else {
    ATH_MSG_DEBUG ( "Got LArAccumulatedCalibDigitContainer with key " << m_contKey );
    accuDigitContainer = Hdl.cptr();
  }
  
  LArAccumulatedCalibDigitContainer::const_iterator it=accuDigitContainer->begin();
  LArAccumulatedCalibDigitContainer::const_iterator it_e=accuDigitContainer->end();
 
  if(it == it_e) {
     ATH_MSG_WARNING ( "LArAccumulatedCalibDigitContainer with key=" << m_contKey << " is empty " );
     return StatusCode::SUCCESS;
   }else{
     ATH_MSG_DEBUG ( "LArAccumulatedCalibDigitContainer with key=" << m_contKey << " has " <<accuDigitContainer->size() << " entries" );
   }
 
  for (;it!=it_e;++it) {   
    // Add protection - Modif from JF. Marchand
    if ( !(*it) ) continue;
 
    m_IEvent    = m_event;
    m_EventNum    = ctx.eventID().event_number();
    HWIdentifier chid=(*it)->channelID();
    m_isPulsed = (long)(*it)->isPulsed();
    if(m_keepPulsed && !(*it)->isPulsed()) continue;
    m_DAC = (*it)->DAC();
    m_Nsteps = (*it)->nSteps();
    m_Ntrigger = (*it)->nTriggers();
    m_delay = (*it)->delay();
    m_StepIndex=(*it)->stepIndex();
    unsigned int trueMaxSample = (*it)->nsamples();
    m_ntNsamples = trueMaxSample;
 
    if(trueMaxSample>m_Nsamples){
      if(!m_pass){
        ATH_MSG_WARNING ( "The number of samples in data is larger than the one specified by JO: " << trueMaxSample << " > " << m_Nsamples << " --> only " << m_Nsamples << " will be available in the ntuple " );
        m_pass=true;
      }
      trueMaxSample = m_Nsamples;
    }
  
    const std::vector<unsigned long>& sampleSum = (*it)->sampleSum();
    const std::vector<unsigned long>& sampleSum2 = (*it)->sample2Sum();
    const std::vector<float>& mean = (*it)->mean();
    const std::vector<float>& RMSv = (*it)->RMS();
 
    for(unsigned int j=0;j<trueMaxSample;j++){
      m_Sum[j]   = sampleSum[j];
      m_SumSq[j] = sampleSum2[j];
      if(m_Ntrigger){
         m_Mean[j]  = mean[j];
         m_RMS[j]   = RMSv[j];
      } else {
         m_Mean[j]=0;
         m_RMS[j]=0;
      }

    }
 
    int FT=m_onlineHelper->feedthrough(chid);
    if(m_keepFT.size() > 0) {
       if(std::find(std::begin(m_keepFT), std::end(m_keepFT), FT) == std::end(m_keepFT)) continue;
    }
 
    fillFromIdentifier(chid);       
    ATH_CHECK( ntupleSvc()->writeRecord(m_nt) );
  }//end loop over cells
  m_event++;
  return StatusCode::SUCCESS;
}// end execute method.
