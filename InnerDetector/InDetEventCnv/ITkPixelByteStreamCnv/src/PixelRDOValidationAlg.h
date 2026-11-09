/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXEL_RDOVALIDATIONALG_H
#define ITKPIXEL_RDOVALIDATIONALG_H

#include <PixelRDOValidationCommons.h>
#include "StoreGate/ReadHandleKey.h"
#include "InDetRawData/PixelRDO_Container.h"

/* Algo that compares two Pixel RDO containers and succeeds if and only if the
 * contents match. The input containers are copied and sorted to not take into
 * account the respective orders of collections into the container and
 * of RDOs into the collections. 
 */

class PixelRDOValidationAlg : public PixelRDOValidationCommons
{
public:
  using PixelRDOValidationCommons::PixelRDOValidationCommons;
  virtual StatusCode initialize() override;

private:
  virtual StatusCode makeSortedReferenceRDOs(const EventContext & ctx,
    TSortedRDOContainer & reference_rdos, size_t & nb_reference_rdos)
    const override;
  virtual StatusCode makeSortedMonitoredRDOs(const EventContext & ctx,
    TSortedRDOContainer & monitored_rdos, size_t & nb_monitored_rdos)
    const override;

  SG::ReadHandleKey<PixelRDO_Container> m_referenceRDOKey{this,
    "ReferenceRDOs", "", "The reference RDO container to which the other will be compared"};
  
  SG::ReadHandleKey<PixelRDO_Container> m_monitoredRDOKey{this,
    "MonitoredRDOs", "", "The RDO container to validate"};
};

#endif // ITKPIXEL_RDOVALIDATIONALG_H