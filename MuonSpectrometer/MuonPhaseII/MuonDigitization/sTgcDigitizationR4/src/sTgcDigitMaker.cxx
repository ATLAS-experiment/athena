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

#include "TF1.h"
#include <cmath>
#include <iostream>
#include <fstream>

//---------------------------------------------------
//  Constructor and Destructor
//---------------------------------------------------

sTgcDigitMaker::sTgcDigitMaker(const Muon::IMuonIdHelperSvc* idHelperSvc,
                               digitMode mode,
                               double meanGasGain,
                               bool doPadChargeSharing)
  : AthMessaging("sTgcDigitMaker"),
    m_idHelperSvc{idHelperSvc},
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
// Execute digitization for a given hit
//------------------------------------------------------
sTgcDigitMaker::sTgcDigitVec sTgcDigitMaker::executeDigi(const DigiConditions& condContainers, const TimedHit& hit) const {
  // Extract energy deposit from the hit
  double energyDeposit = hit->energyDeposit();
  if (energyDeposit == 0.) return {};  // Ignore hits with no energy deposit

  // Retrieve the detector element for the given hit
  const MuonGMR4::MuonDetectorManager* detMgr = condContainers.detMgr;

  // Convert hit to identifier and retrieve sTgc readout element
  const Identifier hitId = hit->identify();
  ATH_MSG_DEBUG("Retrieving detector element for: "<< m_idHelperSvc->toStringDetEl(hitId) << " energyDeposit "<< energyDeposit );
  const MuonGMR4::sTgcReadoutElement* readoutElement = detMgr->getsTgcReadoutElement(hitId);

  // Projecting the hit position on wire surface given the hit position
  // and direction w.r.t. the wire plane
  Amg::Vector3D locHitDir = xAOD::toEigen(hit->localDirection());
  Amg::Vector3D locHitPos = xAOD::toEigen(hit->localPosition());
  ATH_MSG_VERBOSE("sTgc hit:  time " << hit->globalTime() 
  << " position " << Amg::toString(locHitPos, 2) 
  << " direction" << Amg::toString(locHitDir, 2) 
  << " mclink " << hit->genParticleLink() << " PDG ID " << hit->pdgId() );
  ATH_MSG_VERBOSE("Projecting hit to Wire Surface" );
  const double scale = Amg::intersect<3>(locHitPos, locHitDir, Amg::Vector3D::UnitZ(), 0.).value_or(0);
  Amg::Vector3D hitOnWireSurf = locHitPos + scale * locHitDir;
  ATH_MSG_VERBOSE("Hit on Wire Surface: " << Amg::toString(hitOnWireSurf, 2));

  /* Determine the closest wire and the distance of closest approach
   * Since most particles pass through the the wire plane between two wires,
   * the nearest wire should be one of these two wire. Otherwise, the particle's
   * trajectory is uncommon, and such rare case is not supported yet.
   *
   * Finding that nearest wire follows the following steps:
   * - Compute the distance to the wire at the center of the current wire pitch
   * - Compute the distance to the other adjacent wire
   */
  const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
  const Identifier wireLayId = idHelper.channelID(hitId, 
                                                idHelper.multilayer(hitId),
                                                idHelper.gasGap(hitId),
                                                ReadoutChannelType::Wire,
                                                1);

  const MuonGMR4::WireGroupDesign& wireDesign{readoutElement->wireDesign(wireLayId)};
  Amg::Vector2D hitOnWire2D = hitOnWireSurf.block<2,1>(0,0);
  if(!wireDesign.insideTrapezoid(hitOnWire2D)) {
    return {};
  }
  std::pair<int, int> wireGrpWireNum = wireDesign.wireNumber(hitOnWire2D);
  int wireNumber = -1;
  wireNumber = wireDesign.numPitchesToGroup(wireGrpWireNum.first) + wireGrpWireNum.second;
  // If wire number is invalid, verify if hit is near the edge of the chamber.
  // Try to get the wire number from the x and y coordinates of the hit position rather than the hit on wire surface.
  const int numWires = wireDesign.nAllWires();
  if((wireNumber < 1) || (wireNumber > numWires)) {
    wireGrpWireNum = wireDesign.wireNumber(locHitPos.block<2,1>(0,0));
    wireNumber = wireDesign.numPitchesToGroup(wireGrpWireNum.first) + wireGrpWireNum.second;
    if((wireNumber < 1) || (wireNumber > numWires)) {
      ATH_MSG_WARNING("Unable to retrieve the wire number, skipping the hit: " << m_idHelperSvc->toString(hitId));
      return {};
    }
  }
  // Compute the position of the ionization and its distance to the closest wire.
  Ionization ionization = pointClosestApproach(wireDesign, wireNumber, locHitPos, locHitDir, hit->stepLength());
  double distToWire = ionization.distance;
  if(distToWire > 0.) {
    // Determine on which side of the wire does the particle cross
    //  -1 if particle crosses the wire surface between wireNumber-1 and wireNumber
    //  +1 if particle crosses the wire surface between wireNumber and wireNumber+1
    int adjacent = 1;
    if (ionization.posOnSegment.x() < ionization.posOnWire.x()) {adjacent = -1;}
  // Find the position of the ionization with respect to the adjacent wire
  Ionization ionizationAdj = pointClosestApproach(wireDesign, wireNumber+adjacent, locHitPos, locHitDir, hit->stepLength());
  // Determine if the adjacent wire is closer to the hit on wire surface
  double distToWireAdj = ionizationAdj.distance;
  if ((distToWireAdj > 0.) && (distToWireAdj < distToWire)) {
    distToWire = distToWireAdj;
    wireNumber += adjacent;
    ionization = std::move(ionizationAdj);
  }
  } else {
    ATH_MSG_DEBUG("Failed to get the distance between the wire number = " << wireNumber
                  << " and hit at " <<Amg::toString(hitOnWireSurf, 2)
                  << ". Number of wires = " << numWires
                  << ", "<<m_idHelperSvc->toStringGasGap(wireLayId));
    return {};
  }

  // Update the position of ionization on the wire surface
  hitOnWireSurf = ionization.posOnWire;
  ATH_MSG_VERBOSE("Ionization_info: distance: " << ionization.distance
    << " posOnTrack: " <<Amg::toString(ionization.posOnSegment, 3)
    << " posOnWire: " << Amg::toString(ionization.posOnWire, 3)
    << " hit local Pos: " << Amg::toString(locHitPos,2)
    << " hit local Dir: " <<Amg::toString(locHitDir, 2)
    << " EDep: " << hit->energyDeposit() << " EKin: " << hit->kineticEnergy()
    << " pdgId: " << hit->pdgId()
    <<  "gasGap: "<<m_idHelperSvc->toStringGasGap(wireLayId));

  // Distance should be in the range [0, 0.9] mm, excepting
  // - particles pass through the wire plane near the edges
  // - secondary particles created inside the gas gap that go through the gas gap partially.
  //   Most of such particles are not muons and have low kinetic energy.
  // - particle with trajectory parallel to the sTGC wire plane
  const double wirePitch = wireDesign.stripPitch();
  if ((distToWire > 0.) && (std::abs(hit->pdgId()) == 13) && (distToWire > (0.5 * wirePitch))) {
    ATH_MSG_DEBUG("Distance to the nearest wire (" << distToWire << ") is greater than expected.");
    ATH_MSG_DEBUG("Hit local Pos: "<<Amg::toString(locHitPos, 2)
                 << " hit local Dir: " <<Amg::toString(locHitDir, 2)
                 << " EDeposited: " << hit->energyDeposit() 
                 << " EKinetic: " << hit->kineticEnergy()
                 << " pdgID: " << hit->pdgId()
                 << " gasGap: "<<m_idHelperSvc->toStringGasGap(wireLayId));
  }
  // Do not digitize hits that are too far from the nearest wire
  if (distToWire > wirePitch) {
    return {};
  }

  // Get the gamma pdf parameters associated with the distance of closest approach.
  const GammaParameter gamParam = getGammaParameter(distToWire);
  const double par_kappa = gamParam.kParameter;
  const double par_theta = gamParam.thetaParameter;
  const double most_prob_time = getMostProbableArrivalTime(distToWire);
  // Compute the most probable value of the gamma pdf
  double gamma_mpv = (par_kappa - 1) * par_theta;
  // If the most probable value is less than zero, then set it to zero
  if (gamma_mpv < 0.) {gamma_mpv = 0.;}
  const double t0_par = most_prob_time - gamma_mpv;

  // Digit time follows a gamma distribution, so a value val is
  // chosen using a gamma random generator then is shifted by t0
  // to account for drift time.
  // Note: CLHEP::RandGamma takes the parameters k and lambda,
  // where lambda = 1 / theta.
  double digitTime = t0_par + CLHEP::RandGamma::shoot(condContainers.rndEngine, par_kappa, 1/par_theta);

  // Sometimes, digitTime is negative because t0_par can be negative.
  // In such case, discard the negative value and shoot RandGamma for another value.
  // However, if that has already been done many times then set digitTime to zero
  // in order to avoid runaway loop.

  constexpr int shoot_limit = 4;
  int shoot_counter = 0;
  while (digitTime < 0.) {
    if (shoot_counter > shoot_limit) {
      digitTime = 0.;
      break;
    }
    digitTime = t0_par + CLHEP::RandGamma::shoot(condContainers.rndEngine, par_kappa, 1/par_theta);
    ++shoot_counter;
  }

  ATH_MSG_DEBUG("sTgcDigitMaker distance = " << distToWire
    << ", time = " << digitTime
    << ", k parameter = " << par_kappa
    << ", theta parameter = " << par_theta
    << ", most probable time = " << most_prob_time);

  //// HV efficiency correction
  if (condContainers.efficiencies) {
    const double efficiency = condContainers.efficiencies->getEfficiency(wireLayId);
    // Lose Hits to match HV efficiency
    if (CLHEP::RandFlat::shoot(condContainers.rndEngine,0.0,1.0) > efficiency) return {};
  }

  sTgcDigitVec digits{};

  double sDigitTimeWire = digitTime;
  double sDigitTimePad = sDigitTimeWire;
  double sDigitTimeStrip = sDigitTimeWire;

  uint16_t bctag = 0;

  //##################################################################################
  //######################################### strip readout ##########################
  //##################################################################################
  ATH_MSG_DEBUG("sTgcDigitMaker::strip response ");
  bool isValid = false;
  int channelType = ReadoutChannelType::Strip;
  const Identifier stripLayId = idHelper.channelID(wireLayId, idHelper.multilayer(wireLayId), 
                                                    idHelper.gasGap(wireLayId), channelType, 1, isValid);

  const MuonGMR4::StripDesign& stripDesign{readoutElement->stripDesign(stripLayId)};
  // Rotate clockwise 90 in xy plane to get to strip plane orientation, and then extract x,y
  const Amg::Vector2D hitOnStripSurf = (Amg::getRotateZ3D(-90. * Gaudi::Units::deg) * hitOnWireSurf).block<2,1>(0,0);

  bool insideBounds = stripDesign.insideTrapezoid(hitOnStripSurf);
  if(!insideBounds) {
    ATH_MSG_DEBUG("Outside of the strip surface boundary : " <<  m_idHelperSvc->toString(stripLayId) << "; local position " <<Amg::toString(hitOnStripSurf, 2));
    return {};
  }

  //************************************ find the nearest readout element **************************************
  // Required precision on length in mm
  constexpr double tolerance_length = 0.01*Gaudi::Units::mm;
  int stripNumber = stripDesign.stripNumber(hitOnStripSurf);
  if( stripNumber < 0){
    // Verify if the energy deposit is at the boundary
    const double newPosX = (hitOnStripSurf.x() > 0.0)? hitOnStripSurf.x() - tolerance_length
                                                      : hitOnStripSurf.x() + tolerance_length;
    const Amg::Vector2D newPos(newPosX, hitOnStripSurf.y());
    stripNumber = stripDesign.stripNumber(newPos);
    // Skip hit if still unable to obtain strip number
    if (stripNumber < 0) {
      ATH_MSG_WARNING("Failed to obtain strip number " << m_idHelperSvc->toString(stripLayId) );
      ATH_MSG_WARNING("Position on strip surface = (" << hitOnStripSurf.x() << ", " << hitOnStripSurf.y() << ")");
      return {};
    }
  }
  isValid = false;
  const Identifier stripHitId = idHelper.channelID(stripLayId, idHelper.multilayer(stripLayId), 
                          idHelper.gasGap(stripLayId), channelType, stripNumber, isValid);
  if(!isValid && stripNumber != -1) {
  ATH_MSG_ERROR("Failed to obtain identifier " << m_idHelperSvc->toString(stripHitId) );
  return {};
  }

  const int numStrips = stripDesign.numStrips();
  const double stripHalfPitch = 0.5 * stripDesign.stripPitch(); 

  //************************************ conversion of energy to charge **************************************

  // Typical ionized charge in pC per keV deposited. The constant is determined from ionization
  // study with Garfield program. A note titled "Charge Energy Relation" which outlines
  // conversion can be found here:
  // https://gitlab.cern.ch/carleton-ATLAS/carleton-nsw/stgc-ionization-gain/-/blob/master/Charge_Energy_Relation.pdf?ref_type=heads 
  const double ionized_charge = (5.65E-6)*energyDeposit/CLHEP::keV;

  // To get avalanche gain, polya function is taken from Blum paper https://inspirehep.net/literature/807304
  // m_polyaFunction = new TF1("m_polyaFunction","(1.0/[1])*(TMath::Power([0]+1,[0]+1)/TMath::Gamma([0]+1))*TMath::Power(x/[1],[0])*TMath::Exp(-([0]+1)*x/[1])",0,3000000);

  // Mean value for total gain due to E field;
  // To calculate this gain from polya distibution, we replace in gamma PDF:
  //     alpha = 1+theta and
  //     beta = 1+theta/mean
  // With these substitutions, gamma PDF gives the same sampling values as those from polya PDF.
  const double gain =  CLHEP::RandGamma::shoot(condContainers.rndEngine, 1. + m_theta, (1. + m_theta)/m_meanGasGain);

  // total charge after avalanche
  const double total_charge = gain*ionized_charge;

  // Charge Spread including tan(theta) resolution term.
  const double tan_theta = locHitDir.perp()/locHitDir.z();
  // The angle dependance on strip resolution goes as tan^2(angle)
  const double angle_dependency = std::hypot(m_posResIncident, m_posResAngular * tan_theta);

  const double cluster_posX = hitOnStripSurf.x();
  double peak_position = CLHEP::RandGaussZiggurat::shoot(condContainers.rndEngine, cluster_posX, m_StripResolution*angle_dependency);

  // Each readout plane reads about half the total charge produced on the wire,
  // including a tan(theta) term to describe the increase of charge with incident angle
  const double norm = 0.5 * total_charge;

  // Lower limit on strip charge (arbitrary limit), in pC, which has the same units as the parameter ionized_charge. 
  constexpr double tolerance_charge = 0.0005;
  // Set a maximum number of neighbour strips to avoid very long loop. This is an arbitrary number.
  constexpr unsigned int max_neighbor = 10;

  // Spread charge on the strips that are on the upper half of the strip cluster
  for (unsigned int iStrip = 0; iStrip <= max_neighbor; ++iStrip) {
    int currentStrip = stripNumber + iStrip;
    if (currentStrip > numStrips) {
      ATH_MSG_VERBOSE("Breaking the upper half strip loop stripNumber: " << currentStrip << " > " << numStrips);
      break;
    }

    // Get the strip identifier and create the digit
    isValid = false;
    const Identifier currentStripId = idHelper.channelID(stripLayId, idHelper.multilayer(stripLayId), idHelper.gasGap(stripLayId), 
                              channelType, currentStrip, isValid);
    if (isValid) {
      Amg::Vector2D currentStripPos = readoutElement->localChannelPosition(currentStripId);
      if (currentStripPos == Amg::Vector2D::Zero()) {
          ATH_MSG_VERBOSE("Failed to obtain local position for identifier " << m_idHelperSvc->toString(currentStripId) );
      }

      // Estimate the digit charge
      // Position with respect to the peak of the charge curve
      double x_relative = currentStripPos.x() - peak_position;
      // In clusterProfile curve, position should be in the units of strip channel
      double charge = std::hypot(1, m_chargeAngularFactor * tan_theta) * norm * chargeIntegral(x_relative/(2*stripHalfPitch) - 0.5, x_relative/(2*stripHalfPitch) + 0.5);
      // If charge is too small, stop creating neighbor strip
      if (charge < tolerance_charge) {
        ATH_MSG_VERBOSE("Breaking the upper half strip loop stripCharge: " << charge << " < " << tolerance_charge);
        break;
      }

      // Estimate digit time
      double strip_time = sDigitTimeStrip;
      // Strip time response can be delayed due to the resistive layer.
      // A correction would be required if the actual VMM front-end doesn't re-align the strip timing.
      if (m_doTimeOffsetStrip) {
        // Determine how far the current strip is from the middle strip
        int indexFromMiddleStrip = iStrip;
        if ((iStrip > 0) && ((x_relative + (0.75 - iStrip * 2) * stripHalfPitch) < 0))
          indexFromMiddleStrip = iStrip - 1;
        // Add time delay due to resistive layer
        strip_time += getTimeOffsetStrip(indexFromMiddleStrip);
      }

      addDigit(digits, currentStripId, bctag, strip_time, charge);

      ATH_MSG_VERBOSE("Created a strip digit: strip number = " << currentStrip << ", charge = " << charge
                      << ", time = " << strip_time << ", time offset = " << strip_time-sDigitTimeStrip
                      << ", neighbor index = " << iStrip
                      << ", strip position = "<<Amg::toString(currentStripPos, 2));
    }
  }

  // The lower half of the strip cluster
  for (unsigned int iStrip = 1; iStrip <= max_neighbor; ++iStrip) {
    int currentStrip = stripNumber - iStrip;
    if (currentStrip < 1) {
      ATH_MSG_VERBOSE("Breaking the lower half strip loop stripNumber: " << currentStrip << " < 1 ");
      break;
    } 

    isValid = false;
    const Identifier currentStripId = idHelper.channelID(stripLayId, idHelper.multilayer(stripLayId), idHelper.gasGap(stripLayId), 
                              channelType, currentStrip, isValid);
    if (isValid) {
      Amg::Vector2D currentStripPos = readoutElement->localChannelPosition(currentStripId);
      if (currentStripPos == Amg::Vector2D::Zero()) {
          ATH_MSG_WARNING("Failed to obtain local position for identifier " << m_idHelperSvc->toString(currentStripId) );
      }

      // Estimate the digit charge
      double x_relative = currentStripPos.x() - peak_position;
      double charge = std::hypot(1, m_chargeAngularFactor * tan_theta) * norm * chargeIntegral((x_relative/(2*stripHalfPitch) - 0.5), (x_relative/(2*stripHalfPitch) + 0.5));
      if (charge < tolerance_charge) {
        ATH_MSG_VERBOSE("Breaking the lower half strip loop stripCharge: " << charge << " < " << tolerance_charge);
        break;
      }

      // Estimate digit time
      double strip_time = sDigitTimeStrip;
      // Time delay due to resistive layer
      if (m_doTimeOffsetStrip) {
        int indexFromMiddleStrip = ((x_relative + (iStrip * 2 - 0.75) * stripHalfPitch) > 0)? iStrip - 1 : iStrip;
        strip_time += getTimeOffsetStrip(indexFromMiddleStrip);
      }

      addDigit(digits, currentStripId, bctag, strip_time, charge);

      ATH_MSG_VERBOSE("Created a strip digit: strip number = " << currentStrip << ", charge = " << charge
                      << ", time = " << strip_time << ", time offset = " << strip_time-sDigitTimeStrip
                      << ", neighbor index = " << iStrip
                      << ", strip position = " << Amg::toString(currentStripPos, 2));
    }
  }
  // end of strip digitization

  if(m_digitMode == digitMode::StripsOnly) {
    ATH_MSG_WARNING("Only digitize strip response !");
    return digits;
  }

  //##################################################################################
  //######################################### pad readout ##########################
  //##################################################################################
  ATH_MSG_DEBUG("sTgcDigitMaker::pad response ");
  channelType = ReadoutChannelType::Pad;

  Identifier padLayId = idHelper.channelID(wireLayId, idHelper.multilayer(wireLayId), 
                                        idHelper.gasGap(wireLayId), channelType, 1);
  
  const MuonGMR4::PadDesign& padDesign{readoutElement->padDesign(padLayId)};
  //************************************ find the nearest readout element **************************************
  
  // Since the orientation of pad layer is the same as wirelayer, the x,y can be taken directly
  // from the hit position on wire surface
  const Amg::Vector2D hitOnPadSurf = hitOnWireSurf.block<2,1>(0,0);

  
  insideBounds = padDesign.insideTrapezoid(hitOnPadSurf);

  if(insideBounds) {
    std::pair<uint, uint> padEtaPhi = padDesign.channelNumber(hitOnPadSurf);
    unsigned int padEta = padEtaPhi.first;
    unsigned int padPhi = padEtaPhi.second;
    if( padEta < 1 || padPhi < 1){
      // Verify if the energy deposit is at the boundary
      const double newPosX = (hitOnPadSurf.x()>0.0)? hitOnPadSurf.x() - tolerance_length
                                                    : hitOnPadSurf.x() + tolerance_length;
      const double newPosY = (hitOnPadSurf.y()>0.0)? hitOnPadSurf.y() - tolerance_length
                                                    : hitOnPadSurf.y() + tolerance_length;
      const Amg::Vector2D newPosition(newPosX, newPosY);
      padEtaPhi = padDesign.channelNumber(newPosition);
      padEta = padEtaPhi.first;
      padPhi = padEtaPhi.second;
      // Skip hit if still unable to obtain pad number
      if( padEta < 1 || padPhi < 1){
        ATH_MSG_WARNING("Failed to obtain pad number for " << m_idHelperSvc->toString(padLayId) );
        ATH_MSG_WARNING("Position on pad surface = (" << hitOnPadSurf.x() << ", " << hitOnPadSurf.y() << ")");
        return digits;
      }
    }
    isValid = false;
    const Identifier padHitId = idHelper.padID(padLayId, idHelper.multilayer(padLayId), 
                               idHelper.gasGap(padLayId), channelType, padEta, padPhi, isValid);
    if(isValid) {
      // Find centre position of pad
      Amg::Vector2D padPos = readoutElement->localChannelPosition(padHitId);
      if (padPos == Amg::Vector2D::Zero()) {
        ATH_MSG_ERROR("Failed to obtain local position for neighbor pad with identifier " << m_idHelperSvc->toString(padHitId) );
        return digits;
      }

      // Pad sharing needs to look at position on hit vs pad boundaries
      const Amg::Vector2D diff = hitOnPadSurf - padPos;
      double halfPadHeight = 0.5 * readoutElement->padHeight(padHitId);
      // Extracting pad corners to get the width of the pad at hit position
      std::array<Amg::Vector2D, 4> padHitCorners = padDesign.padCorners(padEtaPhi);
      double padBottomBase = (padHitCorners[0] - padHitCorners[1]).norm();
      double padTopBase = (padHitCorners[2] - padHitCorners[3]).norm();
      // Linearly interpolating pad width (as done by channelWidth function in R3)
      // Interpolating pad width at the y-position of the hit (diff.y()) using pad top and bottom bases.
      // The expression below computes the width at a relative vertical position inside the pad.
      // Specifically: (1 + diff.y() / halfPadHeight) * 0.25 = (y_from_bottom / padHeight)
      double halfPadWidth = 0.5 * padBottomBase + 0.25 * (1 + diff.y()/halfPadHeight) * (padTopBase - padBottomBase);

      // Charge sharing happens within 4mm window for pads
      // i.e. the charge is spread over a 2D gaussian of total radius about 4mm
      // This value depends on actual width of the distribution, but we don't have the
      // width of the charge distribution for pads. So lets consider 2.5*sigma, where
      // sigma is the width of the charge distribution for strips.
      double deltaX = halfPadWidth - std::abs(diff.x());
      double deltaY = halfPadHeight - std::abs(diff.y());
      bool isNeighX = deltaX < 2.5*m_clusterParams[1];
      bool isNeighY = deltaY < 2.5*m_clusterParams[1];
      // Pad width can be calculated to be very slightly larger than it should due to rounding errors
      // So if a hit falls on a given pad but is "outside" the width, just define it to be on the boundary of 2 pads.
      if (deltaX < 0.) deltaX = 0.1;
      if (deltaY < 0.) deltaY = 0.1;

      if (m_doPadSharing && (isNeighX || isNeighY)){
        // Phi == 1 at the right in the local geometry
        // In local coordinates, the pad to the right has phi' = phi - 1
        unsigned int newPhi = padEtaPhi.second - Acts::copySign(1, diff.x());
        unsigned int newEta = padEtaPhi.first  + Acts::copySign(1, diff.y());
        bool validEta = newEta > 0 && newEta < readoutElement->numPadEta(padHitId) + 1;
        bool validPhi = newPhi > 0 && newPhi < readoutElement->numPadPhi(padHitId) + 1;
    
        if (isNeighX && isNeighY && validEta && validPhi){
          // 4 pads total; makes life a bit harder. Corner of 4 valid pads
          isValid = false;
          const Identifier neigh_ID_X = idHelper.padID(padHitId, idHelper.multilayer(padHitId), idHelper.gasGap(padHitId),
                                                  channelType, padEta, newPhi, isValid);
          isValid = false;
          const Identifier neigh_ID_Y = idHelper.padID(padHitId, idHelper.multilayer(padHitId), idHelper.gasGap(padHitId),
                                                  channelType, newEta, padPhi, isValid);
          isValid = false;
          const Identifier neigh_ID_XY = idHelper.padID(padHitId, idHelper.multilayer(padHitId), idHelper.gasGap(padHitId),
                                                  channelType, newEta, newPhi, isValid);
          double xQfraction = getPadChargeFraction(deltaX);
          double yQfraction = getPadChargeFraction(deltaY);

          // Main pad gets 1 - Qfraction of total
          addDigit(digits, neigh_ID_X, bctag, sDigitTimePad, xQfraction*(1.-yQfraction)*0.5*total_charge);
          addDigit(digits, neigh_ID_Y, bctag, sDigitTimePad, yQfraction*(1.-xQfraction)*0.5*total_charge);
          addDigit(digits, neigh_ID_XY, bctag, sDigitTimePad, xQfraction*yQfraction*0.5*total_charge);
          addDigit(digits, padHitId, bctag, sDigitTimePad, (1.-xQfraction-yQfraction+xQfraction*yQfraction)*0.5*total_charge);
        } else if (isNeighX && validPhi){
          // There is only 1 neighbor, immediately to the left or right.
          isValid = false;
          const Identifier neigh_ID = idHelper.padID(padHitId, idHelper.multilayer(padHitId), idHelper.gasGap(padHitId), 
                                               channelType, padEta, newPhi, isValid);

          // NeighborPad gets Qfraction of the total pad charge: 0.5*total_charge
          // Main pad gets 1 - Qfraction of total
          double xQfraction = getPadChargeFraction(deltaX);
          addDigit(digits, padHitId, bctag, sDigitTimePad, (1.-xQfraction)*0.5*total_charge);
          addDigit(digits, neigh_ID, bctag, sDigitTimePad, xQfraction*0.5*total_charge);
        }
        else if (isNeighY && validEta){
          // There is only 1 neighbor, immediately above or below
          isValid = false;
          const Identifier neigh_ID = idHelper.padID(padHitId, idHelper.multilayer(padHitId), idHelper.gasGap(padHitId), 
                                               channelType, newEta, padPhi, isValid);

          // NeighborPad gets Qfraction of the total pad charge: 0.5*total_charge
          // Main pad gets 1 - Qfraction of total
          double yQfraction = getPadChargeFraction(deltaX);
          addDigit(digits, padHitId, bctag, sDigitTimePad, (1.-yQfraction)*0.5*total_charge);
          addDigit(digits, neigh_ID, bctag, sDigitTimePad, yQfraction*0.5*total_charge);
        }

      }
      else{
        // No charge sharing: hit is nicely isolated within pad
        addDigit(digits, padHitId, bctag, sDigitTimePad, 0.5*total_charge);
      }

    }
    else if(padEta != 0 || padPhi != 0) {
      ATH_MSG_ERROR("Failed to obtain identifier " << m_idHelperSvc->toString(padHitId) );
    }
  }
  else {
    ATH_MSG_DEBUG("Outside of the pad surface boundary :" << m_idHelperSvc->toString(padLayId)<< " local position " <<Amg::toString(hitOnPadSurf, 2));
  }

  if(m_digitMode == digitMode::StripsAndPads) {
    ATH_MSG_WARNING("Only digitize strip/pad response !");
    return digits;
  }

  //##################################################################################
  //######################################### wire readout ##########################
  //##################################################################################
  ATH_MSG_DEBUG("sTgcDigitMaker::wire response ");
  channelType = ReadoutChannelType::Wire;

  // wireLayId already defined at the start
  //************************************ find the nearest readout element **************************************
  insideBounds = wireDesign.insideTrapezoid(hitOnWireSurf.block<2,1>(0,0));

  if(insideBounds) {
      // Determine the wire number
      int wiregroupNumber = (wireDesign.wireNumber(hitOnWireSurf.block<2,1>(0,0))).first;
      if( wiregroupNumber < 1){
        // Verify if the energy deposit is at the boundary
        const double newPosX = (hitOnWireSurf.x() > 0.0)? hitOnWireSurf.x() - tolerance_length
                                                          : hitOnWireSurf.x() + tolerance_length;
        const Amg::Vector2D newPos(newPosX, hitOnWireSurf.y());
        wiregroupNumber = (wireDesign.wireNumber(newPos)).first;
        // Skip hit if still unable to obtain wire number
        if (wiregroupNumber < 1) {
          ATH_MSG_WARNING("Failed to obtain wire number " << m_idHelperSvc->toString(wireLayId) );
          ATH_MSG_WARNING("Position on wire surface = (" << hitOnWireSurf.x() << ", " << hitOnWireSurf.y() << ")");
          return digits;
        }
      }

      // Find ID of the actual wiregroup
      isValid = false;
      const Identifier wireGroupHitId = idHelper.channelID(wireLayId, idHelper.multilayer(wireLayId), idHelper.gasGap(wireLayId), 
                                  channelType, wiregroupNumber, isValid);

      if(isValid) {
        int numWireGroups = readoutElement->numChannels(wireLayId);
        if(wiregroupNumber >=1 && wiregroupNumber <= numWireGroups) {
          addDigit(digits, wireGroupHitId, bctag, sDigitTimeWire, total_charge);
        }
      } // end of if(isValid)
      else if (wiregroupNumber != -1){
        ATH_MSG_ERROR("Failed to obtain wiregroup identifier " << m_idHelperSvc->toString(wireGroupHitId));
      }
  }
  else {
    ATH_MSG_DEBUG("Outside of the wire surface boundary :" << m_idHelperSvc->toString(wireLayId)<< " local position " <<Amg::toString(hitOnWireSurf, 2));
  }
  // end of wire digitization

  return digits;
}

