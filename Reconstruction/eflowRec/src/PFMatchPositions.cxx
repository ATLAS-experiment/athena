/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
 * PFMatchPositions.cxx
 *
 *  Created on: 25.03.2014
 *      Author: tlodd
 */

#include "PFMatchPositions.h"
#include "PFClusterWidthCalculator.h"

namespace PFMatch {

/* Track position providers */

EtaPhi TrackEtaPhiInFixedLayersProvider::getPosition(ITrack* track) const {
  eflowEtaPhiPosition etaphi = track->etaPhiInLayer(m_barrelLayer);
  if (etaphi.getEta() == -999.){
    etaphi = track->etaPhiInLayer(m_endcapLayer);
  }
  if (etaphi.getEta() == -999.){
    etaphi = track->etaPhiInLayer(m_fcalLayer);
  }
  return etaphi;
}


/* Cluster position providers */

EtaPhi ClusterPlainEtaPhiProvider::getPosition(ICluster* cluster) const {
  eflowEtaPhiPosition etaphi(cluster->eta(), cluster->phi());
  return etaphi;
}

const double ClusterGeometricalCenterProvider::m_etaPhiLowerLimit(0.0025);

EtaPhiWithVariance ClusterGeometricalCenterProvider::getPosition(ICluster* cluster) const {

  /* Check the status to make sure this function only execute once since it is expensive. */
  if(cluster->calVarianceStatus()) {
    return {eflowEtaPhiPosition(cluster->etaMean(), cluster->phiMean()), cluster->etaVariance(), cluster->phiVariance()};
  }
  cluster->setCalVarianceStatus();

  
  unsigned int nCells = cluster->nCells();

  PFClusterWidthCalculator widthCalc;
  PFClusterWidth width = widthCalc.getPFClusterCoordinateWidth(cluster->cellEta(),cluster->cellPhi(),cluster->eta(),cluster->phi(),nCells);

  if (nCells > 1){
    cluster->etaMean(width.etaMean);
    cluster->phiMean(width.phiMean);
  }
  cluster->etaVariance(width.etaVariance);
  cluster->phiVariance(width.phiVariance);
  
  return {eflowEtaPhiPosition(width.etaMean,width.phiMean), width.etaVariance, width.phiVariance};



}

}
