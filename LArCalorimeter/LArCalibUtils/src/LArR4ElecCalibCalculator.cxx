/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArR4ElecCalibCalculator.h"

#include <chai/Container.h>
#include <chai/Database.h>
#include <chai/Iov.h>
#include <chai/Log.h>
#include <chai/PayloadSpec.h>
#include <chai/Types.h>

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloGain.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArRawConditions/LArNoiseMC.h"
#include "LArRawConditions/LArNoiseSym.h"
#include "LArRawConditions/LArPedestalMC.h"
#include "LArRawConditions/LArRampMC.h"
#include "LArRawConditions/LArRampSym.h"

namespace {

// Method to convert coral::Blob to BlobData
chai::BlobData vecToBlobData(const std::vector<float>& input) {
  const std::size_t size = input.size() * sizeof(float);
  const void* inPtr = input.data();
  // Create vector and copy data
  std::vector<uint8_t> bytes(size);
  if (size > 0) {
    memcpy(bytes.data(), inPtr, size);
  }

  return chai::BlobData(std::move(bytes));
}
}  // namespace

StatusCode LArR4ElecCalibCalculator::initialize() {

  ATH_CHECK(detStore()->retrieve(m_onlineHelper, "LArOnlineID"));
  ATH_CHECK(detStore()->retrieve(m_caloCellID, "CaloCell_ID"));

  ATH_CHECK(m_cablingKey.initialize());

  ATH_CHECK(m_lAruA2MeVKey.initialize());
  ATH_CHECK(m_lArDAC2uAKey.initialize());
  ATH_CHECK(m_mcSym.initialize());

  ATH_CHECK(m_caloMgrKey.initialize());

  return StatusCode::SUCCESS;
}

// ********************** EXECUTE ****************************
StatusCode LArR4ElecCalibCalculator::execute(const EventContext& /*ctx*/) {
  return StatusCode::SUCCESS;
}