//------------------------------------------------------
// Compute closest approach between hit trajectory and wire
//------------------------------------------------------
sTgcDigitMaker::Ionization sTgcDigitMaker::pointClosestApproach(const MuonGMR4::WireGroupDesign& wireDesign,
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

  // Geometry setup
  const double wirePitch = wireDesign.stripPitch();
  const double wirePosX = wireDesign.firstStripPos().x() + (wireNumber - 1) * wirePitch;
  const Amg::Vector3D wireDir(wireDesign.stripDir().x(), wireDesign.stripDir().y(), 0.0);
  const Amg::Vector3D wirePos(wirePosX, locHitPos.y(), 0.);

  // Use Amg::intersect to find closest point on hit segment to wire plane
  std::optional<double> scaleHit = Amg::intersect<3>(locHitPos, locHitDir, Amg::Vector3D::UnitZ(), 0);
  if (!scaleHit || std::abs(std::abs(wireDir.dot(locHitDir)) - 1.0) < angular_tolerance) {
    ATH_MSG_DEBUG("The track segment is parallel to the wire, position of digit is undefined");
    ionization.posOnSegment = locHitPos;
    ionization.posOnWire = wirePos;
    ionization.distance = std::hypot(locHitPos.x() - wirePosX, locHitPos.z());
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
                              const uint16_t bctag, 
                              const double digittime,
                              const double charge) {
  
  constexpr double tolerance = 0.1;
  if (!std::ranges::any_of(digits, [&](std::unique_ptr<sTgcDigit>& known) {
      return known->identify() == id && std::abs(digittime - known->time()) < tolerance;
  })) {
    digits.push_back(std::make_unique<sTgcDigit>(id, bctag, digittime, charge, 0, 0));
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
    if (key.compare("bin") == 0) {
      iss >> param.lowEdge >> param.kParameter >> param.thetaParameter;
      m_gammaParameter.push_back(param);
    } else if (key.compare("mpv") == 0)  {
      double mpt;
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
  const double d{std::abs(distance)};
  double mpt{0};
  for (size_t t = 0 ; t < m_mostProbableArrivalTime.size(); ++t){
    mpt += m_mostProbableArrivalTime[t] * std::pow(d, t);
  }
  return mpt;
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
    if (key.compare("strip") == 0) {
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
