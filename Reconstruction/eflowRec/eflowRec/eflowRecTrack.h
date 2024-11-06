/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * eflowRecTrack.h
 *
 *  Created on: 30.09.2013
 *      Author: tlodd
 */

#ifndef EFLOWRECTRACK_H_
#define EFLOWRECTRACK_H_

#include <iostream>
#include <cassert>
#include <map>
#include <string>

#include "CaloEvent/CaloCell.h"
#include "CaloEvent/CaloCellContainer.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ToolHandle.h"

#include "CxxUtils/fpcompare.h"

#include "AthLinks/ElementLink.h"

#include "Particle/TrackParticleContainer.h"

#include "eflowRec/eflowTrackCaloPoints.h"
#include "eflowRec/eflowRingSubtractionManager.h"
#include "eflowRec/PFMatchInterfaces.h"

#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"

class eflowTrackClusterLink;
class eflowTrackExtrapolatorBaseAlgTool;


/**
This class extends the information about a xAOD::Track. It stores an ElementLink to a track, a raw pointer to that track, the expected energy deposit (mean and width of the reference distribution) of the track in the calorimeter, the pull15 variable (used to decide whether to run the charged shower subtraction or not), a bool to tag whether this track is in an energy dense calorimeter environment (if it is, then the charged shower subtraction is not run for that track), a bool to signify if the track has already had its shower removed from the calorimeter clusters and a bool to signify if we found a reference e/p bin (which depends on the track eta,pt and LHED). In addition to these variables we store a pointer to an eflowTrackCaloPoints object, a reference to an eflowRingSubtractionManager object and a vector of pointers to eflowTrackClusterLink. There is also an option to store additional vectors in a map using a string as a key (this allows us to store sets of eflowTrackClusterLink for multiple track-cluster matching schemes).
*/
class eflowRecTrack {
public:
  eflowRecTrack(const ElementLink<xAOD::TrackParticleContainer>& trackElemLink,
                const ToolHandle<eflowTrackExtrapolatorBaseAlgTool>& theTrackExtrapolatorTool);
  eflowRecTrack(const eflowRecTrack& originalEflowRecTrack);
  eflowRecTrack& operator = (const eflowRecTrack& originalEflowRecTrack);
  virtual ~eflowRecTrack();

  const xAOD::TrackParticle* getTrack() const { return m_track; }

  const eflowTrackCaloPoints& getTrackCaloPoints() const { return *m_trackCaloPoints; }

  ElementLink<xAOD::TrackParticleContainer> getTrackElemLink() const { return m_trackElemLink; }
  void addClusterMatch(eflowTrackClusterLink* clusterMatch) { m_clusterMatches.push_back(clusterMatch); }
  void addDeltaRPrime(const float& dRPrime){ m_deltaRPrimes.push_back(dRPrime);}
  void addAlternativeClusterMatch(eflowTrackClusterLink* clusterMatch, const std::string& key) { m_alternativeClusterMatches[key].push_back(clusterMatch); }

  eflowRingSubtractionManager& getCellSubtractionManager() { return m_ringSubtractionManager; }

  int getType() const { return m_type; }

  const std::vector<eflowTrackClusterLink*>& getClusterMatches() const { return m_clusterMatches; }
  const std::vector<float>& getDRPrimes() const { return m_deltaRPrimes;}
  //This must also clear deltaRPrime, which corresponds to dRPrime between the track and each cluster
  void clearClusterMatches() { m_clusterMatches.clear(); m_deltaRPrimes.clear(); }

  const std::vector<eflowTrackClusterLink*>* getAlternativeClusterMatches(const std::string& key) const;// { return m_alternativeClusterMatches.at(key); }

  bool hasBin() const { return m_hasBin; }
  void setHasBin(bool hasBin) { m_hasBin = hasBin; }

  void setCaloDepthArray(const double* depthArray);
  const std::vector<double>& getCaloDepthArray() const { return m_caloDepthArray; }

  bool isSubtracted() const { return m_isSubtracted; }
  void setSubtracted();

  void setEExpect(double eExpect, double varEExpect){ m_eExpect = eExpect; m_varEExpect = varEExpect; }
  double getEExpect() const { return m_eExpect; }
  double getVarEExpect() const { return m_varEExpect; }
  int getTrackId() const { return m_trackId; }
  void setTrackId(int trackId) { m_trackId = trackId; }
  
  
  int getLayerHED() const { return m_layerHED; }
  void setLayerHED(int layerHED) { m_layerHED = layerHED; }
  
  const std::vector<int>& getLayerCellOrderVector() const { return m_layerCellOrderVector; }
  void setLayerCellOrderVector(const std::vector<int>& layerToStoreVector) { m_layerCellOrderVector = layerToStoreVector; }
  
