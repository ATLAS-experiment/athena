/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ILayerArrayCreator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_ILAYERARRAYCREATOR_H
#define TRKDETDESCRINTERFACES_ILAYERARRAYCREATOR_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
// TrkDetDescrUtils - templated class and enum
#include "TrkDetDescrUtils/BinnedArray1D.h"
#include "TrkDetDescrUtils/BinningType.h"
// STL
#include <vector>
#include <memory>

namespace Trk {

  /** forward declarations*/
  class Layer;
  class CylinderLayer;
  class DiscLayer;
  class PlaneLayer;

  /** @typedef LayerArray
      simply for the eye */

  /** @class ILayerArrayCreator
    Interface class ILayerArrayCreators
    It inherits from IAlgTool. The actual implementation of the AlgTool
    can be found in TrkDetDescrTools as the LayerArrayCreator.

    It takes Material Layers and fills the gaps with NavigationLayers that are by
    default material-less.

    @author Andreas.Salzburger@cern.ch
    */
  class ILayerArrayCreator : virtual public IAlgTool {

    public:

    /// Creates the InterfaceID and interfaceID() method
    DeclareInterfaceID(ILayerArrayCreator, 1, 0);

      /**Virtual destructor*/
      virtual ~ILayerArrayCreator() = default;

      /** LayerArrayCreator interface method - for Barrel-like layers */
      virtual std::unique_ptr<Trk::BinnedArray1D<Layer>> cylinderLayerArray(
          const std::vector<CylinderLayer*>& layers,
          double rmin,
          double rmax,
          BinningType btype = arbitrary) const = 0;

      /** LayerArrayCreator interface method - for Endcap-like layers */
      virtual std::unique_ptr<Trk::BinnedArray1D<Layer>> discLayerArray(
          const std::vector<DiscLayer*>& layers,
          double zmin,
          double zmax,
          BinningType btype = arbitrary) const = 0;

      /** LayerArrayCreator interface method - for Planar-like layers */
      virtual std::unique_ptr<Trk::BinnedArray1D<Layer>> planeLayerArray(
          const std::vector<PlaneLayer*>& layers,
          double min, double max,
          BinningType btype = arbitrary,
          Trk::BinningValue bv = Trk::binX) const = 0;

      /** Validation Action:
          Can be implemented optionally, outside access to internal validation steps */
      virtual void validationAction() const {}
  };


} // end of namespace


#endif // TRKDETDESCRINTERFACES_ILAYERARRAYCREATOR_H


