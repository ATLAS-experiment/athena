/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCalibInterfaces/MdtCalibInput.h"

#include "GaudiKernel/PhysicalConstants.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

#include <MuonReadoutGeometry/MdtReadoutElement.h>
#include <MuonReadoutGeometry/MuonDetectorManager.h>
///
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
///
#include <MuonDigitContainer/MdtDigit.h>
#include <MuonPrepRawData/MdtPrepData.h>
#include <TrkSurfaces/StraightLineSurface.h>
#include <GeoModelHelpers/throwExcept.h>

std::ostream& operator<<(std::ostream& ostr, const MdtCalibInput& input){
   ostr<<"adc: "<<input.adc()<<", ";
   ostr<<"tdc: "<<input.tdc()<<", ";
   ostr<<"is masked: "<<(input.isMasked() ? "yay" : "nay")<<", ";
   ostr<<"global position: "<<Amg::toString(input.globalPos(), 2)<<", ";
   ostr<<"closest approach: "<<Amg::toString(input.closestApproach(), 2)<<", ";
   ostr<<"global direction: "<<Amg::toString(input.trackDirection(), 2)<<", ";
   ostr<<"prop distance: "<<input.signalPropagationDistance()<<", ";
   
   ostr<<"ToF: "<<input.timeOfFlight()<<", ";
   ostr<<"trigger time: "<<input.triggerTime();
   return ostr;
}

MdtCalibInput::~MdtCalibInput() = default;
MdtCalibInput::MdtCalibInput(const MdtDigit& digit, 
                             const MuonGM::MuonDetectorManager& detMgr):
   m_id{digit.identify()},
   m_isMasked{digit.is_masked()},
   m_adc{digit.adc()},
   m_tdc{digit.tdc()},
   m_legRE{detMgr.getMdtReadoutElement(m_id)},
   m_globToLoc{m_legRE->globalToLocalTransf(m_id)},
   m_globPos{m_legRE->center(m_id)}  {}

MdtCalibInput::MdtCalibInput(const xAOD::MdtDriftCircle& prd,
                             const ActsGeometryContext& gctx):
   m_id{prd.identify()},
   m_adc{prd.adc()},
   m_tdc{prd.tdc()},
   m_gctx{&gctx},
   m_RE{prd.readoutElement()},
   m_hash{prd.measurementHash()},
   m_globToLoc{m_RE->globalToLocalTrans(gctx, m_hash)},
   m_globPos{m_RE->center(gctx, m_hash)} {}
MdtCalibInput::MdtCalibInput(const MdtDigit& digit,
                             const MuonGMR4::MuonDetectorManager& detMgr,
                             const ActsGeometryContext& gctx):
   m_id{digit.identify()},
   m_isMasked{digit.is_masked()},
   m_adc{digit.adc()},
   m_tdc{digit.tdc()},  
   m_gctx{&gctx},
   m_RE{detMgr.getMdtReadoutElement(m_id)},
   m_hash{m_RE->measurementHash(m_id)},
   m_globToLoc{m_RE->globalToLocalTrans(gctx, m_hash)},
   m_globPos{m_RE->center(gctx, m_hash)} {}

MdtCalibInput::MdtCalibInput(const Identifier& id, const int adc, const int tdc, const Amg::Vector3D& globPos):
   m_id{id},
   m_adc{adc},
   m_tdc{tdc},
   m_globPos{globPos} {}
   
MdtCalibInput::MdtCalibInput(const Muon::MdtPrepData& prd):
   m_id{prd.identify()},
   m_adc{prd.adc()},
   m_tdc{prd.tdc()},
   m_legRE{prd.detectorElement()},
   m_globToLoc{m_legRE->globalToLocalTransf(m_id)},
   m_globPos{prd.globalPosition()} {
}

