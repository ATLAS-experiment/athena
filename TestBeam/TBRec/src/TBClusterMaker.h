/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBCLUSTERMAKER_H
#define TBREC_TBCLUSTERMAKER_H
/**
 * @class TBClusterMaker
 * @version \$Id: TBClusterMaker.h,v 1.2 2009-04-17 09:51:30 pavol Exp $
 * @author Andrey Minaenko <minaenko@ihep.ru>
 * @date 01-September-2005
 * @brief simple cone cluster maker for TB data analysis.
 *
 * Make clusters using simple cone algorithm with the sampling dependent 
 * cone size
 * Like all other cluster maker tools this class derives from
 * CaloClusterMakerTool.  */

class CaloDetDescrElement;

#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "CaloGeoHelpers/CaloSampling.h"
#include "CaloConditions/CaloNoise.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <map>
#include <vector>
#include <string>

class TBClusterMaker: public AthAlgTool, virtual public CaloClusterCollectionProcessor
{
 public:    
  
  TBClusterMaker(const std::string& type, const std::string& name,
		       const IInterface* parent);

  using CaloClusterCollectionProcessor::execute;
  virtual StatusCode execute(const EventContext& ctx,
                             xAOD::CaloClusterContainer* theClusters) const override;
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  
 private: 
  
  /**
   * @brief Vectors containing the list of used samplings and 
   * corresponding cone cuts; list of used calorimeters **/
  std::vector<CaloSampling::CaloSample> m_samplings;
  Gaudi::Property<std::vector<float>> m_coneCuts{this, "coneCuts", {}, "Cone cuts for each (!!) sampling used"};
  std::vector<CaloCell_ID::SUBCALO> m_calos;

  /**
   * @brief Map of cone cuts for calorimeter samplings **/
  std::vector<float> m_samplingConeCuts;

  /**
   * @brief Threshold cut on cell energy in sigma noise units **/
  Gaudi::Property<float> m_cellCut{this, "cellCut", -99999., "Cut on a cell energy in sigma noise units"};

  /**
   * @brief Threshold cut on seed cell energy in sigma noise units to find 
   the 1st approximation of cluster \f$ \eta, \phi \f$ **/
  Gaudi::Property<float> m_seedCut{this, "seedCut", 5., "Cut on a seed cell energy in sigma noise units"};

  /**
   * @brief Maximal claster position shift at the current step to stop 
   iterations **/
  Gaudi::Property<float> m_deltaR{this, "deltaR", 0.02, "Maximal claster position shift at the current step to stop iterations"};

  /**
   * @brief Maximal number of iterations to find cluster position **/
  Gaudi::Property<int> m_maxIter{this, "maxIterations", 4, "Maximal number of iterations to find cluster"};

  /**
   * @brief Flag to fix cluster position ( \f$ \eta,\phi \f$) from JO file **/
  Gaudi::Property<bool> m_fixClusterPosition{this, "fixClusterPosition", false, "Flag to fix cluster position (eta,phi) from JO file"};

  /**
   * @brief Cluster \f$ \eta,\phi \f$) set in JO file **/
  Gaudi::Property<float> m_eta0{this, "etaCluster", 99., "Cluster eta from JO file"};
  Gaudi::Property<float> m_phi0{this, "phiCluster", 99., "Cluster phi from JO file"};

  /** True if cell enrgy is in ADC counts, default = FALSE **/
  Gaudi::Property<bool> m_CellEnergyInADC{this, "CellEnergyInADC", false, "True if cell enrgy is in ADC counts"};

  /** Services */
  const CaloCell_ID* m_calo_id{nullptr};

  SG::ReadCondHandleKey<CaloNoise> m_elecNoiseKey
    { this, "ElecNoiseKey", "electronicNoise", "SG key for electronic noise" };
  
  /** Names */
  Gaudi::Property<std::string> m_caloCellContainerName{this, "caloCellContainerName", "AllCalo", "CaloCell Container Name"};
  std::string m_clusterContainerName;
  Gaudi::Property<std::vector<std::string>> m_samplingNames{this, "samplingNames", {}, "Names of used calorimeter samplings"};

  std::map<std::string, CaloSampling::CaloSample> m_samplingFromNameLookup;
  std::map<CaloSampling::CaloSample, CaloCell_ID::SUBCALO> m_caloLookup;
  std::vector<float> m_adcToMeV;

  /** Setup lookup tables */
  StatusCode setupLookupTables();
};

#endif // TBCLUSTERMAKER_H
