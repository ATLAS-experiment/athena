/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_MUONCHAMBERTOOLTEST_H
#define MUONGEOMODELTESTR4_MUONCHAMBERTOOLTEST_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/Chamber.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>

#include <ActsGeometryInterfaces/GeometryContext.h>
#include <ActsGeometryInterfaces/ITrackingGeometrySvc.h>
#include <StoreGate/ReadCondHandleKey.h>


#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"

namespace MuonGMR4 { 


class MuonChamberToolTest: public AthReentrantAlgorithm {
    public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;
        ~MuonChamberToolTest() = default;

        StatusCode execute(const EventContext& ctx) const override;        
        StatusCode initialize() override;        

        bool isReEntrant() const override final {return false;}   
    
    private:
        /** @brief Returns the 8 edge points from a trapezoidal / cuboid volume */
        std::array<Amg::Vector3D, 8> cornerPoints(const Acts::Volume& volume) const;
        std::array<Amg::Vector3D, 8> cornerPoints(const Acts::GeometryContext& gctx, const Acts::StrawSurface& surface) const;
        std::array<Amg::Vector3D, 4> cornerPoints(const Acts::GeometryContext& gctx, const Acts::PlaneSurface&) const;
        
        
        void saveEnvelope(const ActsTrk::GeometryContext& gctx,
                          const std::string& envName,
                          const Acts::Volume& envelopeVol,
                          const std::vector<const MuonGMR4::MuonReadoutElement*>& assocRE,
                          const std::vector<std::shared_ptr<Acts::Volume>>& subVolumes={}) const;
        /** @brief Check whether the chamber envelopes are consistent */
        StatusCode checkChambers(const ActsTrk::GeometryContext& gctx) const;
        /** @brief Check envelopes */
        StatusCode checkEnvelopes(const ActsTrk::GeometryContext& gctx) const;
        /** @brief Checks whether the readout elements of an enevelope are completely embedded into the envelope */
        template <class EnvelopeType>
          StatusCode allReadoutInEnvelope(const ActsTrk::GeometryContext& ctx,
                                          const EnvelopeType& envelope) const; 
        
        /** @brief Checks whether the point is inside of an envelope object, i.e.
         *         the spectrometer sector or the chamber
         *  @param envelope: Reference to the envelope to check
         *  @param boundVol: Reference to the bounding volume representing the envelope
         *  @param point: Point that needs to be inside the volume
         *  @param descr: Description of the point
         *  @param channelId: Identifier for more information if the point is outside */
        template <class EnvelopeType>        
            StatusCode pointInside(const EnvelopeType& envelope,
                                   const Acts::Volume& boundVol,
                                   const Amg::Vector3D& point,
                                   const std::string& descr,
                                   const Identifier& channelId) const;

        /** @brief Checks whether the point is inside a tracking volume
         *  @param volume: Reference to the tracking volume to check
         *  @param point: Point that needs to be inside the volume
         *  @param descr: Description of the point
         *  @param chamberId: Identifier for more information if the point is outside */
        StatusCode pointInside(const Acts::TrackingVolume& volume,
                                  const Amg::Vector3D& point,
                                  const std::string& descr,
                                  const Identifier& chamberId) const;
                          
        bool hasOverlap(const std::array<Amg::Vector3D, 8>& chamberEdges,
                        const Acts::Volume& volume) const;

        /** @brief Checks whether all channels of a given readout element are fully covered by the
         *         envelope.
         *  @param gctx: Geometry context carrying all alignment & global transformations
         *  @param readOutEle: Readout element to test
         *  @param envelope: Reference to the envelope to check
         *  @param boundVol: Bounding volume representing the envelope */
        template <class EnvelopeType>
          StatusCode testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                    const MdtReadoutElement& readOutEle,
                                    const EnvelopeType& envelope,
                                    const Acts::Volume& boundVol) const;
        template <class EnvelopeType>
          StatusCode testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                    const RpcReadoutElement& readOutEle,
                                    const EnvelopeType& envelope,
                                    const Acts::Volume& boundVol) const;
        template <class EnvelopeType>
          StatusCode testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                    const TgcReadoutElement& readOutEle,
                                    const EnvelopeType& envelope,
                                    const Acts::Volume& boundVol) const;
        template <class EnvelopeType>
          StatusCode testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                    const sTgcReadoutElement& readOutEle,
                                    const EnvelopeType& envelope,
                                    const Acts::Volume& boundVol) const;
        template <class EnvelopeType>
          StatusCode testReadoutEle(const ActsTrk::GeometryContext& gctx,
                                    const MmReadoutElement& readOutEle,
                                    const EnvelopeType& envelope,
                                    const Acts::Volume& boundVol) const;



        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

        ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc","ActsTrackingGeometrySvc"};
        /** @brief Number of points to scan along the lines between two volume corners to check whether they belong to an another volume */
        Gaudi::Property<unsigned> m_overlapSamples{this, "overlapSamples", 100};
        /** @brief Name of the chamber output obj file */
        Gaudi::Property<std::string> m_overlapChambObj{this, "chamberOverlapFile", "OverlapingChambers.obj"};
        /** @brief The overlap of chamber volumes does not lead to a failure. In fact, the overlap between the T4 & BIS78 chambers
         *         is found to be unavoidable without boolean shapes in the tracking geometry*/
        Gaudi::Property<bool> m_ignoreOverlapCh{this, "ignoreChamberOverlap", true};
        /** @brief Dump the chambers & sectors as separate obj files */
        Gaudi::Property<bool> m_dumpObjs{this, "dumpVolumes" , false};
         const MuonDetectorManager* m_detMgr{nullptr};

};
}
#endif