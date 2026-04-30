/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TrigTauCaloRoiUpdater_H
#define  TrigTauCaloRoiUpdater_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODCaloEvent/CaloClusterContainer.h"

#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

/**
 * @class TrigTauCaloRoiUpdater
 * @brief Update the input RoI direction in (eta, phi) to the tau's axis, calculated from the RoI calo clusters
 **/

class TrigTauCaloRoiUpdater : public AthReentrantAlgorithm {
public:
    TrigTauCaloRoiUpdater(const std::string&, ISvcLocator*);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
    Gaudi::Property<float> m_dRForCenter {this, "dRForCenter", 0.2, "Maximum CaloCluster Delta R from the center of RoI"};

    SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roIInputKey {this, "RoIInputKey", "", "RoI input collection key"};
    SG::ReadHandleKey<xAOD::CaloClusterContainer> m_clustersKey {this, "CaloClustersKey", "", "CaloClusters in view key"};
    SG::WriteHandleKey<TrigRoiDescriptorCollection> m_roIOutputKey {this,"RoIOutputKey", "", "Output RoI collection key"};
};

#endif
