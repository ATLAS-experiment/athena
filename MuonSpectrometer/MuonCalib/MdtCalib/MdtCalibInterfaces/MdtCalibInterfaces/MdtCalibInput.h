/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MDTCALIBINTEFACES_MDTCALIBINPUT_H
#define MDTCALIBINTEFACES_MDTCALIBINPUT_H

#include "GeoPrimitives/GeoPrimitives.h"

#include <CxxUtils/CachedValue.h>
#include <xAODMuonPrepData/MdtDriftCircleFwd.h>
#include <Identifier/Identifier.h>
#include <Identifier/IdentifierHash.h>
#include <variant>
///
namespace MuonGM{
  class MuonDetectorManager;
  class MdtReadoutElement;
}
namespace MuonGMR4{
  class MuonDetectorManager;
  class MdtReadoutElement;
}
namespace Muon{
  class MdtPrepData;
}

namespace Trk {
  class StraightLineSurface;
}

namespace ActsTrk {
  class GeometryContext;
}
class MdtDigit;

class MdtCalibInput {
    public:
      /** @brief Minimal constructor in the PhaseII geomerty setup. It takes all necessary ingredients to run
       *         later the calibration loop.
       *  @param id: Identifier of the hit to calibrate
       *  @param adc: Recorded adc counts from the tube
       *  @param tdc: Recorded tdc counts from the tube
       *  @param detMgr: Pointer to the associated readout element
       *  @param gctx: Geometry context to globally align the tube within ATLAS */
      MdtCalibInput(const Identifier& id,
                    const int16_t adc,
                    const int16_t tdc,
                    const MuonGMR4::MdtReadoutElement* reEle,
                    const ActsTrk::GeometryContext& gctx);
    
      /** @brief Minimal constructor in the legacy geomerty setup. It takes all necessary ingredients to run
       *         later the calibration loop.
       *  @param id: Identifier of the hit to calibrate
       *  @param adc: Recorded adc counts from the tube
       *  @param tdc: Recorded tdc counts from the tube
       *  @param reEle: Pointer to the associated readout element  */
      MdtCalibInput(const Identifier& id,
                    const int16_t adc,
                    const int16_t tdc,
                    const MuonGM::MdtReadoutElement* reEle);

       /** @brief Constructor taking the Mdt digit & the legacy II detector manager
         *  @param digit: Digit containing all adc & tdc information
         *  @param detMgr: Reference to the Detector manager to fetch the Proper readout element
         *  @param gctx: Geometry context to globally align the tube within ATLAS */
      MdtCalibInput(const MdtDigit& digit,
                    const MuonGM::MuonDetectorManager& detMgr);
      
      /** @brief Constructor taking the Mdt digit & the Phase II detector manager
       *  @param digit: Digit containing all adc & tdc information
       *  @param detMgr: Reference to the Detector manager to fetch the Proper readout element
       *  @param gctx: Geometry context to globally align the tube within ATLAS */
      MdtCalibInput(const MdtDigit& digit,
                    const MuonGMR4::MuonDetectorManager& detMgr,
                    const ActsTrk::GeometryContext& gctx);
      /** Constructor taking the MdtPrepdata. The   */
      MdtCalibInput(const Muon::MdtPrepData& prd);
      /** Constructor taking taking the xAOD::MdtDriftCircle
        * @param prd: Reference to the uncalibrated Drift circle
        * @param gctx: Geometry context to place the drift circle globally within ATLAS */
      MdtCalibInput(const xAOD::MdtDriftCircle& prd,
                    const ActsTrk::GeometryContext& gctx);


      MdtCalibInput(MdtCalibInput&& other) = default;
      MdtCalibInput& operator=(MdtCalibInput&& other) = default;

      ~MdtCalibInput();
      /// Returns the Identifier of the hit
      const Identifier& identify() const;
      /// Returns the tdc counts of the hit
      int16_t tdc() const;
      /// Returns the amount of accumulated charge
      int16_t adc() const;
      /// Returns the legacy readout element
      const MuonGM::MdtReadoutElement* legacyDescriptor() const;
      /// Returns the R4 readout element
      const MuonGMR4::MdtReadoutElement* decriptor() const;
    
      /// Returns the point of closest approach to the wire
      const Amg::Vector3D& closestApproach() const;
      /// Sets the closest approach
      void setClosestApproach(const Amg::Vector3D& approach);

      /// Returns the track direction (Can be zero)
      const Amg::Vector3D& trackDirection() const;
      /** @brief Sets the direction of the externally determined track
       *  @param trackDir: Direction vector of the track (global frame)
       *  @param hasPhi: Flag whether the track direction contains phi information*/
      void setTrackDirection(const Amg::Vector3D& trackDir,
                             bool hasPhi);
      /** @brief Returns whether the track has a phi constaint or not */
      bool trackDirHasPhi() const;

      /// Returns the time of flight
      double timeOfFlight() const;
      /// Sets the time of flight (Usually globPos.mag() * inverseSpeed of light)
      void setTimeOfFlight(const double toF);

      /// Returns the trigger offset time
      double triggerTime() const;
      /// Sets the trigger offset time
      void setTriggerTime(const double trigTime);
      /// Returns the distance to track (signed)
      double distanceToTrack() const;
      enum class BFieldComp{
        alongWire = 0,
        alongTrack = 1,
      };
      /// Splits the B-field into the components that point along the transverse
      /// track direction & along the tube wire
      Amg::Vector2D projectMagneticField(const Amg::Vector3D& fieldInGlob) const;
      /// Calculates the distance that the signal has to travel along the wire
      double signalPropagationDistance() const;
      /// Returns the assocaited ideal surface  (Throw exception if no legacy RE is available)
      const Trk::StraightLineSurface& legacySurface() const;
      /// Returns the tube length
      double tubeLength() const;
      /// Returns the sign of the readout position in local coordinates
      double readOutSide() const;
      /// Returns the inner tube radius
      double innerTubeR() const;

      friend std::ostream& operator<<(std::ostream& ostr, const MdtCalibInput& input) {
          input.print(ostr);
          return ostr;
      }
  private:
    /** @brief Print the object on screen
     *  @param ostr: Outstream into which the object is piped  */
    void print(std::ostream& ostr) const;
    /** @brief Local to global transformation of the tube */
    const Amg::Transform3D& localToGlobal() const;
    /** @brief Tube identifier */
    Identifier m_id{};
    /** @brief Adc counts of the hit */
    int16_t m_adc{0};
    /** @brief Tdc counts of the hit */
    int16_t m_tdc{0};

    /** @brief Geometry context, needed to fetch the alignment */
    const ActsTrk::GeometryContext* m_gctx{nullptr};
    /** @brief Variant type to store the legacy & Phase-II style readout geometry in a single variable */
    using ReadoutEle_t = std::variant<const MuonGM::MdtReadoutElement*, const MuonGMR4::MdtReadoutElement*>;
    /** @brief Pointer to the associated readout element */
    ReadoutEle_t m_RE{};
    /** @brief Measurement hash of the Identifier (needed for Phase II) */
    IdentifierHash m_hash{};
    /** @brief Point of closest approach of the track */  
    CxxUtils::CachedValue<Amg::Vector3D> m_approach{};
    /** @brief Global track direction */
    Amg::Vector3D m_trackDir{Amg::Vector3D::Zero()};
    /** @brief Does the track direction contain a phi constraint */
    bool m_trackHasPhi{false};
    /// Time of flight 
    CxxUtils::CachedValue<double> m_ToF{};
    /// Trigger time
    double m_trigTime{0.};
    /// Distance to track (signed)
    double m_distToTrack{0.};

};




#endif
