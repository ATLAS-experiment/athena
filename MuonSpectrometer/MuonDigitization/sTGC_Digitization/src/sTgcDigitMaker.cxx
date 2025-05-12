/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "sTGC_Digitization/sTgcDigitMaker.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonDigitContainer/sTgcDigitCollection.h"
#include "MuonSimEvent/sTgcSimIdToOfflineId.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "TrkEventPrimitives/LocalDirection.h"
#include "TrkSurfaces/Surface.h"
#include "GaudiKernel/MsgStream.h"
#include "PathResolver/PathResolver.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandGamma.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "AthenaBaseComps/AthCheckMacros.h"

#include "TF1.h"
#include <cmath>
#include <iostream>
#include <fstream>

//---------------------------------------------------
//  Constructor and Destructor
//---------------------------------------------------

//----- Constructor
sTgcDigitMaker::sTgcDigitMaker(const Muon::IMuonIdHelperSvc* idHelperSvc,
                               const int channelTypes,
                               double meanGasGain,
                               bool doPadChargeSharing,
                               double stripChargeScale)
  : AthMessaging ("sTgcDigitMaker"),
  m_idHelperSvc{idHelperSvc},
  m_channelTypes{channelTypes},
  m_meanGasGain{meanGasGain},
  m_doPadSharing{doPadChargeSharing},
  m_stripChargeScale{stripChargeScale} {}