const Identifier& MdtCalibInput::identify() const { return m_id; }
int MdtCalibInput::tdc() const{ return m_tdc; }
int MdtCalibInput::adc() const{ return m_adc; }
const MuonGM::MdtReadoutElement* MdtCalibInput::legacyDescriptor() const { return m_legRE; }
const MuonGMR4::MdtReadoutElement* MdtCalibInput::decriptor() const { return m_RE; }
bool MdtCalibInput::isMasked() const { return m_isMasked; } 
const Amg::Vector3D& MdtCalibInput::globalPos() const { return m_globPos; }
const Amg::Vector3D& MdtCalibInput::closestApproach() const {return m_approach; }
void MdtCalibInput::setClosestApproach(const Amg::Vector3D& approach) {
   m_approach = approach;
   releaseSurface();
}
std::unique_ptr<Trk::StraightLineSurface> MdtCalibInput::releaseSurface() {
   return m_saggedSurf.release();
}
const Amg::Vector3D& MdtCalibInput::trackDirection() const { return m_trackDir; }
void MdtCalibInput::setTrackDirection(const Amg::Vector3D& trackDir) { m_trackDir = trackDir; }
double MdtCalibInput::timeOfFlight() const { return m_ToF; }
void MdtCalibInput::setTimeOfFlight(const double toF) { m_ToF = toF; }

double MdtCalibInput::triggerTime() const { return m_trigTime; }
void MdtCalibInput::setTriggerTime(const double trigTime) { m_trigTime = trigTime; }

const Amg::Vector3D& MdtCalibInput::surfaceCenter() const {
    return m_legRE->surface(identify()).center();
}
const Amg::Vector3D& MdtCalibInput::saggedSurfCenter() const { return saggedSurface().center();}

Amg::Vector2D MdtCalibInput::projectMagneticField(const Amg::Vector3D& fieldInGlob) const {
   /// Rotate the B-field into the rest frame of the tube (Z-axis along the wire)
   const Amg::Vector3D locBField = m_globToLoc.linear() * fieldInGlob;
   /// In the local coordinate system, the wire points along the z-axis
   const Amg::Vector3D locTrkDir = m_globToLoc.linear() * trackDirection();

   const double perpendComp = locTrkDir.block<2,1>(0,0).dot(locBField.block<2,1>(0,0)) 
                            / locTrkDir.perp();
   const double paralelComp = locBField.z();
   /// Convert kilo tesla into tesla... Waaait whaat? 
   return 1000. * Amg::Vector2D{paralelComp, perpendComp};
}
const Trk::SaggedLineSurface& MdtCalibInput::idealSurface() const {
    if (!m_legRE) {
      THROW_EXCEPTION(" idealSurface() can only be called together with the legacy readout geometry");
   }
   return m_legRE->surface(identify());
}
const Trk::StraightLineSurface& MdtCalibInput::saggedSurface() const {
   if (!m_legRE) {
      THROW_EXCEPTION(" saggedSurface() can only be called together with the legacy readout geometry");
   }
   if (!m_saggedSurf) {
      const Trk::SaggedLineSurface& surf{idealSurface()};
      const Trk::Surface& baseSurf{surf};
      std::optional<Amg::Vector2D> locApproach = baseSurf.globalToLocal(closestApproach(),1000.);
      if (!locApproach) {
         return surf;
      }
      std::unique_ptr<Trk::StraightLineSurface> sagged{surf.correctedSurface(*locApproach)};
      if (!sagged) {
         return surf;
      }
      return (*m_saggedSurf.set(std::move(sagged)));
   }
   return (*m_saggedSurf);
}
double MdtCalibInput::signalPropagationDistance() const {
   double propDist{0.};
   if (m_legRE) {
      const double distToRO = m_legRE->distanceFromRO(closestApproach(), identify());
      propDist = distToRO - m_legRE->RODistanceFromTubeCentre(identify());
   } else if (m_RE) {
      return m_RE->distanceToReadout(*m_gctx, identify(), closestApproach());
   }
   return propDist;
}
double MdtCalibInput::distanceToTrack() const { return m_distToTrack; }

double MdtCalibInput::tubeLength() const {
    if (m_legRE) return m_legRE->tubeLength(identify());
    else if (m_RE) return m_RE->tubeLength(m_hash);
    return 0.;
 }
double MdtCalibInput::readOutSide() const {
   /// By convention the new readout geometry points along the negative z-axis
   if (m_legRE) return m_legRE->tubeFrame_localROPos(identify()).z() > 0. ? 1. : -1.;
   else if (m_RE) return m_RE->getParameters().readoutSide;
   return 0.;
}
