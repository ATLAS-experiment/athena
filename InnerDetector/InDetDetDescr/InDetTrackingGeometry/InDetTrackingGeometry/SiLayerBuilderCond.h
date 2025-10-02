/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_SILAYERBUILDERCOND_H
#define INDETTRACKINGGEOMETRY_SILAYERBUILDERCOND_H

// Athena
#include "InDetTrackingGeometry/SiLayerBuilderImpl.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
// Trk
#include "TrkDetDescrInterfaces/ILayerBuilderCond.h"
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkGeometry/TrackingGeometry.h"
// STL
#include <memory>
#include <vector>

namespace Trk {
  class Surface;
  class CylinderLayer;
  class DiscLayer;
}

namespace InDet {

  /** @class SiLayerBuilderCond

      The SiLayerBuilderCond parses the senstive detector elments and orders them onto a
      Layer surface.

      It also uses the SiNumerology to construct the BinUtility and then orders the representing
      detector surfaces on the layers.

      It performs an automated detector if an equidistant or non-equidistant binning
      is to be used for the barrel case.

      @author Andreas.Salzburger@cern.ch
  */
  class ATLAS_NOT_THREAD_SAFE SiLayerBuilderCond : // const_cast
    public extends<SiLayerBuilderImpl, Trk::ILayerBuilderCond> {

  public:

    /** AlgTool style constructor */
    SiLayerBuilderCond(const std::string&,const std::string&,const IInterface*);

    /** Destructor */
    virtual ~SiLayerBuilderCond() = default;

    /** AlgTool initialize method */
    virtual StatusCode initialize() override;

    /** LayerBuilder interface method - returning Barrel-like layers */
    virtual std::unique_ptr<const std::vector<Trk::CylinderLayer*> >
      cylindricalLayers(const EventContext& ctx,
                        SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const override final;

    /** LayerBuilder interface method - returning Endcap-like layers */
    virtual std::unique_ptr<const std::vector<Trk::DiscLayer*> >
      discLayers(const EventContext& ctx,
                 SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const override final;

    /** LayerBuilder interface method - returning Planar-like layers */
    virtual std::unique_ptr<const std::vector<Trk::PlaneLayer*> >
      planarLayers(const EventContext& ctx,
                   SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const override final;

    /** Name identification */
    virtual const std::string& identification() const override final;

  private:

    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection>
      retrieveSiDetElements(const EventContext& ctx) const;

    /** create the disc layers, if no vector is given, then it's the first
     * pass, else it's the DBM for the Pixels */
    std::unique_ptr<std::vector<Trk::DiscLayer*> >
      createDiscLayers(const EventContext& ctx,
                       SG::WriteCondHandle<Trk::TrackingGeometry>& whandle,
                       std::unique_ptr<std::vector<Trk::DiscLayer*> > discLayers = nullptr) const;

    /** create the disc layers, it is dedicated to ITk implementation of the
     * endcap rings. Used for ITk specific case. */
    std::unique_ptr<std::vector<Trk::DiscLayer*> >
      createRingLayers(const EventContext& ctx,
                       SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const;

  };

  inline std::unique_ptr<const std::vector<Trk::PlaneLayer*> >
  SiLayerBuilderCond::planarLayers(const EventContext&,
                                   SG::WriteCondHandle<Trk::TrackingGeometry>& /*whandle*/) const
  {
    return nullptr;
  }

  inline const std::string&
  SiLayerBuilderCond::identification() const
  {
    return m_identification;
  }

} // end of namespace


#endif // INDETTRACKINGGEOMETRY_SILAYERBUILDERCOND_H
