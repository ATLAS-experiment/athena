// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef LARCLUSTERCELLDUMPER_EVENTREADERALG_H
#define LARCLUSTERCELLDUMPER_EVENTREADERALG_H

#include "LArClusterCellDumper/EventReaderBaseAlg.h"

#include "AthContainers/ConstDataVector.h"

// Calibration constants
#include "AthenaPoolUtilities/AthenaAttributeList.h" // 
#include "CaloCondBlobObjs/CaloCondBlobFlt.h"
#include "CoralBase/Blob.h"
#include "LArCOOLConditions/LArDSPThresholdsFlat.h"
#include "LArRawConditions/LArADC2MeV.h"
#include "LArElecCalib/ILArPedestal.h"
#include "LArElecCalib/ILArOFC.h"
#include "LArElecCalib/ILArMinBiasAverage.h"
#include "LArElecCalib/ILArfSampl.h" 
#include "LArElecCalib/ILArShape.h" 
#include "LumiBlockData/LuminosityCondData.h"
#include "CaloConditions/CaloNoise.h"
#include "LArRecConditions/LArHVCorr.h"

#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTracking/VertexContainer.h"

#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "LArSimEvent/LArHitContainer.h"

#include "LArIdentifier/LArOnlineID.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloIdManager.h"


class ATLAS_NOT_THREAD_SAFE EventReaderAlg: public EventReaderBaseAlg
{ 
 public: 
    EventReaderAlg( const std::string& name, ISvcLocator* pSvcLocator );

    virtual ~EventReaderAlg() = default;

    virtual StatusCode        initialize() override;
    virtual StatusCode        execute() override;
    virtual StatusCode        finalize() override;
    
  private:
    virtual StatusCode        dumpEventInfo(SG::ReadHandle<xAOD::EventInfo> &ei);
    virtual StatusCode        dumpLumiblockInfo(SG::ReadHandle<xAOD::EventInfo> &ei);
    virtual StatusCode        dumpClusterCells(const xAOD::CaloCluster *cl, int clusIndex, const EventContext& ctx); // dump the cluster and its cells with calib.
    virtual StatusCode        dumpOfflineSS(const xAOD::Electron *electron);
    virtual StatusCode        dumpTruthParticle(SG::ReadHandle<xAOD::ElectronContainer> &electronSelectionCnt, SG::ReadHandle<xAOD::TruthParticleContainer> &truthParticleCnt);
    virtual StatusCode        dumpElectrons(const xAOD::Electron* electron);
    virtual StatusCode        dumpZeeCut(SG::ReadHandle<xAOD::EventInfo> &ei, SG::ReadHandle<xAOD::VertexContainer> &primVertexCnt, const EventContext& ctx);
    virtual StatusCode        dumpPrimVertexAssocToElectron(const xAOD::Electron *el, SG::ReadHandle<xAOD::VertexContainer> &primVertexCnt, SG::ReadHandle<xAOD::EventInfo> & evtInfo); // follow the same rules determined by 'trackSelectionElectrons' routine.
    virtual StatusCode        FillNTupleWithSelectedElectrons(SG::ReadHandle<xAOD::EventInfo> &ei, SG::ReadHandle<xAOD::VertexContainer> &primVertexCnt, SG::ReadHandle<xAOD::ElectronContainer> &electronSelectionCnt, std::string& eSelectionText, const EventContext& ctx);

    Gaudi::Property<bool> m_printCellsClus {this, "printCellsClus", false, "Print out the cluster cells basic info during the dump."}; 
    Gaudi::Property<bool> m_doClusterDump {this, "doClusterDump", false, "Perform a cluster cell dump based on Cluster Container name m_clusterName"};
    Gaudi::Property<bool> m_getAssociatedTopoCluster {this, "getAssociatedTopoCluster", true, "Get the topo cluster associated to a super cluster, which was linked to an Electron."};
    Gaudi::Property<bool> m_skipEmptyEvents {this, "skipEmptyEvents", true, "If true, do not fill the event that has no reco electrons."};
    Gaudi::Property<float> m_minZeeMassTP {this, "minZeeMassTP", 66, "Minimum value of Zee mass for checking the TP pairs (GeV)."};
    Gaudi::Property<float> m_maxZeeMassTP {this, "maxZeeMassTP", 116, "Maximum value of Zee mass for checking the TP pairs (GeV)."};
    
  //##################
  // Services and Keys
  //##################

    ServiceHandle<ITHistSvc> m_ntsvc;

