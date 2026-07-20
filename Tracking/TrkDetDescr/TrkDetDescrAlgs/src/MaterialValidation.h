/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// MaterialValidation.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRALGS_MATERIALVALIDATION_H
#define TRKDETDESCRALGS_MATERIALVALIDATION_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/ToolHandle.h"
//Eigen
#include "GeoPrimitives/GeoPrimitives.h"
// TrkGeometry
#include "TrkGeometry/TrackingGeometry.h"

#include "TrkDetDescrInterfaces/IMaterialMapper.h"


namespace Trk {

    class TrackingVolume;
    class Surface;

    typedef std::pair<Amg::Vector3D, const Trk::TrackingVolume*> PositionAtBoundary;

    /** @class MaterialValidation

      @author Andreas.Salzburger@cern.ch      
     */


    // exit the volume
    struct VolumeExit {
        // the triple struct
        const Trk::TrackingVolume* nVolume;
        const Trk::Surface*        bSurface;
        Amg::Vector3D              vExit;
        
        VolumeExit( const Trk::TrackingVolume* itv = 0,
                    const Trk::Surface* ibs = 0,
                    Amg::Vector3D iexit = Amg::Vector3D(0.,0.,0.)) :
          nVolume(itv),
          bSurface(ibs),
          vExit(iexit){}
    
    };

    class MaterialValidation : public AthAlgorithm {

      public:

        /** Standard Athena-Algorithm Constructor */
        using AthAlgorithm::AthAlgorithm;

        /** Destructor */
        ~MaterialValidation();

        /** standard Athena-Algorithm method */
        StatusCode          initialize();
        
        /** standard Athena-Algorithm method */
        StatusCode          execute(const EventContext& ctx);
        
        /** standard Athena-Algorithm method */
        StatusCode          finalize();

      private:
        PositionAtBoundary collectMaterialAndExit(const Trk::TrackingVolume& tvol, 
                                                  const Amg::Vector3D& position, 
                                                  const Amg::Vector3D& direction);
          
        const TrackingGeometry& trackingGeometry() const;

        void throwFailedToGetTrackingGeometry() const;
        const TrackingGeometry* retrieveTrackingGeometry(
          const EventContext& ctx) const
        {
          SG::ReadCondHandle<TrackingGeometry> handle(m_trackingGeometryReadKey,
                                                      ctx);
          if (!handle.isValid()) {
            ATH_MSG_FATAL("Could not load TrackingGeometry with name '"
                          << m_trackingGeometryReadKey.key() << "'. Aborting.");
            throwFailedToGetTrackingGeometry();
          }
          return handle.cptr();
        }

        SG::ReadCondHandleKey<TrackingGeometry>   m_trackingGeometryReadKey
           {this, "TrackingGeometryReadKey", "", "Key of the TrackingGeometry conditions data."};
        
      /** Mapper and Inspector */
      ToolHandle<IMaterialMapper> m_materialMapper
	{this, "MaterialMapper", "Trk::MaterialMapper/MappingMaterialMapper"};

      Rndm::Numbers* m_flatDist = nullptr;  //!< Random generator for flat distribution
      Gaudi::Property<double> m_etaMin{this, "MinEta", -3.}; //!< eta boundary
      Gaudi::Property<double> m_etaMax{this, "MaxEta", 3.}; //!< eta boundary
      Gaudi::Property<bool> m_runNativeNavigation{this, "NativeNavigation", true}; //!< validate the native TG navigation

      double m_accTinX0 = 0.; //!< accumulated t in X0

    };
    
    inline const Trk::TrackingGeometry& Trk::MaterialValidation::trackingGeometry() const {
       const Trk::TrackingGeometry *tracking_geometry = retrieveTrackingGeometry(Gaudi::Hive::currentContext());
       if (!tracking_geometry){
          ATH_MSG_FATAL("Did not get valid TrackingGeometry. Aborting." );
          throw GaudiException("MaterialValidation", "Problem with TrackingGeometry loading.", StatusCode::FAILURE);
       }
       return *tracking_geometry;
    }
}

#endif 
