/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_RDOTOTRACCCCELLCONVERTERALG_H
#define ACTSGPUEVENT_RDOTOTRACCCCELLCONVERTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include <InDetRawData/SCT_RDORawData.h>
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"
#include "RDOtoTracccCellConverterCommons.h"
#include <Gaudi/Property.h>

namespace ActsTrk {

/*! Converts Pixel and Strip RDO container into Traccc cells, the input to the
 *  Acts GPU reconstruction chain.
 *
 * The output of this algorithm is a Traccc container on device (GPU).
 *
 * An option allows to choose whether the sorting of the cells is to take place
 * on CPU, as part of this algorithm, or not. If not, the clusterization
 * algorithm on GPU must be configured to sort the input cells.
 *
 * The common components of this algo and PhaseIIRDOtoTracccCellConverterAlg
 * are factorized in RDOtoTracccCellConverterCommons.
 */

class RDOtoTracccCellConverterAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

private:
  RDOtoTracccCellConverterCommons m_common{*this};

  SG::ReadHandleKey<PixelRDO_Container> m_pixelRDOKey{
      this, "PixelRDO", "ITkPixelRDOs"};
  SG::ReadHandleKey<SCT_RDO_Container> m_stripRDOKey{
      this, "StripRDO", "ITkStripRDOs"};

    Gaudi::Property<std::string> m_pixelManagerKey{
      this, "PixelManager", "ITkPixel"};
    Gaudi::Property<std::string> m_stripManagerKey{
      this, "StripManager", "ITkStrip"};
  const InDetDD::PixelDetectorManager* m_pixelManager{nullptr};
  const InDetDD::SCT_DetectorManager*  m_stripManager{nullptr};

};

} // namespace ActsTrk

#endif // ACTSGPUEVENT_RDOTOTRACCCCELLCONVERTERALG_H