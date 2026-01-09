/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <sTgcDigitizationR4/sTgcDigitizationTool.h>
#include "xAODMuonViews/ChamberViewer.h"
#include "TruthUtils/HepMCHelpers.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "GaudiKernel/PhysicalConstants.h"
namespace MuonR4 {
  StatusCode sTgcDigitizationTool::initialize() {
    ATH_MSG_DEBUG("sTgcDigitizationTool::initialize()");
    ATH_MSG_DEBUG ( "Configuration  sTgcDigitizationTool" );
    ATH_MSG_DEBUG ( "doSmearing             "<< m_doSmearing);
    ATH_MSG_DEBUG("OutputObjectName       " << m_writeKey.key());
    ATH_MSG_DEBUG ( "HV                     " << m_runVoltage);
    ATH_MSG_DEBUG ( "threshold              " << m_chargeThreshold);
    ATH_MSG_DEBUG ( "useCondThresholds      " << m_useCondThresholds);

    ATH_CHECK(MuonDigitizationTool::initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_effiDataKey.initialize(!m_effiDataKey.empty()));
    ATH_CHECK(m_condThrshldsKey.initialize(m_useCondThresholds));
    ATH_CHECK(m_smearingTool.retrieve());
    ATH_CHECK(m_calibrationTool.retrieve());
    
    if (m_doSmearing) {
      ATH_MSG_INFO("Running in smeared mode!");
    }

    /// meanGasGain is the mean value of the polya gas gain function describing the
    /// avalanche of electrons caused by the electric field
    /// Parameterization is obtained from ATL-MUON-PUB-2014-001 and the corrected
    /// fit to data to parameterize gain vs HV in kV
    if (m_runVoltage < 2.3 || m_runVoltage > 3.2){
      ATH_MSG_WARNING("STGC run voltage must be within fit domain of 2.3 kV to 3.2 kV");
      return StatusCode::FAILURE;
    }
    double meanGasGain = 2.15 * 1E-4 * std::exp(6.88*m_runVoltage);
    sTgcDigitMaker::digitMode mode = static_cast<sTgcDigitMaker::digitMode>(m_digitMode.value());
    m_digitizer = std::make_unique<sTgcDigitMaker>(m_detMgr, mode, meanGasGain, m_doPadSharing);
    ATH_CHECK(m_digitizer->initialize());
    
    return StatusCode::SUCCESS;
  }

  uint16_t sTgcDigitizationTool::bcTagging(const double digitTime) const {
    uint16_t bctag = 0;
    int bunchInteger = (digitTime > 0) ? static_cast<int>(std::abs(digitTime / 25.0))
                                       : static_cast<int>(std::abs(digitTime / 25.0)) + 1;
    bctag = (bctag | bunchInteger);
    if (digitTime < 0) bctag = ~bctag;
    return bctag;
  }

  double sTgcDigitizationTool::getChannelThreshold(const EventContext& ctx,
                                                   const Identifier& channelID,
                                                   const NswCalibDbThresholdData& thresholdData) const {
    float threshold = m_chargeThreshold;
    float elecThreshold = 0.0;
    if (!thresholdData.getThreshold(channelID, elecThreshold)) {
      THROW_EXCEPTION("Cannot retrieve VMM threshold from conditions database!");
    }
    
    if (!m_calibrationTool->pdoToCharge(ctx, true, elecThreshold, channelID, threshold)) {
      THROW_EXCEPTION("Cannot convert VMM charge threshold via conditions data!");
    }
    
    return threshold;    
  }
  
  StatusCode sTgcDigitizationTool::digitize(const EventContext& ctx,
                                            const TimedHits& hitsToDigit,
                                            xAOD::MuonSimHitContainer* sdoContainer) const {
    const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
    CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);
    const ActsTrk::GeometryContext &gctx{getGeoCtx(ctx)};

    const Muon::DigitEffiData* efficiencyMap{nullptr};
    ATH_CHECK(SG::get(efficiencyMap, m_effiDataKey, ctx));
    const NswCalibDbThresholdData* thresholdData{nullptr};
    ATH_CHECK(SG::get(thresholdData, m_condThrshldsKey, ctx));

    DigiConditions digiCond{efficiencyMap, thresholdData, rndEngine};
    DigiCache digitCache{};
  
