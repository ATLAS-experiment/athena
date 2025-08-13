/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
#include <xAODMuonPrepData/MdtDriftCircle.h>
#include <MuonDigitContainer/MdtDigit.h>
#include <MuonPrepRawData/MdtPrepData.h>
#include <TrkSurfaces/StraightLineSurface.h>
#include <GeoModelKernel/throwExcept.h>

std::ostream& operator<<(std::ostream& ostr, const MdtCalibInput& input){
   ostr<<"adc: "<<input.adc()<<", ";
   ostr<<"tdc: "<<input.tdc()<<", ";
   ostr<<"closest approach: "<<Amg::toString(input.closestApproach(), 2)<<", ";
   ostr<<"global direction: "<<Amg::toString(input.trackDirection(), 2)<<", ";
   ostr<<"prop distance: "<<input.signalPropagationDistance()<<", ";
   ostr<<"ToF: "<<input.timeOfFlight()<<", ";
   ostr<<"trigger time: "<<input.triggerTime();
   return ostr;
}
MdtCalibInput::~MdtCalibInput() = default;
MdtCalibInput::MdtCalibInput(const Identifier& id,
                             const int16_t adc,
                             const int16_t tdc,
                             const MuonGMR4::MdtReadoutElement* reEle,
                             const ActsGeometryContext& gctx):
   m_id{id},
   m_adc{adc},
   m_tdc{tdc},
   m_gctx{&gctx},
   m_RE{reEle},
   m_hash{std::get<const MuonGMR4::MdtReadoutElement*>(m_RE)->measurementHash(m_id)}  {}

MdtCalibInput::MdtCalibInput(const MdtDigit& digit,
                             const MuonGMR4::MuonDetectorManager& detMgr,
                             const ActsGeometryContext& gctx):
   MdtCalibInput(digit.identify(), digit.adc(), digit.tdc(), 
                 detMgr.getMdtReadoutElement(digit.identify()), gctx){}
 
MdtCalibInput::MdtCalibInput(const Identifier& id,
                             const int16_t adc,
                             const int16_t tdc,
                             const MuonGM::MdtReadoutElement* reEle):
   m_id{id},
   m_adc{adc},
   m_tdc{tdc},
   m_RE{reEle} {}

MdtCalibInput::MdtCalibInput(const MdtDigit& digit,
                             const MuonGM::MuonDetectorManager& detMgr):
   MdtCalibInput(digit.identify(), digit.adc(), digit.tdc(), 
                 detMgr.getMdtReadoutElement(digit.identify())) {}


MdtCalibInput::MdtCalibInput(const xAOD::MdtDriftCircle& prd,
                             const ActsGeometryContext& gctx):
   m_id{prd.identify()},
   m_adc{prd.adc()},
   m_tdc{prd.tdc()},
   m_gctx{&gctx},
   m_RE{prd.readoutElement()},
   m_hash{prd.measurementHash()},  
   m_approach{localToGlobal()* prd.localCirclePosition()} {}
   
   
MdtCalibInput::MdtCalibInput(const Muon::MdtPrepData& prd):
   m_id{prd.identify()},
   m_adc{static_cast<int16_t>(prd.adc())},
   m_tdc{static_cast<int16_t>(prd.tdc())},
   m_RE{prd.detectorElement()},  
   m_approach{prd.globalPosition()} {
}

const Identifier& MdtCalibInput::identify() const { return m_id; }
int16_t MdtCalibInput::tdc() const{ return m_tdc; }
int16_t MdtCalibInput::adc() const{ return m_adc; }
const MuonGM::MdtReadoutElement* MdtCalibInput::legacyDescriptor() const { 
   return std::visit([](const auto& re) -> const MuonGM::MdtReadoutElement*{
         using REType = std::decay_t<decltype(re)>;
         if constexpr( std::is_same_v<REType, const MuonGM::MdtReadoutElement*>){
            return re;
         }
         return nullptr;
   }, m_RE);
}
const MuonGMR4::MdtReadoutElement* MdtCalibInput::decriptor() const { 
      return std::visit([](const auto& re) -> const MuonGMR4::MdtReadoutElement*{
         using REType = std::decay_t<decltype(re)>;
         if constexpr( std::is_same_v<REType, const MuonGMR4::MdtReadoutElement*>){
            return re;
         }
         return nullptr;
   }, m_RE); 
}
const Amg::Vector3D& MdtCalibInput::closestApproach() const {return m_approach; }
void MdtCalibInput::setClosestApproach(const Amg::Vector3D& approach) {
   m_approach = approach;
}
const Amg::Vector3D& MdtCalibInput::trackDirection() const { return m_trackDir; }
void MdtCalibInput::setTrackDirection(const Amg::Vector3D& trackDir, bool hasPhi) { 
   m_trackDir = trackDir;
   m_trackHasPhi = hasPhi;
}
bool MdtCalibInput::trackDirHasPhi() const { return m_trackHasPhi; }
double MdtCalibInput::timeOfFlight() const { return m_ToF; }
void MdtCalibInput::setTimeOfFlight(const double toF) { m_ToF = toF; }

