/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class sTgcDigitMaker
   @section Class, Methods and Properties

   All functionality of sTGC digitization is implemented to this
   class.
*/

#ifndef sTGCDigitizationR4_STGCDIGITMAKER_H
#define sTGCDigitizationR4_STGCDIGITMAKER_H

#include "AthenaBaseComps/AthMessaging.h"
#include "MuonCondData/DigitEffiData.h"
#include "MuonCondData/NswCalibDbThresholdData.h"
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonDigitContainer/sTgcDigitContainer.h"
#include "CxxUtils/ArrayHelper.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "HitManagement/TimedHitPtr.h"

namespace CLHEP {
  class HepRandomEngine;
  class HepRandom;
}
/**
 * @class sTgcDigitMaker
 * @brief Handles the digitization of sTGC hits, converting simulated hits into sTGC digits.
 */

namespace MuonR4{
class sTgcDigitMaker : public AthMessaging {
 public:
  /**
   * @brief Constructor initializing digitization parameters.
   */
  enum class digitMode: std::uint8_t  {
    StripsOnly = 1,
    StripsAndPads = 2,
    AllChType = 3,
  };
  sTgcDigitMaker(const MuonGMR4::MuonDetectorManager* detMgr,
                 digitMode mode,
                 double meanGasGain, 
                 bool doPadChargeSharing);

  /**
   * @brief Destructor.
   */
  virtual ~sTgcDigitMaker();

  /**
   * @brief Initialize digitization parameters, including reading necessary data files.
   */
  StatusCode initialize();

  /**
   * @struct DigiConditions
   * @brief Holds necessary conditions and data for digitization.
   */
  struct DigiConditions {
     const Muon::DigitEffiData* efficiencies{nullptr};
     const NswCalibDbThresholdData* thresholdData{nullptr};
     CLHEP::HepRandomEngine* rndEngine{nullptr};
  };
  
using TimedHit = TimedHitPtr<xAOD::MuonSimHit>;
using ReadoutChannelType = sTgcIdHelper::sTgcChannelTypes;

  /**
   * @brief Digitize a given hit
   * @param condContainers Conditions required for digitization.
   * @param hit The simulated hit to be digitized.
   */
  using sTgcDigitVec = std::vector<std::unique_ptr<sTgcDigit>>;
  sTgcDigitVec executeDigi(const DigiConditions& condContainers, const TimedHit& hit) const;

 private:
  /**
   * @struct GammaParameter
   * @brief Stores gamma distribution parameters for estimating digit time.
   */
  struct GammaParameter {
    double lowEdge{0.};
    double kParameter{0.};
    double thetaParameter{0.};
  };

  /**
   * @struct Ionization
   * @brief Holds information about ionization points in the gas volume.
   */
  struct Ionization {
    double distance{-9.99}; //smallest distance bet the wire and particle trajectory
    double time{0.};        // time of arrival
    Amg::Vector3D posOnSegment{Amg::Vector3D::Zero()}; // Point of closest approach
    Amg::Vector3D posOnWire{Amg::Vector3D::Zero()}; // Position on the wire
  };

  /** @brief Helper struct to carry the digit information around */
  struct DigiInput {
      /** @brief Identifier of the simulated hit to digitize */
      Identifier hitId{};
      /** @brief Position of the hit on the surface */
      Amg::Vector3D posOnSurf{Amg::Vector3D::Zero()};
      /** @brief Direction of the propagating particle */
      Amg::Vector3D hitDir{Amg::Vector3D::Zero()};
      /** @brief Total deposited charge in the gasGap */
      double totalCharge{0.};
      /** @brief Time of arrival on the sensor */
      double time{0.};
      /** @brief Readout element associated */
      const MuonGMR4::sTgcReadoutElement* reEle{nullptr};
  };
  /**
   * @brief Computes the ionization point for a hit
   */
  bool getIonizationPoint(const TimedHit& hit, const DigiConditions& condContainers, Ionization& ionization) const;

  /**
   * @brief Calculates total charge from energy deposit, including gas gain
   */
  double calculateTotalCharge(double energyDeposit, CLHEP::HepRandomEngine* rndEngine) const;

  /**
   * @brief Processes strip digitization for a given hit
   */
  sTgcDigitVec processStripDigitization(const DigiConditions& condContainers,
                                        const DigiInput& digiInput) const;

  /**
   * @brief Processes pad digitization for a given hit
   */
  sTgcDigitVec processPadDigitization(const DigiInput& digiInput) const;

