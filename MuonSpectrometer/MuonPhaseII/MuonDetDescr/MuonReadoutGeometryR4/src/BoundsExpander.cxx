/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometryR4/BoundsExpander.h"

#ifndef SIMULATIONBASE

#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include "MuonReadoutGeometryR4/Chamber.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"

#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Geometry/CuboidVolumeBounds.hpp"
#include "Acts/Geometry/TrapezoidVolumeBounds.hpp"

namespace MuonGMR4 {
    BoundsExpander::BoundsExpander(const Acts::Transform3& toVolume,
                                   std::unique_ptr<const Acts::Logger> loggerObj): 
        m_toVolume{toVolume} {
        if (loggerObj) {
            m_logger = std::move(loggerObj);
        }
    }
    void BoundsExpander::expand(const Acts::GeometryContext& tgContext,
                                const MuonGMR4::Chamber& chamber) {
        expand(tgContext, *chamber.boundingVolume());
    }
    const Acts::Logger& BoundsExpander::logger() const { return *m_logger; }
    
    void BoundsExpander::expand(const Acts::GeometryContext& tgContext,
                                const MuonGMR4::SpectrometerSector& sector) {
        expand(tgContext, *sector.boundingVolume());
    }
    const Acts::Transform3& BoundsExpander::boxMidPoint() const {
        return m_shiftTrf;
    }
    std::shared_ptr<Acts::VolumeBounds> 
        BoundsExpander::makeBounds(const MuonGMR4::MuonReadoutElement& readoutElement,
                                   Acts::VolumeBoundFactory& boundFactory) {
        switch(readoutElement.detectorType()) {
          case ActsTrk::DetectorType::Mdt: {
             const auto& techEle = static_cast<const MdtReadoutElement&>(readoutElement);
             const auto& pars = techEle.getParameters();
             if (std::abs(pars.shortHalfX - pars.longHalfX) < Acts::s_epsilon) {
                return boundFactory.makeBounds<Acts::CuboidVolumeBounds>(pars.shortHalfX, pars.halfY, pars.halfHeight);
             }
             return boundFactory.makeBounds<Acts::TrapezoidVolumeBounds>(pars.shortHalfX, pars.longHalfX, 
                                                                        pars.halfY, pars.halfHeight);
          } case ActsTrk::DetectorType::Rpc: {
            const auto& techEle = static_cast<const RpcReadoutElement&>(readoutElement);
            const auto& pars = techEle.getParameters();
            return boundFactory.makeBounds<Acts::CuboidVolumeBounds>(pars.halfWidth, pars.halfLength, pars.halfThickness);
          } case ActsTrk::DetectorType::Tgc: {
            const auto& techEle = static_cast<const TgcReadoutElement&>(readoutElement);
            const auto& pars = techEle.getParameters();
            return boundFactory.makeBounds<Acts::TrapezoidVolumeBounds>(pars.halfWidthShort, pars.halfWidthLong, 
                                                                        pars.halfHeight, pars.halfThickness );
          } case ActsTrk::DetectorType::sTgc: {
            const auto& techEle = static_cast<const sTgcReadoutElement&>(readoutElement);
            const auto& pars = techEle.getParameters();
            return boundFactory.makeBounds<Acts::TrapezoidVolumeBounds>(pars.sHalfChamberLength, pars.lHalfChamberLength, 
                                                                        pars.halfChamberHeight, pars.halfChamberTck );
          } case ActsTrk::DetectorType::Mm: {
             const auto& techEle = static_cast<const MmReadoutElement&>(readoutElement);
             const auto& pars = techEle.getParameters();
             return boundFactory.makeBounds<Acts::TrapezoidVolumeBounds>(pars.halfShortWidth, pars.halfLongWidth, 
                                                                        pars.halfHeight, pars.halfThickness );
          } default: 
              THROW_EXCEPTION("Unsupported detector type "<<readoutElement.detectorType());
        }
        return nullptr;
    }
    void BoundsExpander::expand(const Acts::GeometryContext& tgContext,
                                const MuonGMR4::MuonReadoutElement& readoutElement,
                                Acts::VolumeBoundFactory& boundFactory) {
        expand(tgContext, Acts::Volume{readoutElement.localToGlobalTransform(tgContext),
                                       makeBounds(readoutElement, boundFactory)});
    }

    Acts::Transform3 BoundsExpander::toVertexCenter() const {
        return  m_shiftTrf.inverse() * m_toVolume;
    }
    void BoundsExpander::reCenter(std::span<const Amg::Vector3D> vertices) {
        ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - User defined center system: "
                    <<Amg::toString(m_toVolume));
        for (const Amg::Vector3D& vert : vertices) {
            const Amg::Vector3D locVert = m_toVolume* vert;
            for (std::size_t k = 0; k < m_locMax.size(); ++k) {
                m_locMax[k] = std::max(m_locMax[k], locVert[k]);
                m_locMin[k] = std::min(m_locMin[k], locVert[k]);
            }
        }
        Amg::Vector3D shiftVec{Amg::Vector3D::Zero()};
        for (std::size_t k =0 ; k < m_locMax.size(); ++k) {
            shiftVec[k] = 0.5*(m_locMax[k] + m_locMin[k]);
        }
        m_shiftTrf = Amg::getTranslate3D(shiftVec);
        ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - New transform: "<<Amg::toString(m_shiftTrf));
    }
    std::span<const double> BoundsExpander::minima() const {
        return m_locMin;
    }
    std::span<const double> BoundsExpander::maxima() const {
        return m_locMax;
    }
    std::vector<Amg::Vector3D> BoundsExpander::cornerPoints(const Acts::GeometryContext& tgContext, 
                                                            const Acts::Volume& volume) {
        std::vector<Amg::Vector3D> edges{};
        const Acts::VolumeBounds& bounds{volume.volumeBounds()};
        const Acts::Transform3& trf{volume.localToGlobalTransform(tgContext)};
        for (const Acts::OrientedSurface& boundary : bounds.orientedSurfaces(trf)) {
            std::vector<Amg::Vector3D> corners = cornerPoints(tgContext, *boundary.surface);
            edges.insert(edges.end(), std::make_move_iterator(corners.begin()), 
                                      std::make_move_iterator(corners.end()));
        }
        auto [begin, end] = std::ranges::unique(edges, [](const Amg::Vector3D& a, const Amg::Vector3D& b) {
                                                        return (a - b).mag2() < Acts::s_onSurfaceTolerance;
        });
        edges.erase(begin, end);
        return edges;
    }
    std::vector<Amg::Vector3D> BoundsExpander::cornerPoints(const Acts::GeometryContext& tgContext, 
                                                             const Acts::Surface& surface) {
        return surface.polyhedronRepresentation(tgContext, 10).vertices;
    }
    std::unique_ptr<Acts::TrackingVolume> BoundsExpander::makeEnvelope(Acts::VolumeBoundFactory& factory,
                                                                       std::string_view volName) {
        auto volume = std::make_unique<Acts::TrackingVolume>(toVertexCenter().inverse(),
                                                             makeBounds(factory), std::string{volName});
        ACTS_DEBUG(__func__<<"() "<<__LINE__<<" - Created new volume envelope "
                <<volName<<" located at @"<<Amg::toString(toVertexCenter().inverse())
                <<" with bounds: "<<volume->volumeBounds());
        return volume;
    }
}
#endif