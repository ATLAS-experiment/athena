/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWEVENT_EFLOWCALOOBJECT_H
#define EFLOWEVENT_EFLOWCALOOBJECT_H
/********************************************************************

NAME:     eflowCaloObject.h
PACKAGE:  offline/Reconstruction/eflowRec

AUTHORS:  D. Tovey
CREATED:  18th November, 2001

PURPOSE:  Calorimeter Object data class

********************************************************************/

#include <vector>
#include <memory>
#include "AthLinks/ElementLink.h"

class eflowRecCluster;
class eflowRecTrack;
class eflowTrackClusterLink;
class eflowLayerIntegrator;
class eflowEEtaBinnedParameters;
class PFEnergyPredictorTool;

/**
An internal EDM object which stores information about systems of associated tracks and calorimeter clusters. Specifically it stores vectors of pointers to eflowRecTracks, eflowRecClusters and eflowTrackClusterLinks. In addition it stores links to an xAOD::CaloClusterContainer and its associated aux container. This class also calculates the expected energy deposit in the calorimeter from a track in the system, and stores that information so that clients can retrieve it. It also calculates the calorimeter cell ordering to be used in the subtraction. Both of these things are done in the simulateShower method which uses the data stored in an eflowEEtaBinnedParameters object, which is filled by e.g the eflowCellEOverP_mc12_JetETMiss tool.
*/
class eflowCaloObject {
public:

  eflowCaloObject()=default;
  ~eflowCaloObject();
  
  void addTrack(eflowRecTrack* track) { m_eflowRecTracks.push_back(track); }
  void addCluster(eflowRecCluster* cluster) { m_eflowRecClusters.push_back(cluster); }
  void addTrackClusterLinks(const std::vector<eflowTrackClusterLink*> &trackClusterLink);
  void addTracks(const std::vector<eflowRecTrack*> &tracks);
  void addClusters(const std::vector<eflowRecCluster*> &clusters);

  /* For a specific eflowTrackClusterLink indicate whether or not it has been fully/partially subtracted by setting the energy ratio
  ** of subtracted cluster energy to original cluster enegry (first float). A value other than nan indicates it has been fully or partially subtracted. 
  ** The second float is the actual subtracted energy at the EM scale  */
  void setTrackClusterLinkSubtractionStatus(unsigned int index, std::pair<float,float> energyRatio_energyValPair) { m_trackClusterLinks[index].second = energyRatio_energyValPair; }
  
  /* Track accessor methods */
  const eflowRecTrack* efRecTrack(int i) const { return m_eflowRecTracks[i]; }
  eflowRecTrack* efRecTrack(int i)  { return m_eflowRecTracks[i]; }
  unsigned nTracks() const{ return m_eflowRecTracks.size(); }
  void clearTracks() { m_eflowRecTracks.clear(); }

  /* Cluster accessor methods */
  const eflowRecCluster* efRecCluster(int i) const { return m_eflowRecClusters[i]; }
  eflowRecCluster* efRecCluster(int i)  { return m_eflowRecClusters[i]; }
  unsigned nClusters() const{ return m_eflowRecClusters.size(); }
  void clearClusters() { m_eflowRecClusters.clear(); }


  /* Link accessor methods */
  std::vector<std::pair<eflowTrackClusterLink*,std::pair<float,float> > > efRecLink() const { return m_trackClusterLinks; }
  void clearLinks() { m_trackClusterLinks.clear(); }

  /* Calculate total tracks energy, total tracks energy variance, total cluster energy for subtraction */
  double getExpectedEnergy() const;
  double getExpectedVariance() const;
  double getClusterEnergy() const ;

  void simulateShower(eflowLayerIntegrator *integrator, const eflowEEtaBinnedParameters* binnedParameters, bool useUpdated2015ChargedShowerSubtraction,
  const PFEnergyPredictorTool* energyP, bool useLegacyEnergyBinIndexing);

private:

  void addTrackClusterLink(eflowTrackClusterLink* trackClusterLink) { m_trackClusterLinks.push_back(std::pair(trackClusterLink,std::pair(NAN,NAN))); }

 private:
  /* Vector of clusters */
  std::vector<eflowRecCluster*> m_eflowRecClusters;

  /* Vector of track-cluster matches - the first float is to be used to indicate the ratio of subtracted cluster energy
  ** to original unsubtracted cluster energy. It is initialiased to nan, and you hence one can use isNan to verify
  ** whether the cluster was subtracted or not (we leave it as nan it not). The second float is the actual amount 
  ** of energy that was subtracted at the EM scale.
  */
  std::vector<std::pair<eflowTrackClusterLink*,std::pair<float,float> > > m_trackClusterLinks;

  /* Vector of tracks */
  std::vector<eflowRecTrack*> m_eflowRecTracks;
  
};

#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"

class eflowCaloObjectContainer : public DataVector< eflowCaloObject >

{

 public:

  void print() { };

};

CLASS_DEF(eflowCaloObjectContainer, 9802, 1)

#endif
