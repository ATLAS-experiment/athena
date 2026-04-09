/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_GEPETASOFTKILLERALG_H
#define TRIGGEPPERF_GEPETASOFTKILLERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

class GepEtaSoftKillerAlg : public AthReentrantAlgorithm {
public:
    GepEtaSoftKillerAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
    SG::ReadHandleKey<xAOD::CaloClusterContainer> m_inputClustersKey{
        this, "inputClustersKey", "", "Input cluster/tower container"};

    SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outputClustersKey{
        this, "outputClustersKey", "", "Output EtaSK-suppressed cluster container"};

    Gaudi::Property<double> m_gridEtaSize{this, "gridEtaSize", 0.6,
        "Grid cell size in eta"};

    Gaudi::Property<double> m_gridPhiSize{this, "gridPhiSize", 0.6,
        "Grid cell size in phi"};

    Gaudi::Property<double> m_etaMax{this, "etaMax", 4.9,
        "Maximum |eta| extent of the grid"};

    Gaudi::Property<int> m_etaBandWidth{this, "etaBandWidth", 2,
        "Number of eta rows per band for median calculation"};
};

#endif //> !TRIGGEPPERF_GEPETASOFTKILLERALG_H
