/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "EventInfoByteStreamAuxCnv.h"
#include "MCEventInfoByteStreamTool.h"
#include "ByteStreamCnvSvcBase/ByteStreamCnvSvcBase.h"
#include "ByteStreamCnvSvcBase/IByteStreamEventAccess.h"
#include "ByteStreamCnvSvcBase/ByteStreamAddress.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"

#include "ByteStreamData/RawEvent.h"
#include "ByteStreamData/ByteStreamMetadata.h"
#include "ByteStreamData/ByteStreamMetadataContainer.h"

#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IRegistry.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventAuxInfo.h"

#include "StoreGate/StoreGateSvc.h"

#include "eformat/StreamTag.h"

#include <time.h>

EventInfoByteStreamAuxCnv::EventInfoByteStreamAuxCnv(ISvcLocator* svcloc)
  : Converter(storageType(), classID(), svcloc)
  , AthMessaging(svcloc != nullptr ? msgSvc() : nullptr, "EventInfoByteStreamAuxCnv")
  , m_robDataProvider("ROBDataProviderSvc", "EventInfoByteStreamAuxCnv")
  , m_mdSvc("InputMetaDataStore", "EventInfoByteStreamAuxCnv")
  , m_isSimulation(false)
  , m_isTestbeam(false)
  , m_isCalibration(false)
{
}

CLID EventInfoByteStreamAuxCnv::classID() 
{
  return ClassID_traits<xAOD::EventAuxInfo>::ID();
}

long EventInfoByteStreamAuxCnv::storageType() 
{
  return ByteStreamAddress::storageType();
}

StatusCode EventInfoByteStreamAuxCnv::initialize() 
{
  ATH_MSG_DEBUG("Initialize");

  CHECK(Converter::initialize());

  CHECK(m_robDataProvider.retrieve());
  CHECK(m_mdSvc.retrieve());

  SmartIF<IProperty> byteStreamCnvSvc(service("ByteStreamCnvSvc"));
  CHECK( byteStreamCnvSvc.isValid() );

  SimpleProperty<bool> propIsSimulation("IsSimulation", m_isSimulation);
  StatusCode sc = byteStreamCnvSvc->getProperty(&propIsSimulation);
  if (sc.isSuccess()) {
    m_isSimulation = propIsSimulation.value();
    ATH_MSG_INFO("IsSimulation : " << m_isSimulation);
  } 
  else {
    ATH_MSG_ERROR("Cannot get IsSimulation");
    return sc;
  }

  SimpleProperty<bool> propIsTestbeam("IsTestbeam", m_isTestbeam);
  sc = byteStreamCnvSvc->getProperty(&propIsTestbeam);
  if (sc.isSuccess()) {
    m_isTestbeam = propIsTestbeam.value();
    ATH_MSG_INFO("IsTestbeam : " << m_isTestbeam);
  } 
  else {
    ATH_MSG_ERROR("Cannot get IsTestbeam");
    return sc;
  }

  SimpleProperty<bool> propIsCalibration("IsCalibration", m_isCalibration);
  sc = byteStreamCnvSvc->getProperty(&propIsCalibration);
  if (sc.isSuccess()) {
    m_isCalibration = propIsCalibration.value();
    ATH_MSG_INFO("IsCalibration : " << m_isCalibration);
  }
  else {
    ATH_MSG_ERROR("Cannot get IsCalibration");
    return sc;
  }

  // Retrieve MC EventInfo decoder tool (optional - only needed for MC BS files)
  if (m_isSimulation) {
    if (m_mcEventInfoTool.retrieve().isFailure()) {
      ATH_MSG_WARNING("Failed to retrieve MCEventInfoByteStreamTool - MC EventInfo will use default values");
      m_mcEventInfoTool.disable();
    } else {
      ATH_MSG_DEBUG("MCEventInfoByteStreamTool retrieved successfully");
    }
  } else {
    m_mcEventInfoTool.disable();
  }

  return StatusCode::SUCCESS;
}

StatusCode EventInfoByteStreamAuxCnv::finalize() 
{
  ATH_MSG_DEBUG("Finalize");
  
  StatusCode sc = Converter::finalize();
  if (sc.isFailure()) {
    ATH_MSG_WARNING("Converter::finalize() failed");
  }
  return sc;
}

