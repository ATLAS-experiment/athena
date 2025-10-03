/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

/**
 * @file
 *
 * Common base class and generic overlaying code for boolean-like hits.
 * Factored out from InDetOverlay.
 *
 * @author Tadej Novak
 * @author Christos Anastopoulos allow DataPool strategy
 * @author Andrei Gaponenko <agaponenko@lbl.gov>, 2006-2009
 */

#ifndef IDC_OVERLAYBASE_H
#define IDC_OVERLAYBASE_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include "AthAllocators/DataPool.h"

class IDC_OverlayBase : public AthReentrantAlgorithm {
 public:
  IDC_OverlayBase(const std::string &name, ISvcLocator *pSvcLocator)
      : AthReentrantAlgorithm(name, pSvcLocator) {}

 protected:
  template <typename IDC_Container>
  StatusCode overlayContainer(const IDC_Container *bkgContainer,
                              const IDC_Container *signalContainer,
                              IDC_Container *outputContainer) const;

  template <typename IDC_Container, typename Type>
  StatusCode overlayContainer(const IDC_Container *bkgContainer,
                              const IDC_Container *signalContainer,
                              IDC_Container *outputContainer,
                              DataPool<Type>& dataItems) const;

  template <typename IDC_Container, typename Type>
  StatusCode overlayContainerWithSorting(const IDC_Container *bkgContainer,
                                         const IDC_Container *signalContainer,
                                         IDC_Container *outputContainer,
                                         DataPool<Type>& dataItems) const;

 private:
  template <bool sortCollections, bool usePool, typename Type, typename IDC_Container>
  StatusCode overlayContainerImpl(const IDC_Container *bkgContainer,
                                  const IDC_Container *signalContainer,
                                  IDC_Container *outputContainer,
                                  DataPool<Type> *dataItems) const;
};

#include "IDC_OverlayBase/IDC_OverlayBase.icc"

#endif
