/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_SILAYERBUILDERIMPL_H
#define INDETTRACKINGGEOMETRY_SILAYERBUILDERIMPL_H

// Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
// Amg
#include "GeoPrimitives/GeoPrimitives.h"
// Trk
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkGeometry/TrackingGeometry.h"
// STL
#include <memory>
#include <vector>
#include <utility> //for std::pair

class PixelID;
class SCT_ID;

#ifndef TRKDETDESCR_TAKESMALLERBIGGER
#define TRKDETDESCR_TAKESMALLERBIGGER
#define takeSmaller(current,test) current = current < test ? current : test
#define takeBigger(current,test)  current = current > test ? current : test
#define takeSmallerBigger(cSmallest, cBiggest, test) takeSmaller(cSmallest, test); takeBigger(cBiggest, test)
#endif

namespace InDetDD {
  class SiDetectorManager;
}

namespace Trk {
  class Surface;
  class CylinderLayer;
  class DiscLayer;
  class BinnedLayerMaterial;
  typedef std::pair< std::shared_ptr<Surface>, Amg::Vector3D > SurfaceOrderPosition;
}

namespace InDet {

  /** @class SiLayerBuilderImpl

      The SiLayerBuilderImpl parses the senstive detector elments and orders them onto a
      Layer surface.

      It also uses the SiNumerology to construct the BinUtility and then orders the representing
      detector surfaces on the layers.

      It performs an automated detector if an equidistant or non-equidistant binning
      is to be used for the barrel case.

      @author Andreas.Salzburger@cern.ch
  */
  class ATLAS_NOT_THREAD_SAFE SiLayerBuilderImpl : public AthAlgTool {

  public:

    /** Destructor */
    virtual ~SiLayerBuilderImpl() = default;

    /** AlgTool initialize method */
    virtual StatusCode initialize() override;
    /** AlgTool finalize method */
    virtual StatusCode finalize() override;

  protected:
    /** AlgTool style constructor */
    SiLayerBuilderImpl(const std::string&,const std::string&,const IInterface*);

    void registerSurfacesToLayer(Trk::BinnedArraySpan<Trk::Surface * const >& layerSurfaces, Trk::Layer& lay) const;

    std::unique_ptr<const std::vector<Trk::CylinderLayer*> >
      cylindricalLayersImpl(const InDetDD::SiDetectorElementCollection& siDetElementCollection) const;

    std::unique_ptr<std::vector< Trk::DiscLayer*> >
      createDiscLayersImpl(const InDetDD::SiDetectorElementCollection& siDetElementCollection, std::unique_ptr<std::vector< Trk::DiscLayer*> > discLayers = nullptr) const;

    std::unique_ptr<std::vector< Trk::DiscLayer*> >
      createRingLayersImpl(const InDetDD::SiDetectorElementCollection& siDetElementCollection) const;

    std::unique_ptr<std::vector<Trk::CylinderLayer*> >
      dressCylinderLayers(const std::vector<Trk::CylinderLayer* >& dLayers) const;

    //!< helper method to construct barrel material
    const Trk::BinnedLayerMaterial barrelLayerMaterial(double r,
                                                       double hz) const;

    //!< helper method to construct endcap material
    const Trk::BinnedLayerMaterial endcapLayerMaterial(double rMin,
                                                       double rMax) const;

    /// Common properties
    BooleanProperty m_pixelCase{this, "PixelCase", true}; //!< flag for pixel/sct
    const InDetDD::SiDetectorManager *m_siMgr{}; //!< the Si Detector Manager
    StringProperty m_siMgrLocation{this, "SiDetManagerLocation", "Pixel"}; //!< the location of the Pixel Manager
    const PixelID *m_pixIdHelper{}; //!< pixel Id Helper
    const SCT_ID *m_sctIdHelper{}; //!< sct Id Helper

    BooleanProperty m_setLayerAssociation{this, "SetLayerAssociation", true}; //!< Set Layer Association

    // For the Active Barrel Material
    // barrel layer section
    DoubleArrayProperty m_barrelAdditionalLayerR{this, "BarrelAdditionalLayerRadii", {} };         //!< Create an additional layer at these radii
    IntegerArrayProperty m_barrelAdditionalLayerType{this, "BarrelAdditionalLayerType", {} };      //!< material layer 1 - navigation layer 0
    UnsignedIntegerProperty m_barrelLayerBinsZ{this, "BarrelLayerBinsZ", 100};               //!< Barrel bins for the material in z
    UnsignedIntegerProperty m_barrelLayerBinsPhi{this, "BarrelLayerBinsPhi", 1};             //!< Barrel bins for the material in phi
    DoubleProperty m_barrelEnvelope{this, "BarrelEnvelope", 0.1};                 //!< envelope around rMin/rMax
    DoubleProperty m_barrelEdbTolerance{this, "BarrelEdbTolerance", 0.05};             //!< tolerance in percent how much the bin sizes can change

    // For the Active Endcap Material
    DoubleArrayProperty m_endcapAdditionalLayerPosZ{this, "EndcapAdditionalLayerPositionsZ", {} };      //!< Create additional endcaps at these z positions
    IntegerArrayProperty m_endcapAdditionalLayerType{this,"EndcapAdditionalLayerType" , {} };      //!< material layer 1 - navigation layer 0 ( for volume adjustment )
    UnsignedIntegerProperty m_endcapLayerBinsR{this, "EndcapLayerBinsR", 100};               //!< Barrel bins for the material in r
    UnsignedIntegerProperty m_endcapLayerBinsPhi{this, "EndcapLayerBinsPhi", 1};             //!< Barrel bins for the material in phi
    DoubleProperty m_endcapEnvelope{this, "EndcapEnvelope", 0.1};                 //!< envelope around rMin/rMax
    BooleanProperty m_endcapComplexRingBinning{this, "EndcapComplexRingBinning", true};       //!< make std::vector<R> rings, could be different for layers

    StringProperty m_identification{this, "Identification", "Pixel"};                  //!< string identification

    BooleanProperty m_runGeometryValidation{this, "GeometryValidation", true};           //!< run the validation of the geometry ( no empty bins)

    IntegerArrayProperty m_layerIndicesBarrel{this, "LayerIndicesBarrel", {} }; //!< indices to be used for layer creation (used for ITk specific case)
    IntegerArrayProperty m_layerIndicesEndcap{this, "LayerIndicesEndcap", {} }; //!< indices to be used for layer creation (used for ITk specific case)
    BooleanProperty m_useRingLayout{this, "UseRingLayout", false}; //!< to enable creation of rings for ITk pixel geometry (used for ITk specific case)
    BooleanProperty m_addMoreSurfaces{this, "AddMoreSurfaces", false}; //!< to add additional surfaces to the PixelOverlapDescriptor, SCT_OverlapDescriptor and DiscOverlapDescriptor (used for ITk specific case)

    // Properties only in SiLayerBuilderCond
    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCT_ReadKey{this, "SCT_ReadKey", "SCT_DetectorElementCollection", "Key of output SiDetectorElementCollection for SCT"};
    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_PixelReadKey{this, "PixelReadKey", "PixelDetectorElementCollection", "Key of output SiDetectorElementCollection for Pixel"};



  };

} // end of namespace


#endif // INDETTRACKINGGEOMETRY_SILAYERBUILDERIMPL_H
