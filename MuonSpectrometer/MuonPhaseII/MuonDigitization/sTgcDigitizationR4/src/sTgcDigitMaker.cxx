/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "sTgcDigitizationR4/sTgcDigitMaker.h"
#include "PathResolver/PathResolver.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandGamma.h"
#include "AthenaBaseComps/AthCheckMacros.h"

#include "Acts/Utilities/detail/Polynomials.hpp"

#include "TF1.h"
#include <cmath>
#include <iostream>
#include <fstream>

namespace MuonR4{
//---------------------------------------------------
//  Constructor and Destructor
//---------------------------------------------------
using sTgcDigitVec = sTgcDigitMaker::sTgcDigitVec;

sTgcDigitMaker::sTgcDigitMaker(const MuonGMR4::MuonDetectorManager* detMgr,
                               digitMode mode,
                               double meanGasGain,
                               bool doPadChargeSharing)
  : AthMessaging("sTgcDigitMaker"),
    m_detMgr{detMgr},
    m_digitMode(mode),
    m_meanGasGain{meanGasGain},
    m_doPadSharing{doPadChargeSharing}{}

sTgcDigitMaker::~sTgcDigitMaker() = default;

//------------------------------------------------------
// Initialize digitization parameters
//------------------------------------------------------
StatusCode sTgcDigitMaker::initialize() {
  // Read arrival time data
  ATH_CHECK(readFileOfTimeArrival());

  // Read strip time correction if enabled
  if (m_doTimeOffsetStrip) {
    ATH_CHECK(readFileOfTimeOffsetStrip());
  }

  return StatusCode::SUCCESS;
}

//------------------------------------------------------
// Compute ionization point for a hit
//------------------------------------------------------
bool sTgcDigitMaker::getIonizationPoint(const TimedHit& hit, 
                                        const DigiConditions& condContainers, 
                                        Ionization& ionization) const {
  const MuonGMR4::sTgcReadoutElement* reEle = m_detMgr->getsTgcReadoutElement(hit->identify());

  // Projecting the hit position on wire surface
  const Amg::Vector3D locHitDir = xAOD::toEigen(hit->localDirection());
  const Amg::Vector3D locHitPos = xAOD::toEigen(hit->localPosition());
  ATH_MSG_VERBOSE("sTgc hit:  time " << hit->globalTime() 
              << " position " << Amg::toString(locHitPos, 2) << " direction" << Amg::toString(locHitDir, 2) 
              << " mclink " << hit->genParticleLink() << " PDG ID " << hit->pdgId() );
  
  const double scale = Amg::intersect<3>(locHitPos, locHitDir, Amg::Vector3D::UnitZ(), 0.).value_or(0);
  Amg::Vector3D hitOnWireSurf = locHitPos + scale * locHitDir;

  const Identifier hitId = hit->identify();
  const IdentifierHash wireLayHash = reEle->createHash(m_idHelper.gasGap(hitId),
                                                       ReadoutChannelType::Wire, 1);
  const MuonGMR4::WireGroupDesign& wireDesign{reEle->wireDesign(wireLayHash)};
  const MuonGMR4::StripLayer& stripLayer = reEle->stripLayer(wireLayHash);
  const Amg::Vector2D hitOnWire2D = stripLayer.to2D(hitOnWireSurf, true);
  
  if(!wireDesign.insideTrapezoid(hitOnWire2D)) {
    return false;
  }

  std::pair<int, int> wireGrpWireNum = wireDesign.wireNumber(hitOnWire2D);
  if (wireGrpWireNum.first < 0) {
    ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Unable to retrieve the wire number, skipping the hit: " 
                 << m_idHelperSvc->toString(hitId)<<" @"<<Amg::toString(hitOnWire2D));
    return false;  
  }
  int wireNumber = wireDesign.numPitchesToGroup(wireGrpWireNum.first) + wireGrpWireNum.second;
  const int numWires = wireDesign.nAllWires();
  
