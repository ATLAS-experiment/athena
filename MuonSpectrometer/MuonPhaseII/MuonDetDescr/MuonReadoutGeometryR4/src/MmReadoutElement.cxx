/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/MmReadoutElement.h>


#include <GeoPrimitives/GeoPrimitivesHelpers.h>

#include <AthenaBaseComps/AthCheckMacros.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <optional>

#ifndef SIMULATIONBASE
#   include "Acts/Surfaces/Surface.hpp"
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
    MuonReadoutElement(args),
      m_pars{std::move(args)} {
}

const parameterBook& MmReadoutElement::getParameters() const { return m_pars; }

StatusCode MmReadoutElement::initElement() {    
    ATH_MSG_DEBUG("Parameter book "<<parameterBook());
    ATH_CHECK(createGeoTransform());
 
    if (m_pars.layers.empty()) {
       ATH_MSG_FATAL("The readout element "<<idHelperSvc()->toStringDetEl(identify())<<" doesn't have any layers defined");
       return StatusCode::FAILURE;
    }
#ifndef SIMULATIONBASE
    ATH_CHECK(planeSurfaceFactory(geoTransformHash(), 
                m_pars.layerBounds->makeBounds<Acts::TrapezoidBounds>(m_pars.halfShortWidth, 
                                                                      m_pars.halfLongWidth, 
                                                                      m_pars.halfHeight,
                                                                      90.*Gaudi::Units::deg)));
#endif
    for (unsigned int layer = 0; layer < m_pars.layers.size(); ++layer) {
      IdentifierHash layHash{layer};
      if (m_pars.layers[layer]->hash() != layHash) {
         ATH_MSG_FATAL("Layer "<<m_pars.layers[layer]<<" has a very strange hash. Expect "<<layer);
         return StatusCode::FAILURE;
      }
      ATH_CHECK(insertTransform<MmReadoutElement>(layHash));
#ifndef SIMULATIONBASE
      const StripDesign& design{m_pars.layers[layer]->design()};

      ATH_CHECK(planeSurfaceFactory(layHash, 
            m_pars.layerBounds->makeBounds<Acts::TrapezoidBounds>(design.shortHalfHeight(),
                                                                  design.longHalfHeight(),
                                                                  design.halfWidth(),
                                                                  90.*Gaudi::Units::deg - design.stereoAngle())));
#endif
    }
#ifndef SIMULATIONBASE
    m_pars.layerBounds.reset();
#endif
    return StatusCode::SUCCESS;
}

Amg::Transform3D MmReadoutElement::fromGapToChamOrigin(const IdentifierHash& layHash) const {
      return stripLayer(layHash).toOrigin();
}


#if defined(FLATTEN) && defined(__GNUC__)
// We compile this function with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
[[gnu::flatten]]
#endif
Amg::Vector3D MmReadoutElement::stripPosition(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const {
   const IdentifierHash lHash = layerHash(measHash);
   if (static_cast<unsigned int>(lHash) < m_pars.layers.size()) {
      return localToGlobalTrans(ctx, lHash) * stripLayer(lHash).localStripPosition(stripNumber(measHash));
   }
   ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<static_cast<unsigned int>(lHash)
                 <<" is out of range. Maximum range "<<m_pars.layers.size());
   return Amg::Vector3D::Zero();
}


#if defined(FLATTEN) && defined(__GNUC__)
// We compile this function with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
[[gnu::flatten]]
#endif
Amg::Vector3D MmReadoutElement::leftStripEdge(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const {
    const IdentifierHash lHash = layerHash(measHash);
    if (static_cast<unsigned int>(lHash) < m_pars.layers.size()) {
       return localToGlobalTrans(ctx, lHash) * stripLayer(lHash).localStripLeftEdge(stripNumber(measHash));
    }
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<static_cast<unsigned int>(lHash)
                 <<" is out of range. Maximum range "<<m_pars.layers.size());
    return Amg::Vector3D::Zero();
}

#if defined(FLATTEN) && defined(__GNUC__)
// We compile this function with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
[[gnu::flatten]]
#endif
Amg::Vector3D MmReadoutElement::rightStripEdge(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const{
    const IdentifierHash lHash = layerHash(measHash);
    if (static_cast<unsigned int>(lHash) < m_pars.layers.size()) {
       return localToGlobalTrans(ctx, lHash) * stripLayer(lHash).localStripRightEdge(stripNumber(measHash));
    }
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The layer hash "<<static_cast<unsigned int>(lHash)
                 <<" is out of range. Maximum range "<<m_pars.layers.size());
    return Amg::Vector3D::Zero();
}


}
