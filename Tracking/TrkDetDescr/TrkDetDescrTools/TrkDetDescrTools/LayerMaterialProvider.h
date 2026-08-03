/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// LayerMaterialProvider.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRTOOLS_LAYERMATERIALPROVIDER_H
#define TRKDETDESCRTOOLS_LAYERMATERIALPROVIDER_H

// Trk
#include "TrkDetDescrInterfaces/IGeometryProcessor.h"
#include "TrkGeometry/LayerMaterialMap.h"
// Gaudi & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadCondHandleKey.h"


#include "CxxUtils/checker_macros.h"
namespace Trk {

    class TrackingGeometry;
    class TrackingVolume;    
    class Layer;
    class Surface;
    class LayerMaterialMap;


    /** @class LayerMaterialProvider

      AlgTool that retrieves the LayerMaterial from COOL and 
      loads it onto the geometry

      @author Andreas.Salzburger@cern.ch
     */

    class LayerMaterialProvider //process methods call non-safe code
      :  public extends<AthAlgTool, IGeometryProcessor> {
     
      public:
        /** Constructor */
        using base_class::base_class;

        virtual StatusCode initialize() override;

        /** Processor Action to work on TrackingGeometry& tgeo */
        virtual StatusCode process(TrackingGeometry& tgeo) const override;

        /** Processor Action to work on TrackingVolumes - the level is for the
         * hierachy tree*/
        virtual StatusCode process(TrackingVolume& tvol,
                                   size_t level = 0) const override;

        StatusCode process(TrackingVolume& lay,
                           const LayerMaterialMap& layerMaterialMap,
                           size_t level) const;

        /** Processor Action to work on Layers */
        virtual StatusCode process(Layer& lay, size_t level = 0) const override;

        StatusCode process(Layer& lay,
                           const LayerMaterialMap& layerMaterialMap,
                           size_t level) const;

        /** Processor Action to work on Surfaces */
        virtual StatusCode process(Surface& surf, size_t level = 0) const override;

      private:
          
        void dumpMaterialMap (const LayerMaterialMap& layerMaterialMap) const;

        SG::ReadCondHandleKey<LayerMaterialMap> m_layerMaterialMapKey
          { this, "LayerMaterialMapKey", "/GLOBAL/TrackingGeo/LayerMaterialV2",
            "COOL folder for material map" };

        Gaudi::Property<std::string> m_layerMaterialMapName
	  {this, "LayerMaterialMapName", "",
	   "If LayerMaterialMapKey is not set, then fall back to retrieving this from the detector store."};
    };

} // end of namespace

#endif // TRKDETDESCRTOOLS_LAYERMATERIALPROVIDER_H

