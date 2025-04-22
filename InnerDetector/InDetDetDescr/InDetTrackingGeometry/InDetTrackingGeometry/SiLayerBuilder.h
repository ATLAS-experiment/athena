/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_SILAYERBUILDER_H
#define INDETTRACKINGGEOMETRY_SILAYERBUILDER_H

// Athena
#include "InDetTrackingGeometry/SiLayerBuilderImpl.h"
// Trk
#include "TrkDetDescrInterfaces/ILayerBuilder.h"
#include "TrkDetDescrUtils/BinnedArray.h"
// STL
#include <memory>
#include <vector>

namespace Trk {
  class Surface;
  class CylinderLayer;
  class DiscLayer;
}

namespace InDet {

  /** @class SiLayerBuilder

      The SiLayerBuilder parses the senstive detector elments and orders them onto a
      Layer surface.

      It also uses the SiNumerology to construct the BinUtility and then orders the representing
      detector surfaces on the layers.

      It performs an automated detector if an equidistant or non-equidistant binning
      is to be used for the barrel case.

      @author Andreas.Salzburger@cern.ch
  */
  class ATLAS_NOT_THREAD_SAFE SiLayerBuilder : // static member variables are used.
    public extends<SiLayerBuilderImpl, Trk::ILayerBuilder> {

  public:

    /** AlgTool style constructor */
    SiLayerBuilder(const std::string&,const std::string&,const IInterface*);

    /** Destructor */
    virtual ~SiLayerBuilder()  = default;

    /** AlgTool initialize method */
    virtual StatusCode initialize() override;

    /** LayerBuilder interface method - returning Barrel-like layers */
    virtual std::unique_ptr<const std::vector<Trk::CylinderLayer* > > cylindricalLayers() const override final;

    /** LayerBuilder interface method - returning Endcap-like layers */
    virtual std::unique_ptr<const std::vector<Trk::DiscLayer* > > discLayers() const override final;

    /** LayerBuilder interface method - returning Planar-like layers */
    virtual std::unique_ptr<const std::vector<Trk::PlaneLayer* > > planarLayers() const override final;

    /** Name identification */
    virtual const std::string& identification() const override final;

  private:

    /** create the disc layers, if no vector is given, then it's the first pass, else it's the DBM for the Pixels */
    std::unique_ptr<std::vector< Trk::DiscLayer*> >
      createDiscLayers(std::unique_ptr<std::vector< Trk::DiscLayer*> > discLayers = nullptr) const;

    /** create the disc layers, it is dedicated to ITk implementation of the
     * endcap rings. Used for ITk specific case. */
    std::unique_ptr<std::vector< Trk::DiscLayer*> >
    createRingLayers() const;
  };

  inline std::unique_ptr<const std::vector<Trk::PlaneLayer*> > SiLayerBuilder::planarLayers() const
  { return 0; }

  inline const std::string& SiLayerBuilder::identification() const
  { return m_identification; }

} // end of namespace


#endif // INDETTRACKINGGEOMETRY_SILAYERBUILDER_H
