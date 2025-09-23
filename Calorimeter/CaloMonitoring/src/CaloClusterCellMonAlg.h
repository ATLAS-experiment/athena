/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#ifndef CALOMONITORING_LARCLUSTERCELLMONALG_H
#define CALOMONITORING_LARCLUSTERCELLMONALG_H

#include "CaloMonAlgBase.h"

#include "CaloDetDescr/CaloDetDescrManager.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArIdentifier/LArOnlineID.h"
          
#include <vector>
#include <string>
#include <array>
#include <map>
#include <limits>

class CaloClusterCellMonAlg : public CaloMonAlgBase {
 
 public:
  using CaloMonAlgBase::CaloMonAlgBase;
  ~CaloClusterCellMonAlg()=default;
  
  virtual StatusCode initialize()  override final;
  virtual StatusCode fillHistograms(const EventContext& ctx) const override final;
  

private:

  // Job properties
  SG::ReadHandleKey<xAOD::CaloClusterContainer>  m_clusterContainerKey{this,"CaloClusterContainer","CaloTopoClusters","SG key of the input cluster container"};

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey {this,"CaloDetDescrManager", "CaloDetDescrManager", "SG Key for CaloDetDescrManager in the Condition Store" };
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};

  Gaudi::Property<std::string> m_MonGroupName  {this, "MonGroupName", "CaloClusterCellMonGroup"};

 
  StringArrayProperty m_layerNames{this, "LayerNames", {"EMBPA", "EMBPC", "EMB1A", "EMB1C", "EMB2A", "EMB2C", "EMB3A", "EMB3C",
				   	    "HEC0A", "HEC0C", "HEC1A", "HEC1C", "HEC2A", "HEC2C", "HEC3A", "HEC3C",
					    "EMECPA", "EMECPC", "EMEC1A", "EMEC1C", "EMEC2A", "EMEC2C", "EMEC3A", "EMEC3C", 
					    "FCAL1A", "FCAL1C", "FCAL2A", "FCAL2C", "FCAL3A", "FCAL3C"},
                                                       "Names of individual layers to monitor"};

  //StringArrayProperty m_partitionNames{this, "PartitionNames", {"EMBA","EMBC","EMECA","EMECC","HECA","HECC","FCALA","FCALC"}};  

  // Trigger Awareness:
  enum TriggerType{RNDM,CALO,MINBIAS,MET,MISC,NOTA,MAXTRIGTYPE};

  BooleanProperty m_useTrigger{this, "useTrigger", true};
  std::array<StringProperty,NOTA> m_triggerNames{{{this,"rndmTriggerNames",""},
                                                 {this,"caloTriggerNames",""},
						 {this,"minBiasTriggerNames",""},
						 {this,"metTriggerNames",""},
						 {this,"miscTriggerNames",""}
    }};


  BooleanArrayProperty m_doBeamBackgroundRemovalProp{this, "DoBeamBackgroundRemoval"}; 
 
  Gaudi::Property<float> m_clusterECut {this,"ClusterEnergyCut",std::numeric_limits<float>::lowest()};
  Gaudi::Property<unsigned> m_nCellsPerCluster{this,"nCellsPerCluster",1,"Monitor the N highest-energy-cells of each cluster (0: all cells)"};
    
  //Enumerate partitions
  enum PartitionEnum{EMBA,EMBC,EMECA,EMECC,HECA,HECC,FCALA,FCALC,MAXPARTITIONS};

  void checkTrigger() const;

  const LArOnlineID* m_onlineID{nullptr};  

  const std::array<std::string,9> m_tileNames{
        "TileBar0","TileBar1","TileBar2",
        "TileGap0","TileGap1","TileGap2",
        "TileExt0","TileExt1","TileExt2"};
};

#endif 
