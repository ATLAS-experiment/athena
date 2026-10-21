/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Implementation of LArRodDecoder class 
#include "LArByteStream/LArRodDecoder.h"
#include "GaudiKernel/TypeNameString.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IToolSvc.h"

#include "LArByteStream/LArRodBlockStructure.h"
#include "LArByteStream/LArRodBlockPhysicsV5.h"
#include "LArByteStream/LArRodBlockPhysicsV6.h"
#include <algorithm>
#include <fstream>

#include "LArRawConditions/LArCalibParams.h"
#include "LArRecConditions/LArCalibLineMapping.h"
#include "LArCabling/LArOnOffIdMapping.h"

static const InterfaceID IID_ILArRodDecoder
            ("LArRodDecoder", 1, 0);

LArRodDecoder::LArRodDecoder ( const std::string& type, const std::string& name,const IInterface* parent )
  : AthAlgTool(type,name,parent),
    m_LArCellEthreshold(-100.),
    m_readtdc(false),
    m_onlineHelper(0)
 {
  declareInterface< LArRodDecoder  >( this );
  declareProperty("IgnoreCheckFEBs",m_IgnoreCheckFEBs);
  declareProperty("CellCorrections",m_LArCellCorrNames ); 
  declareProperty("LArCellEthreshold",m_LArCellEthreshold ); 
  declareProperty("ReadTDC",m_readtdc);
  declareProperty("DelayScale",m_delayScale=(25./240.)*CLHEP::ns);
  declareProperty("FebExchange", m_febExchange=0); //FIXME: Very ugly hack! See explanation in .h file
  declareProperty("FebId1", m_febId1=0);
  declareProperty("FebId2", m_febId2=0);
  declareProperty("FirstSample", m_firstSample=0); //FIXME: Very ugly hack! See explanation in .h file

  declareProperty("BEPreselection",m_vBEPreselection,"For channel-selection: Barrel=0, Endcap=1");
  declareProperty("PosNegPreselection",m_vPosNegPreselection,"For channel-selection: C-Side:0, A-Side: 1");
  declareProperty("FTNumPreselection",m_vFTPreselection,"For channel-selection: Feedthrough numbers (e.g. 0 - 31 for barrel)");
  declareProperty("MultiDSPMode", m_MultiDSPMode=false);
  declareProperty("CheckSum", m_CheckSum=false);
  declareProperty("StatusMask", m_StatusMask=0x00000212);
  declareProperty("RequiredPhysicsNSamples", m_requiredPhysicsNSamples = 0);
}

// destructor 
LArRodDecoder::~LArRodDecoder()
{
}

const InterfaceID& LArRodDecoder::interfaceID( )
{ return IID_ILArRodDecoder;  }

