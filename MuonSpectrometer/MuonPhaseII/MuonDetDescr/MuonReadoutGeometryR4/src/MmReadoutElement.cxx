/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <AthenaBaseComps/AthCheckMacros.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <optional>

#ifndef SIMULATIONBASE
#   include "Acts/Surfaces/TrapezoidBounds.hpp"
#endif

using namespace ActsTrk;

namespace MuonGMR4 {
using parameterBook = MmReadoutElement::parameterBook;
std::ostream& operator<<(std::ostream& ostr, const parameterBook& pars) {
   ostr<<"chamber shortWidth/longWidth/length [mm]: "<<(2.*pars.halfShortWidth)<<"/";
   ostr<<(2.*pars.halfLongWidth)<<"/"<<(2.*pars.halfHeight)<<std::endl;
   return ostr;
}

MmReadoutElement::MmReadoutElement(defineArgs&& args): 
    MuonReadoutElement(std::move(args)),
      m_pars{std::move(args)} {
}

const parameterBook& MmReadoutElement::getParameters() const { return m_pars; }

StatusCode MmReadoutElement::initElement() {    
    ATH_MSG_DEBUG("Parameter book "<<parameterBook());
    if(!alignableTransform()) {
       ATH_MSG_FATAL("The readout element "<<idHelperSvc()->toStringDetEl(identify())<<" has no assigned alignable node");
       return StatusCode::FAILURE;
    } 
    if (m_pars.layers.empty()) {
       ATH_MSG_FATAL("The readout element "<<idHelperSvc()->toStringDetEl(identify())<<" doesn't have any layers defined");
       return StatusCode::FAILURE;
    }
#ifndef SIMULATIONBASE
    ATH_CHECK(planeSurfaceFactory(geoTransformHash(), m_pars.layerBounds->make_bounds(m_pars.halfShortWidth, 
                                                                                      m_pars.halfLongWidth, 
                                                                                      m_pars.halfHeight)));
#endif
    for (unsigned int layer = 0; layer < m_pars.layers.size(); ++layer) {
      IdentifierHash layHash{layer};
      if (m_pars.layers[layer].hash() != layHash) {
         ATH_MSG_FATAL("Layer "<<m_pars.layers[layer]<<" has a very strange hash. Expect "<<layer);
         return StatusCode::FAILURE;
      }
      ATH_CHECK(insertTransform(layHash, 
                                 [this](RawGeomAlignStore* store, const IdentifierHash& hash){
                                    return toStation(store) * fromGapToChamOrigin(hash); 
                                 }));
#ifndef SIMULATIONBASE
      const StripDesign& design{m_pars.layers[layer].design()};
      ATH_CHECK(planeSurfaceFactory(layHash, m_pars.layerBounds->make_bounds(design.shortHalfHeight(),
                                                                                   design.longHalfHeight(),
                                                                                   design.halfWidth())));
#endif
    }
#ifndef SIMULATIONBASE
    m_pars.layerBounds.reset();
#endif
    return StatusCode::SUCCESS;
}

Amg::Transform3D MmReadoutElement::fromGapToChamOrigin(const IdentifierHash& layHash) const {
      const unsigned layIdx{static_cast<unsigned>(layHash)};
      return m_pars.layers[layIdx].toOrigin();
}


Amg::Vector3D MmReadoutElement::stripPosition(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const {
   const IdentifierHash lHash = layerHash(measHash);
   unsigned int layIdx = static_cast<unsigned int>(lHash);
   if (layIdx < m_pars.layers.size()) {
      return localToGlobalTrans(ctx, lHash) * m_pars.layers[layIdx].localStripPos(stripNumber(measHash));
   }
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.layers.size());
   return Amg::Vector3D::Zero();
}


Amg::Vector3D MmReadoutElement::leftStripEdge(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const {
    const IdentifierHash lHash = layerHash(measHash);
    unsigned int layIdx = static_cast<unsigned int>(lHash);
    if (layIdx < m_pars.layers.size()) {
       return localToGlobalTrans(ctx, lHash) * m_pars.layers[layIdx].localStripRightEdge(stripNumber(measHash));
    }
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.layers.size());
    return Amg::Vector3D::Zero();
}

Amg::Vector3D MmReadoutElement::rightStripEdge(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const{
    const IdentifierHash lHash = layerHash(measHash);
    unsigned int layIdx = static_cast<unsigned int>(lHash);
    if (layIdx < m_pars.layers.size()) {
       return localToGlobalTrans(ctx, lHash) * m_pars.layers[layIdx].localStripLeftEdge(stripNumber(measHash));
    }
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<layIdx
                 <<" is out of range. Maximum range "<<m_pars.layers.size());
    return Amg::Vector3D::Zero();
}

}
