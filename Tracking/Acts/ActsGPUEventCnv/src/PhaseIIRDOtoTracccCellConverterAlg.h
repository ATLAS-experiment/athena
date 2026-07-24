/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENT_PHASEIIRDOTOTRACCCCELLCONVERTERALG_H
#define ACTSGPUEVENT_PHASEIIRDOTOTRACCCCELLCONVERTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "InDetRawData/PhaseIIPixelRawDataContainer.h"
#include "InDetRawData/PhaseIIStripRawDataContainer.h"
#include "RDOtoTracccCellConverterCommons.h"

namespace ActsTrk {

/*! Converts Pixel and Strip PhaseII RDO container into Traccc cells, the input
 *  to the Acts GPU reconstruction chain.
 *
 * The output of this algorithm is a Traccc container on device (GPU).
 *
 * An option allows to choose whether the sorting of the cells is to take place
 * on CPU, as part of this algorithm, or not. If not, the clusterization
 * algorithm on GPU must be configured to sort the input cells.
 *
 * The common components of this algo and RDOtoTracccCellConverterAlg are
 * factorized in RDOtoTracccCellConverterCommons.
 */

class PhaseIIRDOtoTracccCellConverterAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

private:
  RDOtoTracccCellConverterCommons m_common{*this};

  SG::ReadHandleKey<PhaseIIPixelRawDataContainer> m_ph2PixelRDOKey{
      this, "PixelRDO", "ITkPixelRDOs"};
  SG::ReadHandleKey<PhaseIIStripRawDataContainer> m_ph2StripRDOKey{
      this, "StripRDO", "ITkStripRDOs"};
};

} // namespace ActsTrk

#endif // ACTSGPUEVENT_PHASEIIRDOTOTRACCCCELLCONVERTERALG_H