/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArLATOMEBuilderAlg.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "LArRawEvent/LArRawSCContainer.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArSCDigitContainer.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "AthAllocators/DataPool.h"
#include "LArCOOLConditions/LArPedestalSC.h"
#include "LArCOOLConditions/LArOFCSC.h"
#include "LArCOOLConditions/LArRampSC.h"
#include "LArCOOLConditions/LArDAC2uASC.h"
#include "LArCOOLConditions/LAruA2MeVSC.h"
#include "LArCOOLConditions/LArMphysOverMcalSC.h"
#include "LArCOOLConditions/LArHVScaleCorrSC.h"
#include <cmath>


namespace {
  inline int pow2(const int x) {return (0x1<<x);}
};

LArLATOMEBuilderAlg::LArLATOMEBuilderAlg(const std::string& name, ISvcLocator* pSvcLocator):
  AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode LArLATOMEBuilderAlg::initialize() {

  ATH_MSG_INFO("LArLATOMEBuilderAlg init");

  ATH_CHECK(m_eventInfoKey.initialize());
  ATH_CHECK(m_digitKey.initialize());
  ATH_CHECK(m_larRawSCKey.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_keyPedestalSC.initialize());
  ATH_CHECK(m_keyOFCSC.initialize());
  ATH_CHECK(m_keyRampSC.initialize());
  ATH_CHECK(m_keyDAC2uASC.initialize());
  ATH_CHECK(m_keyuA2MeVSC.initialize());
  ATH_CHECK(m_keyHVScaleCorrSC.initialize());
  ATH_CHECK(m_keyMphysOverMcalSC.initialize());

  const LArOnline_SuperCellID* ll;
  ATH_CHECK(detStore()->retrieve(ll,"LArOnline_SuperCellID"));
  m_onlineId = static_cast<const LArOnlineID_Base*>(ll);

  return StatusCode::SUCCESS;
}

StatusCode LArLATOMEBuilderAlg::finalize() {
  return StatusCode::SUCCESS;
}

