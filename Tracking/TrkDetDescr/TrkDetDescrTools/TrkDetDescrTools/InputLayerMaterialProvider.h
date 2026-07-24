/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// InputLayerMaterialProvider.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRTOOLS_INPUTLAYERMATERIALPROVIDER_H
#define TRKDETDESCRTOOLS_INPUTLAYERMATERIALPROVIDER_H

// Trk
#include "TrkDetDescrInterfaces/IGeometryProcessor.h"
#include "TrkGeometry/MaterialProperties.h"
// Gaudi & Athena
#include "AthenaBaseComps/AthAlgTool.h"

#include "CxxUtils/checker_macros.h"
namespace Trk {

    class TrackingGeometry;
    class TrackingVolume;    
    class Layer;
    class Surface;
    class LayerMaterialMap;


    /** @class InputLayerMaterialProvider

      AlgTool that assigns LayerMaterial as specified to the TrackingGeometry

      @author Andreas.Salzburger@cern.ch
     */

    class InputLayerMaterialProvider :  public AthAlgTool, virtual public IGeometryProcessor {
     
      public:
        /** Constructor */
        InputLayerMaterialProvider(const std::string&,const std::string&,const IInterface*);

        /** Framework initialize() */
        virtual StatusCode initialize();

        /** Processor Action to work on TrackingGeometry& tgeo */
        virtual StatusCode process(TrackingGeometry& tgeo) const;

        /** Processor Action to work on TrackingVolumes - the level is for the
         * hierachy tree*/
        virtual StatusCode process(TrackingVolume& tvol,
                                   size_t level = 0) const;

        /** Processor Action to work on Layers */
        virtual StatusCode process(Layer& lay, size_t level = 0) const;

        /** Processor Action to work on Surfaces */
        virtual StatusCode process(Surface& surf, size_t level = 0) const;

    private:
      Gaudi::Property<bool> m_constantMaterialToAllLayers
        {this, "AssignConstantMaterial", true,
	 "just assign a dummy material to all layers"};
      Gaudi::Property<float> m_constantThicknessInX0
	{this, "ConstantMaterialInX0", 0.02};
      Gaudi::Property<float> m_constantThicknessInL0
	{this, "ConstantMaterialInL0", 0.06};
      Gaudi::Property<float> m_constantAverageA
	{this, "ConstantMaterialA", 14.};
      Gaudi::Property<float> m_constantAverageZ
	{this, "ConstantMaterialZ", 7.};
      Gaudi::Property<float> m_constantAverageRho
	{this, "ConstantMaterialRho", 0.00233};
      MaterialProperties m_constantMaterialProperties{};   //!< the set together material

                        
        
    };

} // end of namespace

#endif // TRKDETDESCRTOOLS_INPUTLAYERMATERIALPROVIDER_H