  if((wireNumber < 1) || (wireNumber > numWires)) {
    ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Unable to retrieve the wire number, skipping the hit: " 
                 << m_idHelperSvc->toString(hitId)<<" @"<<Amg::toString(hitOnWire2D));
    return false;  
  }

  // Compute the position of the ionization and its distance to the closest wire
  ionization = pointClosestApproach(stripLayer, wireNumber, locHitPos, locHitDir, hit->stepLength());
  double distToWire = ionization.distance;
  
  if(distToWire > 0.) {
    // Determine on which side of the wire does the particle cross
    int adjacent = Acts::copySign(1, ionization.posOnSegment.y() - ionization.posOnWire.y());
    
    Ionization ionizationAdj = pointClosestApproach(stripLayer, wireNumber+adjacent, locHitPos, locHitDir, hit->stepLength());
    double distToWireAdj = ionizationAdj.distance;
    
    if ((distToWireAdj > 0.) && (distToWireAdj < distToWire)) {
      distToWire = distToWireAdj;
      wireNumber += adjacent;
      ionization = std::move(ionizationAdj);
    }
  } else {
    ATH_MSG_DEBUG("Failed to get the distance between the wire and hit");
    return false;
  }

  // Do not digitize hits that are too far from the nearest wire
  if (distToWire > wireDesign.stripPitch()) {
    return false;
  }

  // Get the gamma pdf parameters and calculate digit time
  const GammaParameter gamParam = getGammaParameter(distToWire);
  const double most_prob_time = getMostProbableArrivalTime(distToWire);
  const double gamma_mpv = std::max((gamParam.kParameter - 1) * gamParam.thetaParameter, 0.);
  const double t0_par = most_prob_time - gamma_mpv;
  const double inv_theta = 1./gamParam.thetaParameter;

  double digitTime = t0_par + CLHEP::RandGamma::shoot(condContainers.rndEngine, gamParam.kParameter, inv_theta);

  constexpr unsigned shoot_limit = 4;
  unsigned shoot_counter = 0;
  while (digitTime < 0. && ++shoot_counter <= shoot_limit) {
    digitTime = t0_par + CLHEP::RandGamma::shoot(condContainers.rndEngine, gamParam.kParameter,inv_theta);
  }
  
  ionization.time = std::max(0., digitTime);
  return true;
}

//------------------------------------------------------
// Calculate total charge from energy deposit
//------------------------------------------------------
double sTgcDigitMaker::calculateTotalCharge(double energyDeposit, CLHEP::HepRandomEngine* rndEngine) const {
  // Ionized charge in pC per keV deposited
  const double ionized_charge = (5.65E-6) * energyDeposit / CLHEP::keV;

  // Calculate avalanche gain using gamma distribution (Polya function approximation)
  const double gain = CLHEP::RandGamma::shoot(rndEngine, 1. + m_theta, (1. + m_theta) / m_meanGasGain);

  return gain * ionized_charge;
}

//------------------------------------------------------
// Execute digitization for a given hit
//------------------------------------------------------
sTgcDigitVec sTgcDigitMaker::executeDigi(const DigiConditions& condContainers, 
                                         const TimedHit& hit) const {
  sTgcDigitVec allDigits{};
  // Extract energy deposit from the hit
  const double energyDeposit = hit->energyDeposit();
  if (energyDeposit < std::numeric_limits<float>::epsilon()) {
    return allDigits;  // Ignore hits with no energy deposit
  }

   // Retrieve the detector element for the given hit
  const Identifier hitId = hit->identify();
 
  // HV efficiency correction
  if (condContainers.efficiencies) {
    const double efficiency = condContainers.efficiencies->getEfficiency(hitId);
    if (CLHEP::RandFlat::shoot(condContainers.rndEngine,0.0,1.0) > efficiency) {
      return allDigits;
    }
  }

  
  ATH_MSG_DEBUG("Retrieving detector element for: "<< m_idHelperSvc->toStringDetEl(hitId) 
              << " energyDeposit "<< energyDeposit );
  
  
  // Get ionization point and time
  Ionization ionization{};
  if (!getIonizationPoint(hit, condContainers, ionization)) {
    ATH_MSG_DEBUG("Failed to get ionization point for hit "<< m_idHelperSvc->toStringDetEl(hitId));
    return allDigits;
  }

  DigiInput digiInput{};
  digiInput.hitId = hitId;
  digiInput.totalCharge = calculateTotalCharge(energyDeposit, condContainers.rndEngine);
  digiInput.time = ionization.time;
  digiInput.posOnSurf = ionization.posOnWire;
  digiInput.hitDir = xAOD::toEigen(hit->localDirection());
  digiInput.reEle = m_detMgr->getsTgcReadoutElement(hitId);
  
  
  //##################################################################################
  //######################################### strip readout ##########################
  //##################################################################################
  sTgcDigitVec stripDigits = processStripDigitization(condContainers, digiInput);

  if(m_digitMode == digitMode::StripsOnly) {
    ATH_MSG_WARNING("Only digitize strip response !");
    return stripDigits;
  }
  allDigits.insert(allDigits.end(),
                   std::make_move_iterator(stripDigits.begin()), 
                   std::make_move_iterator(stripDigits.end()));

  //##################################################################################
  //######################################### pad readout ############################
  //##################################################################################
  sTgcDigitVec padDigits = processPadDigitization(digiInput);

  allDigits.insert(allDigits.end(), 
                   std::make_move_iterator(padDigits.begin()),
                   std::make_move_iterator(padDigits.end()));

  if(m_digitMode == digitMode::StripsAndPads) {
    ATH_MSG_WARNING("Only digitize strip/pad response !");
    return allDigits;
  }

  //##################################################################################
  //######################################### wire readout ###########################
  //##################################################################################
  sTgcDigitVec wireDigits = processWireDigitization(digiInput);
  allDigits.insert(allDigits.end(), 
                   std::make_move_iterator(wireDigits.begin()),
                   std::make_move_iterator(wireDigits.end()));

  return allDigits;
}