    // Conditions data objects
    SG::ReadCondHandleKey<CaloNoise>              m_noiseCDOKey{this,"CaloNoiseKey","totalNoise","SG Key of CaloNoise data object"};
    SG::ReadCondHandleKey<AthenaAttributeList>    m_run2DSPThresholdsKey{this, "Run2DSPThresholdsKey","", "SG Key for thresholds to compute time and quality, run 2"};
    SG::ReadCondHandleKey<AthenaAttributeList>    m_EneRescalerFldr{this,"OflEneRescalerKey","/LAR/CellCorrOfl/EnergyCorr", "Key (=foldername) of AttrListCollection"};    
    SG::ReadCondHandleKey<LuminosityCondData>     m_lumiDataKey{this,"LumiKey", "LuminosityCondData","SG Key of LuminosityCondData object"};  
    SG::ReadCondHandleKey<ILArPedestal>           m_pedestalKey{this,"PedestalKey","LArPedestal","SG Key of Pedestal conditions object"};
    SG::ReadCondHandleKey<LArADC2MeV>             m_adc2MeVKey{this,"ADC2MeVKey","LArADC2MeV","SG Key of ADC2MeV conditions object"};
    SG::ReadCondHandleKey<ILArOFC>                m_ofcKey{this,"OFCKey","LArOFC","SG Key of OFC conditions object"};
    SG::ReadCondHandleKey<ILArShape>              m_shapeKey{this,"ShapeKey","LArShape","SG Key of Shape conditions object"};
    SG::ReadCondHandleKey<ILArMinBiasAverage>     m_minBiasAvgKey{this, "MinBiasAvgKey", "LArPileupAverageSym", "SGKey of LArMinBiasAverage object"};
    SG::ReadCondHandleKey<LArHVCorr>              m_offlineHVScaleCorrKey{this, "keyOfflineHVCorr", "LArHVScaleCorrRecomputed","Key for LArHVScaleCorr"};
    SG::ReadCondHandleKey<ILArfSampl>             m_fSamplKey{this, "fSamplKey", "LArfSamplSym","SG key of LArfSampl object."};
    SG::ReadCondHandleKey<LArOnOffIdMapping>      m_larCablingKey{this,"LArOnOffMapKey","LArOnOffIdMap"," SG Key of LArOnOffIdMapping object."};
    
    SG::ReadHandleKey<xAOD::EventInfo>                m_eventInfoSgKey{this, "EventInfoContainerKey", "EventInfo", "Name of the EventInfo Container"};
    SG::ReadHandleKey<xAOD::VertexContainer>          m_primVertSgKey{this, "PrimaryVertexContainerKey", "PrimaryVertices", "Name of the PrimaryVertices Container"};
    SG::ReadHandleKey<xAOD::CaloClusterContainer>     m_caloClusSgKey{this, "CaloClusterContainerKey", "CaloCalTopoClusters", "Name of the CaloCluster Container"};
    SG::ReadHandleKey<xAOD::ElectronContainer>        m_myElecSelectionSgKey{this, "MyElectronSelectionKey", "MySelectedElectrons", "Name of the MySelectedElectrons Container"};
    SG::ReadHandleKey<xAOD::TruthParticleContainer>   m_truthParticleCntSgKey{this, "TruthParticleContainerKey", "TruthParticles", "Name of the TruthParticles Container"};
    SG::ReadHandleKey<xAOD::ElectronContainer>        m_electronCntSgKey{this, "ElectronContainerKey", "Electrons", "Name of the Electrons Container"};
    SG::ReadHandleKey<xAOD::TruthEventContainer>      m_truthEventCntSgKey{this, "TruthEventContainerKey", "TruthEvents", "Name of the TruthEvents Container"};
    SG::ReadHandleKey<LArHitContainer>                m_larEMBHitCntSgKey{this, "LArEMBHitContainerKey", "LArHitEMB", "Name of the LArHitEMB container"};
    SG::ReadHandleKey<LArDigitContainer>              m_larDigitCntSgKey{this, "LArDigitContainerKey", "LArDigitContainer_MC", "Name of the LArDigits container"};
    SG::ReadHandleKey<LArRawChannelContainer>         m_larRawChCntSgKey{this, "LArRawChannelContainerKey", "LArRawChannels", "Name of the LArRawChannel container"};
    SG::ReadHandleKey<CaloCellContainer>              m_allCaloCellCntSgKey{this, "CaloCellContainerKey", "AllCalo", "Name of the CaloCell container"};

    std::unique_ptr<LArDSPThresholdsFlat>       m_run2DSPThresh = nullptr;
    
    const CaloNoise*                            m_noiseCDO    = nullptr;
    const LuminosityCondData*                   m_lumis       = nullptr;
    const ILArPedestal*                         m_peds        = nullptr;
    const LArADC2MeV*                           m_adc2MeVs    = nullptr;
    const ILArOFC*                              m_ofcs        = nullptr;
    const ILArShape*                            m_shapes      = nullptr;
    const ILArMinBiasAverage*                   m_minBiasAvgs = nullptr;
    const LArHVCorr*                            m_oflHVCorr   = nullptr;
    const CaloCondBlobFlt*                      m_EneRescaler = nullptr;
    const ILArfSampl*                           m_fSampl      = nullptr;

    // LAr Tools
    const LArOnlineID*        m_onlineLArID = nullptr;
    const LArOnOffIdMapping*  m_larCabling  = nullptr;
    const CaloIdManager*      m_caloIdMgr   = nullptr;
    const LArEM_ID*           m_larem_id    = nullptr;
    const LArHEC_ID*          m_larhec_id   = nullptr;
    const LArFCAL_ID*         m_larfcal_id  = nullptr;
    const CaloCell_ID*        m_calocell_id  = nullptr;
    
};


#endif 



