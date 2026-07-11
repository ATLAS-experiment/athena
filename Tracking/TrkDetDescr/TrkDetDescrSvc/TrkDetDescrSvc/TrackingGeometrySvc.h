/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackingGeometrySvc.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_TRACKINGGEOMETRYSERVICE_H
#define TRKDETDESCRINTERFACES_TRACKINGGEOMETRYSERVICE_H

#include "TrkDetDescrInterfaces/ITrackingGeometrySvc.h"
#include "TrkDetDescrUtils/LayerIndex.h"
#include "TrkGeometry/LayerMaterialMap.h"
#include "TrkDetDescrInterfaces/IGeometryBuilder.h"
#include "TrkDetDescrInterfaces/IGeometryProcessor.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthService.h"
#include "EventInfoMgt/ITagInfoMgr.h"

#ifdef TRKDETDESCR_MEMUSAGE   
#include "TrkDetDescrUtils/MemoryLogger.h"
#endif


class StoreGateSvc;
class ISvcLocator;
template <class TYPE> class SvcFactory;

namespace Trk {
  class TrackingGeometry;
  class Layer;
  class LayerMaterialProperties;


  /** @class TrackingGeometrySvc
  
      This service builds and registers the TrackingGeometry in the detector store.
      It is called by an ordered callback after GeoModel was created.
  
      @author Andreas.Salzburger@cern.ch */
     
  class ATLAS_NOT_THREAD_SAFE TrackingGeometrySvc :
    public extends <AthService, ITrackingGeometrySvc>, virtual public ITagInfoMgr::Listener
  {
    public:
  
      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;
  
      // TagInfoMgr callback
      virtual void tagInfoUpdated() override final;

      /** Create the geometry */
      StatusCode trackingGeometryInit(bool needsInit = true);
  
      /** Provide the TrackingGeometry */
      virtual const Trk::TrackingGeometry* trackingGeometry() const override;
  
      //!< Returns the name of the TrackingGeometry built with this Svc
      virtual const std::string& trackingGeometryName() const override;
  
      friend class SvcFactory<TrackingGeometrySvc>;
  
      //!< Standard Constructor
      using base_class::base_class;
  

    private:
      void trackingGeometryNotSet() const;

      ToolHandle<Trk::IGeometryBuilder>           m_trackingGeometryBuilder {this, "GeometryBuilder", ""};
      //!< the actual building tool
      mutable const Trk::TrackingGeometry*        m_trackingGeometry     {nullptr};

      //!< the cached TrackingGeometry
      Gaudi::Property<std::string>                m_trackingGeometryName {this, "TrackingGeometryName", "AtlasTrackingGeometry"};
      //!< the name of the TrackingGeometry
      ToolHandleArray<Trk::IGeometryProcessor>    m_geometryProcessors {this, "GeometryProcessors", {}, "Tools to process geometry"};
      //!< processors to help 
      
  #ifdef TRKDETDESCR_MEMUSAGE
      Trk::MemoryLogger                           m_memoryLogger;
      float                                       m_changeVsize {0.0};
      float                                       m_changeRss   {0.0};
  #endif
      
      Gaudi::Property<bool>                       m_rerunOnCallback {this, "RerunOnCallback", false};
      //!< enables the callback
      Gaudi::Property<bool>                       m_buildGeometryFromTagInfo {this, "BuildGeometryFromTagInfo", true};
  };
}

inline const Trk::TrackingGeometry* Trk::TrackingGeometrySvc::trackingGeometry() const
{  if (!m_trackingGeometry) { trackingGeometryNotSet(); }
   return m_trackingGeometry; }

inline const std::string& Trk::TrackingGeometrySvc::trackingGeometryName() const
  { return m_trackingGeometryName; }

#endif // TRKDETDESCRINTERFACES_TRACKINGGEOMETRYSERVICE_H

