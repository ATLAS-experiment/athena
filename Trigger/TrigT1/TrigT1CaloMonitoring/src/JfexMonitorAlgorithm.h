/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_JFEXMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_JFEXMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"

#include "xAODTrigger/jFexLRJetRoIContainer.h"
#include "xAODTrigger/jFexLRJetRoI.h"
#include "xAODTrigger/jFexSRJetRoIContainer.h"
#include "xAODTrigger/jFexSRJetRoI.h"
#include "xAODTrigger/jFexTauRoIContainer.h" 
#include "xAODTrigger/jFexTauRoI.h"
#include "xAODTrigger/jFexFwdElRoIContainer.h"
#include "xAODTrigger/jFexFwdElRoI.h"
#include "xAODTrigger/jFexMETRoIContainer.h"
#include "xAODTrigger/jFexMETRoI.h"
#include "xAODTrigger/jFexSumETRoIContainer.h"
#include "xAODTrigger/jFexSumETRoI.h"

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;


class JfexMonitorAlgorithm : public AthMonitorAlgorithm {
    public:
        JfexMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
        virtual ~JfexMonitorAlgorithm()=default;
        virtual StatusCode initialize() override;
        virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

    private:
        mutable bool m_firstEvent = true;
        mutable std::mutex m_mutex;

        StringProperty m_Grouphist{this,"Grouphist","JfexMonitor","group name for histograming"};
        StringProperty m_Groupmaps{this,"Groupmaps","jFEXMaps","group name for jFEX TOB maps"};
        StringProperty m_GroupmapsHighPt{this,"GroupmapsHighPt","jFEXMapsHighPt","group name for jFEX TOB maps with higher energy thresholds"};
        Gaudi::Property<std::vector<float>> m_jJEtaBins{this,"jJEtaBins"};
        Gaudi::Property<std::vector<float>> m_jTauEtaBins{this,"jTauEtaBins"};
        Gaudi::Property<std::vector<float>> m_jEMEtaBins{this,"jEMEtaBins"};


        // container keys including this, steering parameter, default value and help description
        SG::ReadHandleKey< xAOD::jFexSRJetRoIContainer > m_jFexSRJetContainerKey {this,"jFexSRJetRoIContainer","L1_jFexSRJetRoI","SG key of the input jFex SR Jet Roi container"};
        SG::ReadHandleKey< xAOD::jFexLRJetRoIContainer > m_jFexLRJetContainerKey {this,"jFexLRJetRoIContainer","L1_jFexLRJetRoI","SG key of the input jFex LR Jet Roi container"};
        SG::ReadHandleKey< xAOD::jFexTauRoIContainer   > m_jFexTauContainerKey   {this,"jFexTauRoIContainer"  ,"L1_jFexTauRoI"  ,"SG key of the input jFex Tau Roi container"};
        SG::ReadHandleKey< xAOD::jFexFwdElRoIContainer > m_jFexFwdElContainerKey {this,"jFexFwdElRoIContainer","L1_jFexFwdElRoI","SG key of the input jFex EM Roi container"};
        SG::ReadHandleKey< xAOD::jFexMETRoIContainer   > m_jFexMETContainerKey   {this,"jFexMETRoIContainer"  ,"L1_jFexMETRoI"  ,"SG key of the input jFex MET Roi container"};
        SG::ReadHandleKey< xAOD::jFexSumETRoIContainer > m_jFexSumEtContainerKey {this,"jFexSumETRoIContainer","L1_jFexSumETRoI","SG key of the input jFex SumEt Roi container"};

        int binNumberFromCoordinates(float eta, float phi, const std::vector<float>& etaBinBorders) const;

        StatusCode fillJetMaps(const xAOD::jFexSRJetRoI *tob,
                             Monitored::Scalar<float> &eta,
                             Monitored::Scalar<float> &phi,
                             Monitored::Scalar<int> &binNumber,
                             Monitored::Scalar<int> &lbn,
                             Monitored::Scalar<float> &weight) const;
        StatusCode fillEMMaps(const xAOD::jFexFwdElRoI *tob,
                             Monitored::Scalar<float> &eta,
                             Monitored::Scalar<float> &phi,
                             Monitored::Scalar<int> &binNumber,
                             Monitored::Scalar<int> &lbn,
                             Monitored::Scalar<float> &weight) const;
        // for filling maps where phi binning is ~0.1
        template<typename TOB>
        StatusCode fillMapsCentralAndFCAL(TOB tob,
                             Monitored::Scalar<float> &eta,
                             Monitored::Scalar<float> &phi,
                             Monitored::Scalar<int> &binNumber,
                             Monitored::Scalar<int> &lbn,
                             const std::vector<float>& etaBinBorders,
                             Monitored::Scalar<float> &weight) const;
        // for faking ~0.2 phi binning in endcap
        template<typename TOB>
        StatusCode fillMapsEndcap(TOB tob,
                             Monitored::Scalar<float> &eta,
                             Monitored::Scalar<float> &phi,
                             Monitored::Scalar<int> &binNumber,
                             Monitored::Scalar<int> &lbn,
                             const std::vector<float>& etaBinBorders,
                             Monitored::Scalar<float> &weight) const;
        // special handling for jets in endcap/FCAL overlap
        StatusCode fillMapsOverlap(const xAOD::jFexSRJetRoI* tob,
                             Monitored::Scalar<float> &eta,
                             Monitored::Scalar<float> &phi,
                             Monitored::Scalar<int> &binNumber,
                             Monitored::Scalar<int> &lbn,
                             const std::vector<float>& etaBinBorders,
                             Monitored::Scalar<float> &weight) const;

        bool passesEnergyCut(const xAOD::jFexSRJetRoI *tob) const;
        bool passesEnergyCut(const xAOD::jFexFwdElRoI *tob) const;
        bool passesEnergyCut(const xAOD::jFexTauRoI *tob) const;
};
#endif
