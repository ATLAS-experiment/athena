/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXEL_RDOVALIDATIONCOMMONS_H
#define ITKPIXEL_RDOVALIDATIONCOMMONS_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandle.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/PhaseIIPixelRawDataContainer.h"
#include "InDetIdentifier/PixelID.h"

// Common parts of the Pixel RDO validation Algs

using TSortedRDOContainer = std::map<Identifier, std::map<Identifier, unsigned int>>;

struct PixelRDOValidationCommons : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual StatusCode initialize() override;

protected:
  // Concrete algorithms implements these to feed the containers of RDOs
  virtual StatusCode makeSortedReferenceRDOs(const EventContext & ctx,
    TSortedRDOContainer & reference_rdos, size_t & nb_reference_rdos) const = 0;
  virtual StatusCode makeSortedMonitoredRDOs(const EventContext & ctx,
    TSortedRDOContainer & monitored_rdos, size_t & nb_monitored_rdos) const = 0;

  virtual StatusCode execute(const EventContext& ctx) const override;

  // Concrete algorithms can cell these two common cases to handle Phase-I and
  // Phase-II RDOs
  // Create a sorted RDOs container, legacy flavour
  StatusCode makeSortedRDOs(
    SG::ReadHandle<PixelRDO_Container> & sg_rdos_handle,
    TSortedRDOContainer & sorted_rdos, size_t & nb_sorted_rdos) const;
  // Create a sorted RDOs container, Phase-II flavour
  StatusCode makeSortedPhaseIIRDOs(
    SG::ReadHandle<PhaseIIPixelRawDataContainer> & sg_rdos_handle,
    TSortedRDOContainer & sorted_rdos, size_t & nb_sorted_rdos) const;

  // To make RDO IDs from constituents for the Phase-II flavour
  const PixelID* m_idHelper{nullptr};
};

#endif // ITKPIXEL_RDOVALIDATIONCOMMONS_H