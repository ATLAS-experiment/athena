/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_MUONCHAMBERTOOLTEST_H
#define MUONGEOMODELTESTR4_MUONCHAMBERTOOLTEST_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonReadoutGeometryR4/Chamber.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"

#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "StoreGate/ReadCondHandleKey.h"


#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"

namespace MuonGMR4 { 


class MuonChamberToolTest: public AthReentrantAlgorithm {
    public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;
        ~MuonChamberToolTest() = default;

        StatusCode execute(const EventContext& ctx) const override;        
        StatusCode initialize() override;        

        bool isReEntrant() const override final {return false;}   
    
    private:
        
        void saveEnvelope(const Acts::GeometryContext& tgContext,
                          const std::string& envName,
                          const Acts::Volume& envelopeVol,
                          const std::vector<const Acts::Surface*>& assocSurfaces,
                          const std::vector<const Acts::Volume*>& subVolumes) const;

        StatusCode checkPortals(const Acts::GeometryContext& gctx,
                                const Acts::TrackingVolume& volume) const;
        /** @brief Checks whether all / none of the surfaces are contained in the volume
         *         The method is called to ensure that all surfaces are within the envelope
         *         to which they are attributed or that no surface from a neighbouring volume
         *         is expanding into the volume of interest 
         *  @param tgContext: The Geometry context to position the volume and the surfaces
         *  @param volume: The volume providing the bounds and the refernce frame
         *  @param surfaces: The amount of surfaces to count        
         *  @param badSurfaces: Vector where all surfaces that are not fully contained or 
         *                      overlap with the volume are appended to
         *  @param sameVolume: The surfaces are actually atributed to the volume and
         *                      are required to be constrained */
        StatusCode checkSurfaceVolumeBounds(const Acts::GeometryContext& tgContext,
                                            const Acts::TrackingVolume& volume,
                                            Acts::TrackingVolume::SurfaceRange surfaces,
                                            std::vector<const Acts::Surface*>& badSurfaces,
                                            const bool sameVolume) const;
        /** @brief Check whether the chamber envelopes are consistent */
        StatusCode checkChambers(const ActsTrk::GeometryContext& gctx) const;
        /** @brief Check envelopes */
        StatusCode checkEnvelopes(const ActsTrk::GeometryContext& gctx) const;

        /** @brief Checks whether the readout elements of an enevelope are completely embedded into the envelope */
        StatusCode allReadoutInEnvelope(const ActsTrk::GeometryContext& ctx,
                                        const Chamber& envelope) const;
 
        /** @brief Check the consistency of the tracking volumes */
        StatusCode checkTrackingVolume(const Acts::GeometryContext& tgContext,
                                        const Acts::TrackingVolume& volume) const; 
                          
        /** @brief Checks whether the point is inside of an envelope object, i.e.
         *         the spectrometer sector or the chamber
         *  @param gctx: Geometry context carrying the aligned local -> global transfors
         *  @param envelope: Reference to the envelope to check
         *  @param boundVol: Reference to the bounding volume representing the envelope
         *  @param point: Point that needs to be inside the volume
         *  @param descr: Description of the point
         *  @param channelId: Identifier for more information if the point is outside */
        StatusCode pointInside(const ActsTrk::GeometryContext& gctx,
                                const Acts::Volume& boundVol,
                                const Amg::Vector3D& point,
                                const std::string_view descr,
                                const Identifier& channelId) const;

        /** @brief Checks whether the edge points from a trapezoid/cuboid/diamond form a volume
         *         overlapping with the given volume
         *  @param gctx: Geometry context carrying all alignment & global transformations
         *  @param chamberEdges: Edge points of the volume that might overlap
         *  @param volume: Second volume used for the overlap checking */           
        bool hasOverlap(const ActsTrk::GeometryContext& gctx,
                        const std::vector<Amg::Vector3D>& chamberEdges,
                        const Acts::Volume& volume) const;
        
        /** @brief Tests whether all surfaces defined by the readout element have a valid
         *         geometry identifier and whether they are all contained within the volume
         *  @param gctx: The geometry context to align the readout element and the volume
         *  @param element: The readout element to test
         *  @param boundVol: The surrounding volume in which the element must be contained */
        StatusCode testReadoutSurfaces(const ActsTrk::GeometryContext& gctx,
                                       const MuonReadoutElement& element,
                                       const Acts::Volume& boundVol) const;

        /** @brief Checks whether all channels of a given readout element are fully covered by the
         *         envelope.
         *  @param gctx: Geometry context carrying all alignment & global transformations
         *  @param readOutEle: Readout element to test
         *  @param boundVol: Bounding volume representing the envelope */
        StatusCode testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                  const MdtReadoutElement& readOutEle,
                                  const Acts::Volume& boundVol) const;
        /** @brief Checks whether all channels of a given readout element are fully covered by the
         *         envelope.
         *  @param gctx: Geometry context carrying all alignment & global transformations
         *  @param readOutEle: Readout element to test
         *  @param boundVol: Bounding volume representing the envelope */
        StatusCode testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                  const RpcReadoutElement& readOutEle,
                                  const Acts::Volume& boundVol) const;
        /** @brief Checks whether all channels of a given readout element are fully covered by the
         *         envelope.
         *  @param gctx: Geometry context carrying all alignment & global transformations
         *  @param readOutEle: Readout element to test
         *  @param boundVol: Bounding volume representing the envelope */
        StatusCode testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                  const TgcReadoutElement& readOutEle,
                                  const Acts::Volume& boundVol) const;
        /** @brief Checks whether all channels of a given readout element are fully covered by the
         *         envelope.
         *  @param gctx: Geometry context carrying all alignment & global transformations
         *  @param readOutEle: Readout element to test
         *  @param boundVol: Bounding volume representing the envelope */
        StatusCode testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                  const sTgcReadoutElement& readOutEle,
                                  const Acts::Volume& boundVol) const;
        /** @brief Checks whether all channels of a given readout element are fully covered by the
         *         envelope.
         *  @param gctx: Geometry context carrying all alignment & global transformations
         *  @param readOutEle: Readout element to test
         *  @param boundVol: Bounding volume representing the envelope */
        StatusCode testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                  const MmReadoutElement& readOutEle,
                                  const Acts::Volume& boundVol) const;

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        ActsTrk::GeoContextReadKey_t m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

        ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc","ActsTrackingGeometrySvc"};
        /** @brief Number of points to scan along the lines between two volume corners to check whether they belong to an another volume */
        Gaudi::Property<unsigned> m_overlapSamples{this, "overlapSamples", 50};
        /** @brief Name of the chamber output obj file */
        Gaudi::Property<std::string> m_overlapChambObj{this, "chamberOverlapFile", "OverlapingChambers.obj"};
        /** @brief The overlap of chamber volumes does not lead to a failure. In fact, the overlap between the T4 & BIS78 chambers
         *         is found to be unavoidable without boolean shapes in the tracking geometry*/
        Gaudi::Property<bool> m_ignoreOverlapCh{this, "ignoreChamberOverlap", true};
         /** @brief The exceeding surfaces does not lead to a failure. In fact, the tubes are exceeded in BIS78 (TODO)*/
        Gaudi::Property<bool> m_ignoreOutsideSurf{this, "ignoreOutsideSurface", true};
        /** @brief Dump the chambers & sectors as separate obj files */
        Gaudi::Property<bool> m_dumpObjs{this, "dumpVolumes" , false};
         const MuonDetectorManager* m_detMgr{nullptr};

};
}
#endif
