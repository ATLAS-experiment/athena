/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArShape2Ntuple.h"
#include "LArRawConditions/LArShapeComplete.h"
#include "LArRawConditions/LArShape32MC.h"
#include "LArCOOLConditions/LArShapeFlat.h"
#include "LArCOOLConditions/LArShapeSC.h"
#include "CaloIdentifier/CaloGain.h"

LArShape2Ntuple::LArShape2Ntuple(const std::string& name, ISvcLocator* pSvcLocator): 
  LArCond2NtupleBase(name, pSvcLocator)
{
  declareProperty("NtupleName",   m_ntName   = "SHAPE");
  declareProperty("NtupleFile",   m_ntFile   = "FILE1");
  declareProperty("isComplete",   m_isComplete=false);
  
}

LArShape2Ntuple::~LArShape2Ntuple() 
= default;


StatusCode LArShape2Ntuple::initialize() {
  ATH_CHECK(m_contKey.initialize());
  m_ntTitle="Pulse Shape";
  m_ntpath=std::string("/NTUPLES/")+m_ntFile+std::string("/")+m_ntName;
  return LArCond2NtupleBase::initialize();
}


StatusCode LArShape2Ntuple::stop() {
  const EventContext& ctx = Gaudi::Hive::currentContext();
  // Ntuple booking: Specific
  NTuple::Item<long> gain, phase, nSamples;
  NTuple::Item<float> timeOffset, phasetime;
  NTuple::Array<float> Shape, ShapeDer;

  
  ATH_CHECK(m_nt->addItem("Gain",gain,-1,2));

  //Specific:
  if (m_isComplete) {
    ATH_CHECK(m_nt->addItem("TimeOffset",timeOffset,0,100));
    ATH_CHECK(m_nt->addItem("Phase",phase,0,49));
    ATH_CHECK(m_nt->addItem("PhaseTime",phasetime,0,800));
  }
  ATH_CHECK(m_nt->addItem("nSamples",nSamples,0,100));
  ATH_CHECK(m_nt->addItem("Shape",nSamples,Shape));
  ATH_CHECK(m_nt->addItem("ShapeDer",nSamples,ShapeDer));
  
  const ILArShape* larShape = nullptr ;
  const LArShapeComplete* larShapeComplete = nullptr ;

  if (m_isComplete) {
    ATH_CHECK(detStore()->retrieve(larShapeComplete,m_contKey.key()));
    larShape=larShapeComplete; //Cast to base-class
  }
  else { //Use just the abstract interface (works also for LArShapeFlat and LArShapeMC)
    // For compatibility with existing configurations, look in the detector
    // store first, then in conditions.
    larShape=detStore()->tryConstRetrieve<ILArShape> (m_contKey.key());
    if (!larShape) {
      SG::ReadCondHandle<ILArShape> shapeHandle{m_contKey};
      larShape=*shapeHandle;
    }
  }

  const LArOnOffIdMapping *cabling=nullptr;
  if(m_isSC) {
    ATH_MSG_DEBUG( "LArOFC2Ntuple: using SC cabling" );
    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingSCKey, ctx};
    cabling=*cablingHdl;
  }else{
    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey, ctx};
    cabling=*cablingHdl;
  }


  if(!cabling) {
     ATH_MSG_WARNING( "Do not have cabling object LArOnOffIdMapping" );
     return StatusCode::FAILURE;
  }


  unsigned cellCounter=0;  
  for ( int igain=CaloGain::LARHIGHGAIN; 
	igain<m_NGains.value() ; ++igain )
  {
    for (HWIdentifier chid : m_onlineId->channel_range()) {
      if (!cabling->isOnlineConnected(chid)) continue;
      unsigned nPhase=1;
      if (larShapeComplete) nPhase=larShapeComplete->nTimeBins(chid,gain);
      for (unsigned iphase=0;iphase<nPhase;iphase++) {
	ATH_MSG_VERBOSE("Dumping Shape for channel " << m_onlineId->channel_name(chid) << ", gain " << gain << ", phase " << iphase);
	ILArShape::ShapeRef_t shape=larShape->Shape(chid,igain);
        ILArShape::ShapeRef_t shapeder =larShape->ShapeDer(chid,igain);
	fillFromIdentifier(chid);
	gain  = (long)igain ;
	nSamples=shape.size();
	for (int k=0;k<nSamples;k++ ) {
	  Shape[k] = shape[k] ;
	  ShapeDer[k] = shapeder[k] ;
	}
	if (larShapeComplete) {
	  timeOffset = larShapeComplete->timeOffset(chid,igain);
	  phasetime  = phase*larShapeComplete->timeBinWidth(chid,igain);
	  phase = (long)iphase ;
	}

	ATH_CHECK(ntupleSvc()->writeRecord(m_nt));
	cellCounter++;
      }//loop over phases
    }//loop over channels
  }//loop over gains
     
  ATH_MSG_INFO( "Total number of cells = " << cellCounter );
  ATH_MSG_INFO( "LArShape2Ntuple has finished." );
  return StatusCode::SUCCESS;
} // end finalize-method.
