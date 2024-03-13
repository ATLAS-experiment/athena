/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <AthenaBaseComps/AthCheckMacros.h>
#include <GaudiKernel/SystemOfUnits.h>

#include <optional>

using namespace ActsTrk;

namespace MuonGMR4 {
std::ostream& operator<<(std::ostream& ostr, const MuonGMR4::MdtReadoutElement::parameterBook& pars) {
    ostr << std::endl;
    ostr << " // Chamber half- length (min/max): "<<pars.shortHalfX<<"/"<<pars.longHalfX
         <<",  half-width "<<pars.halfY<<", height: "<<pars.halfHeight;
    ostr << " // Number of tube layers " << pars.tubeLayers.size()<< std::endl;
    ostr << " // Tube pitch: " << pars.tubePitch
         << " wall thickness: " << pars.tubeWall
         << " inner radius: " << pars.tubeInnerRad 
         << " endplug: "<<pars.endPlugLength
         << " deadlength: "<<pars.deadLength<< std::endl;
    for (const MdtTubeLayerPtr& layer : pars.tubeLayers) {
         ostr << " //   **** "<< Amg::toString(layer->tubeTransform(0).translation(), 2)<<std::endl;
    }
    return ostr;
}
MdtReadoutElement::MdtReadoutElement(defineArgs&& args)
    : MuonReadoutElement(std::move(args)),
      m_pars{std::move(args)} {
}
const MdtReadoutElement::parameterBook& MdtReadoutElement::getParameters() const {return m_pars;}
Identifier MdtReadoutElement::measurementId(const IdentifierHash& measHash) const {
    return m_idHelper.channelID(identify(), multilayer(),
                                layerNumber(measHash) + 1,
                                tubeNumber(measHash) + 1);
}
StatusCode MdtReadoutElement::initElement() {
  /// Check that the alignable node has been assigned
  if(!alignableTransform()) {
     ATH_MSG_FATAL("The readout element "<<idHelperSvc()->toStringDetEl(identify())<<" has no assigned alignable node");
     return StatusCode::FAILURE;
  }
  /// First check whether we're having tubes
  if (!numLayers() || !numTubesInLay()) {
     ATH_MSG_FATAL("The readout element "<< idHelperSvc()->toStringDetEl(identify())<<" has no tubes. Please check "<<std::endl<<m_pars);
     return StatusCode::FAILURE;
  }
  if (m_pars.tubePitch<=tubeRadius()) {
     ATH_MSG_FATAL("The tubes of "<<idHelperSvc()->toStringDetEl(identify())<<" will fall together on a single point. Please check "<<std::endl<<m_pars);
     return StatusCode::FAILURE;
  }
#ifndef SIMULATIONBASE
  /// Create bounds that are representing the surface planes of each tube layer & the readout element itself
  ATH_CHECK(planeSurfaceFactory(geoTransformHash(), m_pars.layerBounds->make_bounds(m_pars.shortHalfX, 
                                                                                    m_pars.longHalfX, 
                                                                                    m_pars.halfY)));
#endif
  /// Coordinate system of the trapezoid is in the center while the tubes are defined 
  /// w.r.t. to the chamber edge. Move first tube into the proper position

  std::optional<Amg::Vector3D> prevLayPos{std::nullopt};
  for (unsigned int lay =1 ; lay <= numLayers() ; ++lay) {
     /// Cache the transformations to the chamber layers
     const IdentifierHash layHash = measurementHash(lay,0);
     ATH_CHECK(insertTransform(layHash, 
                [this](RawGeomAlignStore* store, const IdentifierHash& hash){
                    const Amg::Translation3D toCenter{m_pars.halfY * Amg::Vector3D::UnitY()};
                    return toStation(store) * toChamberLayer(hash)*toCenter*Amg::getRotateY3D(90*Gaudi::Units::deg); 
                }));
#ifndef SIMULATIONBASE
     ATH_CHECK(planeSurfaceFactory(layHash, m_pars.layerBounds->make_bounds(m_pars.shortHalfX, 
                                                                            m_pars.longHalfX, 
                                                                            m_pars.halfY)));
#endif
    /// Cache the transformations to the tube layers
    std::optional<Amg::Vector3D> prevTubePos{std::nullopt};
    for (unsigned int tube = 1; tube <= numTubesInLay(); ++ tube) {
      const IdentifierHash idHash = measurementHash(lay,tube);
      if (m_pars.removedTubes.count(idHash)) {
         prevTubePos = std::nullopt;
         continue;
      }

      ATH_CHECK(insertTransform(idHash,
                [this](RawGeomAlignStore* store, const IdentifierHash& hash){
                    return toStation(store) * toTubeFrame(hash); 
                }));
#ifndef SIMULATIONBASE
      ATH_CHECK(strawSurfaceFactory(idHash, m_pars.tubeBounds->make_bounds(innerTubeRadius(), 0.5*tubeLength(idHash))));
#endif
      ///Ensure that all linear transformations are rotations
      const AmgSymMatrix(3) tubeRot = toTubeFrame(idHash).linear();
      if (std::abs(tubeRot.determinant()- 1.) > std::numeric_limits<float>::epsilon()){
         ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Transformation matrix is not a pure rotation for "<<
                       idHelperSvc()->toStringDetEl(identify())<<" in layer: "<<lay<<", tube: "<<tube
                       <<Amg::toString(toTubeFrame(idHash)));
         return StatusCode::FAILURE;
      }
      /// Ensure that all tubes have the same pitch
      const Amg::Vector3D tubePos = toTubeFrame(idHash).translation();
      
      constexpr double pitchTolerance = 20. * Gaudi::Units::micrometer;
      if (prevTubePos) {
         const double dR = std::abs((tubePos - (*prevTubePos)).z());
         if (std::abs(dR - tubePitch()) > pitchTolerance) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Detected irregular tube in "<<
                          idHelperSvc()->toStringDetEl(identify())<<" in layer: "<<lay<<", tube: "<<tube
                          <<". Expected tube pitch: "<<tubePitch()<<" measured tube pitch: "
                          <<dR<<" tube position: "<<Amg::toString(tubePos,2)
                          <<" previous: "<<Amg::toString((*prevTubePos), 2));
            return StatusCode::FAILURE;
         }
      }  
      if (prevLayPos && tube == 1) {
         const double dR = (tubePos - (*prevLayPos)).mag();       
         if (std::abs(dR - tubePitch()) > pitchTolerance) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Detected irregular layer pitch in "<<
                          idHelperSvc()->toStringDetEl(identify())<<" for layer "<<lay
                          <<". Expected tube pitch: "<<tubePitch()<<" measured tube pitch: "
                          <<dR<<" tube position: "<<Amg::toString(tubePos,2)
                          <<" previous:"<<Amg::toString((*prevLayPos), 2));
         }
         prevLayPos = std::make_optional<Amg::Vector3D>(tubePos);
      }
      prevTubePos = std::make_optional<Amg::Vector3D>(tubePos);
    }
  }
