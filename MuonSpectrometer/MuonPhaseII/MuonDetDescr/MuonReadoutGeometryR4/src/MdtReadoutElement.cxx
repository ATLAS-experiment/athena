/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>

#include <ActsGeoUtils/SurfaceBoundSet.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>
#include <GeoModelHelpers/TransformToStringConverter.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <AthenaBaseComps/AthCheckMacros.h>
#include <GaudiKernel/SystemOfUnits.h>

#include <optional>


#ifndef SIMULATIONBASE
#     include <MuonAlignmentDataR4/MdtAlignmentStore.h>
#   include "Acts/Surfaces/TrapezoidBounds.hpp"
#   include "Acts/Surfaces/LineBounds.hpp"
#   include "Acts/Surfaces/Surface.hpp"
#endif

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
MdtReadoutElement::~MdtReadoutElement() = default;
MdtReadoutElement::MdtReadoutElement(defineArgs&& args)
    : MuonReadoutElement(args),
      m_pars{std::move(args)} {
}
const MdtReadoutElement::parameterBook& MdtReadoutElement::getParameters() const {return m_pars;}
Identifier MdtReadoutElement::measurementId(const IdentifierHash& measHash) const {
    return m_idHelper.channelID(identify(), multilayer(),
                                layerNumber(measHash) + 1,
                                tubeNumber(measHash) + 1);
}
StatusCode MdtReadoutElement::initElement() {
  ATH_CHECK(createGeoTransform());
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
     ATH_CHECK(insertTransform<MdtReadoutElement>(layHash));
#ifdef SIMULATIONBASE_REMOVEPLANESURFACE
     ATH_CHECK(planeSurfaceFactory(layHash, m_pars.layerBounds->make_bounds(m_pars.shortHalfX, 
                                                                            m_pars.longHalfX, 
                                                                            m_pars.halfY)));
#endif
    /// Cache the transformations to the tube layers
    std::optional<Amg::Vector3D> prevTubePos{std::nullopt};
    MdtTubeLayer& layer = *m_pars.tubeLayers[lay-1];

    for (unsigned int tube = 1; tube <= numTubesInLay(); ++ tube) {
      const IdentifierHash idHash = measurementHash(lay,tube);
      if (m_pars.removedTubes.count(idHash)) {
         prevTubePos = std::nullopt;
         continue;
      }
      ATH_CHECK(insertTransform<MdtReadoutElement>(idHash));
#ifndef SIMULATIONBASE
      ATH_CHECK(strawSurfaceFactory(idHash, m_pars.tubeBounds->make_bounds(innerTubeRadius(), 0.5*tubeLength(idHash))));
#endif
      ///Ensure that all linear transformations are rotations
      GeoTrf::Transform3D tubeFrame = layer.tubeTransform(tubeNumber(idHash));
      const AmgSymMatrix(3) tubeRot = tubeFrame.linear();
      if (std::abs(tubeRot.determinant()- 1.) > std::numeric_limits<float>::epsilon()){
         ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Transformation matrix is not a pure rotation for "<<
                       idHelperSvc()->toStringDetEl(identify())<<" in layer: "<<lay<<", tube: "<<tube
                       <<Amg::toString(tubeFrame));
         return StatusCode::FAILURE;
      }
      /// Ensure that all tubes have the same pitch
      const Amg::Vector3D tubePos = tubeFrame.translation();
      
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
      }
      if (tube == 1) {
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
   return localToGlobalTrans(ctx, hash).translation();
}

