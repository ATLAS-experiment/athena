
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_IGeometryRealmConvTool_H
#define ACTSGEOMETRYINTERFACES_IGeometryRealmConvTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/IAlgTool.h"


#include "TrkEventPrimitives/SurfaceUniquePtrT.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkSurfaces/Surface.h"
#include "TrkParameters/TrackParameters.h"

#include <memory>

namespace Acts{
    class Surface;
    class BoundTrackParameters;
}

namespace ActsTrk{
    /** @brief Interface for the conversion tool to translate the surface and track parameter objects
     *         between the Acts & the ATLAS Trk realm.
     * 
     *         Surfaces associated with readout planes are taken from the corresponding tracking geometries
     *         Acts portal surfaces and other free surfaces are translated as free trk surfaces.
     *         Analogously, free Trk surfaces are translated to a free Acts surface. */
    class IGeometryRealmConvTool : virtual public IAlgTool {
        public:
            DeclareInterfaceID(IGeometryRealmConvTool, 1, 0);
            /** @brief Abrivation of the surface pointer with memory management for 
             *         free surfaces not associated with a Trk::DetElementBase */
            using SurfacePtr_t = Trk::SurfaceUniquePtrT<const Trk::Surface>;
            /** @brief Translates the parsed Acts surface into a Trk::Surface via associated detector element.
             *         For the ID measurements a direct link is provided and for the muon measurements the look-up
             *         is performed via the associated Identifier and the detector manager.
             *         Other surfaces trigger the creation of new free Trk surfaces
             *  @param ctx: The current event context 
             *  @param actsSurface: Refrence to the acts surface to translate */
            virtual SurfacePtr_t convertSurfaceToTrk(const EventContext& ctx,
                                                     const Acts::Surface& actsSurface) const = 0;
            /** @brief Translate the parsed Trk surface into an Acts surface. The detector element identifier
             *         of the surface needs to be filled into the internal tool's look-up map. Otherwise an exception
             *         is thrown.
             * @param atlasSurface: Refrence to the Trk surface to translate */
            virtual std::shared_ptr<const Acts::Surface> convertSurfaceToActs(const Trk::Surface& atlasSurface) const = 0;
            /** @brief Translates the Trk track parameters into bound Acts track parameters with a particle hypothesis.
             * @param atlasParameter: The Trk parameters to translate.
             * @param gcts: Geometry context needed for special treatment of the annulus bounds
             * @param hypothesis: Track hypothesis to use */
            virtual Acts::BoundTrackParameters 
                    convertTrackParametersToActs(const EventContext& ctx,
                                                 const Trk::TrackParameters& atlasParameter, 
                                                 Trk::ParticleHypothesis hypothesis = Trk::pion) const = 0;
            /** @brief Translates the bounded Acts track parameters to Trk parameters. The bound parameter surface
             *         must be translatble by the tool
             *  @param ctx: EventContext
             *  @param actsParameter: Refrence to the bounded parameters to translate*/
            virtual std::unique_ptr<Trk::TrackParameters> 
                    convertTrackParametersToTrk(const EventContext& ctx,
                                                const Acts::BoundTrackParameters& actsParameters) const = 0;
    };
}
#endif