    double earliestEventTime = std::numeric_limits<double>::max();
    xAOD::ChamberViewer viewer{hitsToDigit, m_idHelperSvc.get()};
    const double angular_tolerance = 1e-3; // ~0.057 degrees

    do {
      if (viewer.size() == 0) {
          ATH_MSG_VERBOSE("No hits to digitize — skipping sTGC digitization.");
          continue;
      }   
      std::array<sTgcSimDigitVec , 3> simDigitsByChType{};
      for (const TimedHit& hit : viewer) {

        if (m_digitizeMuonOnly && !MC::isMuon(hit)) {
          ATH_MSG_VERBOSE("Hit is not from a muon - skipping ");
          continue;
        }
        ATH_MSG_VERBOSE("Hit Particle ID : " << hit->pdgId() );
        double eventTime = hit.eventTime();
        earliestEventTime = std::min(earliestEventTime, eventTime);
        if (hit->energyDeposit() < m_energyDepositThreshold){
          ATH_MSG_VERBOSE("Hit with Energy Deposit of " << hit->energyDeposit()
          << " less than " << m_energyDepositThreshold << ". Skip this hit." );
          continue;
        }

        /// Storing hit position and direction in local frame
        const Amg::Vector3D locHitPos = xAOD::toEigen(hit->localPosition());
        const Amg::Vector3D locHitDir = xAOD::toEigen(hit->localDirection());

        const double hitKineticEnergy = hit->kineticEnergy();
        if (hitKineticEnergy < m_limitElectronKineticEnergy && MC::isElectron(hit)) {
          ATH_MSG_DEBUG("Skip electron hit with kinetic energy " << hitKineticEnergy
                      << ", which is less than the lower limit of " << m_limitElectronKineticEnergy);
          continue;
        }

        // No support for particles with direction perpendicular to the beam line, since such particles
        // can deposit energy on a lot of strips and pads of the gas gap.
        const double theta = std::acos(locHitDir.z());  // polar angle from Z axis
        const double ninetyDegrees = std::numbers::pi / 2;
        // Reject hits that are too close to 90 degrees (i.e., perpendicular to Z)
        if (std::abs(theta - ninetyDegrees) < angular_tolerance) {
          ATH_MSG_VERBOSE("Skipping hit nearly perpendicular to Z-axis (angle: " << theta << ")");
          continue;
        }
        if (std::abs(locHitDir.z()) < 0.00001) {
          ATH_MSG_VERBOSE("Skipping hit nearly perpendicular to Z-axis.");
          continue;
        }
        if(eventTime != 0){
          ATH_MSG_DEBUG("Updated hit global time to include off set of " << eventTime << " ns from OOT bunch.");
        }
        else {
          ATH_MSG_DEBUG("This hit came from the in time bunch.");
        }

        const Identifier hitId = hit->identify();
        /// apply the smearing tool to decide if the hit has to be digitized or not
        /// based on layer efficiency
        if (m_doSmearing) {
          bool acceptHit = true;
          ATH_CHECK(m_smearingTool->isAccepted(hitId, acceptHit, rndEngine));
          if ( !acceptHit ) {
            ATH_MSG_DEBUG("Dropping the hit - smearing tool");
            continue;
          }
        }
        const MuonGMR4::sTgcReadoutElement* readoutElement = m_detMgr->getsTgcReadoutElement(hitId);
        const Amg::Vector3D globalHitPos = readoutElement->localToGlobalTrans(gctx, hitId) * locHitPos;
        double globalHitTime = hit->globalTime() + eventTime;
        double tofCorrection = globalHitPos.mag() / Gaudi::Units::c_light;
        double bunchTime = globalHitTime - tofCorrection;

        const HepMcParticleLink particleLink = hit->genParticleLink();
        // Print some information about the sTGC hit
        ATH_MSG_VERBOSE("hitID  " << m_idHelperSvc->toString(hitId) << " Hit bunch time  " << bunchTime << " tof/G4 hit time " << globalHitTime
                                  << " globalHitPosition " << Amg::toString(globalHitPos) << " hit: r " << globalHitPos.perp() << " z " << globalHitPos.z()
                                  << " mclink " << particleLink << "Kinetic energy " << hitKineticEnergy);
        ATH_MSG_VERBOSE("Total hits passed to digitize: " << hitsToDigit.size());   
        sTgcDigitVec digitizedHits = m_digitizer->executeDigi(digiCond, hit);
        if(digitizedHits.empty()) {
          continue;
        }
        ATH_MSG_VERBOSE("Hit produced " << digitizedHits.size() << " digits." );

        for (std::unique_ptr<sTgcDigit>& digit : digitizedHits) {
          /*
          NOTE:
          -----
          Since not every hit might end up resulting in a
          digit, this construction might take place after the hit loop
          in a loop of its own!
          */
          // make new sTgcDigit
          const Identifier digitId = digit->identify();
          double digitTime = digit->time();
          int digitChType = idHelper.channelType(digitId);
          
          if(digitChType == ReadoutChannelType::Strip) {
            digitTime += CLHEP::RandGaussZiggurat::shoot(rndEngine, 0, m_timeJitterElectronicsStrip);
          }
          else {
            digitTime += CLHEP::RandGaussZiggurat::shoot(rndEngine, 0, m_timeJitterElectronicsPad);
          }

          uint16_t digitBCTag = bcTagging(digitTime + bunchTime);
          digitTime += m_doToFCorrection ? bunchTime : globalHitTime;

          double digitCharge = digit->charge();
          // Create a new digit with updated time and BCTag
          int eventId = hit.eventId();
          bool isDead{false}, isPileup{eventId != 0};
          ATH_MSG_VERBOSE("Hit is from the main signal subevent if eventId is zero, eventId = "
                            << eventId << " newDigit time: " << digitTime);  
          auto newDigitPtr = std::make_unique<sTgcDigit>(digitId, digitBCTag, digitTime, digitCharge, isDead, isPileup);
          if (digitChType == ReadoutChannelType::Strip) {
            ATH_MSG_VERBOSE("Finalizing Digit "<<m_idHelperSvc->toString(digitId)
                        <<" BC tag = "    << newDigitPtr->bcTag() 
                        <<" digitTime = " << newDigitPtr->time()
                        <<" charge = "    << newDigitPtr->charge());
          }
          simDigitsByChType[digitChType].emplace_back(hit, std::move(newDigitPtr));
        }
      }
      SdoIdMap_t sdoIdMap{};
      sTgcDigitCollection* outColl = fetchCollection(viewer.at(0)->identify(), digitCache);
   
      ATH_CHECK(processDigitsWithVMM(ctx, digiCond, std::move(simDigitsByChType[ReadoutChannelType::Strip]), 
                                      m_deadtimeStrip, m_doNeighborOn, *outColl, sdoIdMap));
      ATH_CHECK(processDigitsWithVMM(ctx, digiCond, std::move(simDigitsByChType[ReadoutChannelType::Pad]), 
                                      m_deadtimePad, false, *outColl, sdoIdMap));
      ATH_CHECK(processDigitsWithVMM(ctx, digiCond, std::move(simDigitsByChType[ReadoutChannelType::Wire]), 
                                      m_deadtimeWire, false, *outColl, sdoIdMap));
      for (auto& [simHit, assocIds]: sdoIdMap) {
          /// Add the only the hits and digits that pass VMM simulation to sdo container
          xAOD::MuonSimHit* sdoHit = addSDO(simHit, sdoContainer);
          if (!sdoHit) {
             continue;
          }
          std::ranges::sort(assocIds, [&](const Identifier& a, const Identifier& b){
               const int typeA = idHelper.channelType(a);
               const int typeB = idHelper.channelType(b);
               if (typeA != typeB) {
                  if (typeA == sTgcIdHelper::sTgcChannelTypes::Strip) {
                      return true;
                  } else if (typeB == sTgcIdHelper::sTgcChannelTypes::Strip) {
                      return false;
                  }
                  return typeA > typeB;
               }
               return idHelper.channel(a) < idHelper.channel(b);
          });
          const double globalHitTime = sdoHit->globalTime() + simHit.eventTime();
          sdoHit->setGlobalTime(globalHitTime);

          sdoHit->setIdentifier(assocIds.front());
          assocIds.erase(assocIds.begin());

          using ChVec_t = std::vector<std::uint16_t>;
          static const SG::Decorator<ChVec_t> dec_stripCh{"sTgc_stripChannels"};
          static const SG::Decorator<ChVec_t> dec_wireCh{"sTgc_wireChannels"};
          static const SG::Decorator<ChVec_t> dec_padCh{"sTgc_padChannels"};
          ChVec_t& stripCh{dec_stripCh(*sdoHit)}, wireCh{dec_wireCh(*sdoHit)}, padCh{dec_padCh(*sdoHit)};

          std::ranges::for_each(assocIds,[&](const Identifier& secId){
              const int ch = idHelper.channel(secId);
              switch(idHelper.channelType(secId)){
                 using enum sTgcIdHelper::sTgcChannelTypes;
                 case Strip: {
                    stripCh.push_back(ch);
                    break;
                 }case Wire: {
                    wireCh.push_back(ch);
                    break;
                 }case Pad: {
                    padCh.push_back(ch);
                    break;
                 }
              }
          });


      }
    } while (viewer.next());
    /// Digits are sorted by 
    ATH_CHECK(writeDigitContainer(ctx, m_writeKey, std::move(digitCache), idHelper.module_hash_max()));
    