Amg::Vector3D MdtReadoutElement::localTubePos(const IdentifierHash& hash) const {
  return toTubeFrame(hash).translation();
}
Amg::Vector3D MdtReadoutElement::readOutPos(const ActsGeometryContext& ctx,
                                            const IdentifierHash& hash) const {
   return localToGlobalTrans(ctx, hash) * 
          (0.5*activeTubeLength(hash) * m_pars.readoutSide * Amg::Vector3D::UnitZ());
}
Amg::Vector3D MdtReadoutElement::highVoltPos(const ActsGeometryContext& ctx,
                                             const IdentifierHash& hash) const {
   return localToGlobalTrans(ctx, hash) * 
          (-0.5*activeTubeLength(hash) * m_pars.readoutSide * Amg::Vector3D::UnitZ());
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
double MdtReadoutElement::uncutTubeLength(const IdentifierHash& tubeHash) const {
   const unsigned int layer = layerNumber(tubeHash);
   const unsigned int tube = tubeNumber(tubeHash);
   const MdtTubeLayer& zeroT{*m_pars.tubeLayers[layer]}; 
   return 2.*zeroT.uncutHalfLength(tube);
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
    return distanceToReadout(measHash, globalToLocalTrans(ctx, measHash) * globPoint);

}
double MdtReadoutElement::distanceToReadout(const IdentifierHash& measHash,
                                            const Amg::Vector3D& localPoint) const {
    /// The position of the readout chip is at the negative tube side
    const unsigned int layer = layerNumber(measHash);
    const unsigned int tube = tubeNumber(measHash);
    const MdtTubeLayer& zeroT{*m_pars.tubeLayers[layer]};
    const double readOutPos = m_pars.readoutSide * zeroT.tubeHalfLength(tube);
    return std::abs(readOutPos - localPoint.z());
}

void MdtReadoutElement::setComplementaryReadoutEle(const MdtReadoutElement* other) {
      m_reOtherMl = other;
}

Amg::Transform3D MdtReadoutElement::asBuiltRefFrame() const {
      return alignableTransform()->getDefTransform().inverse()*
             getMaterialGeom()->getParent()->getX() *
             getMaterialGeom()->getX();
}

Amg::Vector3D MdtReadoutElement::bLineReferencePoint() const {
   /* The fix point is the point that's closest to the interaction point. 
   /  Recall that the readout element origin is expressed in the geometric centre & 
      the x-axis points upwards. The point is then transformed into the Muon station frame */ 
   const MdtReadoutElement* refEle{multilayer() == 1 ? this : m_reOtherMl};
   return refEle->asBuiltRefFrame()*(-0.5*( refEle->moduleThickness() *Amg::Vector3D::UnitX() + 
                                            (stationEta() >0 || !isBarrel() ? 1. : -1.)*refEle->moduleHeight() * Amg::Vector3D::UnitZ()));
}
#ifndef SIMULATIONBASE    
Amg::Vector3D MdtReadoutElement::wireEndpointAsBuilt(const MdtAsBuiltPar&  params,
                                                     const IdentifierHash& tubeHash,
                                                     const Amg::Vector3D& wireEnd, 
                                                     const tubeSide_t side) const {
      

   ATH_MSG_VERBOSE( "Applying as-built parameters for chamber " << idHelperSvc()->toString(measurementId(tubeHash)));

  
   using tubeSide_t = MdtAsBuiltPar::tubeSide_t;
   using multilayer_t = MdtAsBuiltPar::multilayer_t;
   const multilayer_t ml = (multilayer() == 1) ? multilayer_t::ML1 : multilayer_t::ML2;

   const IdentifierHash refLayer{measurementHash((ml == multilayer_t::ML1) ? numLayers() : 1, 1)};
   const Amg::Vector3D refTube{localTubePos(refLayer)};

  
   const double sideSign = 0.5*(1.- 2.*(side ==tubeSide_t::NEG));
   // Compute the reference for the as-built parameters
   const Amg::Vector3D reference_point = asBuiltRefFrame() *
                                             Amg::Vector3D {refTube.x() + (1. - 2.*(ml == multilayer_t::ML2)) * tubeRadius(),
                                                            sideSign*uncutTubeLength(refLayer),
                                                            -0.5*moduleHeight()};
   ATH_MSG_VERBOSE("AMDB transform "<<" "<<idHelperSvc()->toStringDetEl(identify())<<
                  " "<<GeoTrf::toString(asBuiltRefFrame(), true)<<", reference point: "<<Amg::toString(reference_point));

   
   const int tubeLayer = layerNumber(tubeHash);
   const int tube = tubeNumber(tubeHash);
   const int layer_delta = ml == multilayer_t::ML1 ?  numLayers() - tubeLayer : tubeLayer +1;


   // Get the As-Built parameters for this ML and side of the chamber
   const double zpitch = params.zpitch(ml, side);
   const double ypitch = params.ypitch(ml, side);
   const double stagg =  params.stagg(ml, side);
   const double alpha = params.alpha(ml, side);
   const double y0 = params.y0(ml, side);
   const double z0 = params.z0(ml, side);

   const Amg::Transform3D planeRot{Amg::getRotateX3D(-alpha)};
   ATH_MSG_VERBOSE("Rotation angle "<<(alpha/Gaudi::Units::deg));

   // Find the vector from the reference_point to the endplug
   // 0 for layer 1 and 3, 1 for layer 2 and 4
   const double offset_stagg = 0.5 * zpitch * stagg * ( (layer_delta-1)  % 2);
   const Amg::Vector3D endPlug = planeRot * Amg::Vector3D{0., 
                                                            tube* zpitch + offset_stagg, 
                                                            (layer_delta-1) * ypitch};

   // Calculate x position, which varies for endcap chambers
   const double xshift = sideSign*(uncutTubeLength(tubeHash) - uncutTubeLength(refLayer));
   ATH_MSG_VERBOSE("Off set staggering "<<offset_stagg<<", layer_delta: "<<layer_delta<<", zpitch: "<<zpitch<<", ypitch: "<<ypitch
                 <<", xshift: "<<xshift);

   Amg::Vector3D ret(reference_point.x() + xshift, 
                     reference_point.y() + z0 + endPlug.y(),
                     reference_point.z() + y0 + endPlug.z());

   
   if ((ret - wireEnd).mag() > 3. * Gaudi::Units::mm) {
         ATH_MSG_WARNING( "Large as-built correction for chamber " << idHelperSvc()->toString(measurementId(tubeHash))
                        << ", side "<< (side == tubeSide_t::POS ? "positive" : "negative") 
                        << ", endpoint "<<Amg::toString(wireEnd)<<", return: "<<Amg::toString(ret));
   }
   ATH_MSG_VERBOSE((side == tubeSide_t::POS ? "positive" : "negative")<<" wire end has moved from "
                   <<Amg::toString(wireEnd)<<" to "<<Amg::toString(ret)<<" End plug: "<<Amg::toString(endPlug));
   return ret;
}

Amg::Vector3D MdtReadoutElement::applyBlineCorrections(const BLinePar& bline,
                                                       const Amg::Vector3D& localTubeEndPoint,
                                                       const Amg::Vector3D& fixedPoint,
                                                       const double chamberThickness) const {
   
   using Parameter = BLinePar::Parameter;
   Amg::Vector3D deformedPos{localTubeEndPoint};
   // MDT deformations like tube bow: bz,bp,bn bend the tube while the wire endpoints are not affected
   //                                 => the wire keeps it's nominal straight line trajectory but it is not concentric to the tube
   //                                 ==> in this function bz, bp and bn are ignored or set to 0
   // MDT deformations that extend/contract the wire longitudinally (while keeping it straight):
   //                                 delta_s from eg and tr are irrelevant for the tube geometry
   //                                 and for the wire trajectory => set to 0 here
   //                                 (should be applied as a correction to the
   //                                 tube lenght => tube surface bounds
   //                                 =++>>>> effect in tracking just through the gravitational sagging TOTALLY NEGLIGIBLE=> ignore)
   // pg is irrelevant for tracking purposes and (at least for the endcaps) is applies to the internal bars only, not to the tubes !!!
   //                                 =++>>>> IGNORE IT
   // ep,en: bend the tube by moving (differently) the endplugs ===> the wire straight trajectory is moved w.r.t. the nominal one
   //                                 in addition the tubes keep their nominal position at the center => the wire is not concentric to
   //                                 the tube delta_s from ep,en must also be considered for the implementation of the realistic tube
   //                                 trajectory induced by ep,en
   // tw,sp,sn,pg (for deltaT and deltaZ) are geometrical effects, that impact on tracking and keep the wire straight.
   const double chamberHeight = std::max(moduleHeight(), m_reOtherMl->moduleHeight());
      

   // NOTE s0,z0,t0 are the coord. in the amdb frame of this point: the origin of the frame can be different than the fixed point for
   // deformations s0mdt,z0mdt,t0mdt
   //    (always equal to the point at lowest t,z and s=0 of the MDT stack)
   ATH_MSG_VERBOSE( "** In "<<__func__<<" - moduleWidthS " << moduleWidthS()<<", moduleWidthL: "
                       <<moduleWidthL() <<", height: "<<chamberHeight<<", thickness: " <<chamberThickness << "." );
   ATH_MSG_VERBOSE( "** In "<<__func__<<" - going to correct for B-line the position of Point at " << Amg::toString(localTubeEndPoint));

   double s0mdt = localTubeEndPoint.x();  // always I think !
   if (std::abs(fixedPoint.x()) > 0.01) s0mdt = localTubeEndPoint.x() - fixedPoint.x();
   double z0mdt = localTubeEndPoint.y();  
   // unless in the D section of this station there's a dy diff. from 0 for the innermost MDT multilayer (sometimes
   // in the barrel)
   // unless in the D section of this station there's a dz diff. from 0 for the innermost MDT multilayer (often in barrel)
   if (std::abs(fixedPoint.y()) > 0.01) z0mdt = localTubeEndPoint.y() - fixedPoint.y();
   double t0mdt = localTubeEndPoint.z();  
   if (std::abs(fixedPoint.z()) > 0.01) t0mdt = localTubeEndPoint.z() - fixedPoint.z();
   if (z0mdt < 0 || t0mdt < 0) {
      ATH_MSG_WARNING(""<<__func__<<": correcting the local position of a point outside the mdt station (2 multilayers) volume -- RE "
                  << idHelperSvc()->toStringDetEl(identify()) << " local point: szt=" <<Amg::toString(localTubeEndPoint)
                  << " fixedPoint " <<Amg::toString(fixedPoint) );
   }
   ATH_MSG_VERBOSE( "** In "<<__func__<<" - correct for offset of B-line fixed point " << s0mdt << " " << z0mdt << " " << t0mdt);

   const double width_actual = moduleWidthS() + (moduleWidthL() - moduleWidthS()) * (z0mdt / chamberHeight);
   const double s_rel = s0mdt / (width_actual / 2.);
   const double z_rel = (z0mdt - chamberHeight / 2.) / (chamberHeight / 2.);
   const double t_rel = (t0mdt - chamberThickness / 2.) / (chamberThickness / 2.);

   ATH_MSG_VERBOSE( "** In "<<__func__<<" - width_actual, s_rel, z_rel, t_rel  " << width_actual << " " << s_rel << " "
                                 << z_rel << " " << t_rel );
   double ds{0.},dz{0.},dt{0.};
   
   // sp, sn - cross plate sag out of plane
   if ( bline.getParameter(Parameter::sp) || bline.getParameter(Parameter::sn)) {
      double ztmp = z_rel * z_rel - 1;
      dt += 0.5 * (bline.getParameter(Parameter::sp) + bline.getParameter(Parameter::sn)) * ztmp 
         +  0.5 * (bline.getParameter(Parameter::sp) - bline.getParameter(Parameter::sn)) * ztmp * s_rel;
   }

   // tw     - twist
   if (bline.getParameter(Parameter::tw)) {
      dt -= bline.getParameter(Parameter::tw) * s_rel * z_rel;
      dz += bline.getParameter(Parameter::tw) * s_rel * t_rel * chamberThickness / chamberHeight;
      ATH_MSG_VERBOSE( "** In "<<__func__<<": tw=" << bline.getParameter(Parameter::tw) << " dt, dz " << dt << " " << dz );
   }

   constexpr double expansionScale = BLinePar::expansionScale;
     // eg     - global expansion
   if (bline.getParameter(Parameter::eg)) {
      double egppm = bline.getParameter(Parameter::eg) * expansionScale;
      ds += 0.;
      dz += z0mdt * egppm;
      dt += t0mdt * egppm;
   }

   // ep, en - local expansion
   //
   // Imporant note: the chamber height and length, as they denoted in Christoph's talk,
   // correspond to thickness and height parameters of this function;
   //

   if (bline.getParameter(Parameter::ep) || bline.getParameter(Parameter::en)) {
      const double ep = bline.getParameter(Parameter::ep) * expansionScale;
      const double en = bline.getParameter(Parameter::en) * expansionScale;
      const double phi = 0.5 * (ep + en) * s_rel * s_rel + 0.5 * (ep - en) * s_rel;
      const double localDt = phi * (t0mdt - chamberThickness / 2.);
      const double localDz = phi * (z0mdt - chamberHeight / 2.);
      dt += localDt;
      dz += localDz;
   }

   ATH_MSG_VERBOSE( "posOnDefChamStraighWire: ds,z,t = " << ds << " " << dz << " " << dt );
   deformedPos[0] = localTubeEndPoint[0] + ds;
   deformedPos[1] = localTubeEndPoint[1] + dz;
   deformedPos[2] = localTubeEndPoint[2] + dt;

        
   return deformedPos;
}
#endif
Amg::Transform3D MdtReadoutElement::fromIdealToDeformed(const IdentifierHash& tubeHash,
                                                        const ActsTrk::DetectorAlignStore* store) const {
#ifdef SIMULATIONBASE
   ATH_MSG_VERBOSE("No deformed transformation available "<<idHelperSvc()->toString(measurementId(tubeHash))
                  <<"store address to make the compiler happy: "<<store);
   return Amg::Transform3D::Identity();
#else
   /// No deformation parameters were parsed at all
   if (!store || !store->internalAlignment) {
      ATH_MSG_VERBOSE("No deformed transformation available "<<idHelperSvc()->toString(measurementId(tubeHash)));
      return Amg::Transform3D::Identity();
   }
   using ChamberDistortions = MdtAlignmentStore::chamberDistortions;

   /// No deformation parameters were stored for this particular readout element
   const ChamberDistortions distortPars = static_cast<const MdtAlignmentStore&>(*store->internalAlignment).getDistortion(identify());
   if (!distortPars) {
      ATH_MSG_VERBOSE("No set of deformation parameters is in the DB for "<<idHelperSvc()->toString(measurementId(tubeHash)));
      return Amg::Transform3D::Identity();
   }

   const Amg::Vector3D fixedPoint{bLineReferencePoint()};
   
   
   const double height = std::max(moduleHeight(), m_reOtherMl->moduleHeight()) - 
                         (tubePitch() - 2. * tubeRadius());

   const Amg::Transform3D toAMDB{asBuiltRefFrame()};
   /// Relative sign to calculate the thickness. If multilayer == 1,
   /// we want the lower point and the upper point otherwise.
   const double relSign = (multilayer() == 1 ? -1. : 1.);
   
   const double modHalfThick{0.5*relSign*moduleThickness()}, 
                modHalTHickO{-0.5*relSign*m_reOtherMl->moduleThickness()};

   const double thickness = relSign*( (toAMDB* (modHalfThick*Amg::Vector3D::UnitX())) -
                                      (m_reOtherMl->asBuiltRefFrame()*(modHalTHickO* Amg::Vector3D::UnitX()))).z();

   
   Amg::Vector3D idealTube = localTubePos(tubeHash);
   /// Move the tube into the center
   idealTube[Amg::y] = 0.;

   Amg::Vector3D positiveEnd = toAMDB * (idealTube + 0.5 * uncutTubeLength(tubeHash) * Amg::Vector3D::UnitY());
   Amg::Vector3D negativeEnd = toAMDB * (idealTube - 0.5 * uncutTubeLength(tubeHash) * Amg::Vector3D::UnitY());
   /// Apply as-built correction
   if (distortPars.asBuilt) {
      positiveEnd = wireEndpointAsBuilt(*distortPars.asBuilt, tubeHash, positiveEnd, tubeSide_t::POS);
      negativeEnd = wireEndpointAsBuilt(*distortPars.asBuilt, tubeHash, negativeEnd, tubeSide_t::NEG);
   }
   Amg::Vector3D positiveEndBline{positiveEnd};
   Amg::Vector3D negativeEndBline{negativeEnd};
   if (distortPars.bLine) {
      positiveEndBline = applyBlineCorrections(*distortPars.bLine, positiveEndBline, fixedPoint, thickness);
      negativeEndBline = applyBlineCorrections(*distortPars.bLine, negativeEndBline, fixedPoint, thickness); 
   }

   ATH_MSG_VERBOSE("Calculate the deformation parameters of "<<idHelperSvc()->toString(measurementId(tubeHash))
                <<", ideal tube "<<Amg::toString(idealTube)
                <<", fixed point "<<Amg::toString(fixedPoint)
                <<"/ "<<Amg::toString(alignableTransform()->getDefTransform() * fixedPoint)
                <<", thickness: "<<thickness<<", height: "<<height);

   const Amg::Transform3D fromAMDB{toAMDB.inverse()};
   /// Switch tube ends back to MGM coordinates
   positiveEnd = fromAMDB * positiveEnd;
   negativeEnd = fromAMDB * negativeEnd;
   positiveEndBline = fromAMDB * positiveEndBline;
   negativeEndBline = fromAMDB * negativeEndBline;

   // Calculate deformation. Make sure that the wire length stays the same.
   // Code in positionOnDeformedChamber does not provide this by default.
   // Break transformation into translation of the wire center and the rotation of the wire
   // Move to the coordinate system originated at the wire center, then rotate the wire, then
   // move wire center to the new position
   const Amg::Transform3D to_center{Amg::getTranslate3D(-idealTube)};
   const Amg::Transform3D from_center{Amg::getTranslate3D( 0.5 * (positiveEndBline + negativeEndBline))};
   const Amg::Vector3D old_direction = (positiveEnd - negativeEnd).unit();
   const Amg::Vector3D new_direction = (positiveEndBline - negativeEndBline).unit();
   const Amg::Vector3D rotation_vector = old_direction.cross(new_direction);

  Amg::Transform3D deformedTransform{Amg::Transform3D::Identity()};
   if (rotation_vector.mag() > 10. * std::numeric_limits<double>::epsilon()) {
      const Amg::AngleAxis3D wire_rotation(std::asin(rotation_vector.mag()), rotation_vector.unit());
      deformedTransform = from_center * wire_rotation * to_center;
   } else {
      deformedTransform = from_center * to_center;
   }
   ATH_MSG_VERBOSE("To center "<<GeoTrf::toString(to_center)<<" from: "<<GeoTrf::toString(from_center)<<
            " -- direction: "<<GeoTrf::toString(old_direction)<<" vs. "<<GeoTrf::toString(new_direction)
            <<" --> rot: "<<GeoTrf::toString(rotation_vector)<<" ==> "<<GeoTrf::toString(deformedTransform,true));
   return deformedTransform;
#endif
}


}  // namespace MuonGMR4