// ********************** FINALIZE ****************************
StatusCode LArR4ElecCalibCalculator::stop() {
  
  // Values below are mA, in the lower gain
  const std::map<CaloSampling::CaloSample, float> dynRange{
      {CaloSampling::PreSamplerB, 2},
      {CaloSampling::EMB1, 2},
      {CaloSampling::EMB2, 10},
      {CaloSampling::EMB3, 10},
      {CaloSampling::PreSamplerE, 2},
      {CaloSampling::EME1, 2},
      {CaloSampling::EME2, 10},
      {CaloSampling::EME3, 10},
      {CaloSampling::HEC0, 2},
      {CaloSampling::HEC1, 2},
      {CaloSampling::HEC2, 2},
      {CaloSampling::HEC3, 2,},
      {CaloSampling::FCAL0, 10},
      {CaloSampling::FCAL1, 10},
      {CaloSampling::FCAL2, 10},
  };

  // Values are in ADC counts in HIGH gain
  // Structure: maps of samplings < vector <eta-boundary,value> >
  const std::map<CaloSampling::CaloSample, std::vector<std::pair<float, float> > > noiseMap{
      {CaloSampling::PreSamplerB, {{5, 3.2}}},
      {CaloSampling::EMB1, {{0.8, 1.8}, {5, 2.1}}},
      {CaloSampling::EMB2, {{0.8, 1.4}, {5, 1.7}}},
      {CaloSampling::EMB3, {{0.8, 1.4}, {5, 1.7}}},
      {CaloSampling::PreSamplerE, {{5, 3.5}}},
      {CaloSampling::EME1, {{2, 2.7}, {5, 2.9}}},
      {CaloSampling::EME2, {{2, 1.3}, {5, 1.5}}},
      {CaloSampling::EME3, {{2, 1.3}, {5, 1.5}}},
      {CaloSampling::HEC0, {{5, 3.1}}},
      {CaloSampling::HEC1, {{5, 3.1}}},
      {CaloSampling::HEC2, {{5, 2.1}}},
      {CaloSampling::HEC3, {{5, 2.1}}},

      {CaloSampling::FCAL0, {{5, 4.3}}},
      {CaloSampling::FCAL1, {{5, 4.6}}},
      {CaloSampling::FCAL2, {{5, 4.5}}},
  };

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey};
  SG::ReadCondHandle<LArMCSym> larMCsymHdl{m_mcSym};
  SG::ReadCondHandle<ILAruA2MeV> uA2MeVHdl{m_lAruA2MeVKey};
  SG::ReadCondHandle<ILArDAC2uA> DAC2uAHdl{m_lArDAC2uAKey};

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey};

  std::unique_ptr<LArRampMC> rampMC = std::make_unique<LArRampMC>();
  ATH_CHECK(rampMC->setGroupingType("Single", msg()));
  ATH_CHECK(rampMC->initialize());

  const auto& symIDs = larMCsymHdl->symIds();

  const int nUsableBits = int(pow(2, m_nADCBits) * 3 / 4.);


  //Calculate the R4 ramps (ADC2DAC) values, store them in run 1/2/3 style objects (LArRampMC)
  //Can be stored in COOL-reference POOL files if the RegistrationSvc and OutptuConditionsAlg are scheduled in job-config 
  for (const HWIdentifier hwid : symIDs) {
    const Identifier id = (*cablingHdl)->cnvToIdentifier(hwid);
    const auto sampling = (CaloSampling::CaloSample)m_caloCellID->calo_sample(id);
    const auto rangeIt = dynRange.find(sampling);
    if (rangeIt == dynRange.end())[[unlikely]]{
      ATH_MSG_WARNING("LArR4ElecCalibCalculator::stop: sampling not found in dynRange.");
      continue;
    }
    const double mARange = rangeIt->second;
    const double uAperADC = 1000 * mARange / nUsableBits;
    const double dac2uA = (*DAC2uAHdl)->DAC2UA(hwid);  // uA/DAC
    const double ADC2DAC = uAperADC / dac2uA;

    // For cross-check:
    const double ADC2MeV = ADC2DAC * dac2uA * uA2MeVHdl->UA2MEV(hwid);
    const double adc2uA = ADC2DAC * dac2uA;
    ATH_MSG_DEBUG("sub_calo=" << m_caloCellID->sub_calo(id) << ", layer " << m_caloCellID->sampling(id) << ", region=" << m_caloCellID->region(id) << ": uA/ADC "
              << uAperADC << ", ADC/mA " << nUsableBits / mARange << ", DAC2uA " << dac2uA << ", Ramp " << ADC2DAC << ", ADC2MeV=" << ADC2MeV
              << ", ADC2DAC*dac2uA=" << adc2uA);

    std::vector<float> vRamp;
    vRamp.push_back(0);  // ignore intercept for MC
    vRamp.push_back(ADC2DAC);

    rampMC->set(hwid, CaloGain::LARMEDIUMGAIN, vRamp);
    // re-use the same short vector for higher gain
    vRamp[1] /= 23.0;  // HIGH gain =23* lower gain
    rampMC->set(hwid, CaloGain::LARHIGHGAIN, vRamp);
  }

  // With the backward compatible MT-migration, the LArRampMC object became
  // "write only". It is stored in the database but on read-back turned into a
  // LArRampSym object. Only the latter can be used
  std::unique_ptr<LArRampSym> rampSym = std::make_unique<LArRampSym>(*larMCsymHdl, rampMC.get());

  StatusCode sc = detStore()->record(std::move(rampMC), m_keyoutput);
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Failed to record LArRampSym object with key " << m_keyoutput);
  }
  sc = detStore()->symLink(ClassID_traits<LArRampMC>::ID(), m_keyoutput, ClassID_traits<ILArRamp>::ID());
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Failed to symlink LArRampMC to ILArRamp base-class");
  }

  m_keyoutput += "Sym";
  sc = detStore()->record(std::move(rampSym), m_keyoutput);
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Failed to record LArRampSym object with key " << m_keyoutput);
  }
  sc = detStore()->symLink(ClassID_traits<LArRampSym>::ID(), m_keyoutput, ClassID_traits<ILArRamp>::ID());
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Failed to symlink LArRampSym to ILArRamp base-class");
  }


  //Pedestal: Use only one value for all channels in the MC case
  //Again, create a run 1/2/3 style LArPedestalMC object that can be store in a COOL-referenced POOL file
  std::vector<float> pedVec, pedRMSVec;
  pedVec.push_back(m_pedestalValue.value());
  pedRMSVec.push_back(m_pedestalRMS.value());
  std::unique_ptr<LArPedestalMC> pedPtr = std::make_unique<LArPedestalMC>();
  pedPtr->set(pedVec, pedRMSVec);
  ATH_CHECK(detStore()->record(std::move(pedPtr), m_pedkeyoutput));



  //Noise: Fill a run 1/2/3-style LArNoiseMC object that can be store in a COOL-referenced POOL file
  std::unique_ptr<LArNoiseMC> noisePtr = std::make_unique<LArNoiseMC>();
  ATH_CHECK(noisePtr->setGroupingType("Single", msg()));
  ATH_CHECK(noisePtr->initialize());

  SG::ReadCondHandle<CaloDetDescrManager> caloDDM{m_caloMgrKey};
  for (const HWIdentifier hwid : symIDs) {
    const Identifier id = (*cablingHdl)->cnvToIdentifier(hwid);
    const auto sampling = (CaloSampling::CaloSample)m_caloCellID->calo_sample(id);
    const CaloDetDescrElement* dde = caloDDM->get_element(id);
    const float eta = dde->eta_raw();

    const auto& pairs = noiseMap.find(sampling)->second;
    float noise = -1;
    for (auto [e, n] : pairs) {
      if (eta < e) {
        noise = n;
        ATH_MSG_DEBUG("Sampling: " << sampling << ", eta=" << eta << ", noise=" << noise);
        break;
      }
    }
    noisePtr->set(hwid, CaloGain::LARHIGHGAIN, noise);
    noisePtr->set(hwid, CaloGain::LARMEDIUMGAIN, noise / 23.0);
  }  // end loop over sym-ids

  std::unique_ptr<LArNoiseSym> noiseSym = std::make_unique<LArNoiseSym>(*larMCsymHdl, noisePtr.get());

  ATH_CHECK(detStore()->record(std::move(noisePtr), "LArNoise"));
  sc = detStore()->symLink(ClassID_traits<LArNoiseMC>::ID(), "LArNoise", ClassID_traits<ILArNoise>::ID());
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Failed to symlink LArNoiseMC to ILArNoise base-class");
  }

  sc = detStore()->record(std::move(noiseSym), "LArNoiseSym");
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Failed to record LArNoiseSym object with key " << m_keyoutput);
  }
  sc = detStore()->symLink(ClassID_traits<LArNoiseSym>::ID(), "LArNoiseSym", ClassID_traits<ILArNoise>::ID());
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Failed to symlink LArNoiseSym to ILArNoise base-class");
  }




  //If a CREST database is also given, we store the calibration constants in CREST
  //In this case, the values are stored 'inline' in a Blob, so it can be read back into LArXYZFlat classes that are
  //used for data-processing also in run 2 and 3.
  if (!m_crestDBStr.value().empty()) {

    chai::Tag::Metadata chaiMD{.iovType=chai::Tag::IovType::RunNumberLumiBlock, 
        .objectType="crest-json-single-iov",
        .synchronization=chai::Tag::Synchronization::All, 
        .status=chai::Tag::Status::Unlocked,
        .nodeDescription = chai::Tag::buildNodeDescription(chai::Tag::IovType::RunNumberLumiBlock, "CondAttrListCollection", 1238547719u)
       };

    std::vector<float> rampsHG, rampsMG, noiseHG, noiseMG, ped, pedRMS;
    const LArRampSym* rampsIn = nullptr;
    ATH_CHECK(detStore()->retrieve(rampsIn, "LArRampSym"));
    const LArNoiseSym* noiseIn = nullptr;
    ATH_CHECK(detStore()->retrieve(noiseIn, "LArNoiseSym"));

    const unsigned int hashmax = m_onlineHelper->channelHashMax();

    //Store only one pedestal value for all channels:
    ped.push_back((m_pedestalValue.value()));
    pedRMS.push_back(m_pedestalRMS.value());
    for (unsigned idx = 0; idx < hashmax; ++idx) {
      IdentifierHash hIdx(idx);
      HWIdentifier hwid = m_onlineHelper->channel_Id(hIdx);
      if (!(*cablingHdl)->isOnlineConnectedFromHash(hIdx)) {
        rampsHG.push_back(0);
        rampsHG.push_back(-999);
        rampsMG.push_back(0);
        rampsMG.push_back(-999);
        noiseHG.push_back(-999);
        noiseMG.push_back(-999);
      } else {
        const auto rampvecHG = rampsIn->ADC2DAC(hwid, CaloGain::LARHIGHGAIN);
        rampsHG.push_back(rampvecHG[0]);
        rampsHG.push_back(rampvecHG[1]);
        const auto rampvecMG = rampsIn->ADC2DAC(hwid, CaloGain::LARMEDIUMGAIN);
        rampsMG.push_back(rampvecMG[0]);
        rampsMG.push_back(rampvecMG[1]);

        noiseHG.push_back(noiseIn->noise(hwid, CaloGain::LARHIGHGAIN));
        noiseMG.push_back(noiseIn->noise(hwid, CaloGain::LARMEDIUMGAIN));
      }
    }
    chai::Database db = chai::Database(m_crestDBStr);

    // Pedestals
    {
      chai::PayloadSpec spec(chai::FieldSpec({{"Pedestal", chai::Type::Blob}, {"PedestalRMS", chai::Type::Blob}, {"version", chai::Type::UInt32}}),

                             chai::ChannelSpec({
                                 {0, "HIGHGain"},
                                 {1, "MEDGain"},
                             }));

      auto tag = db.createTag("LARElecCalibPedestal-R4-00", "Pedestal of FEB 2", spec, chaiMD);
       

      chai::Container container = tag->buildContainer();
      for (size_t g = 0; g < 2; ++g) {
        container[g].push(std::move(vecToBlobData(ped)));
        container[g].push(std::move(vecToBlobData(pedRMS)));
        container[g].push(0);  // version number
      }
      tag->addPayload(container, 0);
    }
    // Ramps
    {
      chai::PayloadSpec spec(chai::FieldSpec({{"RampVec", chai::Type::Blob}, {"nPoints", chai::Type::UInt32}, {"version", chai::Type::UInt32}}),

                             chai::ChannelSpec({
                                 {0, "HIGHGain"},
                                 {1, "MEDGain"},
                             }));

      auto tag = db.createTag("LARElecCalibRamp-R4-00", "Electronic gain of FEB 2", spec, chaiMD);

      chai::Container container = tag->buildContainer();
      container[0].push(std::move(vecToBlobData(rampsHG)));
      container[0].push(2);  // number of points (intercept,gradient)
      container[0].push(0);  // version number

      container[1].push(std::move(vecToBlobData(rampsMG)));
      container[1].push(2);  // number of points (intercept,gradient)
      container[1].push(0);  // version number

      tag->addPayload(container, 0);
    }
    // Noise
    {
      chai::PayloadSpec spec(chai::FieldSpec({{"Noise", chai::Type::Blob}, {"version", chai::Type::UInt32}}),

                             chai::ChannelSpec({
                                 {0, "HIGHGain"},
                                 {1, "MEDGain"},
                             }));

      auto tag = db.createTag("LARElecCalibNoise-R4-00", "Noise of FEB 2 (in ADC counts)", spec, chaiMD);
      chai::Container container = tag->buildContainer();

      container[0].push(std::move(vecToBlobData(noiseHG)));
      container[0].push(0);  // version number

      container[1].push(std::move(vecToBlobData(noiseMG)));
      container[1].push(0);  // version number
      tag->addPayload(container, 0);
    }
  }
  ATH_MSG_INFO("LArR4RampCalculator has finished.");
  return StatusCode::SUCCESS;
}  // end finalize-method.
