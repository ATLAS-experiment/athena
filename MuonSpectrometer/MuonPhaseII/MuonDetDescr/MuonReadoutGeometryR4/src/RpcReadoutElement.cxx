/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>


#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <ActsGeoUtils/SurfaceBoundSet.h>

#include <AthenaBaseComps/AthCheckMacros.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <optional>

#ifndef SIMULATIONBASE
#  include "Acts/Surfaces/RectangleBounds.hpp"
#endif
using namespace ActsTrk;

namespace MuonGMR4 {
using parameterBook = RpcReadoutElement::parameterBook;
std::ostream& operator<<(std::ostream& ostr, const parameterBook& pars) {
   ostr<<"chamber width/length/thickness [mm]: "<<(2.*pars.halfWidth)<<"/";
   ostr<<(2.*pars.halfLength)<<"/"<<(2.*pars.halfThickness)<<std::endl;
   if (pars.etaDesign) ostr<<"Eta strips: "<<(*pars.etaDesign)<<std::endl;
   if (pars.phiDesign) ostr<<"Phi strips: "<<(*pars.phiDesign)<<std::endl;   
   return ostr;
}
RpcReadoutElement::~RpcReadoutElement() = default;
RpcReadoutElement::RpcReadoutElement(defineArgs&& args)
    : MuonReadoutElement(args),
      m_pars{std::move(args)} {
}

const parameterBook& RpcReadoutElement::getParameters() const { return m_pars; }

StatusCode RpcReadoutElement::initElement() {   
    /// Check that the alignable node has been assigned
    ATH_CHECK(createGeoTransform());
 
    ATH_MSG_DEBUG("Parameter book "<<parameterBook());
    if (m_pars.layers.empty()) {
       ATH_MSG_FATAL("The readout element "<<idHelperSvc()->toStringDetEl(identify())<<" doesn't have any layers defined");
       return StatusCode::FAILURE;
    }
#ifndef SIMULATIONBASE
    ATH_CHECK(planeSurfaceFactory(geoTransformHash(), m_pars.layerBounds->make_bounds(m_pars.halfWidth, 
                                                                                      m_pars.halfLength)));
#endif
    for (unsigned int layer = 0; layer < m_pars.layers.size(); ++layer) {
      IdentifierHash layHash{layer};
      if (!m_pars.layers[layer]) {
         ATH_MSG_VERBOSE("Layer "<<layer <<" has not sensor layout associated.");
         continue;
      }
      ATH_CHECK(insertTransform<RpcReadoutElement>(layHash));
#ifndef SIMULATIONBASE
      const StripDesign& design{sensorLayout(layHash).design()};
      ATH_CHECK(planeSurfaceFactory(layHash, m_pars.layerBounds->make_bounds(design.halfWidth(),
                                                                             design.shortHalfHeight())));
#endif
    }
    m_gasThickness = (chamberStripPos(createHash(1, 2, doubletPhi(), false)) - 
                      chamberStripPos(createHash(1, 1, doubletPhi(), false))).mag();
#ifndef SIMULATIONBASE
    m_pars.layerBounds.reset();
#endif
    return StatusCode::SUCCESS;
}

Amg::Transform3D RpcReadoutElement::fromGapToChamOrigin(const IdentifierHash& hash) const{
   return sensorLayout(hash).toOrigin();
}

Amg::Vector3D RpcReadoutElement::stripPosition(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const {
   return localToGlobalTrans(ctx, layerHash(measHash)) * 
           sensorLayout(measHash).localStripPos(stripNumber(measHash));
}
Amg::Vector3D RpcReadoutElement::rightStripEdge(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const{
      return localToGlobalTrans(ctx, layerHash(measHash)) * 
              sensorLayout(measHash).localStripLeftEdge(stripNumber(measHash));
}
Amg::Vector3D RpcReadoutElement::leftStripEdge(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const {
    return localToGlobalTrans(ctx, layerHash(measHash)) * 
           sensorLayout(measHash).localStripRightEdge(stripNumber(measHash));
}

Amg::Vector3D RpcReadoutElement::chamberStripPos(const IdentifierHash& measHash) const {
   return sensorLayout(measHash).stripPosition(stripNumber(measHash));
}

double RpcReadoutElement::distanceToEdge(const IdentifierHash& layerHash, 
                                         const Amg::Vector2D& posInStripPlane,
                                         const EdgeSide side) const {
    const StripDesign& design{measuresPhi(layerHash) ? *m_pars.phiDesign : *m_pars.etaDesign};
    /// For the moment define the readOut to be at negative y while the highVolt is at positive
    const double refPoint{design.longHalfHeight() * (side == EdgeSide::readOut ? -1. : 1.)};
    return std::abs(refPoint - posInStripPlane.y());                                    
}


}
