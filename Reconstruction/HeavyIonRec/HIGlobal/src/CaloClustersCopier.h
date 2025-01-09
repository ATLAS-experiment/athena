/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef HIGLOBAL_CALOTOWERSCOPIER_H
#define HIGLOBAL_CALOTOWERSCOPIER_H

// Framework includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODHIEvent/HIEventShapeContainer.h"  
#include "AthenaKernel/Units.h"
// STL includes
#include <string>

/**
 * @class CaloClustersCopier
 * @brief This simple algorithm copies CaloClusters to a separate container 
 * but only for events when FCal Et is below configurable threshold
 **/
class CaloClustersCopier : public AthReentrantAlgorithm {
public:
  CaloClustersCopier(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~CaloClustersCopier() override;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& context) const override;
  virtual StatusCode finalize() override;

private:
  Gaudi::Property<float> m_fcalEtCut{this, "FCalEtCut", 60*Athena::Units::GeV, "Copy clusters only if FCal ET energy is below this value"};
  SG::ReadHandleKey<xAOD::HIEventShapeContainer> m_HIEventShapeKey {this, "HIEventShapeKey", "HIEventShape"};
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_inputClustersKey {this, "InputClustersKey", "CaloCalTopoClusters"};
  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outputClustersKey {this, "OutputClustersKey", "PeripheralCaloCalTopoClusters"};

};

#endif // HIGLOBAL_CALOTOWERSCOPIER_H