//----- Destructor
sTgcDigitMaker::~sTgcDigitMaker() = default;
//------------------------------------------------------
// Initialize
//------------------------------------------------------
StatusCode sTgcDigitMaker::initialize() {

  // Read share/sTGC_Digitization_timeArrivale.dat, containing the digit time of arrival
  ATH_CHECK(readFileOfTimeArrival());

  // Read share/sTGC_Digitization_timeOffsetStrip.dat if the the strip time correction is enable
  if (m_doTimeOffsetStrip) {
    ATH_CHECK(readFileOfTimeOffsetStrip());
  }
  // check the digitization channel type
  if(m_channelTypes!=1 && m_channelTypes!=2 && m_channelTypes!=3) {
      ATH_MSG_ERROR("Invalid ChannelTypes : " << m_channelTypes << " (valid values are : 1 --> strips ; 2 --> strips / wires ; 3 --> strips / wires / pads)");
      return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------
// Execute Digitization
//---------------------------------------------------
sTgcDigitMaker::sTgcDigitVec sTgcDigitMaker::executeDigi(const DigiConditions& condContainers, 
                                                         const sTGCSimHit& hit) const {
  // SimHits without energy loss are not recorded.
  double energyDeposit = hit.depositEnergy(); // Energy deposit in MeV
  if(energyDeposit==0.) return {};

  //////////  convert ID for this digitizer system
  const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
  sTgcSimIdToOfflineId simToOffline{&idHelper};
  const Identifier offlineId = simToOffline.convert(hit.sTGCId());
  const Identifier layid = idHelper.channelID(offlineId, 
                                              idHelper.multilayer(offlineId),
                                              idHelper.gasGap(offlineId), 
                                              sTgcIdHelper::sTgcChannelTypes::Wire, 1);

  ATH_MSG_VERBOSE("sTgc hit:  time " << hit.globalTime() 
              << " position " << Amg::toString(hit.globalPosition(), 2) 
              << " mclink " << hit.particleLink() << " PDG ID " << hit.particleEncoding() );

  
  ATH_MSG_DEBUG("Retrieving detector element for: "<< m_idHelperSvc->toStringDetEl(layid) << " energyDeposit "<<energyDeposit );

  const MuonGM::sTgcReadoutElement* detEl = condContainers.detMgr->getsTgcReadoutElement(layid);
  if( !detEl ){ return {}; }
  

  // DO THE DIGITIZATTION HERE ////////

  // Required precision on length in mm
  constexpr double length_tolerance = 0.01;

  // Retrieve the wire surface
  int surfHash_wire = detEl->surfaceHash(layid);
  const Trk::PlaneSurface& SURF_WIRE = detEl->surface(surfHash_wire); // get the wire surface

  // Hit post-Step global position
  const Amg::Vector3D& GPOS{hit.globalPosition()};
  // Hit pre-Step global position
  const Amg::Vector3D& pre_pos{hit.globalPrePosition()};
  // Hit global direction
  const Amg::Vector3D& GLODIRE{hit.globalDirection()};

  // Hit position in the wire surface's coordinate frame
  Amg::Vector3D hitOnSurface_wire{SURF_WIRE.transform().inverse() * GPOS};
  Amg::Vector3D pre_pos_wire_surf{SURF_WIRE.transform().inverse() * pre_pos};
  Amg::Vector2D posOnSurf_wire{0.5 * (hitOnSurface_wire + pre_pos_wire_surf).block<2,1>(0,0)};

  /* Determine the closest wire and the distance of closest approach
   * Since most particles pass through the the wire plane between two wires,
   * the nearest wire should be one of these two wire. Otherwise, the particle's
   * trajectory is uncommon, and such rare case is not supported yet.
   *
   * Finding that nearest wire follows the following steps:
   * - Compute the distance to the wire at the center of the current wire pitch
   * - Compute the distance to the other adjacent wire
   */

  // Wire number of the current wire pitch
  int wire_number = detEl->getDesign(layid)->wireNumber(posOnSurf_wire);

  // If wire number is invalid, verify if hit is near the edge of the chamber.
  const int number_wires = detEl->numberOfWires(layid);
  if ((wire_number < 1) || (wire_number > number_wires)) {
    // Compute the wire number using either the pos-step position or
    // the pre-step position, whichever yields a valid wire number.
    wire_number = detEl->getDesign(layid)->wireNumber(hitOnSurface_wire.block<2,1>(0,0));
    if ((wire_number < 1) || (wire_number > number_wires)) {
      wire_number = detEl->getDesign(layid)->wireNumber(pre_pos_wire_surf.block<2,1>(0,0));
    }
  }

  // Compute the position of the ionization and its distance relative to a given sTGC wire.
  Ionization ionization = pointClosestApproach(detEl, layid, wire_number, pre_pos_wire_surf, hitOnSurface_wire);
  double dist_wire = ionization.distance;
  if (dist_wire > 0.) {
    // Determine the other adjacent wire, which is
    //  -1 if particle crosses the wire surface between wire_number-1 and wire_number
    //  +1 if particle crosses the wire surface between wire_number and wire_number+1
    int adjacent = 1;
    if (ionization.posOnSegment.x() < ionization.posOnWire.x()) {adjacent = -1;}

    // Find the position of the ionization with respect to the adjacent wire
    Ionization ion_adj = pointClosestApproach(detEl, layid, wire_number+adjacent, pre_pos_wire_surf, hitOnSurface_wire);
    // Keep the closest point
    double dist_wire_adj = ion_adj.distance;
    if ((dist_wire_adj > 0.) && (dist_wire_adj < dist_wire)) {
      dist_wire = dist_wire_adj;
      wire_number += adjacent;
      ionization = std::move(ion_adj);
    }
  } else {
    ATH_MSG_DEBUG("Failed to get the distance between the wire number = " << wire_number
                    << " and hit at " <<Amg::toString(hitOnSurface_wire, 2)
                    << ". Number of wires = " << number_wires
                    << ", "<<m_idHelperSvc->toStringGasGap(layid));
    return {};
  }

  // Update the position of ionization on the wire surface
  posOnSurf_wire = ionization.posOnWire.block<2,1>(0,0);
  // Position of the ionization in the global coordinate frame
  const Amg::Vector3D glob_ionization_pos = SURF_WIRE.transform() * ionization.posOnWire;

  ATH_MSG_VERBOSE("Ionization_info: distance: " << ionization.distance
    << " posOnTrack: " <<Amg::toString(ionization.posOnSegment, 3)
    << " posOnWire: " << Amg::toString(ionization.posOnWire, 3)
    << " hitGPos: " << Amg::toString(GPOS,3)
    << " hitPrePos: " <<Amg::toString(pre_pos, 3)
    << " EDep: " << hit.depositEnergy() << " EKin: " << hit.kineticEnergy()
    << " pdgId: " << hit.particleEncoding()
    <<" "<<m_idHelperSvc->toStringGasGap(layid));

  // Distance should be in the range [0, 0.9] mm, excepting
  // - particles pass through the wire plane near the edges
  // - secondary particles created inside the gas gap that go through the gas gap partially.
  //   Most of such particles are not muons and have low kinetic energy.
  // - particle with trajectory parallel to the sTGC wire plane
  const double wire_pitch = detEl->wirePitch();
  if ((dist_wire > 0.) && (std::abs(hit.particleEncoding()) == 13) && (dist_wire > (wire_pitch/2))) {
    ATH_MSG_DEBUG("Distance to the nearest wire (" << dist_wire << ") is greater than expected.");
    ATH_MSG_DEBUG("Hit globalPos: "<<Amg::toString(GPOS,2)
                 << " globalPrePos: " << Amg::toString(pre_pos, 2)
                 << " EDeposited: " << hit.depositEnergy() 
                 << " EKinetic: " << hit.kineticEnergy()
                 << " pdgID: " << hit.particleEncoding()
                 << " "<<m_idHelperSvc->toStringGasGap(layid));
  }

  // Do not digitize hits that are too far from the nearest wire
  if (dist_wire > wire_pitch) {
    return {};
  }

  // Get the gamma pdf parameters associated with the distance of closest approach.
  const GammaParameter gamParam = getGammaParameter(dist_wire);
  const double par_kappa = gamParam.kParameter;
  const double par_theta = gamParam.thetaParameter;
  const double most_prob_time = getMostProbableArrivalTime(dist_wire);
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
  double digit_time = t0_par + CLHEP::RandGamma::shoot(condContainers.rndmEngine, par_kappa, 1/par_theta);

  // Sometimes, digit_time is negative because t0_par can be negative.
  // In such case, discard the negative value and shoot RandGamma for another value.
  // However, if that has already been done many times then set digit_time to zero
  // in order to avoid runaway loop.
  constexpr int shoot_limit = 4;
  int shoot_counter = 0;
  while (digit_time < 0.) {
    if (shoot_counter > shoot_limit) {
      digit_time = 0.;
      break;
    }
    digit_time = t0_par + CLHEP::RandGamma::shoot(condContainers.rndmEngine, par_kappa, 1/par_theta);
    ++shoot_counter;
  }

  ATH_MSG_DEBUG("sTgcDigitMaker distance = " << dist_wire
                << ", time = " << digit_time
                << ", k parameter = " << par_kappa
                << ", theta parameter = " << par_theta
                << ", most probable time = " << most_prob_time);

  bool isValid = false;
  //// HV efficiency correction
  if (condContainers.efficiencies) {
    const double efficiency = condContainers.efficiencies->getEfficiency(layid);
    // Lose Hits to match HV efficiency
    if (CLHEP::RandFlat::shoot(condContainers.rndmEngine,0.0,1.0) > efficiency) return {};
  }

  
  sTgcDigitVec digits{};

  //const double stripPropagationTime = 3.3*CLHEP::ns/CLHEP::m * detEl->distanceToReadout(posOnSurf_strip, elemId); // 8.5*ns/m was used until MC10.
  const double stripPropagationTime = 0.; // 8.5*ns/m was used until MC10.

  double sDigitTimeWire = digit_time;
  double sDigitTimePad = sDigitTimeWire;
  double sDigitTimeStrip = sDigitTimeWire + stripPropagationTime;

  uint16_t bctag = 0;

  //##################################################################################
  //######################################### strip readout ##########################
  //##################################################################################
  ATH_MSG_DEBUG("sTgcDigitMaker::strip response ");
  int channelType = sTgcIdHelper::sTgcChannelTypes::Strip;

  Identifier newId = idHelper.channelID(layid,idHelper.multilayer(layid), 
                                        idHelper.gasGap(layid), channelType, 1, isValid);

  // get strip surface
  int surfHash_strip =  detEl->surfaceHash(newId);
  const Trk::PlaneSurface& SURF_STRIP = detEl->surface(surfHash_strip); // get the strip surface

  const Amg::Vector3D hitOnSurface_strip = SURF_STRIP.transform().inverse()*glob_ionization_pos;

  //This block is used to apply As-Built and BLine corrections for dedicated studies. In a standart digi job the readout element does not have As-Built or BLine corrections loaded so the position does not change while piping it through the spacePointPosition function.
  Amg::Vector3D posAfterAsBuilt {Amg::Vector3D::Zero()};
  detEl->spacePointPosition(newId, hitOnSurface_strip.x(), hitOnSurface_strip.y(), posAfterAsBuilt);
  Amg::Vector2D posOnSurf_strip = posAfterAsBuilt.block<2,1>(0,0);

  bool insideBounds = SURF_STRIP.insideBounds(posOnSurf_strip);
  if(!insideBounds) {
    ATH_MSG_DEBUG("Outside of the strip surface boundary : " <<  m_idHelperSvc->toString(newId) << "; local position " <<posOnSurf_strip );
    return {};
  }

  //************************************ find the nearest readout element **************************************

  int stripNumber = detEl->stripNumber(posOnSurf_strip, newId);
  if( stripNumber == -1 ){
    // Verify if the energy deposit is at the boundary
    const double new_posX = (posOnSurf_strip.x() > 0.0)? posOnSurf_strip.x() - length_tolerance
                                                      : posOnSurf_strip.x() + length_tolerance;
    const Amg::Vector2D new_position(new_posX, posOnSurf_strip.y());
    stripNumber = detEl->stripNumber(new_position, newId);
    // Skip hit if still unable to obtain strip number
    if (stripNumber < 1) {
      ATH_MSG_WARNING("Failed to obtain strip number " << m_idHelperSvc->toString(newId) );
      ATH_MSG_WARNING("Position on strip surface = (" << posOnSurf_strip.x() << ", " << posOnSurf_strip.y() << ")");
      return {};
    }
  }
  newId = idHelper.channelID(layid, idHelper.multilayer(layid), 
                             idHelper.gasGap(layid), channelType, stripNumber, isValid);
  if(!isValid && stripNumber != -1) {
    ATH_MSG_ERROR("Failed to obtain identifier " << m_idHelperSvc->toString(newId) );
    return {};
  }

  const int NumberOfStrips = detEl->numberOfStrips(newId);
  const double stripHalfPitch = detEl->channelPitch(newId)*0.5; // 3.2/2 = 1.6 mm

  //************************************ conversion of energy to charge **************************************

  // Typical ionized charge in pC per keV deposited. The constant is determined from ionization
  // study with Garfield program. A note titled "Charge Energy Relation" which outlines
  // conversion can be found here:
  //   https://cernbox.cern.ch/index.php/apps/files/?dir=/__myshares/Documents (id:274113) // link is dead
  const double ionized_charge = (5.65E-6)*energyDeposit/CLHEP::keV;

 // To get avalanche gain, polya function is taken from Blum paper https://inspirehep.net/literature/807304
 // m_polyaFunction = new TF1("m_polyaFunction","(1.0/[1])*(TMath::Power([0]+1,[0]+1)/TMath::Gamma([0]+1))*TMath::Power(x/[1],[0])*TMath::Exp(-([0]+1)*x/[1])",0,3000000);

  // Mean value for total gain due to E field;
  // To calculate this gain from polya distibution, we replace in gamma PDF:
  //     alpha = 1+theta and
  //     beta = 1+theta/mean
  // With these substitutions, gamma PDF gives the same sampling values as those from polya PDF.
  const double gain =  CLHEP::RandGamma::shoot(condContainers.rndmEngine, 1. + m_theta, (1. + m_theta)/m_meanGasGain);

  // total charge after avalanche
  const double total_charge = gain*ionized_charge;

  //************************************ spread charge among readout element **************************************

  // Charge Spread including tan(theta) resolution term.
  const double tan_theta = GLODIRE.perp()/GLODIRE.z();
  // The angle dependance on strip resolution goes as tan^2(angle)
  const double angle_dependency = std::hypot(m_posResIncident, m_posResAngular * tan_theta);

  const double cluster_posX = posOnSurf_strip.x();
  double peak_position = CLHEP::RandGaussZiggurat::shoot(condContainers.rndmEngine, cluster_posX, m_StripResolution*angle_dependency);

  // Each readout plane reads about half the total charge produced on the wire,
  // including a tan(theta) term to describe the increase of charge with incident angle
  const double norm = 0.5 * total_charge * m_stripChargeScale * std::hypot(1, m_chargeAngularFactor * tan_theta);
  // Strip cluster charge profile described by a double Gaussian.
  // The independent parameter x is in the units of strip channel, one strip channel = 3.2 mm,
  //   so convert position from mm to strip channel if it is not already.
  std::unique_ptr<TF1> clusterProfile = std::make_unique<TF1>("fgaus",
                                         "[0]*exp(-0.5*(x/[1])^2)+[2]*exp(-0.5*(x/[3])^2)",
                                         -300., 300.);
  clusterProfile->SetParameters(norm * m_clusterProfile[0], // normalization of 1st Gaussian
                                m_clusterProfile[1], // sigma of 1st Gaussian
                                norm * m_clusterProfile[2], // normalization of 2nd Gaussian
                                m_clusterProfile[3]); // sigma of 2nd Gaussian

  // Lower limit on strip charge (arbitrary limit), in pC, which has the same units as the parameter ionized_charge. 
  constexpr double tolerance_charge = 0.0005;
  // Set a maximum number of neighbour strips to avoid very long loop. This is an arbitrary number.
  constexpr unsigned int max_neighbor = 10;

  // Spread charge on the strips that are on the upper half of the strip cluster
  for (unsigned int iStrip = 0; iStrip <= max_neighbor; ++iStrip) {
    int currentStrip = stripNumber + iStrip;
    if (currentStrip > NumberOfStrips) break;

    // Get the strip identifier and create the digit
    newId = idHelper.channelID(layid, idHelper.multilayer(layid), idHelper.gasGap(layid), 
                               channelType, currentStrip, isValid);
    if (isValid) {
      Amg::Vector2D locpos{Amg::Vector2D::Zero()};
      if (!detEl->stripPosition(newId, locpos)) {
          ATH_MSG_WARNING("Failed to obtain local position for identifier " << m_idHelperSvc->toString(newId) );
      }

      // Estimate the digit charge
      // Position with respect to the peak of the charge curve
      double x_relative = locpos.x() - peak_position;
      // In clusterProfile curve, position should be in the units of strip channel
      double charge = clusterProfile->Integral(x_relative/(2*stripHalfPitch) - 0.5, x_relative/(2*stripHalfPitch) + 0.5);
      // If charge is too small, stop creating neighbor strip
      if (charge < tolerance_charge) break;

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

      addDigit(digits, newId, bctag, strip_time, charge);

      ATH_MSG_VERBOSE("Created a strip digit: strip number = " << currentStrip << ", charge = " << charge
                      << ", time = " << strip_time << ", time offset = " << strip_time-sDigitTimeStrip
                      << ", neighbor index = " << iStrip
                      << ", strip position = "<<Amg::toString(locpos, 2));
    }
  }

  // The lower half of the strip cluster
  for (unsigned int iStrip = 1; iStrip <= max_neighbor; ++iStrip) {
    int currentStrip = stripNumber - iStrip;
    if (currentStrip < 1) break;

    newId = idHelper.channelID(layid,idHelper.multilayer(layid), 
                              idHelper.gasGap(layid), channelType, currentStrip, isValid);
    if (isValid) {
      Amg::Vector2D locpos{Amg::Vector2D::Zero()};
      if (!detEl->stripPosition(newId, locpos)) {
        ATH_MSG_WARNING("Failed to obtain local position for identifier " << m_idHelperSvc->toString(newId) );
      }

      // Estimate the digit charge
      double x_relative = locpos.x() - peak_position;
      double charge = clusterProfile->Integral(x_relative/(2*stripHalfPitch) - 0.5, x_relative/(2*stripHalfPitch) + 0.5);
      if (charge < tolerance_charge) break;

      // Estimate digit time
      double strip_time = sDigitTimeStrip;
      // Time delay due to resistive layer
      if (m_doTimeOffsetStrip) {
        int indexFromMiddleStrip = ((x_relative + (iStrip * 2 - 0.75) * stripHalfPitch) > 0)? iStrip - 1 : iStrip;
        strip_time += getTimeOffsetStrip(indexFromMiddleStrip);
      }

      addDigit(digits, newId, bctag, strip_time, charge);

      ATH_MSG_VERBOSE("Created a strip digit: strip number = " << currentStrip << ", charge = " << charge
                      << ", time = " << strip_time << ", time offset = " << strip_time-sDigitTimeStrip
                      << ", neighbor index = " << iStrip
                      << ", strip position = " << Amg::toString(locpos, 2));
    }
  }
  // end of strip digitization

  if(m_channelTypes==1) {
    ATH_MSG_WARNING("Only digitize strip response !");
    return digits;
  }

  //##################################################################################
  //######################################### pad readout ##########################
  //##################################################################################
  ATH_MSG_DEBUG("sTgcDigitMaker::pad response ");
  channelType = sTgcIdHelper::sTgcChannelTypes::Pad;

  Identifier PAD_ID = idHelper.channelID(layid, idHelper.multilayer(layid), 
                                        idHelper.gasGap(layid), channelType, 1);// find the a pad id
  //************************************ find the nearest readout element **************************************
  int  surfHash_pad =  detEl->surfaceHash(PAD_ID);
  const Trk::PlaneSurface& SURF_PAD = detEl->surface(surfHash_pad); // get the pad surface

  const Amg::Vector3D hitOnSurface_pad = SURF_PAD.transform().inverse()*glob_ionization_pos;
  const Amg::Vector2D posOnSurf_pad(hitOnSurface_pad.x(), hitOnSurface_pad.y());

  
  insideBounds = SURF_PAD.insideBounds(posOnSurf_pad);

  if(insideBounds) {
    int padNumber = detEl->stripNumber(posOnSurf_pad, PAD_ID);
    if( padNumber == -1 ){
      // Verify if the energy deposit is at the boundary
      const double new_posX = (posOnSurf_pad.x()>0.0)? posOnSurf_pad.x()-length_tolerance
                                                    : posOnSurf_pad.x()+length_tolerance;
      const double new_posY = (posOnSurf_pad.y()>0.0)? posOnSurf_pad.y()-length_tolerance
                                                    : posOnSurf_pad.y()+length_tolerance;
      const Amg::Vector2D new_position(new_posX, new_posY);
      padNumber = detEl->stripNumber(new_position, PAD_ID);
      // Skip hit if still unable to obtain pad number
      if (padNumber < 1) {
        ATH_MSG_WARNING("Failed to obtain pad number " << m_idHelperSvc->toString(PAD_ID) );
        ATH_MSG_WARNING("Position on pad surface = (" << posOnSurf_pad.x() << ", " << posOnSurf_pad.y() << ")");
        return digits;
      }
    }
    newId = idHelper.channelID(layid, idHelper.multilayer(layid), 
                               idHelper.gasGap(layid), channelType, padNumber, isValid);
    if(isValid) {
      // Find centre position of pad
      Amg::Vector2D padPos{Amg::Vector2D::Zero()};
      if (!detEl->stripPosition(newId, padPos)) {
        ATH_MSG_ERROR("Failed to obtain local position for neighbor pad with identifier " << m_idHelperSvc->toString(newId) );
        return digits;
      }
      // Pad sharing needs to look at position on hit vs pad boundaries
      const Amg::Vector2D diff = posOnSurf_pad - padPos;
      double halfPadWidthX = 0.5*detEl->getPadDesign(newId)->channelWidth(posOnSurf_pad, true, true);
      double halfPadWidthY = 0.5*detEl->getPadDesign(newId)->channelWidth(posOnSurf_pad, false);

      // Charge sharing happens within 4mm window for pads
      // i.e. the charge is spread over a 2D gaussian of total radius about 4mm
      // This value depends on actual width of the distribution, but we don't have the
      // width of the charge distribution for pads. So lets consider 2.5*sigma, where
      // sigma is the width of the charge distribution for strips.
      double deltaX = halfPadWidthX - std::abs(diff.x());
      double deltaY= halfPadWidthY - std::abs(diff.y());
      bool isNeighX = deltaX < 2.5*m_clusterProfile[3];
      bool isNeighY = deltaY < 2.5*m_clusterProfile[3];
      // Pad width can be calculated to be very slightly larger than it should due to rounding errors
      // So if a hit falls on a given pad but is "outside" the width, just define it to be on the boundary of 2 pads.
      if (deltaX < 0.) deltaX = 0.1;
      if (deltaY < 0.) deltaY = 0.1;

      if (m_doPadSharing && (isNeighX || isNeighY)){
        std::pair<int, int> padEtaPhi(detEl->getPadDesign(newId)->channelNumber(padPos));
        // Phi == 1 at the right in the local geometry
        // In local coordinates, the pad to the right has phi' = phi - 1
        int newPhi = diff.x() > 0 ? padEtaPhi.second-1 : padEtaPhi.second+1;
        int newEta = diff.y() > 0 ? padEtaPhi.first+1 : padEtaPhi.first-1;
        bool validEta = newEta > 0 && newEta < detEl->getPadDesign(newId)->nPadH+1;
        bool validPhi = newPhi > 0 && newPhi < detEl->getPadDesign(newId)->nPadColumns+1;
    
        if (isNeighX && isNeighY && validEta && validPhi){
          // 4 pads total; makes life a bit harder. Corner of 4 valid pads
          Identifier neigh_ID_X = idHelper.padID(newId, idHelper.multilayer(newId), idHelper.gasGap(newId),
                                                  channelType, padEtaPhi.first, newPhi, isValid);
          Identifier neigh_ID_Y = idHelper.padID(newId, idHelper.multilayer(newId), idHelper.gasGap(newId),
                                                  channelType, newEta, padEtaPhi.second, isValid);
          Identifier neigh_ID_XY = idHelper.padID(newId, idHelper.multilayer(newId), idHelper.gasGap(newId),
                                                  channelType, newEta, newPhi, isValid);
          double xQfraction = getPadChargeFraction(deltaX);
          double yQfraction = getPadChargeFraction(deltaY);

          // Main pad gets 1 - Qfraction of total
          addDigit(digits, neigh_ID_X, bctag, sDigitTimePad, xQfraction*(1.-yQfraction)*0.5*total_charge);
          addDigit(digits, neigh_ID_Y, bctag, sDigitTimePad, yQfraction*(1.-xQfraction)*0.5*total_charge);
          addDigit(digits, neigh_ID_XY, bctag, sDigitTimePad, xQfraction*yQfraction*0.5*total_charge);
          addDigit(digits, newId, bctag, sDigitTimePad, (1.-xQfraction-yQfraction+xQfraction*yQfraction)*0.5*total_charge);
        } else if (isNeighX && validPhi){
          // There is only 1 neighbor, immediately to the left or right.
          Identifier neigh_ID = idHelper.padID(newId, idHelper.multilayer(newId), idHelper.gasGap(newId), 
                                               channelType, padEtaPhi.first, newPhi, isValid);

          // NeighborPad gets Qfraction of the total pad charge: 0.5*total_charge
          // Main pad gets 1 - Qfraction of total
          double xQfraction = getPadChargeFraction(deltaX);
          addDigit(digits, newId, bctag, sDigitTimePad, (1.-xQfraction)*0.5*total_charge);
          addDigit(digits, neigh_ID, bctag, sDigitTimePad, xQfraction*0.5*total_charge);
        }
        else if (isNeighY && validEta){
          // There is only 1 neighbor, immediately above or below
          Identifier neigh_ID = idHelper.padID(newId, idHelper.multilayer(newId), idHelper.gasGap(newId), 
                                               channelType, newEta, padEtaPhi.second, isValid);

          // NeighborPad gets Qfraction of the total pad charge: 0.5*total_charge
          // Main pad gets 1 - Qfraction of total
          double yQfraction = getPadChargeFraction(deltaX);
          addDigit(digits, newId, bctag, sDigitTimePad, (1.-yQfraction)*0.5*total_charge);
          addDigit(digits, neigh_ID, bctag, sDigitTimePad, yQfraction*0.5*total_charge);
        }

      }
      else{
        // No charge sharing: hit is nicely isolated within pad
        addDigit(digits, newId, bctag, sDigitTimePad, 0.5*total_charge);
      }

    }
    else if(padNumber != -1) {
      ATH_MSG_ERROR("Failed to obtain identifier " << m_idHelperSvc->toString(newId) );
    }
  }
  else {
    ATH_MSG_DEBUG("Outside of the pad surface boundary :" << m_idHelperSvc->toString(PAD_ID)<< " local position " <<posOnSurf_pad );
  }

  if(m_channelTypes==2) {
    ATH_MSG_WARNING("Only digitize strip/pad response !");
    return digits;
  }


  //##################################################################################
  //######################################### wire readout ##########################
  //##################################################################################
  ATH_MSG_DEBUG("sTgcDigitMaker::wire response ");
  channelType = sTgcIdHelper::sTgcChannelTypes::Wire;

    // Find the ID of the first wiregroup
  Identifier WIREGP_ID = idHelper.channelID(layid, idHelper.multilayer(layid),
                                            idHelper.gasGap(layid), channelType, 1);

   //************************************ find the nearest readout element **************************************
   insideBounds = SURF_WIRE.insideBounds(posOnSurf_wire);

    if(insideBounds) {
        // Determine the wire number

        int wiregroupNumber = detEl->stripNumber(posOnSurf_wire, WIREGP_ID);
        if( wiregroupNumber == -1 ){
          // Verify if the energy deposit is at the boundary
          const double new_posX = (posOnSurf_wire.x() > 0.0)? posOnSurf_wire.x() - length_tolerance
                                                           : posOnSurf_wire.x() + length_tolerance;
          const Amg::Vector2D new_position(new_posX, posOnSurf_wire.y());
          wiregroupNumber = detEl->stripNumber(new_position, WIREGP_ID);
          // Skip hit if still unable to obtain pad number
          if (wiregroupNumber < 1) {
            ATH_MSG_WARNING("Failed to obtain wire number " << m_idHelperSvc->toString(WIREGP_ID) );
            ATH_MSG_WARNING("Position on wire surface = (" << posOnSurf_wire.x() << ", " << posOnSurf_wire.y() << ")");
            return digits;
          }
        }

        // Find ID of the actual wiregroup
        newId = idHelper.channelID(layid, idHelper.multilayer(layid), idHelper.gasGap(layid), 
                                   channelType, wiregroupNumber, isValid);

        if(isValid) {
          int NumberOfWiregroups = detEl->numberOfStrips(newId);
          if(wiregroupNumber>=1&&wiregroupNumber<=NumberOfWiregroups) {
            addDigit(digits, newId, bctag, sDigitTimeWire, total_charge);
          }
        } // end of if(isValid)
        else if (wiregroupNumber != -1){
          ATH_MSG_ERROR("Failed to obtain wiregroup identifier " << m_idHelperSvc->toString(newId) );
        }
    }
    else {
      ATH_MSG_DEBUG("Outside of the wire surface boundary :" << m_idHelperSvc->toString(WIREGP_ID)<< " local position " <<posOnSurf_wire );
    }
    // end of wire digitization

  return digits;
}

//+++++++++++++++++++++++++++++++++++++++++++++++
sTgcDigitMaker::Ionization sTgcDigitMaker::pointClosestApproach(const MuonGM::sTgcReadoutElement* detEl,
                                                                const Identifier& id, 
                                                                int wireNumber,
                                                                const Amg::Vector3D& preStepPos,
                                                                const Amg::Vector3D& postStepPos) const {
  // tolerance in mm
  constexpr double tolerance = 0.001;
  // minimum segment length
  constexpr double min_length = 0.1;

  // Position of the ionization
  Ionization ionization;

  // Wire number should be one or greater
  if (wireNumber < 1) return ionization;

  // Wire number too large
  if (wireNumber > detEl->numberOfWires(id)) return ionization;

  // Finding smallest distance and the points at the smallest distance.
  //  The smallest distance between two lines is perpendicular to both lines.
  //  We can construct two lines in the wire surface local coordinate frame:
  //  - one for the hit segment with equation h0 + t * v_h, where h0 is a point
  //    and v_h is the unit vector of the hit segment
  //  - another for the wire with similar equation w0 + s * v_w, where w0 is a
  //    point and v_w is the unit vector of the wire line
  //  Then it is possible to determine the factors s and t by requiring the
  //  dot product to be zero:
  //   1. (h0 - w0) \dot v_h = 0
  //   2. (h0 - w0) \dot v_w = 0

  // Wire pitch
  const double wire_pitch = detEl->wirePitch();
  // Wire local position on the wire plane, the y-coordinate is arbitrary and z-coordinate is zero
  const double wire_posX = detEl->positionFirstWire(id) + (wireNumber - 1) * wire_pitch;
  const Amg::Vector3D wire_position(wire_posX, postStepPos.y(), 0.);
  // The wires are parallel to Y in the wire plane's coordinate frame
  const Amg::Vector3D wire_direction{Amg::Vector3D::UnitY()};

  // particle trajectory
  Amg::Vector3D hit_direction{postStepPos - preStepPos};
  const double seg_length = hit_direction.mag();
  if (seg_length > tolerance) hit_direction /= seg_length;

  // Find the point on the track segment that is closest to the wire
  if (seg_length < min_length) {
    ionization.posOnSegment = postStepPos;
    ionization.posOnWire = wire_position;
    ionization.distance = std::hypot(postStepPos.x() - wire_posX, postStepPos.z());
    return ionization;
  }

  // Dot product between the wire and hit direction
  const double cos_theta = wire_direction.dot(hit_direction);
  // distance between the hit and wire
  const Amg::Vector3D dist_wire_hit = postStepPos - wire_position;

  // Verifier the special case where the two lines are parallel
  if (std::abs(cos_theta - 1.0) < tolerance) {
    ATH_MSG_DEBUG("The track segment is parallel to the wire, position of digit is undefined");
    ionization.posOnSegment = 0.5 * (postStepPos + preStepPos);
    ionization.posOnWire = Amg::Vector3D(wire_posX, ionization.posOnSegment.y(), 0.0);
    ionization.distance = std::hypot(ionization.posOnSegment.x() - wire_posX,
                                     ionization.posOnSegment.z());
    return ionization;
  }

  // Perpendicular component squared
  const double sin_theta_2 = 1.0 - cos_theta * cos_theta;

  //* Point on the hit segment
  const double factor_hit = (-dist_wire_hit.dot(hit_direction)
                           + dist_wire_hit.dot(wire_direction) * cos_theta) / sin_theta_2;
  Amg::Vector3D ionization_pos = postStepPos + factor_hit * hit_direction;

  // If the point is on the track segment, then compute the other point on the wire.
  // Otherwise, set the ionization at the pre-step position and compute where it
  // should be on the wire.
  Amg::Vector3D pos_on_wire{Amg::Vector3D::Zero()};
  if (factor_hit < seg_length) {
    //* Point on the wire line
    const double factor_wire = (dist_wire_hit.dot(wire_direction)
                              - dist_wire_hit.dot(hit_direction) * cos_theta) / sin_theta_2;
    pos_on_wire = wire_position + factor_wire * wire_direction;
  } else {
    ionization_pos = preStepPos;
    pos_on_wire = wire_position - (seg_length * cos_theta) * wire_direction;
  }

  // Save the distance and ionization position
  ionization.distance = (ionization_pos - pos_on_wire).mag();
  ionization.posOnSegment = ionization_pos;
  ionization.posOnWire = pos_on_wire;

  return ionization;
}

//+++++++++++++++++++++++++++++++++++++++++++++++
void sTgcDigitMaker::addDigit(sTgcDigitVec& digits, 
                              const Identifier& id, 
                              const uint16_t bctag, 
                              const double digittime,
                              const double charge) {
  
  constexpr double tolerance = 0.1;
  if (std::find_if(digits.begin(),digits.end(), [&](std::unique_ptr<sTgcDigit>& known) {
      return known->identify() == id && std::abs(digittime - known->time()) < tolerance;
  }) == digits.end()) {
    digits.push_back(std::make_unique<sTgcDigit>(id, bctag, digittime, charge, 0, 0));
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++
StatusCode sTgcDigitMaker::readFileOfTimeArrival() {
  // Verify the file sTGC_Digitization_timeArrival.dat exists
  const std::string file_name = "sTGC_Digitization_timeArrival.dat";
  std::string file_path = PathResolver::find_file(file_name, "DATAPATH");
  if(file_path.empty()) {
    ATH_MSG_FATAL("readFileOfTimeWindowOffset(): Could not find file " << file_name );
    return StatusCode::FAILURE;
  }

  // Open the sTGC_Digitization_timeArrival.dat file
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

  // Close the file
  ifs.close();
  return StatusCode::SUCCESS;
}

//+++++++++++++++++++++++++++++++++++++++++++++++
sTgcDigitMaker::GammaParameter sTgcDigitMaker::getGammaParameter(double distance) const {

  const double d = std::abs(distance); 
  // Find the parameters assuming the container is sorted in ascending order of 'lowEdge'
  int index{-1};
  for (const auto& par: m_gammaParameter) {
    if (d < par.lowEdge) {
      break;
    }
    ++index;
  }
  return m_gammaParameter.at(index);
}

//+++++++++++++++++++++++++++++++++++++++++++++++
double sTgcDigitMaker::getMostProbableArrivalTime(double distance) const {

  const double d{std::abs(distance)};
  double mpt{0};
  for (size_t t = 0 ; t < m_mostProbableArrivalTime.size(); ++t){
    mpt += m_mostProbableArrivalTime[t] * std::pow(d, t);
  }
  return mpt;
}

//+++++++++++++++++++++++++++++++++++++++++++++++
StatusCode sTgcDigitMaker::readFileOfTimeOffsetStrip() {
  // Verify the file sTGC_Digitization_timeOffsetStrip.dat exists
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

  // Read the input file
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

//+++++++++++++++++++++++++++++++++++++++++++++++
double sTgcDigitMaker::getTimeOffsetStrip(size_t neighbor_index) const {
    return m_timeOffsetStrip.at(std::min(neighbor_index, m_timeOffsetStrip.size() -1));
}

//+++++++++++++++++++++++++++++++++++++++++++++++
double sTgcDigitMaker::getPadChargeFraction(double distance) {
  // The charge fraction that is found past a distance x away from the
  // centre of a 2D gaussian distribution of width of cluster profile is
  // described by a modified error function.

  // The modified error function perfectly describes
  // the pad charge sharing distribution figure 16 of the sTGC
  // testbeam paper https://arxiv.org/pdf/1509.06329.pdf

  return 0.5 * (1.0 - std::erf( distance / (std::sqrt(2) * m_clusterProfile[3])));
}
