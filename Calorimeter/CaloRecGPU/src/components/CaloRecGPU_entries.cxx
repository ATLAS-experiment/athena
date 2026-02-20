//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

// Local include(s).
#include "../CaloGPUHybridClusterProcessor.h"
#include "../BasicConstantGPUDataExporter.h"
#include "../BasicEventDataGPUExporter.h"
#include "../BasicGPUToAthenaImporter.h"
#include "../CaloGPUOutput.h"
#include "../CaloCPUOutput.h"
#include "../TopoAutomatonClustering.h"
#include "../CaloCellsCounterCPU.h"
#include "../CaloCellsCounterGPU.h"
#include "../BasicGPUClusterInfoCalculator.h"
#include "../TopoAutomatonSplitting.h"
#include "../CaloClusterDeleter.h"
#include "../CaloGPUClusterAndCellDataMonitor.h"
#include "../GPUClusterInfoAndMomentsCalculator.h"
#include "../GPUToAthenaImporterWithMoments.h"
#include "../CaloMomentsDumper.h"
#include "../GPUKernelSizeOptimizerSvc.h"
#include "../CaloClusterStoreRawProperties.h"
#include "../CaloPerformancePropertiesOutput.h"
#include "../GPUClusterSorter.h"

// Declare the "components".
DECLARE_COMPONENT( CaloGPUHybridClusterProcessor )
DECLARE_COMPONENT( BasicConstantGPUDataExporter )
DECLARE_COMPONENT( BasicEventDataGPUExporter )
DECLARE_COMPONENT( BasicGPUToAthenaImporter )
DECLARE_COMPONENT( CaloGPUOutput )
DECLARE_COMPONENT( CaloCPUOutput )
DECLARE_COMPONENT( TopoAutomatonClustering )
DECLARE_COMPONENT( CaloCellsCounterCPU )
DECLARE_COMPONENT( CaloCellsCounterGPU )
DECLARE_COMPONENT( BasicGPUClusterInfoCalculator )
DECLARE_COMPONENT( CaloClusterDeleter )
DECLARE_COMPONENT( TopoAutomatonSplitting )
DECLARE_COMPONENT( CaloGPUClusterAndCellDataMonitor )
DECLARE_COMPONENT( GPUClusterInfoAndMomentsCalculator )
DECLARE_COMPONENT( GPUToAthenaImporterWithMoments )
DECLARE_COMPONENT( CaloMomentsDumper )
DECLARE_COMPONENT( GPUKernelSizeOptimizerSvc )
DECLARE_COMPONENT( CaloClusterStoreRawProperties )
DECLARE_COMPONENT( CaloPerformancePropertiesOutput )
DECLARE_COMPONENT( GPUClusterSorter )

