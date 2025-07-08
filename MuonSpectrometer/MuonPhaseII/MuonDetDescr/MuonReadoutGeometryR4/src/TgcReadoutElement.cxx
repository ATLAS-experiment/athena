/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonReadoutGeometryR4/TgcReadoutElement.h>

#include <AthenaBaseComps/AthCheckMacros.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <optional>

#ifndef SIMULATIONBASE
#   include "Acts/Surfaces/TrapezoidBounds.hpp"
#   include "Acts/Surfaces/Surface.hpp"
#endif


using namespace ActsTrk;


namespace MuonGMR4 {
using parameterBook = TgcReadoutElement::parameterBook;
std::ostream& operator<<(std::ostream& ostr, const parameterBook& pars) {
   ostr<<"chamber dimensions --- ";
   ostr<<"thickness: "<<pars.halfThickness<<" ";
   ostr<<"height: "<<pars.halfHeight<<" ";
   ostr<<"shortWidth: "<<pars.halfWidthShort<<" ";
   ostr<<"longWidth: "<<pars.halfWidthLong<<" --- ";
   return ostr;
}
TgcReadoutElement::~TgcReadoutElement() = default;
TgcReadoutElement::TgcReadoutElement(defineArgs&& args)
    : MuonReadoutElement(args),
      m_pars{std::move(args)} {
}

const parameterBook& TgcReadoutElement::getParameters() const { return m_pars; }
StatusCode TgcReadoutElement::initElement() {
    
    ATH_CHECK(createGeoTransform());
    /// Check that the readoutelement has sensor layouts
    bool hasSensor{false};
    for (size_t s = 0; s < m_pars.sensorLayouts.size(); ++s) {
       const StripLayerPtr& layPtr{m_pars.sensorLayouts[s]};
       if (!layPtr) continue;
       if (layPtr->hash() != s) {
          ATH_MSG_FATAL("Layer "<<(*layPtr)<<" has an unexpected hash "<<s);
          return StatusCode::FAILURE;
       }
       ATH_MSG_VERBOSE(idHelperSvc()->toStringDetEl(identify())<<" gasGap: "<<gasGapNumber(layPtr->hash())
                      <<" isStrip: "<<isStrip(layPtr->hash())<<" hash: "<<s);
       hasSensor = true;
    }
    if (!hasSensor) {
         ATH_MSG_FATAL("No active layer is defined for "<<idHelperSvc()->toStringDetEl(identify()));
         return StatusCode::FAILURE;
    }
#ifndef SIMULATIONBASE
    ATH_CHECK(planeSurfaceFactory(geoTransformHash(),
                                  m_pars.layerBounds->makeBounds<Acts::TrapezoidBounds>(m_pars.halfWidthShort,
                                                                                       m_pars.halfWidthLong,
                                                                                       m_pars.halfHeight)));
#endif
    for (unsigned int gap = 1; gap <= nGasGaps(); ++gap) {
         if (numWireGangs(gap)) {
            const IdentifierHash layHash{constructHash(0, gap, false)}; 
            ATH_CHECK(insertTransform<TgcReadoutElement>(layHash));
#ifndef SIMULATIONBASE
            const StripDesign& layout{wireGangLayout(gap)};
            ATH_CHECK(planeSurfaceFactory(layHash, m_pars.layerBounds->makeBounds<Acts::TrapezoidBounds>(layout.shortHalfHeight(),
                                                                                                         layout.longHalfHeight(),
                                                                                                         layout.halfWidth(),
                                                                                                         90.* Gaudi::Units::deg)));
#endif
         }
         if (numStrips(gap)) {
            const IdentifierHash layHash{constructHash(0, gap, true)}; 
            ATH_CHECK(insertTransform<TgcReadoutElement>(layHash));
#ifndef SIMULATIONBASE
            const StripDesign& layout{stripLayout(gap)};
            /// Strips are rotated bounds
            ATH_CHECK(planeSurfaceFactory(layHash, m_pars.layerBounds->makeBounds<Acts::TrapezoidBounds>(layout.shortHalfHeight(),
                                                                                                         layout.longHalfHeight(),
                                                                                                         layout.halfWidth())));
#endif
         }
    }
#ifndef SIMULATIONBASE
    m_pars.layerBounds.reset();
#endif
    const IdentifierHash firstLay  = constructHash(0, 1, false);
    const IdentifierHash secondLay = constructHash(0, 2, false);
    ActsGeometryContext gctx{};
    m_gasThickness =(center(gctx, firstLay) - center(gctx, secondLay)).mag(); 
    return StatusCode::SUCCESS;
}

Amg::Transform3D TgcReadoutElement::fromGapToChamOrigin(const IdentifierHash& layHash) const {
      const unsigned layIdx{static_cast<unsigned>(layHash)};
      return m_pars.sensorLayouts[layIdx]->toOrigin();
}
Amg::Vector3D TgcReadoutElement::channelPosition(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const { 
   const StripLayerPtr& layDesign{sensorLayout(measHash)};
   if (!layDesign) {
       ATH_MSG_WARNING("The gasGap "<<gasGapNumber(measHash)<<" & strip:"<<isStrip(measHash)<<" is unknown");
       return Amg::Vector3D::Zero();
   }
   return localToGlobalTrans(ctx, layerHash(measHash)) * layDesign->localStripPosition(channelNumber(measHash));
}
}
