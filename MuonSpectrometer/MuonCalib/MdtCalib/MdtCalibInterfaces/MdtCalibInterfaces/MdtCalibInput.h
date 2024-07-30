/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MDTCALIBINTEFACES_MDTCALIBINPUT_H
#define MDTCALIBINTEFACES_MDTCALIBINPUT_H

#include "GeoPrimitives/GeoPrimitives.h"

#include <GaudiKernel/PhysicalConstants.h>
#include <xAODMuonPrepData/MdtDriftCircle.h>
#include <CxxUtils/CachedUniquePtr.h>
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
  class SaggedLineSurface;
}

class MdtDigit;
class ActsGeometryContext;
class Identifier;
class IdentifierHash;

class MdtCalibInput {
public:
    /** Constructor taking the MdtDigits --
     *        Typically used inside digitization or RDO -> PRD conversion
     *  Given that digits don't have intrinsically any readout element attached,
     *  the detector manager needs to parsed for fetching the proper element
     */
    MdtCalibInput(const MdtDigit& digit,
                  const MuonGM::MuonDetectorManager& detMgr);
    
    MdtCalibInput(const MdtDigit& digit,
                  const MuonGMR4::MuonDetectorManager& detMgr,
                  const ActsGeometryContext& gctx);
    
    /** Constructor taking the MdtPrepdata  */
    MdtCalibInput(const Muon::MdtPrepData& prd);
    /** Constructor taking taking the xAOD::MdtDriftCircle
      * @param prd: Reference to the uncalibrated Drift circle
      * @param gctx: Geometry context to place the drift circle globally within ATLAS
     */
    MdtCalibInput(const xAOD::MdtDriftCircle& prd,
                  const ActsGeometryContext& gctx);
    
    /** Constructor taking the */
    MdtCalibInput(const Identifier& id, 
                  const int adc, 
                  const int tdc, 
                  const Amg::Vector3D& globalPos);

    ~MdtCalibInput();
    /// Returns the Identifier of the hit
    const Identifier& identify() const;
    /// Returns the tdc counts of the hit
    int tdc() const;
    /// Returns the amount of accumulated charge
    int adc() const;
    /// Returns whether the constructing digit has been masked
    bool isMasked() const;
    /// Returns the legacy readout element
    const MuonGM::MdtReadoutElement* legacyDescriptor() const;
    /// Returns the R4 readout element
    const MuonGMR4::MdtReadoutElement* decriptor() const;
    
    /// Returns the global position of the hit
    const Amg::Vector3D& globalPos() const;
    /// Returns the point of closest approach to the wire
    const Amg::Vector3D& closestApproach() const;
    /// Sets the closest approach
    void setClosestApproach(const Amg::Vector3D& approach);

    /// Returns the track direction (Can be zero)
    const Amg::Vector3D& trackDirection() const;
    /// Sets the track direction if it's given from an external seed
    void setTrackDirection(const Amg::Vector3D& trackDir);
    

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
    const Trk::SaggedLineSurface& idealSurface() const;

    /// Returns the surface modeling the wire sagging at the point of 
    /// the closest approach (Throw exceptions if no legacy RE is available)
    const Trk::StraightLineSurface& saggedSurface() const;
    /// Releases the sagged line surface (Can be a nullptr)
    std::unique_ptr<Trk::StraightLineSurface> releaseSurface();

    /// Returns the center of the associated surface
    const Amg::Vector3D& surfaceCenter() const;
    /// Returns the center of the sagged line surface
    const Amg::Vector3D& saggedSurfCenter() const;
    /// Returns the tube length
    double tubeLength() const;
    /// Returns the sign of the readout position in local coordinates
    double readOutSide() const;
private:
  
  Identifier m_id{0};
  bool m_isMasked{false};
  int m_adc{0};
  int m_tdc{0};
  /// Pointer to the readout elements of the legacy geometry 
  const MuonGM::MdtReadoutElement* m_legRE{nullptr};

  const ActsGeometryContext* m_gctx{nullptr};
  const MuonGMR4::MdtReadoutElement* m_RE{nullptr};
  IdentifierHash m_hash{};
  
  Amg::Transform3D m_globToLoc{Amg::Transform3D::Identity()};
  Amg::Vector3D m_globPos{Amg::Vector3D::Zero()};
  Amg::Vector3D m_approach{m_globPos};
  Amg::Vector3D m_trackDir{Amg::Vector3D::Zero()};
  
  /// Time of flight 
  static constexpr double s_inverseSpeed{1. / Gaudi::Units::c_light};
  double m_ToF{m_globPos.mag() * s_inverseSpeed};
  /// Trigger time
  double m_trigTime{0.};
  /// Distance to track (signed)
  double m_distToTrack{0.};

  CxxUtils::CachedUniquePtrT<Trk::StraightLineSurface> m_saggedSurf{};

};

std::ostream& operator<<(std::ostream& ostr, const MdtCalibInput& input);


#endif
