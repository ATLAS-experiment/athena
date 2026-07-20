/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSEVENTCNV_GeometryRealmConvTool_H
#define ACTSEVENTCNV_GeometryRealmConvTool_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "ActsGeometryInterfaces/IGeometryRealmConvTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"

#include "ActsEvent/ContextUtility.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadCondHandleKey.h"


namespace Acts{
    class SurfaceBounds;
}

namespace ActsTrk{
    /** @brief Implementation of the IGeometryRealmConvTool. At initialization stage the tool associates
     *         every Acts::Surface with an ATLAS identifier to convert the sensitive surfaces without 
     *         extra memory allocation. If portals and free surfaces are converted as new free Trk::Surfaces
     *         with bounds. To return aligned muon surfaces in the Acts -> Trk conversion, the tool has a
     *         data dependency on the legacy Muon::DetectorManager */
    class GeometryRealmConvTool : public extends<AthAlgTool, IGeometryRealmConvTool> {
        public:
            using base_class::base_class;
            ~GeometryRealmConvTool();

            virtual StatusCode initialize() override final;
            /** @copydoc IGeometryRealmConvTool::convertSurfaceToTrk */
            virtual SurfacePtr_t convertSurfaceToTrk(const EventContext& ctx,
                                                     const Acts::Surface& actsSurface) const override final;
            /** @copydoc IGeometryRealmConvTool::convertSurfaceToActs */
            virtual std::shared_ptr<const Acts::Surface> convertSurfaceToActs(const Trk::Surface& atlasSurface) const override final;
            /** @copydoc IGeometryRealmConvTool::convertTrackParametersToActs */
            virtual Acts::BoundTrackParameters 
                    convertTrackParametersToActs(const EventContext& ctx,
                                                 const Trk::TrackParameters& atlasParameter, 
                                                 Trk::ParticleHypothesis hypothesis = Trk::pion) const override final;
            /** @copydoc IGeometryRealmConvTool::convertTrackParametersToTrk */
            virtual std::unique_ptr<Trk::TrackParameters> 
                    convertTrackParametersToTrk(const EventContext& ctx,
                                                const Acts::BoundTrackParameters& actsParameters) const override final;
 
        private:
            /** @brief Translate the Acts surface bounds to its equivalent in the Trk realm.
             *          @note Not all bounds are implemented
             *  @param bounds: Refrence to the bounds to be translated */
            std::shared_ptr<Trk::SurfaceBounds> translateBounds(const Acts::SurfaceBounds& bounds) const;
            /** @brief Translate a surface that is not associated with any detector element.
             *         Bounds of the surface are also translated
             *  @param ctx: The event context to access the aligned detector
             *  @param surface Reference to the Acts surface for translation */
            SurfacePtr_t translateFreeSurface(const EventContext& ctx,
                                              const Acts::Surface& surface) const;
           
            PublicToolHandle<ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};

            /** @brief Context provider for geometry, magnetic field and calibration contexts */
            ActsTrk::ContextUtility m_ctxProvider{this};

            std::unordered_map<Identifier, std::shared_ptr<const Acts::Surface>> m_actsSurfaceMap{};

            Gaudi::Property<bool> m_extractMuonSurfaces{this, "ExtractMuonSurfaces", false,
                                                        "If True, use the MuonDetectorManager to extract the Muon surfaces"};

            /** @brief Detector manager to fetch the legacy Trk surfaces */
            SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muonMgrKey{this, "MuonManagerKey", "MuonDetectorManager"};



    };
}

#endif