//------------------------------------------------------
// Process strip digitization
//------------------------------------------------------
sTgcDigitVec sTgcDigitMaker::processStripDigitization(const DigiConditions& condContainers,
                                                      const DigiInput& digiInput) const {
  ATH_MSG_DEBUG("sTgcDigitMaker::strip response ");
  
  const int gasGap = m_idHelper.gasGap(digiInput.hitId);
  const IdentifierHash stripLayHash = digiInput.reEle->createHash(gasGap,
                                                                  ReadoutChannelType::Strip, 1);
  const Identifier stripLayId = digiInput.reEle->measurementId(stripLayHash);
  const MuonGMR4::StripDesign& stripDesign{digiInput.reEle->stripDesign(stripLayHash)};  
  const Amg::Vector2D hitOnStripSurf = digiInput.reEle->stripLayer(stripLayHash).to2D(digiInput.posOnSurf, false);
  
  if(!stripDesign.insideTrapezoid(hitOnStripSurf)) {
    ATH_MSG_DEBUG("Outside of the strip surface boundary : " 
               << m_idHelperSvc->toString(stripLayId) 
               << "; local position " <<Amg::toString(hitOnStripSurf, 2));
    return {};
  }

  constexpr double tolerance_length = 0.01*Gaudi::Units::mm;
  int stripNumber = stripDesign.stripNumber(hitOnStripSurf);
  if( stripNumber < 0){
    const double newPosX = hitOnStripSurf.x() - std::copysign(tolerance_length, hitOnStripSurf.x());
    const Amg::Vector2D newPos(newPosX, hitOnStripSurf.y());
    stripNumber = stripDesign.stripNumber(newPos);
    if (stripNumber < 0) {
      ATH_MSG_WARNING("Failed to obtain strip number " << m_idHelperSvc->toString(stripLayId) );
      return {};
    }
  }
  
  
  const double tan_theta = digiInput.hitDir.perp() / digiInput.hitDir.z();
  const double angle_dependency = std::hypot(m_posResIncident, m_posResAngular * tan_theta);

  const double peak_position = CLHEP::RandGaussZiggurat::shoot(condContainers.rndEngine, 
                                                               hitOnStripSurf.x(), m_StripResolution*angle_dependency);
  ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<" Smeared hit from "<<hitOnStripSurf.x()<<" -> "<<
                peak_position<<". Assign strip charges");
  return processStripChargeSharing(digiInput,
                                   peak_position,
                                   stripNumber);
}
//------------------------------------------------------
// Process strip charge sharing across neighboring strips
//------------------------------------------------------
sTgcDigitVec sTgcDigitMaker::processStripChargeSharing(const DigiInput& digiInput,
                                                       const double peak_position,
                                                       const int stripNumber) const {
  
  const double norm = 0.5 * digiInput.totalCharge;

  sTgcDigitVec digits{};
  const double tan_theta = digiInput.hitDir.perp() / digiInput.hitDir.z();
  
  constexpr double tolerance_charge = 0.0005;
  constexpr int max_neighbor = 10;

  const int gasGap = m_idHelper.gasGap(digiInput.hitId);
  const auto& design = digiInput.reEle->stripDesign(digiInput.reEle->measurementHash(digiInput.hitId));
  // Upper half of the strip cluster
  for (int iStrip = 0; iStrip <= max_neighbor; ++iStrip) {
    for (int sign : {-1 , 1}) {
      int currentStrip = stripNumber + sign*iStrip;
      if (currentStrip > design.numStrips() || currentStrip < 1) {
        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<" Breaking the upper half strip loop stripNumber: " 
                        << currentStrip << " > " << design.numStrips());
        break;
      }
      bool isValid = false;
      const Identifier currentStripId = m_idHelper.channelID(digiInput.hitId, 
                                                             digiInput.reEle->multilayer(), 
                                                             gasGap, ReadoutChannelType::Strip,
                                                             currentStrip, isValid);
      if (!isValid) {
          continue;
      }
      const Amg::Vector2D currentStripPos = digiInput.reEle->localChannelPosition(digiInput.reEle->measurementHash(currentStripId));
      const double x_relative = (currentStripPos.x() - peak_position);
      const double normX = x_relative / design.stripPitch();
      const double charge = std::hypot(1., m_chargeAngularFactor * tan_theta) * norm * 
                            chargeIntegral(normX - 0.5, normX + 0.5);
      
      if (charge < tolerance_charge) {
        ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<" Breaking the upper half strip loop stripCharge: " 
                        << charge << " < " << tolerance_charge);
        break;
      }

      double strip_time = digiInput.time;
      if (m_doTimeOffsetStrip) {
        const bool shift = ((iStrip > 0) && ((x_relative + (0.5*0.75 - iStrip) * design.stripPitch()) < 0));
        strip_time += getTimeOffsetStrip(iStrip -  shift);
      }

      addDigit(digits, currentStripId, strip_time, charge);
      ATH_MSG_VERBOSE(__func__<<"() - "<<__LINE__<<" Created a strip digit: strip number = " 
                    << currentStrip << ", charge = " << charge);
    }
  }

  return digits;
}


