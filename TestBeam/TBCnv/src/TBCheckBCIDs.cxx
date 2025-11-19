//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "TBCnv/TBCheckBCIDs.h"

#include "GaudiKernel/MsgStream.h"
#include "ByteStreamData/RawEvent.h"
#include "ByteStreamCnvSvcBase/FullEventAssembler.h" 
//#include "CLIDSvc/CLASS_DEF.h"
#include <map> 


TBCheckBCIDs::TBCheckBCIDs(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator),
    m_count(0),
    m_rdpSvc("ROBDataProviderSvc", name),
    m_ntuplePtr(0)
{
}

TBCheckBCIDs::~TBCheckBCIDs() {}

StatusCode TBCheckBCIDs::initialize()
{
  ATH_MSG_DEBUG( " initialize "  );

  //Get ByteStreamInputSvc (only necessary for reading of digits, not for writing and for channels)
  ATH_CHECK( m_rdpSvc.retrieve() );

  //Book Ntuple
  NTupleFilePtr file1(ntupleSvc(),"/NTUPLES/FILE1");
  if (!file1) {
    ATH_MSG_ERROR( "Booking of NTuple failed"  );
    return StatusCode::FAILURE;
   }
  NTuplePtr nt(ntupleSvc(),"/NTUPLES/FILE1/BCIDS");
  if (!nt) {
    nt=ntupleSvc()->book("/NTUPLES/FILE1/BCIDS",CLID_ColumnWiseTuple,"BCIDs");
  }
  if (!nt) {
    ATH_MSG_ERROR( "Booking of NTuple failed"  );
    return StatusCode::FAILURE;
  }

  ATH_CHECK( nt->addItem("GlobalEvtID",m_EventID,0,50000) );
  ATH_CHECK( nt->addItem("LVL1ID",m_LVL1ID,0,50000) );
  ATH_CHECK( nt->addItem("RODIndex",m_RODIndex,0,100) );
  ATH_CHECK( nt->addItem("RODID",m_RODIndex,m_RODID) );
  ATH_CHECK( nt->addItem("BCID",m_RODIndex,m_BCID) );
  m_ntuplePtr=nt;
  return StatusCode::SUCCESS;
}


//LArRawDataContByteStreamTool::ReadLArDigits(const RawEvent* re,LArDigitContainer* digitCont, const int gain, MsgStream& log)
StatusCode TBCheckBCIDs::execute()
{ 
  ATH_MSG_DEBUG( "Executing event #"<< m_count++  );

  unsigned RODCounter=0;
  const RawEvent* re = m_rdpSvc->getEvent(Gaudi::Hive::currentContext());
  if (!re) {
    ATH_MSG_FATAL( "RawEvent retrieved by TBCheckBCIDs is a null pointer!"  );
    return StatusCode::FAILURE;
  }
  
  //Get ROB fragements from the raw event. 
  const size_t MAX_ROBFRAGMENTS = 2048*1024;
  std::vector<OFFLINE_FRAGMENTS_NAMESPACE::PointerType> robF(MAX_ROBFRAGMENTS);
  OFFLINE_FRAGMENTS_NAMESPACE::PointerType rePointer;
  re->start(rePointer);
  size_t robcount = re->children(robF.data(),MAX_ROBFRAGMENTS);
  if (robcount == MAX_ROBFRAGMENTS)
    {
      ATH_MSG_FATAL( "ROB buffer overflow"  );
      return StatusCode::FAILURE;
    }
  

  m_LVL1ID=re->lvl1_id();
  m_EventID=re->global_id();
  

  
  for (size_t irob=0; irob<robcount; ++irob) //Loop over all ROB-Fragments
    {
      OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment rob(robF[irob]);
      
      try
	{
	  rob.check();
	}
      catch (...)
	{
	  ATH_MSG_ERROR( "Got invalid ROB fragment!"  );
	  return StatusCode::FAILURE;
	}
      
      ATH_MSG_VERBOSE( MSG::hex << "\t\tFull source ID: " << rob.source_id()<<MSG::dec );
      ATH_MSG_VERBOSE( "This ROD-Fragment consists of " << rob.rod_fragment_size_word() << " words"  );
      
      m_RODIndex=RODCounter;
      m_RODID[m_RODIndex]=rob.rod_source_id();
      m_BCID[m_RODIndex] =rob.rod_bc_id();
      ATH_MSG_DEBUG( "RODID 0x" << MSG::hex << rob.rod_source_id() << " BCID 0x" 
                     << rob.rod_bc_id() << MSG::dec  );
      RODCounter++;
    }
  
  ATH_CHECK( ntupleSvc()->writeRecord(m_ntuplePtr) );
  return StatusCode::SUCCESS;
}

StatusCode TBCheckBCIDs::finalize() 
{return StatusCode::SUCCESS;}