    return StatusCode::SUCCESS;
  }

  StatusCode sTgcDigitizationTool::processDigitsWithVMM(const EventContext& ctx,
                                                        const DigiConditions& digiCond,
                                                        sTgcSimDigitVec&& digitsInChamber,
                                                        const double vmmDeadTime,
                                                        const bool isNeighbourOn,
                                                        sTgcDigitCollection& outColl,
                                                        SdoIdMap_t& sdoIdMap) const {
    
    if (digitsInChamber.empty()) {
      ATH_MSG_DEBUG("Empty hits from VMM Simulation" );
      return StatusCode::SUCCESS;
    }
    const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
    
    /// Sort all digits from the same chamber according to layer->channelType->time 
    sTgcSimDigitVec mergedDigits = mergeDigitsVMM(ctx, digiCond, vmmDeadTime, 
                                                  isNeighbourOn, std::move(digitsInChamber));
    /// Update the container iterator to go to the next chamber
    if (mergedDigits.empty()) {
      return StatusCode::SUCCESS;
    }

    for (sTgcSimDigitHit& merged : mergedDigits) {
      
      /// apply the smearing before adding the digit
      bool acceptDigit{true};
      float chargeAfterSmearing = merged.getDigit().charge();
      if (m_doSmearing) {
          ATH_CHECK(m_smearingTool->smearCharge(merged.identify(), chargeAfterSmearing, acceptDigit, 
                                                digiCond.rndEngine));
      }
      if (!acceptDigit) {
          continue;
      }
      /// Select strips with charge > 0.001 pC to avoid having zero ADC count when converting
      /// charge [pC] to PDO [ADC count]
      if (idHelper.channelType(merged.identify()) == ReadoutChannelType::Strip &&
          chargeAfterSmearing < 0.001) {
          continue;
      }
      auto finalDigit = merged.releaseDigit();
      if (m_doSmearing) {
          finalDigit->set_charge(chargeAfterSmearing);
      }
      ATH_MSG_VERBOSE("Final Digit "<<m_idHelperSvc->toString(finalDigit->identify())<<
                      " BC tag = "    << finalDigit->bcTag()<<
                      " digitTime = " << finalDigit->time() <<
                      " charge = "    << finalDigit->charge());

      /// Add the only the hits and digits that pass VMM simulation to sdo container
      sdoIdMap[merged.getSimHit()].push_back(finalDigit->identify());
      /// Add the VMM processed digit to cache
      outColl.push_back(std::move(finalDigit)); 
    }    
    return StatusCode::SUCCESS; 
  }

  sTgcDigitizationTool::sTgcSimDigitVec 
      sTgcDigitizationTool::mergeDigitsVMM(const EventContext& ctx,
                                           const DigiConditions& digiCond, 
                                           const double vmmDeadTime, 
                                           const bool isNeighbourOn,    
                                           sTgcSimDigitVec&& unmergedDigits) const {

    const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
    /// Sort Digits in the unmergedDigits vector by gasgap -> channelType -> time
    std::ranges::stable_sort(unmergedDigits,
      [&idHelper](const sTgcSimDigitHit& a, const sTgcSimDigitHit& b) {
        const int layA = idHelper.gasGap(a.identify()); 
        const int layB = idHelper.gasGap(b.identify());
        if (layA != layB) {
          return layA < layB;
        }
        const int chA = idHelper.channel(a.identify());
        const int chB = idHelper.channel(b.identify());
        if (chA != chB) {
          return chA < chB;
        }
        return a.time() < b.time();
      }
    );
    sTgcSimDigitVec savedDigits{}, premerged{};
    
    premerged.reserve(unmergedDigits.size());
    savedDigits.reserve(premerged.capacity());

    auto passNeigbourLogic = [&](const sTgcSimDigitHit& candidate) {
      if (!isNeighbourOn || savedDigits.empty()) {
        return false;
      }
      if (savedDigits.back().identify() == candidate.identify() &&
          std::abs(savedDigits.back().time() - candidate.time()) < vmmDeadTime) {
            ATH_MSG_VERBOSE("Digits are too close in time ");
            return false;
      }
      const Identifier digitId = candidate.identify();
      const int channel = idHelper.channel(digitId);
      const MuonGMR4::sTgcReadoutElement* reEle = m_detMgr->getsTgcReadoutElement(digitId);
      const IdentifierHash hitHash = reEle->measurementHash(digitId);
      const int maxChannel = reEle->numChannels(hitHash);
      for (int neighbour : {std::max(1, channel -1), std::min(maxChannel, channel+1)}) {
        /// Catch the cases where the channel is 1 or maxChannel
        if (neighbour == channel) {
          continue;
        }
        const Identifier neighbourId = idHelper.channelID(digitId, 
                                                          reEle->multilayer(),
                                                          idHelper.gasGap(digitId), 
                                                          idHelper.channelType(digitId), neighbour);
        const double threshold = m_useCondThresholds ? getChannelThreshold(ctx, neighbourId, *digiCond.thresholdData)  
                                                      : m_chargeThreshold.value();          
        if (std::ranges::any_of(savedDigits, [&](const sTgcSimDigitHit& known){
            return known.identify() == neighbourId && 
                    known.getDigit().charge() > threshold &&
                    std::abs(known.time() - candidate.time()) <  m_hitTimeMergeThreshold;
        })) {
          return true;
        }
      }
      return false;
    };
    // Sort digits on every channel by earliest to latest time
    // Also do hit merging to help with neighborOn logic
    double threshold = m_chargeThreshold;
    for (sTgcSimDigitVec::iterator merge_me = unmergedDigits.begin(); merge_me!= unmergedDigits.end(); ++merge_me) {
      if(m_useCondThresholds) {
        threshold = getChannelThreshold(ctx, (*merge_me).identify(), *digiCond.thresholdData);
      }
      /// merge digits in time. Do weighted average to find time of
      /// digits originally below threshold. Follows what we expect from real VMM.
      sTgcDigit& digit1{(*merge_me).getDigit()};
      double totalCharge = digit1.charge();
      double weightedTime = digit1.time();
        
      sTgcSimDigitVec::iterator merge_with = merge_me + 1;
      for ( ; merge_with!= unmergedDigits.end(); ++merge_with) {
        /// We reached another digit. No need to merge
        if ((*merge_with).identify() != (*merge_me).identify()) {
            break;
        }
        const sTgcDigit& mergeDigit{(*merge_with).getDigit()};
        // If future digits are within window, digit1 absorbs its charge
        if (mergeDigit.time() - digit1.time() > m_hitTimeMergeThreshold) break;
        // If digit1 is not above threshold prior to merging, the new time is
        // a weighted average. Do it for every merging pair.
        if (totalCharge < threshold) {
          weightedTime = (weightedTime * totalCharge + mergeDigit.time() * mergeDigit.charge())
                        / (totalCharge + mergeDigit.charge());
        }
        totalCharge += mergeDigit.charge();
      }
      digit1.set_charge(totalCharge);
      digit1.set_time(weightedTime);
      sTgcSimDigitHit& mergedHit{*merge_me};
      if (!savedDigits.empty() && 
          savedDigits.back().identify() == digit1.identify() &&
          std::abs(savedDigits.back().time() - digit1.time()) <= vmmDeadTime) continue;
      if (digit1.charge() > threshold || passNeigbourLogic(mergedHit)){
          savedDigits.emplace_back(std::move(mergedHit));
      } else if (isNeighbourOn) {
          premerged.emplace_back(std::move(mergedHit));
      }    
    } // end of time-ordering and hit merging loop
    std::copy_if(std::make_move_iterator(premerged.begin()),
                 std::make_move_iterator(premerged.end()),
                 std::back_inserter(savedDigits), passNeigbourLogic);
    return savedDigits;
  }
}