/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class sTgcDigitMaker
   @section Class, Methods and Properties

   All functionality of sTGC digitization is implemented to this
   class.
*/

#ifndef STGCDIGITMAKER_H
#define STGCDIGITMAKER_H

#include "AthenaBaseComps/AthMessaging.h"
#include "MuonSimEvent/sTGCSimHit.h"
#include "MuonSimEvent/sTgcHitIdHelper.h"
#include "MuonCondData/DigitEffiData.h"
#include "MuonCondData/NswCalibDbThresholdData.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "CxxUtils/ArrayHelper.h"
#include "MuonDigitContainer/sTgcDigit.h"
namespace CLHEP {
  class HepRandomEngine;
  class HepRandom;
}


class sTgcDigitCollection;
class sTgcHitIdHelper;
class sTgcIdHelper;

//--- class description
class sTgcDigitMaker : public AthMessaging {
  //------ for public
 public:

  sTgcDigitMaker(const Muon::IMuonIdHelperSvc* idHelperSvc,
                 const int channelTypes,
                 double meanGasGain, 
                 bool doPadChargeSharing,
                 double stripChargeScale,
                 bool applyAsBuiltBLines);

  virtual ~sTgcDigitMaker();

  /**
     Initializes sTgcHitIdHelper and sTgcIdHelper,
     and call the functions to read files containing digitization parameters.
  */
  StatusCode initialize();

  /**
     Digitize a given hit, determining the time and charge spread on wires, pads and strips.
  */
  struct DigiConditions {
     const MuonGM::MuonDetectorManager* detMgr{nullptr};
     const DigitEffiData* efficiencies{nullptr};
     const NswCalibDbThresholdData* thresholdData{nullptr};
     CLHEP::HepRandomEngine* rndmEngine{nullptr};
  };

  using sTgcDigitVec = std::vector<std::unique_ptr<sTgcDigit>>;
  sTgcDigitVec executeDigi(const DigiConditions& condContainers, const sTGCSimHit& hit) const;

  //====== for private
 private:

  /** Parameters of a gamma probability distribution function, required for
   *  estimating wire digit's time of arrival.
   *  More detail in the dat file.
   */
  struct GammaParameter {
    double lowEdge{0.}; // low side of the interval in ns
    double kParameter{0.};
    double thetaParameter{0.};
  };

  /** Ionization object with distance, position on the hit segment and
   *  position on the wire.
   */
  struct Ionization {
    double distance{-9.99}; //smallest distance bet the wire and particle trajectory
    Amg::Vector3D posOnSegment{Amg::Vector3D::Zero()}; // Point of closest approach
    Amg::Vector3D posOnWire{Amg::Vector3D::Zero()}; // Position on the wire
  };

  //uint16_t bcTagging(const double digittime, const int channelType) const;
  static void addDigit(sTgcDigitVec& digits, 
                const Identifier& id, 
                const uint16_t bctag, 
                const double digittime, 
                const double charge) ;

  /** Read share/sTGC_Digitization_timeArrival.dat */
  StatusCode readFileOfTimeArrival();
  /** Read share/sTGC_Digitization_timeOffsetStrip.dat */
  StatusCode readFileOfTimeOffsetStrip();

  
  
  /** Determine the points where the distance between two segments is smallest.
   *  Given two segments, e.g. a particle trajectory and a sTGC wire, solve for the
   *  two points, the point on the trajectory and the point on the wire, where the
   *  distance between the two segments is the smallest.
   *
   *  Positions returned are in the local coordinate frame of the wire plane.
   *  Returns an object with distance of -9.99 in case of error.
   */
  Ionization pointClosestApproach(const MuonGM::sTgcReadoutElement* readoutEle,
                                  const Identifier& id, 
                                  int wireNumber, 
                                  const Amg::Vector3D& preStepPos,
                                  const Amg::Vector3D& postStepPos) const;

  /** Get digit time offset of a strip depending on its relative position to
   *  the strip at the centre of the cluster.
   *  It returns 0 ns by default, as well as when it fails or container is empty.
   */
  double getTimeOffsetStrip(size_t neighbor_index) const;

  static double getPadChargeFraction(double distance) ;

  /** Find the gamma pdf parameters of a given distance */
  GammaParameter getGammaParameter(double distance) const;
  /** Get the most probable time of arrival */
  double getMostProbableArrivalTime(double distance) const;

  // Parameters of the gamma pdf required for determining digit time
  std::vector<GammaParameter> m_gammaParameter;
  // 4th-order polymonial describing the most probable time as function of the distance of closest approach
  std::array<double, 5> m_mostProbableArrivalTime{make_array<double, 5>(0.)};

  // Time offset to add to Strip timing
  std::array<double, 6> m_timeOffsetStrip{make_array<double, 6>(0.)};

  const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};

  /**
     define offsets and widths of time windows for signals from
     wiregangs and strips. The offsets are defined as relative time
     diffference with respect to the time after TOF and cable
     length corrections. Bunch crossing time is specified.
  */

  int m_channelTypes{3}; // 1 -> strips, 2 -> strips+wires, 3 -> strips/wires/pads
  double m_theta{0.8}; // theta=0.8 value best matches the PDF
  double m_meanGasGain{5.e4};  // mean gain estimated from ATLAS note "ATL-MUON-PUB-2014-001"
  bool m_doPadSharing{false};


  // Flag to enable strip time offset
  bool m_doTimeOffsetStrip{false};
  // Angular strip resolution parameter
  double m_StripResolution{0.0949};
  double m_posResIncident{1.};
  double m_posResAngular{0.305/m_StripResolution};
  // Strip cluster charge profile: [0] = norm of inner Gaussian, [1] = sigma of inner Gaussian,
  //   [2] = norm of outer Gaussian, [3] = sigma of outer Gaussian
  static constexpr std::array<double, 4> m_clusterProfile{0.350, 0.573, 0.186, 1.092};
  // Dependence of energy deposited on incident angle
  double m_chargeAngularFactor{4.0};
  // Overall factor to scale the total strip cluster charge
  double m_stripChargeScale{0.4};

  bool m_applyAsBuiltBLines{false}; // apply as built and B-lines parameters
};

#endif