//------------------------------------------------------
// Process pad digitization
//------------------------------------------------------
sTgcDigitVec sTgcDigitMaker::processPadDigitization(const DigiInput& digiInput) const {
  ATH_MSG_DEBUG("sTgcDigitMaker::pad response ");
  
  const int gasGap = m_idHelper.gasGap(digiInput.hitId);
  const IdentifierHash padLayHash = digiInput.reEle->createHash(gasGap,ReadoutChannelType::Pad, 1);
  const Amg::Vector2D hitOnPadSurf = digiInput.reEle->stripLayer(padLayHash).to2D(digiInput.posOnSurf, true);
  const MuonGMR4::PadDesign& padDesign{digiInput.reEle->padDesign(padLayHash)};
  if(!padDesign.insideTrapezoid(hitOnPadSurf)) {
    ATH_MSG_DEBUG(__func__<<"() -"<<__LINE__<<" Outside of the pad surface boundary :" 
                  << m_idHelperSvc->toString(digiInput.hitId)
                 << " local position " <<Amg::toString(hitOnPadSurf, 2));
    return {};
  }

  constexpr double tolerance_length = 0.01*Gaudi::Units::mm;
  auto [padEta, padPhi] = padDesign.channelNumber(hitOnPadSurf);
  
  if( padEta < 1 || padPhi < 1){
    const double newPosX = hitOnPadSurf.x() - std::copysign(tolerance_length, hitOnPadSurf.x());
    const double newPosY = hitOnPadSurf.y() - std::copysign(tolerance_length, hitOnPadSurf.y());
    const Amg::Vector2D newPosition(newPosX, newPosY);
    auto [newPadEta, newPadPhi] = padDesign.channelNumber(newPosition);
    padEta = newPadEta;
    padPhi = newPadPhi;
    if( padEta < 1 || padPhi < 1) {
      ATH_MSG_WARNING("Failed to obtain pad number for " << m_idHelperSvc->toString(digiInput.hitId) );
      return {};
    }
  }
  
  return processPadChargeSharing(digiInput, padEta, padPhi);
}