  /**
   * @brief Processes wire digitization for a given hit
   */
  sTgcDigitVec processWireDigitization(const DigiInput& digiInput) const;

  /**
   * @brief Handles charge sharing for strip clusters
   */
  sTgcDigitVec processStripChargeSharing(const DigiInput& digiInput,
                                         const double peak_position,
                                         const int stripNumber) const;

  /**
   * @brief Handles charge sharing for pad clusters
   */
  sTgcDigitVec processPadChargeSharing(const DigiInput& digiInput,
                                       const int padEta,
                                       const int padPhi) const;

  /**
   * @brief Adds a digit to the appropriate cache.
   */
  static void addDigit(sTgcDigitVec& digits, 
                       const Identifier& id, 
                       double digittime, 
                       double charge);

  /**
   * @brief Reads time arrival data file.
   */
  StatusCode readFileOfTimeArrival();
  
  /**
   * @brief Reads strip time offset data file.
   */
  StatusCode readFileOfTimeOffsetStrip();
  
  /**
   * @brief Computes the closest approach between a trajectory and a wire segment.
   */
  /** Given two segments, e.g. a particle trajectory and a sTGC wire, solve for the
   *  two points, the point on the trajectory and the point on the wire, where the
   *  distance between the two segments is the smallest.
   *
   *  Positions returned are in the local coordinate frame of the wire plane.
   *  Returns an object with distance of -9.99 in case of error.
   */
  Ionization pointClosestApproach(const MuonGMR4::StripLayer& stripLayer,
                                  int wireNumber, 
                                  const Amg::Vector3D& locHitPos,
                                  const Amg::Vector3D& locHitDir,
                                  const double stepLength) const;

  /**
   * @brief Gets the time offset for a strip cluster.
   */
  /** Get digit time offset of a strip depending on its relative position to
   *  the strip at the centre of the cluster.
   *  It returns 0 ns by default, as well as when it fails or container is empty.
   */
  double getTimeOffsetStrip(size_t neighbor_index) const;
  
  /**
   * @brief Computes charge fraction shared among pads.
   */
  static double getPadChargeFraction(double distance);

  /**
   * @brief Retrieves gamma distribution parameters based on distance.
   */
  GammaParameter getGammaParameter(double distance) const;
  
  /**
   * @brief Computes the most probable arrival time based on the distance of closest approach.
   */
  double getMostProbableArrivalTime(double distance) const;

  // Parameters of the gamma probability distribution function
  std::vector<GammaParameter> m_gammaParameter;
  std::array<double, 5> m_mostProbableArrivalTime{make_array<double, 5>(0.)};
  std::array<double, 6> m_timeOffsetStrip{make_array<double, 6>(0.)};

  const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
  const Muon::IMuonIdHelperSvc* m_idHelperSvc{m_detMgr->idHelperSvc()};
  const sTgcIdHelper& m_idHelper{m_idHelperSvc->stgcIdHelper()};
  // Computes the charge on a strip given the limits of integral using the error function
  // In R3, strip cluster charge profile was defined by a double gaussian for every digit
  // which is computationally inefficient, here we use result of double gaussian integral
  // to find the charge on each strip of the cluster, where M, N are upper and lower 
  // limits respectively 
   // Strip cluster charge profile: [0] = sigma of inner Gaussian, [1] = sigma of outer Gaussian
  static constexpr std::array<double, 2> m_clusterParams{0.573, 1.092};
  double chargeIntegral(double N, double M) const;

  /**
    define offsets and widths of time windows for signals from
    wiregroups and strips. The offsets are defined as relative time
    diffference with respect to the time after TOF and cable
    length corrections. Bunch crossing time is specified.
  */
  // Digitization parameters
  digitMode m_digitMode{digitMode::AllChType};
  double m_theta{10}; // theta=10 value best matches the PDF
  double m_meanGasGain{5.e4};  // mean gain estimated from ATLAS note "ATL-MUON-PUB-2014-001"
  bool m_doPadSharing{false};
  
  // Flag to enable strip time offset
  bool m_doTimeOffsetStrip{false};
  // Angular strip resolution parameter
  double m_StripResolution{0.0949};
  double m_posResIncident{1.};
  double m_posResAngular{0.305/m_StripResolution};

  // Dependence of energy deposited on incident angle
  double m_chargeAngularFactor{4.0};
};
}
#endif
