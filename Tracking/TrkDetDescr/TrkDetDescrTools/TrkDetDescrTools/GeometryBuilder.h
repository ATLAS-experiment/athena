/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GeometryBuilder.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRTOOLS_GEOMETRYBUILDER_H
#define TRKDETDESCRTOOLS_GEOMETRYBUILDER_H

// Amg
#include "GeoPrimitives/GeoPrimitives.h"
// Trk
#include "TrkDetDescrInterfaces/IGeometryBuilder.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeArrayCreator.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeHelper.h"
#include "TrkDetDescrUtils/GeometrySignature.h"
#include "TrkGeometry/TrackingVolumeManipulator.h"
#include "TrkGeometry/Material.h"
// Gaudi & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#ifdef TRKDETDESCR_MEMUSAGE   
#include "TrkDetDescrUtils/MemoryLogger.h"
#endif  


#include "CxxUtils/checker_macros.h"
namespace Trk {

    class TrackingGeometry;
    class TrackingVolume;

    /** @class GeometryBuilder

      The Trk::TrackingGeometry Builder for ATLAS Geometry

      It retrieves Trk::TrackingGeometry builders for the subdetectors and joins them together 
      to a single Trk::TrackingGeometry.

      @author Andreas.Salzburger@cern.ch   
     */

    class GeometryBuilder :
      public AthAlgTool,
      public TrackingVolumeManipulator,
      virtual public IGeometryBuilder {

      public:
        /** Constructor */
        GeometryBuilder(const std::string&,const std::string&,const IInterface*);

        /** AlgTool initialize method */
        StatusCode initialize();

        
        /** TrackingGeometry Interface method - optionally a pointer to Bounds */
        std::unique_ptr<TrackingGeometry> trackingGeometry(TrackingVolume* tvol = 0) const;

        /** The unique signature */
        GeometrySignature geometrySignature() const { return Trk::Global; }

      private:

        /** TrackingGeometry for ATLAS setup */
        std::unique_ptr<TrackingGeometry> atlasTrackingGeometry() const;

#ifdef TRKDETDESCR_MEMUSAGE         
        MemoryLogger                        m_memoryLogger{};                //!< in case the memory is logged
#endif      

        Gaudi::Property<bool> m_createWorld{this, "CreateWorldManually", true,
	   "Boolean Switch to create World manually"};
        Gaudi::Property<int> m_navigationLevel{this, "NavigationLevel", 2};

        Gaudi::Property<std::vector<double>> m_worldDimension
	  {this, "WorldDimension", {},
	   "The dimensions of the manually created world"};
        Gaudi::Property<std::vector<double>> m_worldMaterialProperties
	  {this, "WorldMaterialProperties", {},
	   "The material properties of the created world"};
        Material m_worldMaterial{};               //!< the world material

        // -------------------------- Tools for geometry building ------------------------------------------------------ //

        ToolHandle<ITrackingVolumeArrayCreator> m_trackingVolumeArrayCreator
	  {this, "TrackingVolumeArrayCreator",
	   "Trk::TrackingVolumeArrayCreator/TrackingVolumeArrayCreator",
	   "Helper Tool to create TrackingVolume Arrays"};

        ToolHandle<ITrackingVolumeHelper> m_trackingVolumeHelper
	  {this, "TrackingVolumeHelper",
	   "Trk::TrackingVolumeHelper/TrackingVolumeHelper",
	   "Helper Tool to create TrackingVolumes"};

        ToolHandle<IGeometryBuilder> m_inDetGeometryBuilder
	  {this, "InDetTrackingGeometryBuilder", "",
	   "GeometryBuilder for the InnerDetector"};

        ToolHandle<IGeometryBuilder> m_caloGeometryBuilder
	  {this, "CaloTrackingGeometryBuilder", "",
	   "GeometryBuilder for the Calorimeters"};

        ToolHandle<IGeometryBuilder> m_muonGeometryBuilder
	  {this, "MuonTrackingGeometryBuilder", "",
	   "GeometryBuilder for the Muon System"};
        
        Gaudi::Property<bool> m_compactify{this, "Compactify", true,
	  "optimize event memory usage: register all surfaces with TG"};
        Gaudi::Property<bool> m_synchronizeLayers
	  {this, "SynchronizeLayers", true,
	   "synchronize contained layer dimensions to volumes"};

    };

} // end of namespace

#endif // TRKDETDESCRTOOLS_GEOMETRYBUILDER_H