//------------------------------------------------------
// Process pad charge sharing
//------------------------------------------------------
sTgcDigitVec sTgcDigitMaker::processPadChargeSharing(const DigiInput& digiInput,
                                                     const int padEta,
                                                     const int padPhi) const {
  
  bool isValid = false;
  const int gasGap = m_idHelper.gasGap(digiInput.hitId);
  const Identifier padHitId = m_idHelper.padID(digiInput.hitId, 
                                              digiInput.reEle->multilayer(), 
                                              gasGap, ReadoutChannelType::Pad, padEta, padPhi, isValid);
  if(!isValid) {
    return {};
  }
  sTgcDigitVec digits{};
  
  const IdentifierHash padHitHash = digiInput.reEle->measurementHash(padHitId);
  const MuonGMR4::PadDesign& padDesign{digiInput.reEle->padDesign(padHitHash)};
 
  const Amg::Vector2D padPos = digiInput.reEle->localChannelPosition(padHitHash);
  const Amg::Vector2D hitOnPadSurf = digiInput.reEle->stripLayer(padHitHash).to2D(digiInput.posOnSurf, true);
  
  const Amg::Vector2D diff = hitOnPadSurf - padPos;
  const double halfPadHeight = 0.5 * digiInput.reEle->padHeight(padHitHash);
  
  const std::array<Amg::Vector2D, 4> padHitCorners = padDesign.padCorners(std::make_pair(padEta, padPhi));
  const double padBottomBase = (padHitCorners[0] - padHitCorners[1]).norm();
  const double padTopBase = (padHitCorners[2] - padHitCorners[3]).norm();
  const double halfPadWidth = 0.5 * padBottomBase + 0.25 * (1 + diff.y()/halfPadHeight) * (padTopBase - padBottomBase);

  double deltaX = halfPadWidth - std::abs(diff.x());
  double deltaY = halfPadHeight - std::abs(diff.y());
  const bool isNeighX = deltaX < 2.5*m_clusterParams[1];
  const bool isNeighY = deltaY < 2.5*m_clusterParams[1];
  
  if (deltaX < 0.) { 
    deltaX = 0.1; 
  }
  if (deltaY < 0.) { 
    deltaY = 0.1; 
  }

  if (m_doPadSharing && (isNeighX || isNeighY)) {
    unsigned newPhi = padPhi - Acts::copySign(1, diff.x());
    unsigned newEta = padEta  + Acts::copySign(1, diff.y());
    bool validEta = newEta > 0 && newEta <= digiInput.reEle->numPadEta(padHitHash);
    bool validPhi = newPhi > 0 && newPhi <= digiInput.reEle->numPadPhi(padHitHash);

    if (isNeighX && isNeighY && validEta && validPhi) {
      const Identifier neigh_ID_X = m_idHelper.padID(padHitId, digiInput.reEle->multilayer(), 
                                                     gasGap, ReadoutChannelType::Pad, 
                                                     padEta, newPhi);
      const Identifier neigh_ID_Y = m_idHelper.padID(padHitId, digiInput.reEle->multilayer(), gasGap,
                                                     ReadoutChannelType::Pad, newEta, padPhi);
      const Identifier neigh_ID_XY = m_idHelper.padID(padHitId, digiInput.reEle->multilayer(), gasGap,
                                                      ReadoutChannelType::Pad, newEta, newPhi);
      double xQfraction = getPadChargeFraction(deltaX);
      double yQfraction = getPadChargeFraction(deltaY);

      addDigit(digits, neigh_ID_X, digiInput.time, xQfraction*(1.-yQfraction)*0.5*digiInput.totalCharge);
      addDigit(digits, neigh_ID_Y, digiInput.time, yQfraction*(1.-xQfraction)*0.5*digiInput.totalCharge);
      addDigit(digits, neigh_ID_XY, digiInput.time, xQfraction*yQfraction*0.5*digiInput.totalCharge);
      addDigit(digits, padHitId, digiInput.time, (1.-xQfraction-yQfraction+xQfraction*yQfraction)*0.5*digiInput.totalCharge);
    } else if (isNeighX && validPhi){
      const Identifier neigh_ID = m_idHelper.padID(padHitId, digiInput.reEle->multilayer(), gasGap, 
                                                   ReadoutChannelType::Pad, padEta, newPhi);
      double xQfraction = getPadChargeFraction(deltaX);
      addDigit(digits, padHitId, digiInput.time, (1.-xQfraction)*0.5*digiInput.totalCharge);
      addDigit(digits, neigh_ID, digiInput.time, xQfraction*0.5*digiInput.totalCharge);
    }
    else if (isNeighY && validEta){
      const Identifier neigh_ID = m_idHelper.padID(padHitId, digiInput.reEle->multilayer(), gasGap, 
                                                   ReadoutChannelType::Pad, newEta, padPhi);
      double yQfraction = getPadChargeFraction(deltaY);
      addDigit(digits, padHitId, digiInput.time, (1.-yQfraction)*0.5*digiInput.totalCharge);
      addDigit(digits, neigh_ID, digiInput.time, yQfraction*0.5*digiInput.totalCharge);
    }
  } else{
    addDigit(digits, padHitId, digiInput.time, 0.5*digiInput.totalCharge);
  }
  return digits;
}


