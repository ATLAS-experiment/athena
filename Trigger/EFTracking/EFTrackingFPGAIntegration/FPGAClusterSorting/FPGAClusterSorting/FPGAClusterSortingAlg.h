//  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGACLUSTERSORTING
#define FPGACLUSTERSORTING

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include <AthContainers/ConstDataVector.h>

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"


class FPGAClusterSortingAlg : public ::AthReentrantAlgorithm {
public:
    FPGAClusterSortingAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~FPGAClusterSortingAlg() = default;

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

private:
    SG::ReadHandleKey <xAOD::PixelClusterContainer> m_xAODPixelClusterContainerKey{ this, "xAODPixelClusterContainer", "", "" };
    SG::ReadHandleKey <xAOD::StripClusterContainer> m_xAODStripClusterContainerKeys{ this, "xAODStripClusterContainer", "", "" }; 

    SG::WriteHandleKey <ConstDataVector<xAOD::PixelClusterContainer>> m_sortedxAODPixelClusterContainerKey{ this, "sortedxAODPixelClusterContainer", "", "" };
    SG::WriteHandleKey <ConstDataVector<xAOD::StripClusterContainer>> m_sortedxAODStripClusterContainerKeys{ this, "sortedxAODStripClusterContainer", "", "" }; 
    
};

#endif