StatusCode
LArRodDecoder::initialize ATLAS_NOT_THREAD_SAFE ()
{
  // Before anything, access message service
  m_StatusNMask = !m_StatusMask;

  StatusCode sc = detStore()->retrieve(m_onlineHelper, "LArOnlineID");
  if (sc.isFailure()) {
    ATH_MSG_ERROR ("Could not get LArOnlineID helper !");
    return sc;
  }

  ATH_CHECK( m_evt.initialize() );

  std::vector<std::string>::const_iterator it = m_LArCellCorrNames.begin();
  std::vector<std::string>::const_iterator it_end = m_LArCellCorrNames.end();
  for(; it!=it_end;++it)
  {
    IAlgTool* tool;
    CaloCellCorrection* corr; 
    Gaudi::Utils::TypeNameString li(*it);
    if((toolSvc()->retrieveTool(li.type(), li.name(), tool)).isFailure() )
    {
      ATH_MSG_ERROR(" Can't get AlgTool for CaloCellCorrection ");
      return StatusCode::FAILURE;
    }

    corr = dynamic_cast<CaloCellCorrection*> (tool); 
    if(!corr  ) 
    {
      ATH_MSG_ERROR(" Can't d-cast to CaloCellCorrection*  ");
      return StatusCode::FAILURE;
    }
    m_LArCellCorrTools.push_back(corr); 
  }

  //Build list of preselected Feedthroughs
  if (m_vBEPreselection.size() &&  m_vPosNegPreselection.size() && m_vFTPreselection.size()) {
    ATH_MSG_INFO("Building list of selected feedthroughs");
    for (const unsigned BE : m_vBEPreselection) {
      for (const unsigned PN : m_vPosNegPreselection) {
        for (const unsigned FT : m_vFTPreselection) {
          HWIdentifier finalFTId=m_onlineHelper->feedthrough_Id(BE,PN,FT);
          unsigned int finalFTId32 = finalFTId.get_identifier32().get_compact();
          ATH_MSG_INFO("Adding feedthrough Barrel/Endcap=" << BE << " pos/neg=" << PN << " FT=" << FT
                       << " (0x" << std::hex << finalFTId32 << std::dec << ")");
          m_vFinalPreselection.push_back(finalFTId32);
        }
      }
    }
    std::sort(m_vFinalPreselection.begin(),m_vFinalPreselection.end());
  }//end if something set
  else {
    if (m_vBEPreselection.size() ||  m_vPosNegPreselection.size() || m_vFTPreselection.size()) {
      ATH_MSG_ERROR("Feedthrough preselection: jobOption inconsistency! ");
      ATH_MSG_ERROR("Need to set all three jobOptions BEPreselection PNPreselecton and FTPreselection");
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("No feedthrough preselection by jobOptions.");
  }
  return sc;
}


//*******


void LArRodDecoder::fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
                                   const uint32_t* p, uint32_t n, LArRawChannelContainer& coll, CaloGain::CaloGain RequestedGain) const
{ 
  int32_t energy;
  int32_t time;
  int32_t quality;
  uint32_t gain;
  int fcNb;
  HWIdentifier cId;
  LArRodBlockStructure* BlStruct=prepareBlockStructure(robFrag, p, n, RequestedGain);
  if (!BlStruct) return;

  do {
    HWIdentifier fId( Identifier32(BlStruct->getFEBID()) );
    unsigned int fId32 = fId.get_identifier32().get_compact();
    if (!m_onlineHelper->isValidId(fId)) {
      ATH_MSG_WARNING("Invalid FEB identifer " << std::hex << fId32 << std::dec << ". Skipping");
      continue;
    }

    if(m_febExchange) {
      if     (fId32 == m_febId1) {
        fId = HWIdentifier(Identifier32(m_febId2));
        fId32 = fId.get_identifier32().get_compact();
      }
      else if(fId32 == m_febId2) {
        fId = HWIdentifier(Identifier32(m_febId1));
        fId32 = fId.get_identifier32().get_compact();
      }
    }
    if (!BlStruct->hasPhysicsBlock()) {
      ATH_MSG_DEBUG("No Physics Data for this FEB - NULL event for FEBID 0x" << std::hex << BlStruct->getFEBID() << std::dec);
      continue;
    }

    // RL 04.17.2008 skip check for some FEBs
    int do_check=1;
    std::vector<unsigned int>::const_iterator it_feb     = m_IgnoreCheckFEBs.begin();
    std::vector<unsigned int>::const_iterator it_feb_end = m_IgnoreCheckFEBs.end();
    for(; it_feb!=it_feb_end;++it_feb)
      if(fId==*it_feb) {
        do_check=0;
        break;
      }
    if(do_check) {
      //WL 31.10.2007 //check RodStatus-word to catch corrupt events
      if (BlStruct->getStatus() & m_StatusNMask) {
        ATH_MSG_WARNING("RodStatus&0x" << std::hex << m_StatusNMask << " indicates corrupt data for FEB  "<< std::hex << fId32 << std::dec <<".  Ignored.");
        continue;
      }
    }
      
    // RL 05.11.2007 checksum
    if(m_CheckSum) {
      const uint32_t onsum  = BlStruct->onlineCheckSum();
      const uint32_t offsum = BlStruct->offlineCheckSum();
      if(onsum!=offsum) {
        ATH_MSG_WARNING("Checksum error:");
        ATH_MSG_WARNING(" online checksum  = " << MSG::hex << onsum);
        ATH_MSG_WARNING(" offline checksum = " << MSG::hex << offsum);
        continue;
      }
    }

    if (m_vFTPreselection.size()) {
      int ftId=m_onlineHelper->feedthrough_Id(fId).get_identifier32().get_compact();
      if (!std::binary_search(m_vFTPreselection.begin(), m_vFTPreselection.end(),ftId)) {
        ATH_MSG_DEBUG("Feedthrough with id " << std::hex << ftId << std::dec <<" not in preselection. Ignored.");
        continue;
      }
    }
    const int NthisFebChannel=m_onlineHelper->channelInSlotMax(fId);
    while (BlStruct->getNextEnergy(fcNb,energy,time,quality,gain)) {
      if (fcNb>=NthisFebChannel)
        continue;
      cId = m_onlineHelper->channel_Id(fId,fcNb);
      uint16_t iquality = 0;
      uint16_t iprovenance = 0x1000;
      if (quality>0) {
        iprovenance |= 0x2000;
        iquality = (quality & 0xFFFF);
      }
      LArRawChannel chan(cId, energy, time, iquality, iprovenance, (CaloGain::CaloGain)gain);
      coll.push_back(chan);
    }
  }
  while (BlStruct->nextFEB()); //Get NextFeb
}