//------------------------------------------------------
// Process wire digitization
//------------------------------------------------------
sTgcDigitVec sTgcDigitMaker::processWireDigitization(const DigiInput& digiInput) const {
  ATH_MSG_DEBUG("sTgcDigitMaker::wire response ");
  
  const int gasGap = m_idHelper.gasGap(digiInput.hitId);
  const IdentifierHash wireLayHash = digiInput.reEle->createHash(gasGap,
                                                                ReadoutChannelType::Wire, 1);
  const MuonGMR4::WireGroupDesign& wireDesign{digiInput.reEle->wireDesign(wireLayHash)};
  
  const Amg::Vector2D hitOnWireSurf = digiInput.reEle->stripLayer(wireLayHash).to2D(digiInput.posOnSurf, true);
  if(!wireDesign.insideTrapezoid(hitOnWireSurf)) {
    ATH_MSG_DEBUG("Outside of the wire surface boundary :" << m_idHelperSvc->toString(digiInput.hitId)
                 << " local position " <<Amg::toString(hitOnWireSurf, 2));
    return {};
  }

  constexpr double tolerance_length = 0.01*Gaudi::Units::mm;
  int wiregroupNumber = (wireDesign.wireNumber(hitOnWireSurf)).first;
  if( wiregroupNumber < 1) {
    const double newPosX = hitOnWireSurf.x() - std::copysign(tolerance_length, hitOnWireSurf.x());
    const Amg::Vector2D newPos{newPosX, hitOnWireSurf.y()};
    wiregroupNumber = (wireDesign.wireNumber(newPos)).first;
    if (wiregroupNumber < 1) {
      ATH_MSG_WARNING("Failed to obtain wire number " << m_idHelperSvc->toString(digiInput.hitId) );
      return {};
    }
  }

  bool isValid = false;
  const Identifier wireGroupHitId = m_idHelper.channelID(digiInput.hitId, digiInput.reEle->multilayer(), gasGap, 
                                                         ReadoutChannelType::Wire, wiregroupNumber, isValid);

  if(!isValid) {
    return {};
  }
  sTgcDigitVec digits{};
  addDigit(digits, wireGroupHitId, digiInput.time, digiInput.totalCharge);
  return digits;
}

