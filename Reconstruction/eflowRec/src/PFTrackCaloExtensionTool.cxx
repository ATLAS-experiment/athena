/* Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration */

#include "PFTrackCaloExtensionTool.h"

#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Material/MaterialInteraction.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Surfaces/CurvilinearSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"

#include "ActsEvent/CaloExtension.h"
#include "ActsGeometryInterfaces/GeometryDefs.h"
#include "ActsInterop/UnitConverters.h"

#include "CaloGeoHelpers/CaloSampling.h"

#include "StoreGate/ReadDecorHandle.h"

#include "eflowTrackCaloPoints.h"

StatusCode PFTrackCaloExtensionTool::initialize() {

  ATH_CHECK( AthAlgTool::initialize() );
  ATH_CHECK(m_ctxProvider.initialize());

  ATH_CHECK(m_extensionReadDecorKey.initialize());

  return StatusCode::SUCCESS;
}

std::unique_ptr<eflowTrackCaloPoints> PFTrackCaloExtensionTool::execute(const EventContext& ctx, const xAOD::TrackParticle* track) const {

    ATH_MSG_DEBUG("About to extrapolate track with pt, eta and phi: " << track->pt() << ", " << track->eta() << " and " << track->phi());

    SG::ReadDecorHandle<xAOD::TrackParticleContainer, ElementLink<ActsTrk::CaloExtensionContainer>> extensionReadDecorHandle(m_extensionReadDecorKey, ctx);
    ElementLink<ActsTrk::CaloExtensionContainer> trackCaloExtensionLink = extensionReadDecorHandle(*track);

    std::map<eflowCalo::LAYER, const Acts::BoundTrackParameters*> parametersMap;
    const Acts::GeometryContext gctx = m_ctxProvider.getGeometryContext(ctx);

    if (!trackCaloExtensionLink.isValid()) {
       ATH_MSG_DEBUG("Decoration exists, but this track has no valid CaloExtension link (null link)");
    }
    else{
      const ActsTrk::CaloExtension* trackCaloExtension = *trackCaloExtensionLink;
      ATH_MSG_DEBUG("Have CaloExtensions with size " << trackCaloExtension->size() << " for this track");
      ATH_MSG_DEBUG("Are the parameters empty?: " << trackCaloExtension->parameters().empty());

      //loop over the calo sampling - simple loop over CaloSampling::CaloSample enum values, and check if we have a corresponding entry in the parameters map
      for (int i = CaloSampling::EMB1; i < CaloSampling::Unknown; ++i) {
        //skip the other presampler
        if (i == CaloSampling::PreSamplerE) continue;

        std::vector<Acts::BoundTrackParameters> caloPars = trackCaloExtension->caloParameters(static_cast<CaloSampling::CaloSample>(i));
        if (0 == caloPars.size()) continue;
         /* parametersMap may have several entries for Tile1,2,3 
            due to the simplfification of eflowCalo::LAYER which only considers
            Tile in general and not barrel, exctended barrel and gap.
            The impact is negligible as the eta/phi of these entries are very similar
            https://its.cern.ch/jira/browse/ATLJETMET-242
            THIS HAS NOT BEEN CHECKED FOR ACTS YET, BUT SHOULD BE THE SAME AS THE ABOVE CASE
        */
        parametersMap[eflowCalo::translateSampl(static_cast<CaloSampling::CaloSample>(i))] = &caloPars[0];
        if (caloPars.size() > 1) {
          //print eta, phi and details of naming etc
          ATH_MSG_DEBUG("Multiple parameters for CaloSample: " << i << " with size: " << caloPars.size());
          double etaNominal = caloPars[0].position(gctx).eta();
          double phiNominal = caloPars[0].position(gctx).phi();
          for (const Acts::BoundTrackParameters& pars : caloPars) {
            double etaDiff = std::abs(pars.position(gctx).eta() - etaNominal);
            double phiDiff = std::abs(pars.position(gctx).phi() - phiNominal);
            //for now we print the difference if larger than smallest possible cell width/2 in the calorimeter, 
            //which is 0.025 in both eta and phi
            double cellWidthReference = 0.025/2.0;
            if (etaDiff > cellWidthReference || phiDiff > cellWidthReference) {
              ATH_MSG_WARNING("Eta, phi difference from nominal: " << etaDiff << ", " << phiDiff);
            }
          }
          ATH_MSG_DEBUG("Using the first one for the eflowTrackCaloPoints object");
        }
      }
    }

    return std::make_unique<eflowTrackCaloPoints>(parametersMap, gctx);
}

StatusCode PFTrackCaloExtensionTool::finalize() {
  return StatusCode::SUCCESS;
}
