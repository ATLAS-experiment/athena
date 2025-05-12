/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_TRT_LAYERBUILDERIMPL_H
#define INDETTRACKINGGEOMETRY_TRT_LAYERBUILDERIMPL_H

// Amg
#include "GeoPrimitives/GeoPrimitives.h"
// Athena
#include "AthenaBaseComps/AthAlgTool.h"
// Gaudi
#include "GaudiKernel/SystemOfUnits.h"
// Trk
#include "TrkGeometry/TrackingGeometry.h"
// InDet
#include "TRT_ReadoutGeometry/TRT_DetElementContainer.h"
// StoreGate
#include "StoreGate/ReadCondHandleKey.h"
// STL
#include <memory>
#include <vector>

#ifndef TRKDETDESCR_TAKESMALLERBIGGER
#define TRKDETDESCR_TAKESMALLERBIGGER
#define takeSmaller(current,test) current = current < test ? current : test
#define takeBigger(current,test)  current = current > test ? current : test
#define takeSmallerBigger(cSmallest, cBiggest, test) takeSmaller(cSmallest, test); takeBigger(cBiggest, test)
#endif

namespace InDetDD {
  class TRT_DetectorManager;
}

namespace Trk {
  class Surface;
  class Layer;
  class CylinderLayer;
  class DiscLayer;
  class ExtendedMaterialProperties;
  typedef std::pair< std::shared_ptr<Surface>, Amg::Vector3D > SurfaceOrderPosition;
}

namespace InDet {

  /** @class TRT_LayerBuilderImpl

     @author Andreas.Salzburger@cern.ch
    */
  class ATLAS_NOT_THREAD_SAFE TRT_LayerBuilderImpl :  //const_cast
    public AthAlgTool {

    /** Declare the TRT_VolumeBuilder as friend */
    friend class TRT_VolumeBuilder;

    public:

      /** Destructor */
      virtual ~TRT_LayerBuilderImpl() = default;

    protected:
      /** AlgTool style constructor */
      TRT_LayerBuilderImpl(const std::string&,const std::string&,const IInterface*);

      std::unique_ptr<const std::vector<Trk::CylinderLayer*> >
        cylindricalLayersImpl(const InDetDD::TRT_DetElementContainer* trtContainer) const;

      std::unique_ptr<const std::vector<Trk::DiscLayer*> >
        discLayersImpl(const InDetDD::TRT_DetElementContainer* trtContainer) const;

      double m_layerStrawRadius{2.0*Gaudi::Units::mm}; //!< straw radius
      DoubleProperty m_layerThickness{this, "LayerThickness", 0.1*Gaudi::Units::mm};           //!< modelled layer thickness
      BooleanProperty m_modelGeometry{this, "ModelLayersOnly", true};            //!< Build the geometry with model layers
      UnsignedIntegerProperty m_modelBarrelLayers{this, "ModelBarrelLayers", 7};        //!< model barrel layers with material
      UnsignedIntegerProperty m_modelEndcapLayers{this, "ModelEndcapLayers", 14};        //!< model endcap layers with material

      UnsignedIntegerProperty m_barrelLayerBinsZ{this, "BarrelLayerBinsZ", 25};         //!< Bins for the Barrel material - in z
      UnsignedIntegerProperty m_barrelLayerBinsPhi{this, "BarrelLayerBinsPhi", 1};       //!< Bins for the Barrel material - in phi
      UnsignedIntegerProperty m_endcapLayerBinsR{this, "EndcapLayerBinsR", 25};         //!< Bins for the Endcap material - in r
      UnsignedIntegerProperty m_endcapLayerBinsPhi{this, "EndcapLayerBinsPhi", 1};       //!< Bins for the Endcap material - in phi
      BooleanProperty m_endcapConly{this, "EndcapConly", false};              //!< Only build the endcapC

      BooleanProperty m_registerStraws{this, "RegisterStraws", false};           //!< register the straws
      IntegerProperty m_barrelSectorAtPiBoundary{this, "BarrelSectorAtPi", 16}; //!< this is the barrel Sector where +pi/-pi is within

      StringProperty m_identification{this, "Identification", "TRT"};           //!< string identification


  };
} // end of namespace


#endif // INDETTRACKINGGEOMETRY_TRT_LAYERBUILDERIMPL_H
