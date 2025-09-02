// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATRACKSIMSEEDINGALG_H
#define FPGATRACKSIMSEEDINGALG_H

// Athena libraries
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"

// ACTS
#include "ActsEvent/Seed.h"
#include "ActsEvent/SeedContainer.h"

// Handle keys
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"

// FPGATrackSim
#include "FPGATrackSimObjects/FPGATrackSimRoadCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackCollection.h"

namespace FPGATrackSim {

class FPGATrackSimSeedingAlg : public ::AthReentrantAlgorithm {
public:
	FPGATrackSimSeedingAlg(const std::string& name, ISvcLocator* pSvcLocator);
	virtual ~FPGATrackSimSeedingAlg() = default;

	virtual StatusCode initialize() override final;
	virtual StatusCode execute(const EventContext& ctx) const override final;

private:

	Gaudi::Property<size_t> m_minSpacePointsPerSeed{this, "MinSpacePointsPerSeed", 3, "Minimum number of space points per seed"};
	Gaudi::Property<size_t> m_maxSpacePointsPerSeed{this, "MaxSpacePointsPerSeed", 3, "Maximum number of space points per seed"};

	// ReadHandleKeys
	SG::ReadHandleKey<FPGATrackSimTrackCollection> m_FPGATrackCollectionKey{this, "FPGATrackSimTrackKey","","FPGA Tracks 1st stage key"};
	SG::ReadHandleKey<xAOD::PixelClusterContainer> m_pixelClusterContainerKey{this, "FPGAPixelClustersKey", "", "Pixel Cluster Container"};
	SG::ReadHandleKey<xAOD::SpacePointContainer> m_spacePointContainerKey{this, "FPGASpacePointsKey", "", "Pixel Space Point Container"};

	// WriteHandleKeys
	SG::WriteHandleKey< ActsTrk::SeedContainer > m_seedKey {this,"OutputSeeds","","Output Seeds"};
};

} // namespace FPGATrackSim

#endif // FPGATRACKSIMSEEDINGALG_H