LArRodBlockStructure*
LArRodDecoder::prepareBlockStructure1 (const uint16_t rodMinorVersion, const uint32_t robBlockType) const
{ 
  const unsigned MAXMINOR = 12;
  const unsigned MAXTYPE = 10;

  if (rodMinorVersion > MAXMINOR || robBlockType > MAXTYPE) {
    ATH_MSG_ERROR("Bad Rod block type " <<  robBlockType
                  << " / " << rodMinorVersion);
    return nullptr;
  }
  std::vector<std::unique_ptr<LArRodBlockStructure> >& blstructs =
    *m_blstructs.get();
  unsigned int index = robBlockType * (MAXMINOR+1) + rodMinorVersion;
  if (blstructs.empty()) {
    blstructs.resize ((MAXMINOR+1)*(MAXTYPE+1));
  }
  if (!blstructs[index]) {
    blstructs[index] = makeBlockStructure (robBlockType, rodMinorVersion);
  }
  if (!blstructs[index]) {
    ATH_MSG_ERROR("Bad Rod block type " <<  robBlockType
                  << " / " << rodMinorVersion);
    return nullptr;
  }

#ifndef NDEBUG
  ATH_MSG_DEBUG("Found version " << rodMinorVersion << " of Rod block type " << robBlockType);
#endif

  return blstructs[index].get();
}


LArRodBlockStructure*
LArRodDecoder::prepareBlockStructure(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
                                     const uint32_t* p, uint32_t n, const CaloGain::CaloGain RequestedGain) const
{ 
#ifndef NDEBUG
  ATH_MSG_DEBUG("Prepare LArRodBlockStructure. Got a fragement of size " << n);
#endif

  //Get version and blocktype form header
  eformat::helper::Version ver(robFrag.rod_version());
  const uint16_t rodMinorVersion=ver.minor_version();
  const uint32_t rodBlockType=robFrag.rod_detev_type()&0xff;
  LArRodBlockStructure* BlStruct = prepareBlockStructure1 (rodMinorVersion, rodBlockType);
  if (!BlStruct) {
    return nullptr;
  }

  if (!BlStruct->setFragment(p,n)) {
    constexpr int maxMess = 100;
    static std::atomic<int> nMess = 1;
    int thismess = nMess++;
    if (thismess <= maxMess) {
      ATH_MSG_ERROR("Could not set fragment (wrong number of samples in data ?) - container will not be filled");
      if (thismess == maxMess)
        ATH_MSG_ERROR("This message will not be repeated");
    }
    return NULL;
  }
#ifndef NDEBUG
  ATH_MSG_VERBOSE("Set Fragment at address "<<  &(p[0]) << " " << p[5]);
#endif

  BlStruct->setGain(RequestedGain); //Will be ignored if BlockStructure does not support fixed gains.    
  //FIXME: Very ugly hack! See explanation in LArRodDecoder.h 
  if (m_firstSample) {
    BlStruct->setFirstSample(m_firstSample);
  }
  return BlStruct;
}


std::unique_ptr<LArRodBlockStructure>
LArRodDecoder::makeBlockStructure (unsigned int rodBlockType,
                                   unsigned int rodMinorVersion) const
{
  switch (rodBlockType) {
  case 2:
    break;

  case 3:
    // RodBlockType 3 = Test mode
    //return std::make_unique<LArRodBlockTransparentV0<LArRodBlockHeaderTransparentV0> >(this->msgSvc().get());
    break;

  case 4:
    // RodBlockType 4 = Physics mode
    switch (rodMinorVersion) {
    case 10: // Physics mode v5 16.06.2008 for LHC 
    case 11: // Physics mode v5 16.06.2008 for LHC
      {
        auto bl = std::make_unique<LArRodBlockPhysicsV5>(this->msgSvc().get());
        if (m_requiredPhysicsNSamples > 0) {
          bl->setRequiredNSamples(m_requiredPhysicsNSamples);
        }
        return bl;
      }
    case 12: // Physics mode v5 09.03.2011 for LHC
      {
        auto bl = std::make_unique<LArRodBlockPhysicsV6>(this->msgSvc().get());
        if (m_requiredPhysicsNSamples > 0) {
          bl->setRequiredNSamples(m_requiredPhysicsNSamples);
        }
        return bl;
      }
    default:
      break;
    }
    break;

  case 5:
    // RodBlockType 5 = Physics simulation mode
    // Physics mode v3 11.04.2005 for simulation
    //return std::make_unique<LArRodBlockPhysicsV3>(this->msgSvc().get());
    break;

  case 6:
    // RodBlockType 6 = Physics test mode
    break;

  case 7:
    // RodBlockType 7 = Calibration mode
    break;

  case 10:
    // RodBlockType 10 = Accumulated mode (used for pre-processed pedestal runs)
    // Accumulated mode v3 10.06.2008
    //return std::make_unique<LArRodBlockAccumulatedV3>(this->msgSvc().get());
    break;

  default:
    break;
  }

  return std::unique_ptr<LArRodBlockStructure>();
}
