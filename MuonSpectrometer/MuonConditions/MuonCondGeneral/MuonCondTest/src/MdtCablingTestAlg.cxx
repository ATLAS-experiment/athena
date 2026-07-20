/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCablingTestAlg.h"
#include "StoreGate/ReadCondHandle.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonCablingData/MdtMezzanineCard.h"

#include <fstream>

MdtCablingTestAlg::MdtCablingTestAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthAlgorithm(name,pSvcLocator) {}


StatusCode MdtCablingTestAlg::initialize(){
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_DetectorManagerKey.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_deadChanKey.initialize(!m_deadChanKey.empty()));
  return StatusCode::SUCCESS;
} 

StatusCode MdtCablingTestAlg::execute(const EventContext& ctx){
  std::unique_ptr<std::fstream> f_dump = !m_dumpFile.value().empty() ? 
                                         std::make_unique<std::fstream>(m_dumpFile, std::fstream::out) : nullptr;
  ATH_MSG_INFO("Start validation of the Mdt cabling. Dump complete mapping into "<<m_dumpFile);
  {
    using Mapping = MdtMezzanineCard::Mapping;
    using OffChnl = MdtMezzanineCard::OfflineCh;
    Mapping staggering{};
    for (unsigned i = 0 ; i < staggering.size(); ++i){
        staggering[i] = (staggering.size() -1) -i;
    }
    for (unsigned i = 0 ; i < staggering.size()  ; i+=2){
        std::swap(staggering[i], staggering[i+1]);
    }
    for (unsigned nLay : {3,4}){
      MdtMezzanineCard testCard(staggering, nLay , 0);
      if (!testCard.checkConsistency(msgStream())) return StatusCode::FAILURE;
      ATH_MSG_INFO("Test mezzanine "<<testCard);
      /// Test the online -> offline conversion
      for (unsigned ch = 0 ; ch < staggering.size() ; ++ch) {
          OffChnl off = testCard.offlineTube(ch, msgStream());
          if (!off.isValid) {
            ATH_MSG_FATAL("Failed to load tube & layer for channel "<<ch);
            return StatusCode::FAILURE;
          }
          unsigned back_ch = testCard.tdcChannel(off.layer, off.tube +1 , msgStream());
          if (ch != back_ch) {
            ATH_MSG_FATAL("Forward conversion of "<<ch<<" lead to "
                            <<static_cast<int>(off.layer)<<","
                            <<static_cast<int>(off.tube +1)<<" and then back to "
                            <<back_ch);
            return StatusCode::FAILURE;
          }
      }
      /// Test the shift by n channels
      for (unsigned lay = 1 ; lay <= nLay ; ++lay) {
        for (unsigned int tube = 1 ; tube <= testCard.numTubesPerLayer() ; ++tube) {
            const unsigned ch1 = testCard.tdcChannel(lay,tube,msgStream());
            const unsigned ch2 = testCard.tdcChannel(lay,tube  + testCard.numTubesPerLayer(),msgStream());
            
            if(ch1 != ch2) {
                ATH_MSG_FATAL("The tube "<<tube<<" in layer "<<lay
                              <<" leads to 2 different answers "<<ch1<<" vs. "<<ch2);
                return StatusCode::FAILURE;
            }
        }
      }
    }
  }
  
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detectorMgr{m_DetectorManagerKey, ctx};
  if (!detectorMgr.isValid()){
      ATH_MSG_FATAL("Failed to retrieve the Detector manager "<<m_DetectorManagerKey.fullKey());
      return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<MuonMDT_CablingMap> cabling{m_cablingKey,ctx};
  if (!cabling.isValid()) {
     ATH_MSG_ERROR("Failed to retrieve the Mdt cabling "<<m_cablingKey.fullKey());
     return StatusCode::FAILURE;
  }

  const MdtCondDbData* deadChan{nullptr};
  if (!m_deadChanKey.empty()) {
     SG::ReadCondHandle<MdtCondDbData> deadChanHandle{m_deadChanKey,ctx};
     if (!deadChanHandle.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve Mdt conditions "<<m_deadChanKey.fullKey());
        return StatusCode::FAILURE;
     }
     deadChan = deadChanHandle.cptr();

  }
  
   const MdtIdHelper& idHelper = m_idHelperSvc->mdtIdHelper();
   unsigned int n_elements{0}, n_success{0};
   bool failure{false};
  
  for (auto det_itr = idHelper.detectorElement_begin(); det_itr != idHelper.detectorElement_end(); ++det_itr){
    const MuonGM::MdtReadoutElement* readEle = detectorMgr->getMdtReadoutElement(*det_itr);

    if (!readEle) {
        ATH_MSG_DEBUG("Detector element does not exist. ");
        continue;
    }
    if (deadChan && !deadChan->isGoodChamber(readEle->identify())) {
      ATH_MSG_ALWAYS("Dead station found "<<m_idHelperSvc->toStringChamber(readEle->identify()));
      continue;
    }
    ATH_MSG_DEBUG("Check station "<<m_idHelperSvc->toStringChamber(readEle->identify()));
    for (int layer = 1 ; layer <= readEle->getNLayers(); ++layer){
      for (int tube = 1 ; tube <= readEle->getNtubesperlayer(); ++tube){
          bool is_valid{false};
          const Identifier tube_id = idHelper.channelID(readEle->identify(), readEle->getMultilayer(), layer, tube, is_valid);
          if (!is_valid){
            ATH_MSG_VERBOSE("Invalid element");
            continue;
          }
          if (deadChan && !deadChan->isGood(tube_id)) {
            ATH_MSG_ALWAYS("Dead dube detected "<<m_idHelperSvc->toString(tube_id));
            continue;
          }
          ++n_elements;
          /// Create the cabling object
          MdtCablingData cabling_data{};
          cabling->convert(tube_id,cabling_data);
          /// Test if the online channel can be found
          if (!cabling->getOnlineId(cabling_data,msgStream())){
             ATH_MSG_ERROR("Could no retrieve a valid online channel for "<<m_idHelperSvc->toString(tube_id)<<" from station ID "<<m_idHelperSvc->toString(readEle->identify()));
             failure = true;
             continue;
          }
          if (f_dump) (*f_dump)<<cabling_data<<std::endl;
          /// Test if the online channel can be transformed back
          
          /// Reset the offline cabling          
          const MdtCablingOffData off_data{};
          static_cast<MdtCablingOffData&> (cabling_data) = off_data;
          if (!cabling->getOfflineId(cabling_data, msgStream())){
            ATH_MSG_ERROR("Could not convert the online cabling "<<cabling_data<<" to an offline identifier. Initial identifier "<<m_idHelperSvc->toString(tube_id));
            failure = true;
            continue;
          }
          
          Identifier test_id{};
          if (!cabling->convert(cabling_data, test_id, true)){
            ATH_MSG_ERROR("The extracted offline identifier "<<cabling_data<<" does not make sense");
            failure = true;
            continue;
          }
          if (test_id != tube_id){
            ATH_MSG_ERROR("The forward -> backward conversion failed. Started with "<<m_idHelperSvc->toString(tube_id)<<" ended with "<<m_idHelperSvc->toString(test_id));
            failure = true;
            continue;
          }
          
          /// Reset the offline cabling again
          static_cast<MdtCablingOffData&>(cabling_data) = off_data;
          
          /// Test whether the online module can be decoded successfully
          cabling_data.tdcId = 0xff;
          cabling_data.channelId =0xff;
          if (!cabling->getOfflineId(cabling_data, msgStream()) || !cabling->convert(cabling_data, test_id, true)) {
              ATH_MSG_ERROR("Failed to decode Mdt module "<<m_idHelperSvc->toString(tube_id));
              failure = true;
              continue;
          }
          MdtCablingData mrod_module{cabling_data};
          if (!cabling->getOnlineId(mrod_module,msgStream())){
             ATH_MSG_ERROR("Could no retrieve a valid online channel for "<<m_idHelperSvc->toString(tube_id));
             failure = true;
             continue;
          }
          /// There might be channels sitting on the same CSM but on differnt stations
          if (idHelper.elementID(test_id) != idHelper.elementID(readEle->identify()) && static_cast<const MdtCablingOnData&>(mrod_module)!=(cabling_data)) {
              ATH_MSG_ERROR("Failed to decode module "<<m_idHelperSvc->toString(tube_id)<<" got "<<m_idHelperSvc->toString(test_id) );
              failure = true;
          } else ++n_success;
       }
    }  
  }
  ATH_MSG_INFO( n_success<<" out of "<<n_elements<<" channels were successfully validated.");
  return failure ? StatusCode::FAILURE: StatusCode::SUCCESS;
} 
   