  const std::vector<float>& getRadiusCellOrderVector() const { return m_radiusCellOrderVector; }
  void setRadiusCellOrderVector(const std::vector<float>& radiusToStoreVector) { m_radiusCellOrderVector = radiusToStoreVector; }
  
  const std::vector<float>& getAvgEDensityCellOrderVector() const { return m_avgEdensityCellOrderVector; }
  void setAvgEDensityCellOrderVector(const std::vector<float>& avgEdensityToStoreVector) { m_avgEdensityCellOrderVector = avgEdensityToStoreVector; }
  
  
  bool isInDenseEnvironment() const { return m_isInDenseEnvironment;}
  void setIsInDenseEnvironment() { m_isInDenseEnvironment = true; }

  bool isRecovered() const { return m_isRecovered;}
  void setIsRecovered() { m_isRecovered= true; }
  
  void setpull15(double pull15){ m_pull15 = pull15; }
  double getpull15() const { return m_pull15; }

  void insertTruthEnergyPair (const CaloCell* cell, double truthEnergy);
  double getCellTruthEnergy (const CaloCell* cell) const;

  void addSubtractedCaloCell(ElementLink<CaloCellContainer> theCellLink, const double& weight) { m_subtractedCells.push_back(std::make_pair(theCellLink,weight)); }
  const std::vector<std::pair<ElementLink<CaloCellContainer>, double> >& getSubtractedCaloCells() const { return m_subtractedCells; }

private:

  int m_trackId;
  ElementLink<xAOD::TrackParticleContainer> m_trackElemLink;
  const xAOD::TrackParticle* m_track;
  int m_type;
  double m_pull15;
  
  int m_layerHED{};
  std::vector<int> m_layerCellOrderVector;
  std::vector<float> m_radiusCellOrderVector;
  std::vector<float> m_avgEdensityCellOrderVector;

  double m_eExpect;
  double m_varEExpect;

  bool m_isInDenseEnvironment;
  bool m_isSubtracted;
  /** Tells us whether this track underwent split shower revovery */
  bool m_isRecovered;
  bool m_hasBin;

  std::vector<double> m_caloDepthArray;

  std::unique_ptr<eflowTrackCaloPoints> m_trackCaloPoints;
  eflowRingSubtractionManager m_ringSubtractionManager;
  std::vector<eflowTrackClusterLink*> m_clusterMatches;
  /** List of distance measurements between track and cluster used in first pass matching - i.e dRPrime */
  std::vector<float> m_deltaRPrimes;
  std::map<std::string,std::vector<eflowTrackClusterLink*> > m_alternativeClusterMatches;

  //for truth cheating mode only, we store the list of cells and their truth energies
  std::vector<std::pair<const CaloCell*, double> > m_cellTruthEnergyList;
  std::map<Identifier,double> m_cellTruthEnergyStore;

  //list of cells that were removed from CaloCluster (only used in doCPData mode)
  std::vector<std::pair<ElementLink<CaloCellContainer>, double> > m_subtractedCells;

public:
  class SortDescendingPt {
  public:
    bool operator()(const eflowRecTrack* a, const eflowRecTrack* b) {
      return CxxUtils::fpcompare::greater(a->getTrack()->pt(),
                                          b->getTrack()->pt());
    }
  };
};

/**
This class, which inherits from the pure virtual ITrack,  stores a pointer to an eflowRecTrack and has an interface which returns an eflowEtaPhiPosition (representing the track eta, phi coordinates in the requested calorimeter layer)
*/
class eflowRecMatchTrack: public PFMatch::ITrack {
public:
  eflowRecMatchTrack(const eflowRecTrack* efRecTrack): m_efRecTrack(efRecTrack) { assert(m_efRecTrack); }
  virtual ~eflowRecMatchTrack() { }

  virtual eflowEtaPhiPosition etaPhiInLayer(PFMatch::LayerType layer) const {
    if (m_efRecTrack) return m_efRecTrack->getTrackCaloPoints().getEtaPhiPos(layer);
    else {
      std::cerr << "eflowRecMatchTrack ERROR: Invalid pointer to eflowRecTrack " << std::endl;
      return eflowEtaPhiPosition(-999.,-999.);
    }
  }

private:
  const eflowRecTrack* m_efRecTrack;
};

#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"

class eflowRecTrackContainer : public DataVector< eflowRecTrack >

{

 public:

  void print() { };

};

CLASS_DEF(eflowRecTrackContainer, 9803, 1)
#endif /* EFLOWRECTRACK_H_ */
