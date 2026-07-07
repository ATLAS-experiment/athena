/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGLONGLIVEDPARTICLES_MUONCLUSTER_H
#define TRIGLONGLIVEDPARTICLES_MUONCLUSTER_H
/*
  MuonCluster Algorithm:  Trigger/TrigAlgorithm/TrigMuonCluster

  Authors: Stefano.Giagu@cern.ch
           Antonio.Policicchio@cern.ch

  Algorithm for selection of Long-Lived neutral particle decays (Hidden Valley sector):
  Clustering of LVL1 muon RoIs, access to LVL2 jets and SITRACK tracks.
*/


#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODTrigger/TrigCompositeContainer.h"

#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

#include <string>

#define kMAX_ROI 20

class MuonCluster : public AthReentrantAlgorithm {

public:
  /** Constructor */
  MuonCluster(const std::string& name, ISvcLocator* svc);
  ~MuonCluster();

  SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey{ this,
      "RoIs",                                       // property name
      "HLT_muVtxCluster_RoIs",                      // default value of StoreGatekey
      "input RoICollection"};

  SG::WriteHandleKey<xAOD::TrigCompositeContainer> m_outputCompositesKey{ this,
      "TrigRoIs_CompositeContainer",                        // property name
      "HLT_MuRoICluster_Composites",                        // default value of StoreGatekey
      "output Composites container"};

  SG::WriteHandleKey<TrigRoiDescriptorCollection> m_outputRoiDescriptorKey{ this,
      "TrigRoiDescriptorDataVector",
      "HLT_MuRoICluster_Descriptors",
      "output RoI Descriptor container with descriptor for cluster with maximum number of RoIs. For ID."};

  SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_muRoiClusEtaKey{ this,
      "MuonRoiClusterEta",
      "HLT_MuRoICluster_Composites.ClusterEta",
      "Average Eta of the muon RoI Cluster"};

  SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_muRoiClusPhiKey{ this,
      "MuonRoiClusterPhi",
      "HLT_MuRoICluster_Composites.ClusterPhi",
      "Average Phi of the muonRoI Cluster"};

  SG::WriteDecorHandleKey<xAOD::TrigCompositeContainer> m_muRoiClusNRoiKey{ this,
      "MuonRoiClusterNRoIs",
      "HLT_MuRoICluster_Composites.nRoIs",
      "Number of muon RoIs used to construct this muon RoI cluster"};

  /** hltInitialize() */
  virtual StatusCode initialize() override;
  /** hltExecute(), main code of the algorithm */
  virtual StatusCode execute(const EventContext& ctx) const override;


private:

    ToolHandle<GenericMonitoringTool> m_monTool{this,"MonTool","","Monitoring Tool"};

protected:

  typedef struct  {
    float eta{};
    float phi{};
    int nroi{};
  } lvl1_muclu_roi;

  // JobOption properties
  /** A property which specifies the radius of the cluster */
  Gaudi::Property<float> m_DeltaR{this, "DeltaR", 0.4, "radius of the muon cluster"};
  Gaudi::Property<std::string> m_featureLabel{"MuonCluLabel", "MuonClusterInput", "label for the MuonCluster feature in the HLT Navigation, for the xAOD::TrigCompositeContainer"};

  /** calculcate the deltaR between two Rois */
  float DeltaR(const lvl1_muclu_roi& , const lvl1_muclu_roi& ) const;

};

#endif // TRIGLONGLIVEDPARTICLES_MUONCLUSTER_H