//------------------------------------------------------
// Compute closest approach between hit trajectory and wire
//------------------------------------------------------
sTgcDigitMaker::Ionization sTgcDigitMaker::pointClosestApproach(const MuonGMR4::StripLayer& stripLayer,
                                                                int wireNumber, 
                                                                const Amg::Vector3D& locHitPos,
                                                                const Amg::Vector3D& locHitDir,
                                                                const double stepLength) const {

  constexpr double angular_tolerance = 1e-3;
  // Position of the ionization
  Ionization ionization;
  // Finding smallest distance and the points at the smallest distance.
  //  The smallest distance between two lines is perpendicular to both lines.
  // Previous logic was to find the perpendicular distance between two lines using projection geometry
  //  We can construct two lines in the wire surface local coordinate frame:
  //  - one for the hit segment with equation h0 + t * v_h, where h0 is a point
  //    and v_h is the unit vector of the hit segment
  //  - another for the wire with similar equation w0 + s * v_w, where w0 is a
  //    point and v_w is the unit vector of the wire line
  //  Then it is possible to determine the closest points on each line
  //  by requiring that the vector between them is perpendicular to both:
  //   1. (h0 + t*v_h - w0 - s*v_w) · v_h = 0
  //   2. (h0 + t*v_h - w0 - s*v_w) · v_w = 0

  // We have replaced this logic with using Amg::intersect<3> method
  const MuonGMR4::WireGroupDesign& wireDesign = static_cast<const MuonGMR4::WireGroupDesign&>(stripLayer.design(true));
  // Geometry setup defined in the eta surface local coordinate frame
  const double wirePitch = wireDesign.stripPitch();
  const double wirePosX = wireDesign.firstStripPos().x() + (wireNumber - 1) * wirePitch;
  const Amg::Vector3D wireDir{stripLayer.to3D(wireDesign.stripDir(),true)};
  const Amg::Vector3D wirePos(locHitPos.x(), wirePosX, 0.);

  // Use Amg::intersect to find closest point on hit segment to wire plane
  std::optional<double> scaleHit = Amg::intersect<3>(locHitPos, locHitDir, Amg::Vector3D::UnitZ(), 0);
  if (!scaleHit || std::abs(std::abs(wireDir.dot(locHitDir)) - 1.0) < angular_tolerance) {
    ATH_MSG_DEBUG("The track segment is parallel to the wire, position of digit is undefined");
    ionization.posOnSegment = locHitPos;
    ionization.posOnWire = wirePos;
    ionization.distance = std::hypot(locHitPos.y() - wirePosX, locHitPos.z());
    return ionization;
  }
  // Position on hit segment
  Amg::Vector3D ionizationPos = locHitPos + scaleHit.value() * locHitDir;

  if (scaleHit.value() > stepLength) {
    ionization.posOnSegment = locHitPos;
    const Amg::Vector3D closestPointToWirePlane = locHitPos + stepLength * locHitDir;
    ionization.posOnWire = wirePos;
    ionization.distance = std::abs(closestPointToWirePlane.z());
    return ionization;
  }

  // Project ionization position onto the wire line
  const double scaleWire = (ionizationPos - wirePos).dot(wireDir);
  const Amg::Vector3D closestPointOnWire = wirePos + scaleWire * wireDir;

  // Fill ionization result
  ionization.posOnSegment = ionizationPos;
  ionization.posOnWire = closestPointOnWire;
  ionization.distance = (ionizationPos - closestPointOnWire).mag();

  return ionization;
}

//------------------------------------------------------
// Adds a digit to the appropriate cache
//------------------------------------------------------
void sTgcDigitMaker::addDigit(sTgcDigitVec& digits, 
                              const Identifier& id, 
                              const double digittime,
                              const double charge) {
  
  constexpr double tolerance = 0.1;
  if (!std::ranges::any_of(digits, [&](std::unique_ptr<sTgcDigit>& known) {
      return known->identify() == id && std::abs(digittime - known->time()) < tolerance;
  })) {
    digits.push_back(std::make_unique<sTgcDigit>(id, 0, digittime, charge, 0, 0));
  }
}

//------------------------------------------------------
// Reads time arrival data file
//------------------------------------------------------
StatusCode sTgcDigitMaker::readFileOfTimeArrival() {
  const std::string file_name = "sTGC_Digitization_timeArrival.dat";
  std::string file_path = PathResolver::find_file(file_name, "DATAPATH");
  if(file_path.empty()) {
    ATH_MSG_FATAL("readFileOfTimeWindowOffset(): Could not find file " << file_name );
    return StatusCode::FAILURE;
  }

  std::ifstream ifs{file_path, std::ios::in};
  if(ifs.bad()) {
    ATH_MSG_FATAL("sTgcDigitMaker: Failed to open time of arrival file " << file_name );
    return StatusCode::FAILURE;
  }

  // Read the sTGC_Digitization_timeWindowOffset.dat file
  std::string line;
  GammaParameter param{};
  while (std::getline(ifs, line)) {
    std::string key;
    std::istringstream iss(line);
    iss >> key;
    if (key == "bin") {
      iss >> param.lowEdge >> param.kParameter >> param.thetaParameter;
      m_gammaParameter.push_back(param);
    } else if (key == "mpv")  {
      double mpt{};
      int idx{0};
      while (iss >> mpt) {m_mostProbableArrivalTime[idx++] = mpt;}
    }
  }
  ifs.close();
  return StatusCode::SUCCESS;
}