StatusCode LArLATOMEBuilderAlg::execute(const EventContext& ctx) const {

  SG::ReadHandle<xAOD::EventInfo> thisEvent{m_eventInfoKey,ctx};
  unsigned int event_bcid = thisEvent->bcid();

  //Get event inputs from read handles:
  SG::ReadHandle<LArDigitContainer> inputContainer(m_digitKey,ctx);
  //Write output via write handle
  SG::WriteHandle<LArRawSCContainer> outputContainerHdl(m_larRawSCKey,ctx);
  ATH_CHECK(outputContainerHdl.record(std::make_unique<LArRawSCContainer>()));
  auto *outputContainer = outputContainerHdl.ptr();
  outputContainer->clear(SG::VIEW_ELEMENTS);
  outputContainer->reserve(inputContainer->size());
  DataPool<LArRawSC> dataItemsPool(ctx);
  dataItemsPool.reserve(inputContainer->size());

  //Get Conditions input
  SG::ReadCondHandle<ILArPedestal> pedHdl(m_keyPedestalSC,ctx);
  const LArPedestalSC* Peds=dynamic_cast<const LArPedestalSC*>(pedHdl.cptr());
  if (!Peds) return StatusCode::FAILURE;

  SG::ReadCondHandle<ILArOFC> ofcHdl(m_keyOFCSC,ctx);
  const LArOFCSC* OFCs=dynamic_cast<const LArOFCSC*>(ofcHdl.cptr());
  if (!OFCs) return StatusCode::FAILURE;

  SG::ReadCondHandle<ILArRamp> rampHdl(m_keyRampSC,ctx);
  const LArRampSC* Ramps=dynamic_cast<const LArRampSC*>(rampHdl.cptr());
  if (!Ramps) return StatusCode::FAILURE;

  SG::ReadCondHandle<ILArDAC2uA> dac2uaHdl(m_keyDAC2uASC,ctx);
  const LArDAC2uASC* DAC2uAs=dynamic_cast<const LArDAC2uASC*>(dac2uaHdl.cptr());
  if (!DAC2uAs) return StatusCode::FAILURE;

  SG::ReadCondHandle<ILAruA2MeV> ua2mevHdl(m_keyuA2MeVSC,ctx);
  const LAruA2MeVSC* uA2MeVs=dynamic_cast<const LAruA2MeVSC*>(ua2mevHdl.cptr());
  if (!uA2MeVs) return StatusCode::FAILURE;

  SG::ReadCondHandle<ILArMphysOverMcal> mphysHdl(m_keyMphysOverMcalSC,ctx);
  const LArMphysOverMcalSC* MphysOverMcals=dynamic_cast<const LArMphysOverMcalSC*>(mphysHdl.cptr());
  if (!MphysOverMcals) return StatusCode::FAILURE;

  SG::ReadCondHandle<ILArHVScaleCorr> hvHdl(m_keyHVScaleCorrSC,ctx);
  const LArHVScaleCorrSC* HVScaleCorrs=dynamic_cast<const LArHVScaleCorrSC*>(hvHdl.cptr());
  if (!HVScaleCorrs) return StatusCode::FAILURE;

  SG::ReadCondHandle<LArOnOffIdMapping> cabling(m_cablingKey,ctx);

  unsigned int nEnergies=m_nEnergies;

  //Loop over digits:
  for (const LArDigit* digit : *inputContainer) {

    const LArSCDigit* digitSC=dynamic_cast<const LArSCDigit*>(digit);
    if(!digitSC){
      ATH_MSG_ERROR("container elements not of type LArSCDigit");
      return StatusCode::FAILURE;
    }
    const std::vector<uint16_t>& bcids = digitSC->BCId();
    const HWIdentifier id=digit->hardwareID();
    std::flush(std::cout);
    const std::vector<short>& samples=digit->samples();
    int gain=digit->gain();
    float ped=Peds->pedestal(id,gain);
    LArOFCSC::OFCRef_t ofca=OFCs->OFC_a(id,gain);
    LArOFCSC::OFCRef_t ofcb=OFCs->OFC_b(id,gain);
    ILArRamp::RampRef_t ramp=Ramps->ADC2DAC(id,gain);
    float dac2ua=DAC2uAs->DAC2UA(id);
    float ua2mev=uA2MeVs->UA2MEV(id);
    float mphys=MphysOverMcals->MphysOverMcal(id,gain);
    float hvcorr=HVScaleCorrs->HVScaleCorr(id);
    float ELSB = 12.5; /// will take from DB later

    if (ATH_UNLIKELY(ped==ILArPedestal::ERRORCODE)) {
      ATH_MSG_ERROR("No valid pedestal for connected channel " << id.get_identifier32().get_compact() << " gain " << gain);
      return StatusCode::FAILURE;
    }
    if(ATH_UNLIKELY(!ofca.valid())){
      ATH_MSG_ERROR("No valid ofca for connected channel " << id.get_identifier32().get_compact() << " gain " << gain);
      return StatusCode::FAILURE;
    }
    if(ATH_UNLIKELY(!ofcb.valid())){
      ATH_MSG_ERROR("No valid ofcb for connected channel " << id.get_identifier32().get_compact() << " gain " << gain);
      return StatusCode::FAILURE;
    }
    if(ATH_UNLIKELY(!ramp.valid())){
      ATH_MSG_ERROR("No valid ramp for connected channel " << id.get_identifier32().get_compact() << " gain " << gain);
      return StatusCode::FAILURE;
    }
    if(ATH_UNLIKELY(ramp.size()!=2)){
      ATH_MSG_ERROR("wrong ramp size for connected channel " << id.get_identifier32().get_compact() << " gain " << gain);
      return StatusCode::FAILURE;
    }
    if (ATH_UNLIKELY(dac2ua==ILArDAC2uA::ERRORCODE)) {
      ATH_MSG_ERROR("No valid dac2ua for connected channel " << id.get_identifier32().get_compact());
      return StatusCode::FAILURE;
    }
    if (ATH_UNLIKELY(ua2mev==ILAruA2MeV::ERRORCODE)) {
      ATH_MSG_ERROR("No valid ua2mev for connected channel " << id.get_identifier32().get_compact());
      return StatusCode::FAILURE;
    }
    if (ATH_UNLIKELY(mphys==ILArHVScaleCorr::ERRORCODE)) {
      ATH_MSG_ERROR("No valid mphys for connected channel " << id.get_identifier32().get_compact() << " gain " << gain);
      return StatusCode::FAILURE;
    }
    if (ATH_UNLIKELY(hvcorr==ILArMphysOverMcal::ERRORCODE)) {
      ATH_MSG_ERROR("No valid hvcorr for connected channel " << id.get_identifier32().get_compact());
      return StatusCode::FAILURE;
    }

    /// lets store the floats in case we decide to do a float computation here in the future
    std::vector<float> ofca_mev(ofca.size());
    std::vector<float> ofcb_mev(ofca.size());
    float peda=ped;
    float pedb=ped;
    for (unsigned int i = 0; i < ofca.size(); ++i) {
      ofca_mev[i] = ofca[i] * ramp[1] * dac2ua * ua2mev / ELSB;
      ofcb_mev[i] = ofcb[i] * ramp[1] * dac2ua * ua2mev / ELSB;
      if (m_applyMphysOverMcal) {
        ofca_mev[i] /= mphys;
        ofcb_mev[i] /= mphys;
      }
      if (m_applyHVCorrection) {
        ofca_mev[i] *= hvcorr;
        ofcb_mev[i] *= hvcorr;
      }
    }
    if (m_useR0) {
      float suma=0; for(auto a:ofca)suma+=a;
      float sumb=0; for(auto b:ofcb)sumb+=b;
      peda=ped-ramp[0]/ramp[1]*suma;
      pedb=ped-ramp[0]/ramp[1]*sumb;
    }

    bool aoverflow=false;
    bool boverflow=false;
    bool pedoverflow=false;

    std::vector<int> ofca_int(ofca.size(),0);
    std::vector<int> ofcb_int(ofca.size(),0);
    int peda_int=0;
    int pedb_int=0;

    const int pedHardpoint  = 3;
    const int firLSBdropped = 8;
    const int satLSBdropped = 6;
    const int paramBitSize  = 18;

    for (unsigned int i = 0; i < ofca.size(); ++i) {
      if (!floatToInt(ofca_mev[i], ofca_int[i], firLSBdropped - pedHardpoint,
                      paramBitSize)) {
        aoverflow = true;
      }
      if (!floatToInt(ofcb_mev[i], ofcb_int[i], satLSBdropped - pedHardpoint,
                      paramBitSize)) {
        boverflow = true;
      }
    }
    if(!floatToInt(peda,peda_int,pedHardpoint,paramBitSize))pedoverflow=true;
    if(!floatToInt(pedb,pedb_int,pedHardpoint,paramBitSize))pedoverflow=true;

    unsigned int nsamples = samples.size();
    unsigned int firsamples=4;
    int startSample=-1;
    for(unsigned int is=0; is<nsamples; ++is){
      if(bcids[is]==event_bcid) startSample=is;
    }
    int maxNenergies = 0;
    if(startSample<0){
      ATH_MSG_WARNING("could not find correct BCID for recomputing the energies, event BCID="<<event_bcid<< " first sample BCID " << (nsamples?bcids[0]:-1));
    }
    else{
      maxNenergies=nsamples-firsamples-startSample+1;
      if(m_startSample) maxNenergies -= m_startSample;
    }
    if(maxNenergies<0) {
       maxNenergies=0;
    } else {
       startSample += m_startSample;
    }
    if((int)nEnergies>maxNenergies){
      ATH_MSG_WARNING("requested nEnergies > maxNenergies " << m_nEnergies << ">" <<maxNenergies<<". setting nEnegries to maxNenergies");
      nEnergies=maxNenergies;
    }

    std::vector<unsigned short> newBCIDs(nEnergies,0);
    std::vector<int> newEnergies(nEnergies,0);
    std::vector<int> tauEnergies(nEnergies,0);
    std::vector<bool> passSelections(nEnergies,false);
    std::vector<bool> satur(nEnergies,false);
    const unsigned int nMaxBitsEnergy=18;
    const unsigned int nMaxBitsEnergyTau=22;

    for(unsigned int ss=0; ss<nEnergies; ++ss){

      int64_t computedE=0;
      int64_t computedEtau=0;
      unsigned short bcid = bcids[startSample+ss];
      newBCIDs[ss]=bcid;
      for(unsigned int is=0; is<firsamples; ++is){
        int sample = samples[startSample + ss + is];
        if (!m_isADCBas)
          sample *= std::pow(2, pedHardpoint);
        computedE += static_cast<int64_t>(sample - peda_int) * ofca_int[is];
        computedEtau += static_cast<int64_t>(sample - pedb_int) * ofcb_int[is];
      }
      computedE=computedE>>firLSBdropped;
      computedEtau=computedEtau>>satLSBdropped;

      if(std::abs(computedE)>std::pow(2,nMaxBitsEnergy-1)){
        if (computedE >= 0)
          computedE = std::pow(2, nMaxBitsEnergy - 1) - 1;
        else
          computedE = 0;
      }
      if (std::abs(computedEtau) > std::pow(2, nMaxBitsEnergyTau - 1)) {
        if (computedEtau >= 0)
          computedEtau = std::pow(2, nMaxBitsEnergyTau - 1) - 1;
        else
          computedEtau = -std::pow(2, nMaxBitsEnergyTau - 1) + 1;
      }
      newEnergies[ss] = computedE;
      tauEnergies[ss] = computedEtau;
      bool passSelection = false;
      if (computedE < 0 && computedE > -80) {
        if (computedEtau > 8 * computedE && computedEtau < -8 * computedE)
          passSelection = true;
      } else if (computedE < 800) {
        if (computedEtau > -8 * computedE && computedEtau < 8 * computedE)
          passSelection = true;
      } else if (computedE >= 800) {
        if (computedEtau > -8 * computedE && computedEtau < 16 * computedE)
          passSelection = true;
      }
      passSelections[ss] = passSelection;
    }
    LArRawSC* scraw = dataItemsPool.nextElementPtr();

    scraw->setHardwareId(id);
    scraw->setChannel(digitSC->Channel());
    scraw->setSourceId(digitSC->SourceId());
    scraw->setBCIds(std::move(newBCIDs));
    scraw->setSaturation(std::move(satur));
    scraw->setEnergies(std::move(newEnergies));
    scraw->setTauEnergies(std::move(tauEnergies));
    scraw->setPassTauSelection(std::move(passSelections));
    scraw->setOFCaOverflow(aoverflow);
    scraw->setOFCbOverflow(boverflow);
    scraw->setPedOverflow(pedoverflow);

    outputContainer->push_back(scraw);

  } /// scs


  return StatusCode::SUCCESS;
}

/// reproduce LDPB package computation in https://gitlab.cern.ch/atlas-lar-online/onlinelatomedb/-/blob/master/src/CondFloatDB.cpp
bool LArLATOMEBuilderAlg::floatToInt(float val, int &newval, int hardpoint, int size) {
  if( std::isnan(val) )return false;
  //int intVal = std::round(val*(0x1<<hardpoint)); //was round(val*pow(2,hardpoint));
  int intVal=std::round(val*pow2(hardpoint));
  bool isNeg = (intVal<0);
  unsigned int posVal = std::abs(intVal);
  if( (posVal >> (size -1)) != 0 ) return false;
  newval=posVal;
  if(isNeg)newval=-posVal;
  return true;

}

