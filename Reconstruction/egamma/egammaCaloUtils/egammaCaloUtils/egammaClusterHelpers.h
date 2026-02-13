/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMACLUSTERHELPERS_H
#define EGAMMACLUSTERHELPERS_H

#include "xAODCaloEvent/CaloCluster.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloGeoHelpers/CaloSampling.h"

namespace egammaClusterHelpers
{
  /**
   * Duplicate code
   * @brief Return eta/phi ranges encompassing +- 1 cell.
   * @param eta Central eta value.
   * @param phi Central phi value.
   * @param sampling The sampling to use.
   * @param[out] deta Range in eta.
   * @param[out] dphi Range in phi.
   *
   * This can be a little tricky due to misalignments and the fact
   * that cells have different sizes in different regions.  Also,
   * CaloLayerCalculator takes only a symmetric eta range.
   * We try to find the neighboring cells by starting from the center
   * cell and looking a little bit more than half its width in either
   * direction, and finding the centers of those cells.  Then we use
   * the larger of these for the symmetric range.
   */
  std::pair<const double, const double>
    etaphi_range(const CaloDetDescrManager& mgr,
		 double eta,
		 double phi,
		 CaloCell_ID::CaloSample sampling,
		 const CaloDetDescrElement* elt);
  
  /** Function to decorate the calo cluster with position variables.
   * Filling eta phi in calo-frame:
   * - xAOD::CaloCluster::ETACALOFRAME
   * - xAOD::CaloCluster::PHICALOFRAME
   * - xAOD::CaloCluster::ETA2CALOFRAME
   * - xAOD::CaloCluster::PHI2CALOFRAME
   * - xAOD::CaloCluster::ETA1CALOFRAME
   * - xAOD::CaloCluster::PHI1CALOFRAME
   */
  void
    fillPositionsInCalo(xAOD::CaloCluster* cluster, const CaloDetDescrManager& mgr);

  /** functions to make 1st sampling (strips) specific corrections*/
  void
    makeCorrection1(xAOD::CaloCluster* cluster,
		    const CaloDetDescrManager& mgr,
		    const CaloSampling::CaloSample sample);

  /** function to refine position in eta1*/
  void
    refineEta1Position(xAOD::CaloCluster* cluster, const CaloDetDescrManager& mgr);
}

#endif