//------------------------------------------------------
// Retrieves gamma distribution parameters based on distance
//------------------------------------------------------
sTgcDigitMaker::GammaParameter sTgcDigitMaker::getGammaParameter(double distance) const {
  const double d = std::abs(distance); 
  // Find the parameters assuming the container is sorted in ascending order of 'lowEdge'
  if (d < m_gammaParameter.front().lowEdge) {
    return m_gammaParameter.front();
  }
  int index{-1};
  for (const auto& par: m_gammaParameter) {
    if (d < par.lowEdge) {
      break;
    }
    ++index;
  }
  return m_gammaParameter.at(index);
}

//------------------------------------------------------
// Computes the most probable arrival time based on the distance of closest approach
//------------------------------------------------------
double sTgcDigitMaker::getMostProbableArrivalTime(double distance) const {
  return Acts::detail::polynomialSum(std::abs(distance), m_mostProbableArrivalTime);
}
//------------------------------------------------------
// Computes the charge on each strip
//------------------------------------------------------

double sTgcDigitMaker::chargeIntegral(double N, double M) const {

    double term1 = 0.25 * std::erf( M / (std::sqrt(2) * m_clusterParams[0]));
    double term2 = 0.25 * std::erf( N / (std::sqrt(2) * m_clusterParams[0]));
    double term3 = 0.25 * std::erf( M / (std::sqrt(2) * m_clusterParams[1]));
    double term4 = 0.25 * std::erf( N / (std::sqrt(2) * m_clusterParams[1]));

    return (term1 - term2 + term3 - term4);
}
//------------------------------------------------------
// Reads strip time offset data file
//------------------------------------------------------
StatusCode sTgcDigitMaker::readFileOfTimeOffsetStrip() {
  const std::string file_name = "sTGC_Digitization_timeOffsetStrip.dat";
  std::string file_path = PathResolver::find_file(file_name, "DATAPATH");
  if(file_path.empty()) {
    ATH_MSG_FATAL("readFileOfTimeWindowOffset(): Could not find file " << file_name );
    return StatusCode::FAILURE;
  }

  // Open the sTGC_Digitization_timeOffsetStrip.dat file
  std::ifstream ifs{file_path, std::ios::in};
  if(ifs.bad()) {
    ATH_MSG_FATAL("Failed to open time of arrival file " << file_name );
    return StatusCode::FAILURE;
  }

  // Initialize the container to store the time offset.
  // The number of parameters, 6, corresponds to the number of lines to be read
  // from sTGC_Digitization_timeOffsetStrip.dat.
  // Setting the default offset to 0 ns.
  std::string line;
  size_t index{0};
  double value{0.0};
  while (std::getline(ifs, line)) {
    std::string key;
    std::istringstream iss(line);
    iss >> key;
    if (key == "strip") {
      iss >> index >> value;
      if (index >= m_timeOffsetStrip.size()) continue;
      m_timeOffsetStrip.at(index) = value;
    }
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------
// Retrieves time offset for strip clusters
//------------------------------------------------------
double sTgcDigitMaker::getTimeOffsetStrip(size_t neighbor_index) const {
    return m_timeOffsetStrip.at(std::min(neighbor_index, m_timeOffsetStrip.size() -1));
}

//------------------------------------------------------
// Computes pad charge fraction
//------------------------------------------------------
double sTgcDigitMaker::getPadChargeFraction(double distance) {
  // The charge fraction that is found past a distance x away from the
  // centre of a 2D gaussian distribution of width of cluster profile is
  // described by a modified error function.

  // The modified error function perfectly describes
  // the pad charge sharing distribution figure 16 of the sTGC
  // testbeam paper https://arxiv.org/pdf/1509.06329.pdf
  return 0.5 * (1.0 - std::erf( distance / (std::sqrt(2) * m_clusterParams[1])));
}
}