StatusCode EventInfoByteStreamAuxCnv::createObj(IOpaqueAddress* pAddr, DataObject*& pObj) 
{
  ByteStreamAddress *pRE_Addr{nullptr};
  pRE_Addr = dynamic_cast<ByteStreamAddress*>(pAddr);
  if (!pRE_Addr) {
    ATH_MSG_ERROR("Cannot cast to ByteStreamAddress ");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Creating Objects");

  // get RawEvent
  const RawEvent* re = m_robDataProvider->getEvent(Gaudi::Hive::currentContext());
  if (!re) {
    ATH_MSG_ERROR("Can not get RawEvent ");
    return StatusCode::FAILURE;
  }

  // Run Number
  int runNumber = re->run_no();

  // Event Number
  uint64_t eventNumber;
  if (re->version() < 0x03010000) {
    eventNumber=re->lvl1_id();
  } else {
    eventNumber=re->global_id();
  }
  
  // Time Stamp
  uint32_t bc_time_sec = re->bc_time_seconds();
  uint32_t bc_time_ns  = re->bc_time_nanoseconds();
  // bc_time_ns should be lt 1e9.
  if (bc_time_ns > 1000000000) {
    if (runNumber < 20920) {
      //  In M4 we saw otherwise, because sec/ns was swapped in raw data.
      uint32_t temp = bc_time_ns;
      bc_time_ns = bc_time_sec;
      bc_time_sec= temp;
      ATH_MSG_DEBUG("bc_time  second/nanosecond swapped, sec/ns = " << bc_time_sec << " " << bc_time_ns);
    } 
    else { 
      // For later runs, the nanosecond clock sometimes is not reset, making it overrun 1e9. Round it off to 1e9
      ATH_MSG_WARNING("bc_time nanosecond number larger than 1e9, it is " << bc_time_ns << ", reset it to 1 sec");
      bc_time_ns = 1000000000;
    }
  }

  // luminosity block number
  uint16_t lumiBlock = re->lumi_block();

  // bunch crossing identifier
  uint16_t bcID = re->bc_id();

  unsigned int detMask0 = 0xFFFFFFFF, detMask1 = 0xFFFFFFFF, detMask2 = 0xFFFFFFFF, detMask3 = 0xFFFFFFFF;
  // Get ByteStream Metadata from Input MetaData Store
  const ByteStreamMetadataContainer* metadatacont{nullptr};
  StatusCode status = m_mdSvc->retrieve(metadatacont, "ByteStreamMetadata");
  if (!status.isSuccess()) {
    ATH_MSG_WARNING("Unable to retrieve Input MetaData for ByteStream");
  } 
  else {
    const ByteStreamMetadata* metadata = *(metadatacont->begin());
    uint64_t detectorMask = metadata->getDetectorMask();
    detMask0 = (unsigned int)(detectorMask & 0x00000000FFFFFFFF);
    detMask1 = (unsigned int)(detectorMask >> 32);
    uint64_t detectorMask2 = metadata->getDetectorMask2();
    detMask2 = (unsigned int)(detectorMask2 & 0x00000000FFFFFFFF);
    detMask3 = (unsigned int)(detectorMask2 >> 32);
  }


  xAOD::EventInfo evtInfo;
  xAOD::EventAuxInfo* pEvtInfoAux = new xAOD::EventAuxInfo();
  evtInfo.setStore(pEvtInfoAux);

  evtInfo.setRunNumber(runNumber);
  evtInfo.setEventNumber(eventNumber);
  evtInfo.setLumiBlock(lumiBlock);
  evtInfo.setTimeStamp(bc_time_sec);
  evtInfo.setTimeStampNSOffset(bc_time_ns);
  evtInfo.setBCID(bcID);
  evtInfo.setDetectorMask(detMask0,detMask1);
  evtInfo.setDetectorMaskExt(detMask2,detMask3);

  // Set default MC values (will be overwritten if MC ROB is present)
  evtInfo.setMCChannelNumber(0);
  evtInfo.setMCEventNumber(0);
  evtInfo.setMCEventWeights(std::vector<float>(1,1));

  // Set Event Type
  uint32_t eventTypeBitmask{0};
  if (m_isSimulation) {
    eventTypeBitmask |= xAOD::EventInfo::IS_SIMULATION;
  }
  if (m_isTestbeam) {
    eventTypeBitmask |= xAOD::EventInfo::IS_TESTBEAM;
  }
  if (m_isCalibration) {
    eventTypeBitmask |= xAOD::EventInfo::IS_CALIBRATION;
  }
  evtInfo.setEventTypeBitmask(eventTypeBitmask);

  // Try to decode MC EventInfo from dedicated ROB fragment (for MC ByteStream files)
  bool mcEventInfoDecoded = false;
  if (m_isSimulation && m_mcEventInfoTool.isEnabled()) {
    // Request the MC EventInfo ROB
    std::vector<uint32_t> mcRobIds = m_mcEventInfoTool->robIds();
    std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> mcRobFragments;
    m_robDataProvider->getROBData(Gaudi::Hive::currentContext(), mcRobIds, mcRobFragments, "EventInfoByteStreamAuxCnv");

    if (!mcRobFragments.empty()) {
      ATH_MSG_DEBUG("Found MC EventInfo ROB fragment, decoding...");
      StatusCode sc = m_mcEventInfoTool->convertFromBS(mcRobFragments[0], evtInfo);
      if (sc.isFailure()) {
        ATH_MSG_WARNING("Failed to decode MC EventInfo from ROB fragment, using default values");
      } else {
        ATH_MSG_DEBUG("Successfully decoded MC EventInfo from ROB fragment");
        mcEventInfoDecoded = true;
      }
    } else {
      ATH_MSG_DEBUG("No MC EventInfo ROB fragment found, using default values");
    }
  }

  // Trigger Info
  const OFFLINE_FRAGMENTS_NAMESPACE::DataType* buffer;
  // status element
  re->status(buffer);
  uint32_t statusElement = *buffer;

  // extended LVL1ID
  uint32_t extendedLevel1ID = re->lvl1_id();

  // LVL1 trigger type
  uint32_t level1TriggerType = re->lvl1_trigger_type();

  // stream tag
  std::vector<xAOD::EventInfo::StreamTag> streamTags;
  std::vector<eformat::helper::StreamTag> onl_streamTags;
  re->stream_tag(buffer);
  eformat::helper::decode(re->nstream_tag(), buffer, onl_streamTags);
  for (const eformat::helper::StreamTag& onl_streamTag : onl_streamTags) {
    std::set<uint32_t> tmp_off_dets = std::set<uint32_t>();
    if (!onl_streamTag.dets.empty()) {
      std::set<eformat::SubDetector> tmp_onl_dets = onl_streamTag.dets;
      for (const eformat::SubDetector& subdet : tmp_onl_dets) {
        tmp_off_dets.insert((uint32_t) subdet);
      }
    }
    streamTags.push_back(xAOD::EventInfo::StreamTag(onl_streamTag.name
						    , onl_streamTag.type
						    , onl_streamTag.obeys_lumiblock
						    , onl_streamTag.robs
						    , tmp_off_dets)
			 );
  }

  evtInfo.setStatusElement(statusElement);
  // Only set extendedLevel1ID from RawEvent header if MC EventInfo wasn't decoded
  // (MC EventInfo ROB contains the original extendedLevel1ID value)
  if (!mcEventInfoDecoded) {
    evtInfo.setExtendedLevel1ID(extendedLevel1ID);
  }
  evtInfo.setLevel1TriggerType(level1TriggerType);
  evtInfo.setStreamTags(streamTags);

  // record EventInfo
  evtInfo.setEventFlags(xAOD::EventInfo::Core, m_robDataProvider->getEventStatus(Gaudi::Hive::currentContext()));
  pObj = SG::asStorable(pEvtInfoAux);

  ATH_MSG_DEBUG(" New xAOD::EventAuxInfo made, run/event= " << runNumber
		<< " " << eventNumber
		<< " Time stamp  = " << ascTime(bc_time_sec) 
		);

  return StatusCode::SUCCESS;
}

StatusCode EventInfoByteStreamAuxCnv::createRep(DataObject* /*pObj*/, IOpaqueAddress*& /*pAddr*/)
{
  ATH_MSG_DEBUG("createRep for xAOD::EventAuxInfo");

  // For MC ByteStream files, create the MC EventInfo ROB fragment
  if (m_isSimulation && m_mcEventInfoTool.isEnabled()) {
    ATH_MSG_DEBUG("Creating MC EventInfo ROB fragment for simulation");

    // Get RawEventWrite via ByteStreamCnvSvc
    SmartIF<IByteStreamEventAccess> byteStreamCnvSvc(service("ByteStreamCnvSvc"));
    if (!byteStreamCnvSvc.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve ByteStreamCnvSvc");
      return StatusCode::FAILURE;
    }
    RawEventWrite* re = byteStreamCnvSvc->getRawEvent();
    if (!re) {
      ATH_MSG_ERROR("Failed to get RawEventWrite");
      return StatusCode::FAILURE;
    }

    // Create MC EventInfo ROB fragment
    std::vector<OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment*> mcRobs;
    ATH_CHECK(m_mcEventInfoTool->convertToBS(mcRobs, Gaudi::Hive::currentContext()));

    // Append ROB fragments to the event
    for (auto* rob : mcRobs) {
      // Set LVL1 ID and trigger type from the full event
      rob->rod_lvl1_id(re->lvl1_id());
      rob->rod_lvl1_type(re->lvl1_trigger_type());
      re->append(rob);
      ATH_MSG_DEBUG("Added MC EventInfo ROB fragment to output event");
    }
  }

  return StatusCode::SUCCESS;
}

std::string EventInfoByteStreamAuxCnv::ascTime(unsigned int tstamp) 
{
   struct tm t;
   t.tm_sec   = tstamp;
   t.tm_min   = 0;
   t.tm_hour  = 0;
   t.tm_mday  = 0;
   t.tm_mon   = 0;
   t.tm_year  = 70;
   t.tm_wday  = 00;
   t.tm_yday  = 00;
   t.tm_isdst = 0;
   time_t ct = mktime(&t);
   struct tm t2;
   gmtime_r(&ct, &t2);
   char buf[32];
   asctime_r (&t2, buf);
   return std::string (buf);
}