#ifndef SIMULATIONBASE
  m_pars.tubeBounds.reset();
  m_pars.layerBounds.reset();
#endif
  return StatusCode::SUCCESS;
}

Amg::Vector3D MdtReadoutElement::globalTubePos(const ActsGeometryContext& ctx,
                                               const IdentifierHash& hash) const {
   return localToGlobalTrans(ctx) * localTubePos(hash);
}

Amg::Vector3D MdtReadoutElement::localTubePos(const IdentifierHash& hash) const {
  return toTubeFrame(hash).translation();
}
Amg::Vector3D MdtReadoutElement::readOutPos(const ActsGeometryContext& ctx,
                                            const IdentifierHash& hash) const {
   const unsigned int layer = layerNumber(hash);
   const unsigned int tube  = tubeNumber(hash);
   const MdtTubeLayer& zeroT{*m_pars.tubeLayers[layer]};
   const double length = zeroT.tubeHalfLength(tube) * m_pars.readoutSide;
   return localToGlobalTrans(ctx) * zeroT.tubeTransform(tube)*(length * Amg::Vector3D::UnitZ());
}
Amg::Vector3D MdtReadoutElement::highVoltPos(const ActsGeometryContext& ctx,
                                             const IdentifierHash& hash) const {
   const unsigned int layer = layerNumber(hash);
   const unsigned int tube  = tubeNumber(hash);
   const MdtTubeLayer& zeroT{*m_pars.tubeLayers[layer]};
   const double length = - zeroT.tubeHalfLength(tube) * m_pars.readoutSide;
   return localToGlobalTrans(ctx) * zeroT.tubeTransform(tube)*(length * Amg::Vector3D::UnitZ());
}
Amg::Transform3D MdtReadoutElement::toChamberLayer(const IdentifierHash& hash) const {   
   const unsigned int layer = layerNumber(hash);
   const MdtTubeLayer& zeroT{*m_pars.tubeLayers[layer]};
   return zeroT.layerTransform();
}
Amg::Transform3D MdtReadoutElement::toTubeFrame(const IdentifierHash& hash) const {
   const unsigned int layer = layerNumber(hash);
   const unsigned int tube = tubeNumber(hash);
   const MdtTubeLayer& zeroT{*m_pars.tubeLayers[layer]};
   return zeroT.tubeTransform(tube);  
}
double MdtReadoutElement::activeTubeLength(const IdentifierHash& hash) const {
   const unsigned int layer = layerNumber(hash);
   const unsigned int tube = tubeNumber(hash);
   const MdtTubeLayer& zeroT{*m_pars.tubeLayers[layer]}; 
   return 2. * zeroT.tubeHalfLength(tube);
}
double MdtReadoutElement::tubeLength(const IdentifierHash& hash) const {
  return activeTubeLength(hash) + 2.*m_pars.deadLength;
}
double MdtReadoutElement::wireLength(const IdentifierHash& hash) const {
   return tubeLength(hash) - 2.*m_pars.endPlugLength;
}
double MdtReadoutElement::distanceToReadout(const ActsGeometryContext& ctx,
                                            const IdentifierHash& measHash,
                                            const Amg::Vector3D& globPoint) const {
    const Amg::Vector3D locPoint = globalToLocalTrans(ctx, measHash) * globPoint;
    /// The position of the readout chip is at the negative tube side
    const unsigned int layer = layerNumber(measHash);
    const unsigned int tube = tubeNumber(measHash);
    const MdtTubeLayer& zeroT{*m_pars.tubeLayers[layer]};
    const Amg::Vector3D readOutPos = m_pars.readoutSide * 
                                     zeroT.tubeHalfLength(tube) *
                                     Amg::Vector3D::UnitZ();
    return readOutPos.z() - locPoint.z();
}
}  // namespace MuonGMR4
