#ifndef ACTSTRK_SPACEPOINTADAPTER_H
#define ACTSTRK_SPACEPOINTADAPTER_H

#include "GaudiKernel/EventContext.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"
#include "InDetIdentifier/PixelID.h"

#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Surfaces/Surface.hpp"

#include <cassert>

namespace ActsTrk {
   struct GeoCache {
   private:
      const Acts::TrackingGeometry *m_trackingGeometry;
      const Acts::GeometryContext m_geometryContext;
      const DetectorElementToActsGeometryIdMap *m_geoIdMap;
      DetectorElementKey m_currentKey=std::numeric_limits<DetectorElementKey>::max();
      const Amg::Transform3D *m_currentTransform=nullptr;
      GeoCache(const Acts::TrackingGeometry *trackingGeometry,
               Acts::GeometryContext &&geometryContext,
               const DetectorElementToActsGeometryIdMap *geoIdMap)
         : m_trackingGeometry(trackingGeometry),
           m_geometryContext(std::move(geometryContext)),
           m_geoIdMap(geoIdMap)
      {}

      static GeoCache check(GeoCache &&cache) {
         assert(cache.m_trackingGeometry != nullptr);
         assert(cache.m_geoIdMap != nullptr);
         return cache;
      }
   public:
      const Amg::Transform3D &getTransform(DetectorElementKey key) {
         if (key != m_currentKey) {
            Acts::GeometryIdentifier geoId = m_geoIdMap->at(key);
            const Acts::Surface *surface = m_trackingGeometry->findSurface(geoId);
            assert(surface);
            m_currentTransform = &(surface->localToGlobalTransform(m_geometryContext));
            m_currentKey=key;
         }
         return *m_currentTransform;
      }
      static GeoCache make(const EventContext &ctx, const ActsTrk::ITrackingGeometryTool *actsTrackingGeometryTool) {
         if (actsTrackingGeometryTool) {
            return check(GeoCache(actsTrackingGeometryTool->trackingGeometry().get(),
                                  actsTrackingGeometryTool->getGeometryContext(ctx).context(),
                                  actsTrackingGeometryTool->surfaceIdMap()));
         }
         else {
            return GeoCache(nullptr,
                            Acts::GeometryContext::dangerouslyDefaultConstruct(),
                            nullptr);
         }
      }
   };

   
   template <typename T>
   struct SpacePointAdapter {
   protected:
      const T *m_srcObject;
   public:
      SpacePointAdapter(const T *srcObject) : m_srcObject(srcObject) {}
      xAOD::ConstVectorMap<3> globalPosition() const { return m_srcObject->globalPosition(); }
      // float x() const { return srcObject->x(); }
      // float y() const { return srcObject->y(); }
      // float z() const { return srcObject->z(); }
      std::optional<float> t() const { return m_srcObject->t(); }
      // only returns wafer Id on which the space point is located.
      Identifier identifier(const PixelID &pixelID) const { return Identifier(pixelID.wafer_id(identifierHash())); }
      IdentifierHash identifierHash() const { return IdentifierHash(m_srcObject->elementIdList().front()); }
      std::span<const xAOD::UncalibratedMeasurement * const> measurements() const {
         const std::vector< const xAOD::UncalibratedMeasurement* >& measurements=m_srcObject->measurements();
         return std::span<const xAOD::UncalibratedMeasurement * const>(measurements.begin(),measurements.end());
      }
      std::array<float,2> varianceZR(GeoCache &) const {
         return std::array<float,2>{ m_srcObject->varianceZ(),m_srcObject->varianceR() };
      }
   };

   template <>
   struct SpacePointAdapter<xAOD::PixelCluster> {
   protected:
      const xAOD::PixelCluster *m_srcObject;
   public:
      SpacePointAdapter(const xAOD::PixelCluster *srcObject) : m_srcObject(srcObject) {}
      xAOD::ConstVectorMap<3> globalPosition() const { return m_srcObject->globalPosition(); }
      // float x() const { return srcObject->globalPosition()[0]; }
      // float y() const { return srcObject->globalPosition()[1]; }
      // float z() const { return srcObject->globalPosition()[2]; }
      std::optional<float> t() const { return 0.f; }

      // returns pixel Id of first associated RDO
      Identifier identifier([[maybe_unused]] const PixelID &pixelID) const { return Identifier(m_srcObject->rdoList().front()); }
      IdentifierHash identifierHash() const { return IdentifierHash(m_srcObject->identifierHash()); }
      std::span<const xAOD::UncalibratedMeasurement * const> measurements() const {
         return std::span<const xAOD::UncalibratedMeasurement * const>(reinterpret_cast<const xAOD::UncalibratedMeasurement *const*>(&m_srcObject),1ul);
      }
      std::array<float,2> varianceZR(GeoCache &cache) const {
         const Amg::Transform3D &Tp = cache.getTransform(makeDetectorElementKey(m_srcObject->type(),
                                                                                m_srcObject->identifierHash()));
         // from PixelSpacePointFormationTool
         static constexpr double oneOverTwelve{0.08333};
         float width = m_srcObject->widthInEta();
         float covTerm = width*width*oneOverTwelve;
         auto localCov = m_srcObject->localCovariance<2>();
         if( covTerm < localCov(1, 1) )
            covTerm = localCov(1, 1);
         float cov_z = 6.f*covTerm*static_cast<float>(Tp(0, 2)*Tp(0, 2)+Tp(1, 2)*Tp(1, 2));
         float cov_r = 6.f*covTerm*static_cast<float>(Tp(2, 2)*Tp(2, 2));
                  
         return std::array<float,2>{ cov_z, cov_r };
      }
   };

   template <typename T>
   SpacePointAdapter<T> makeSpacePointAdapter(const T *space_point) { return SpacePointAdapter<T>(space_point); }
}
#endif
