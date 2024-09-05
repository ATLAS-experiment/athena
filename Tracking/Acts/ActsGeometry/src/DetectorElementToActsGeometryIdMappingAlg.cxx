/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#include "DetectorElementToActsGeometryIdMappingAlg.h"

// PACKAGE
#include "ActsGeometryInterfaces/ActsGeometryContext.h"

// ATHENA
#include "AthenaKernel/IOVInfiniteRange.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "StoreGate/WriteCondHandle.h"

#include "ActsGeometry/ActsDetectorElement.h"
#include "Acts/Geometry/TrackingGeometry.hpp"

using namespace ActsTrk;
DetectorElementToActsGeometryIdMappingAlg::DetectorElementToActsGeometryIdMappingAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

DetectorElementToActsGeometryIdMappingAlg::~DetectorElementToActsGeometryIdMappingAlg() = default;

StatusCode DetectorElementToActsGeometryIdMappingAlg::initialize() {
    ATH_CHECK(m_detectorElementToGeometryIdMapKey.initialize());
    ATH_CHECK(m_trackingGeometryTool.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode DetectorElementToActsGeometryIdMappingAlg::execute(const EventContext& ctx) const {
    SG::WriteCondHandle<ActsTrk::DetectorElementToActsGeometryIdMap>
       detectorElementToGeometryIdMap{m_detectorElementToGeometryIdMapKey, ctx};
    if (detectorElementToGeometryIdMap.isValid()) {
       return StatusCode::SUCCESS;
    }
    detectorElementToGeometryIdMap.addDependency (IOVInfiniteRange::infiniteTime());

    const Acts::TrackingGeometry *acts_tracking_geometry=m_trackingGeometryTool->trackingGeometry().get();
    ATH_CHECK( acts_tracking_geometry != nullptr);

    std::unique_ptr<DetectorElementToActsGeometryIdMap>
       detector_element_to_geoid = std::make_unique<DetectorElementToActsGeometryIdMap>();
    createDetectorElementToGeoIdMap(*acts_tracking_geometry,
                                    *detector_element_to_geoid);
    ATH_CHECK( detectorElementToGeometryIdMap.record( std::move(detector_element_to_geoid) ) );

    return StatusCode::SUCCESS;
}

namespace {
   template <typename T_EnumClass>
   constexpr std::size_t maxType();

   template <> constexpr std::size_t maxType<ActsTrk::DetectorType>() { return to_underlying(ActsTrk::DetectorType::UnDefined); }
   template <> [[maybe_unused]] constexpr std::size_t maxType<xAOD::UncalibMeasType>() { return to_underlying(xAOD::UncalibMeasType::nTypes); }

   template <typename T_EnumClass>
   constexpr unsigned char toChar(T_EnumClass value) {
      assert( sizeof(T_EnumClass) <= sizeof(std::size_t));
      assert(    static_cast<std::size_t>(to_underlying(value)) < static_cast<std::size_t>(std::numeric_limits<unsigned char>::max())
              && static_cast<std::size_t>(to_underlying(value)) <= maxType<T_EnumClass>() );
      return static_cast<unsigned char>(to_underlying(value));
   }

   constexpr std::array<unsigned char, maxType<ActsTrk::DetectorType>()> makeMeasurementTypeMap() {
      std::array<unsigned char, maxType<ActsTrk::DetectorType>()> measurement_type_map;
        std::fill(measurement_type_map.begin(),measurement_type_map.end(), toChar(xAOD::UncalibMeasType::nTypes));
        static_assert( to_underlying(xAOD::UncalibMeasType::nTypes) == 8 + 1 /* Other */ ); //otherwise some types are not included in this map
        measurement_type_map[toChar(ActsTrk::DetectorType::Pixel)]=toChar(xAOD::UncalibMeasType::PixelClusterType);
        measurement_type_map[toChar(ActsTrk::DetectorType::Sct)]=toChar(xAOD::UncalibMeasType::StripClusterType);
        measurement_type_map[toChar(ActsTrk::DetectorType::Mdt)]=toChar(xAOD::UncalibMeasType::MdtDriftCircleType);
        measurement_type_map[toChar(ActsTrk::DetectorType::Rpc)]=toChar(xAOD::UncalibMeasType::RpcStripType);
        measurement_type_map[toChar(ActsTrk::DetectorType::Tgc)]=toChar(xAOD::UncalibMeasType::TgcStripType);
        measurement_type_map[toChar(ActsTrk::DetectorType::Mm)]=toChar(xAOD::UncalibMeasType::MMClusterType);
        measurement_type_map[toChar(ActsTrk::DetectorType::sTgc)]=toChar(xAOD::UncalibMeasType::sTgcStripType);
        measurement_type_map[toChar(ActsTrk::DetectorType::Hgtd)]=toChar(xAOD::UncalibMeasType::HGTDClusterType);
        return measurement_type_map;
   }
}

void DetectorElementToActsGeometryIdMappingAlg::createDetectorElementToGeoIdMap(const Acts::TrackingGeometry &acts_tracking_geometry,
                                                                                DetectorElementToActsGeometryIdMap &detector_element_to_geoid)
const
{
   // create map from
   static constexpr std::array<unsigned char, maxType<ActsTrk::DetectorType>()> detector_to_measurement_type = makeMeasurementTypeMap();

   using Counter = struct { unsigned int n_detector_elements, n_missing_detector_elements, n_wrong_type; };
   Counter counter {0u,0u,0u};
   acts_tracking_geometry.visitSurfaces([&counter, &detector_element_to_geoid](const Acts::Surface *surface_ptr) {
      if (!surface_ptr) return;
      const Acts::Surface &surface = *surface_ptr;
      const Acts::DetectorElementBase*detector_element = surface.associatedDetectorElement();
      if (detector_element) {
         const ActsDetectorElement *acts_detector_element = dynamic_cast<const ActsDetectorElement*>(detector_element);
         if (acts_detector_element) {
            xAOD::UncalibMeasType
               meas_type = static_cast<xAOD::UncalibMeasType>(detector_to_measurement_type.at(to_underlying(acts_detector_element->detectorType())));
            const auto*trk_detector_element  = dynamic_cast<const Trk::TrkDetElementBase*>(acts_detector_element->upstreamDetectorElement());
            if(trk_detector_element  != nullptr) {
               detector_element_to_geoid.insert( std::make_pair( makeDetectorElementKey(meas_type, trk_detector_element->identifyHash()),
                                                                 DetectorElementToActsGeometryIdMap::makeValue(surface.geometryId())) );
            }
            else {
               ++counter.n_wrong_type;
            }
         }
         else {
            ++counter.n_wrong_type;
         }
         ++counter.n_detector_elements;
      }
      else {
         ++counter.n_missing_detector_elements;
      }
   }, true /*sensitive surfaces*/);

   ATH_MSG_INFO( "Surfaces without associated detector elements " << counter.n_missing_detector_elements
                 << " (with " << counter.n_detector_elements << ")" );
   if (counter.n_detector_elements==0) {
      ATH_MSG_ERROR( "No surface with associated detector element" );
   }
   if (counter.n_wrong_type>0) {
      ATH_MSG_WARNING( "Surfaces associated to detector elements not of type Trk::TrkDetElementBase :" << counter.n_wrong_type);
   }
}