double MdtCalibInput::triggerTime() const { return m_trigTime; }
void MdtCalibInput::setTriggerTime(const double trigTime) { m_trigTime = trigTime; }

const Amg::Vector3D& MdtCalibInput::surfaceCenter() const {
    return legacyDescriptor()->surface(identify()).center();
}

Amg::Vector2D MdtCalibInput::projectMagneticField(const Amg::Vector3D& fieldInGlob) const {
   const Amg::Transform3D trf{localToGlobal().inverse()};
   /// Rotate the B-field into the rest frame of the tube (Z-axis along the wire)
   const Amg::Vector3D locBField = trf.linear() * fieldInGlob;
   /// In the local coordinate system, the wire points along the z-axis
   const Amg::Vector3D locTrkDir = trf.linear() * trackDirection();

   const double perpendComp = locTrkDir.block<2,1>(0,0).dot(locBField.block<2,1>(0,0)) 
                            / locTrkDir.perp();
   const double paralelComp = locBField.z();
   /// Convert kilo tesla into tesla... Waaait whaat? 
   return 1000. * Amg::Vector2D{paralelComp, perpendComp};
}
const Trk::StraightLineSurface& MdtCalibInput::legacySurface() const {
   const MuonGM::MdtReadoutElement* re = legacyDescriptor();
   assert(re != nullptr);
   return re->surface(identify());
}
double MdtCalibInput::signalPropagationDistance() const {
   const double propDist = std::visit([this](const auto& re) ->double {
               using REType = std::decay_t<decltype(re)>;
               if constexpr(std::is_same_v<REType, const MuonGMR4::MdtReadoutElement*>){
                  assert(m_gctx != nullptr);
                  return re->distanceToReadout(*m_gctx, m_hash, closestApproach());
               } else if (std::is_same_v<REType, const MuonGM::MdtReadoutElement*>) {
                  return re->distanceFromRO(closestApproach(), identify()) -
                         re->RODistanceFromTubeCentre(identify());
               }
            }, m_RE); 
   return propDist;
}
double MdtCalibInput::distanceToTrack() const { return m_distToTrack; }

double MdtCalibInput::tubeLength() const {
   const double tubeLength  = std::visit([this](const auto& re) ->double{
               using REType = std::decay_t<decltype(re)>;
               if constexpr(std::is_same_v<REType, const MuonGMR4::MdtReadoutElement*>){
                  return re->tubeLength(m_hash);
               } else if (std::is_same_v<REType, const MuonGM::MdtReadoutElement*>) {
                  return re->tubeLength(identify());
               }
            }, m_RE);
    return tubeLength;
 }
 double MdtCalibInput::innerTubeR() const {
   const double tubeR = std::visit([this](const auto& re) -> double {
       using REType = std::decay_t<decltype(re)>;
      if constexpr(std::is_same_v<REType, const MuonGMR4::MdtReadoutElement*>){
         return re->innerTubeRadius();
      } else if (std::is_same_v<REType, const MuonGM::MdtReadoutElement*>) {
         return re->innerTubeRadius();
      }
   }, m_RE);
   return tubeR;
 }
double MdtCalibInput::readOutSide() const {
   /// By convention the new readout geometry points along the negative z-axis
   const double roSide = std::visit([this](const auto& re) ->double{
               using REType = std::decay_t<decltype(re)>;
               if constexpr(std::is_same_v<REType, const MuonGMR4::MdtReadoutElement*>){
                  return re->getParameters().readoutSide;
               } else if (std::is_same_v<REType, const MuonGM::MdtReadoutElement*>) {
                  return re->tubeFrame_localROPos(identify()).z() > 0. ? 1. : -1.;
               }
            }, m_RE);
   return roSide;
}
const Amg::Transform3D& MdtCalibInput::localToGlobal() const {
   return std::visit([this](const auto& re) ->const Amg::Transform3D&{
         using REType = std::decay_t<decltype(re)>;
         if constexpr(std::is_same_v<REType, const MuonGMR4::MdtReadoutElement*>){
            assert(m_gctx != nullptr);
            return re->localToGlobalTrans(*m_gctx, m_hash);
         } else if (std::is_same_v<REType, const MuonGM::MdtReadoutElement*>) {
            return re->localToGlobalTransf(identify());
         }
   }, m_RE);
}
Amg::Vector3D MdtCalibInput::center() const {
   return localToGlobal().translation();
}
