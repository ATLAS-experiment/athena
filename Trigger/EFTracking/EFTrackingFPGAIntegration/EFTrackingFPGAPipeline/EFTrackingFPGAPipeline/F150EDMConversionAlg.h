/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef EFTRACKING_FPGA_F150EDMCONVERSION_H
#define EFTRACKING_FPGA_F150EDMCONVERSION_H

// Athena include
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "EFTrackingFPGAUtility/xAODClusterMaker.h"
#include "EFTrackingFPGAUtility/EFTrackingTransient.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"

// ACTS
#include "ActsEvent/SeedContainer.h"

// STL include
#include <string>
#include <vector>

/**
 * @brief The class for enconding RDO to FPGA format. 
 * 
 */
namespace EFTrackingFPGAIntegration
{
    class F150EDMConversionAlg : public AthReentrantAlgorithm
    {
    public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;

        virtual StatusCode initialize() override;

        StatusCode execute(const EventContext &ctx) const override;


    protected:
        Gaudi::Property<size_t> m_minSpacePointsPerSeed{this, "MinSpacePointsPerSeed", 3, "Minimum number of space points per seed"};
        Gaudi::Property<size_t> m_maxSpacePointsPerSeed{this, "MaxSpacePointsPerSeed", 3, "Maximum number of space points per seed"};


        SG::ReadHandleKey<std::vector<uint64_t>> m_FPGATrackOutput{this, "FPGAOutputTrackKey", "FPGATrackOutput", "Track output from FPGA format"};

	    SG::ReadHandleKey<xAOD::SpacePointContainer> m_spacePointContainerKey{this, "FPGASpacePointsKey", "", "Pixel Space Point Container"};

	    // WriteHandleKeys
	    SG::WriteHandleKey< ActsTrk::SeedContainer > m_seedKey {this,"OutputSeeds","","Output Seeds"};

    };
}

#endif // EFTRACKING_FPGA_EDMCONVERSION_H
 