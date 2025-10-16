/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkInDet/EventInfoPixelModuleStatusMonitoring.h"

#include <xAODEventInfo/EventInfo.h>
#include "DerivationFrameworkInDet/DecoratorUtils.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"

namespace DerivationFramework {

  EventInfoPixelModuleStatusMonitoring::EventInfoPixelModuleStatusMonitoring(const std::string& type, const std::string& name, const IInterface* parent):
    base_class(type,name,parent),
    m_pixelID(nullptr) {
  }

  StatusCode EventInfoPixelModuleStatusMonitoring::initialize() {

    if (m_prefix.empty()) {
      ATH_MSG_WARNING("No decoration prefix name provided for the output of EventInfoPixelModuleStatusMonitoring!");
    }

    ATH_CHECK(m_eventInfoKey.initialize());
    ATH_CHECK(detStore()->retrieve(m_pixelID,"PixelID"));

    ATH_CHECK(m_readKeyTemp.initialize());
    ATH_CHECK(m_readKeyHV.initialize());
    ATH_CHECK(m_condDCSStateKey.initialize());
    ATH_CHECK(m_condDCSStatusKey.initialize());
    ATH_CHECK(m_condDeadMapKey.initialize());
    ATH_CHECK(m_pixelSummary.retrieve());

    m_readoutTechnologyMask =   Pixel::makeReadoutTechnologyBit( InDetDD::PixelReadoutTechnology::FEI4, m_useByteStreamFEI4)
                              | Pixel::makeReadoutTechnologyBit( InDetDD::PixelReadoutTechnology::FEI3, m_useByteStreamFEI3)
                              | Pixel::makeReadoutTechnologyBit( InDetDD::PixelReadoutTechnology::RD53, m_useByteStreamRD53);
    ATH_CHECK(m_idcErrContKey.initialize(m_readoutTechnologyMask));


    {
      std::vector<std::string> moduleConditionList;
      moduleConditionList.emplace_back("PixelBiasVoltagePerLB");
      moduleConditionList.emplace_back("PixelTemperaturePerLB");
      createDecoratorKeys(*this,m_eventInfoKey,m_prefix.value(),moduleConditionList,m_moduleConditionKeys);
    }

    {
      std::vector<std::string> moduleFEmaskList;
      moduleFEmaskList.emplace_back("PixelFEmaskIndex");
      moduleFEmaskList.emplace_back("PixelFEmaskPerLB");
      moduleFEmaskList.emplace_back("PixelDCSStatePerLB");
      moduleFEmaskList.emplace_back("PixelDCSStatusPerLB");
      createDecoratorKeys(*this,m_eventInfoKey,m_prefix.value(),moduleFEmaskList,m_moduleFEmaskKeys);
    }

    {
      std::vector<std::string> moduleBSErrList;
      moduleBSErrList.emplace_back("PixelBSErrIndex");
      moduleBSErrList.emplace_back("PixelBSErrWord");
      createDecoratorKeys(*this,m_eventInfoKey,m_prefix.value(),moduleBSErrList,m_moduleBSErrKeys);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode EventInfoPixelModuleStatusMonitoring::finalize() {
    return StatusCode::SUCCESS;
  }

  StatusCode EventInfoPixelModuleStatusMonitoring::addBranches(const EventContext& ctx) const {

    ATH_MSG_DEBUG("Adding Pixel module status in EventInfo");

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey,ctx);
    ATH_CHECK(eventInfo.isValid() ? StatusCode::SUCCESS : StatusCode::FAILURE);

    const xAOD::EventInfo *eventInfoLB = nullptr;
    ATH_CHECK(evtStore()->retrieve(eventInfoLB, "EventInfo"));
    int LB = eventInfoLB->lumiBlock();
    int chkLB = m_lbCounter;
    if (chkLB==0) { chkLB=-1; }

    bool isMC = false;
    if (eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) { isMC=true; }

    SG::ReadCondHandle<PixelDCSHVData> dcsHV(m_readKeyHV,ctx);
    SG::ReadCondHandle<PixelDCSTempData> dcsTemp(m_readKeyTemp,ctx);
    SG::ReadCondHandle<PixelDCSStateData> dcsState(m_condDCSStateKey,ctx);
    SG::ReadCondHandle<PixelDCSStatusData> dcsStatus(m_condDCSStatusKey, ctx);
    SG::ReadCondHandle<PixelDeadMapCondData> deadMap(m_condDeadMapKey,ctx);

    int maxHash = m_pixelID->wafer_hash_max();
    std::vector<float> biasVoltage;
    std::vector<float> temperature;
    std::vector<int> activeState;
    std::vector<int> activeStatus;
    std::vector<int> feMaskIndex;
    std::vector<int> feMaskStatus;
    bool storeLB = false;
    if (!isMC) {
      if (chkLB!=LB) {
        storeLB = true;
        m_lbCounter = LB;
      }
    }
    else {
      if (chkLB==-1) {
        storeLB = true;
        m_lbCounter = 1;
      }
    }

    if (storeLB) {
      for (int ihash=0; ihash<maxHash; ihash++) {
        biasVoltage.push_back(dcsHV->getBiasVoltage(ihash));
        temperature.push_back(dcsTemp->getTemperature(ihash));
        activeState.push_back(dcsState->getModuleStatus(ihash));
        activeStatus.push_back(dcsStatus->getModuleStatus(ihash));

        int moduleStatus = deadMap->getModuleStatus(ihash);
        int chipStatus   = deadMap->getChipStatus(ihash);
        if (moduleStatus || chipStatus) {
          feMaskIndex.push_back(ihash);
          if (moduleStatus) {
            feMaskStatus.push_back(0);
          }
          else {
            feMaskStatus.push_back(chipStatus);
          }
        }
      }
    }

    //====================================================================================
    // This is an example how to read the Error informaiton.
    //
    // The Error word is defined in
    //    InDetConditions/PixelConditionsData/PixelConditionsData/PixelByteStreamErrors.h
    //
    // The IDCInDetBSErrContainer can be accessed through
    //    m_pixelCondSummaryTool->getBSErrorWord(i,ctx)
    // where
    //    i= [    0,  2047] : module error
    //        ( [0, 11] - DBMC, [12, 155] - ECC, [156, 435] - IBL,
    //         [436, 721] - B0, [722, 1215] - B1, [1216, 1891] - B2,
    //         [1892, 2035] - ECA, [2036, 2047] - DBMA )
    //
    //  for PIXEL(FEI3):
    //     = [ 2048,  4095] :   FE-0 error
    //     = [ 4096,  6143] :   FE-1 error
    //     = [ 6144,  8191] :   FE-2 error
    //          ...    ...      ...
    //          ...    ...      ...
    //     = [30720, 32767] :  FE-14 error
    //     = [32768, 34815] :  FE-15 error
    //
    //  for IBL(FEI4):
    //     = [ 2048,  4095] :   FE-0 error
    //     = [ 4096,  6143] :   FE-1 error
    //     = [34816, 35375] :  Error counter in bit#=0 from ServiceRecords (shift: modHash*nFE+iFE)
    //     = [35376, 35935] :  Error counter in bit#=1 from ServiceRecords
    //          ...    ...      ...
    //          ...    ...      ...
    //     = [52176, 52735] :  Error counter in bit#=31 from ServiceRecords
    //
    //====================================================================================
    std::vector<uint64_t> bsErrIndex;
    std::vector<uint64_t> bsErrWord;
    if (!isMC) {
      SG::ReadHandle<InDet::SiDetectorElementStatus> pixel_active = getPixelDetElStatus(m_pixelDetElStatusActiveOnly, ctx);
      SG::ReadHandle<IDCInDetBSErrContainer> idcErrCont;
      if (m_readoutTechnologyMask) {
        idcErrCont = SG::ReadHandle<IDCInDetBSErrContainer>(m_idcErrContKey,ctx);
        if (!idcErrCont.isValid()) {
          ATH_MSG_FATAL("Faled to get BS error container" << m_idcErrContKey.key());
        }
      }

      if (maxHash==2048) { // only valid for RUN2/3
        // First, access BS error for each FE chip
        for (int ihash=0; ihash<maxHash; ihash++) {
          for (int chFE=0; chFE<16; chFE++) {
            int indexFE = (1+chFE)*maxHash+ihash;    // (FE_channel+1)*2048 + moduleHash
            uint64_t word = (!m_pixelDetElStatusActiveOnly.empty() && m_readoutTechnologyMask
                ? InDet::getBSErrorWord(*pixel_active,*idcErrCont,ihash,indexFE,m_readoutTechnologyMask) 
                : m_pixelSummary->getBSErrorWord(ihash,indexFE,ctx));
            VALIDATE_STATUS_ARRAY(!m_pixelDetElStatusActiveOnly.empty() && m_readoutTechnologyMask, 
                InDet::getBSErrorWord(*pixel_active,*idcErrCont,ihash,indexFE,m_readoutTechnologyMask),
                m_pixelSummary->getBSErrorWord(ihash,indexFE,ctx));

            if (word>0) {
              bsErrIndex.push_back(indexFE);
              bsErrWord.push_back(word);
            }
          }
        }
        // Next, access IBL service record
        int indexOffset = 17*maxHash;
        for (int ihash=156; ihash<436; ihash++) {
          for (int chFE=0; chFE<2; chFE++) {
            for (int serviceCode=0; serviceCode<32; serviceCode++) {
              int indexSvcCounter = indexOffset+serviceCode*280*2+2*(ihash-156)+chFE;
              uint64_t word = (!m_pixelDetElStatusActiveOnly.empty() && m_readoutTechnologyMask
                  ? InDet::getBSErrorWord(*pixel_active,*idcErrCont,ihash,indexSvcCounter,m_readoutTechnologyMask) 
                  : m_pixelSummary->getBSErrorWord(ihash,indexSvcCounter,ctx));
              VALIDATE_STATUS_ARRAY(!m_pixelDetElStatusActiveOnly.empty() && m_readoutTechnologyMask, 
                  InDet::getBSErrorWord(*pixel_active,*idcErrCont,ihash,indexSvcCounter,m_readoutTechnologyMask),
                  m_pixelSummary->getBSErrorWord(ihash,indexSvcCounter,ctx));

              if (word>0) {
                bsErrIndex.push_back(indexSvcCounter);
                bsErrWord.push_back(word);
              }
            }
          }
        }
      }
    }
    std::vector<SG::WriteDecorHandle<xAOD::EventInfo,std::vector<float>>> decorModuleCondition(createDecorators<xAOD::EventInfo,std::vector<float>>(m_moduleConditionKeys,ctx));
    assert(decorModuleCondition.size()==2);
    decorModuleCondition[0](*eventInfo) = std::move(biasVoltage);
    decorModuleCondition[1](*eventInfo) = std::move(temperature);

    std::vector<SG::WriteDecorHandle<xAOD::EventInfo,std::vector<int>>> decorModuleFEmask(createDecorators<xAOD::EventInfo,std::vector<int>>(m_moduleFEmaskKeys,ctx));
    assert(decorModuleFEmask.size()==2);
    decorModuleFEmask[0](*eventInfo) = std::move(feMaskIndex);
    decorModuleFEmask[1](*eventInfo) = std::move(feMaskStatus);
    decorModuleFEmask[2](*eventInfo) = std::move(activeState);
    decorModuleFEmask[3](*eventInfo) = std::move(activeStatus);

    std::vector<SG::WriteDecorHandle<xAOD::EventInfo,std::vector<uint64_t>>> decorModuleBSErr(createDecorators<xAOD::EventInfo,std::vector<uint64_t>>(m_moduleBSErrKeys,ctx));
    assert(decorModuleBSErr.size()==2);
    decorModuleBSErr[0](*eventInfo) = std::move(bsErrIndex);
    decorModuleBSErr[1](*eventInfo) = std::move(bsErrWord);

    return StatusCode::SUCCESS;
  }  
  
}
