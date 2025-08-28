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

#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <ActsGeometryInterfaces/IDetectorVolumeSvc.h>
#include <StoreGate/ReadCondHandleKey.h>

#include "ActsGeometryInterfaces/IActsTrackingGeometrySvc.h"

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
        std::array<Amg::Vector3D, 8> cornerPoints(const Acts::Volume& volume) const;
        std::array<Amg::Vector3D, 8> cornerPoints(const Acts::GeometryContext& gctx, const Acts::StrawSurface& surface) const;
        std::array<Amg::Vector3D, 4> cornerPoints(const Acts::GeometryContext& gctx, const Acts::PlaneSurface&) const;
        /** @brief Checks whether the readout elements of an enevelope are completely embedded into the envelope */
        template <class EnvelopeType>
          StatusCode allReadoutInEnvelope(const ActsGeometryContext& ctx,
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

        /** @brief Checks whether all channels of a given readout element are fully covered by the
         *         envelope.
         *  @param gctx: Geometry context carrying all alignment & global transformations
         *  @param readOutEle: Readout element to test
         *  @param envelope: Reference to the envelope to check
         *  @param boundVol: Bounding volume representing the envelope */
        template <class EnvelopeType>
          StatusCode testReadoutEle(const ActsGeometryContext& gctx,
                                    const MdtReadoutElement& readOutEle,
                                    const EnvelopeType& envelope,
                                    const Acts::Volume& boundVol) const;
        template <class EnvelopeType>
          StatusCode testReadoutEle(const ActsGeometryContext& gctx,
                                    const RpcReadoutElement& readOutEle,
                                    const EnvelopeType& envelope,
                                    const Acts::Volume& boundVol) const;
        template <class EnvelopeType>
          StatusCode testReadoutEle(const ActsGeometryContext& gctx,
                                    const TgcReadoutElement& readOutEle,
                                    const EnvelopeType& envelope,
                                    const Acts::Volume& boundVol) const;
        template <class EnvelopeType>
          StatusCode testReadoutEle(const ActsGeometryContext& gctx,
                                    const sTgcReadoutElement& readOutEle,
                                    const EnvelopeType& envelope,
                                    const Acts::Volume& boundVol) const;
        template <class EnvelopeType>
          StatusCode testReadoutEle(const ActsGeometryContext& gctx,
                                    const MmReadoutElement& readOutEle,
                                    const EnvelopeType& envelope,
                                    const Acts::Volume& boundVol) const;



        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

        ServiceHandle<IActsTrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc","ActsTrackingGeometrySvc"};

        const MuonDetectorManager* m_detMgr{nullptr};

};
}
#endif