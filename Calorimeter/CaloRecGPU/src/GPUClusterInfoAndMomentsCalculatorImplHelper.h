//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_GPUCLUSTERINFOANDMOMENTSCALCULATORHELPER_CUDA_H
#define CALORECGPU_GPUCLUSTERINFOANDMOMENTSCALCULATORHELPER_CUDA_H

#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "GPUClusterInfoAndMomentsCalculatorImpl.h"
#include "FPHelpers.h"
#include "TemporaryHelpers.h"

#include "CaloGeoHelpers/CaloSampling.h"
//Just enums and stuff, CUDA compatible.

#include <cmath>
#include <type_traits>

namespace
{
  namespace CMCTemporaries
  {
    // BIG TABLE OF VARIABLE COEXISTENCE!
    // I know this is a big comment to have here, but it is the most immediate way to have a reference for what lives where.
    //
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | CLUSTER STORAGE VARIABLE ||   Isolation Cluster Pass   |    Isolation Cell Pass     |    Zeroth Cluster Pass     |      First Cell Pass       |     First Cluster Pass     |      Second Cell Pass      |      Shower Axis Pass      |    Second Cluster Pass     |      Third Cell Pass       |     Third Cluster Pass     |     Final Cluster Pass     ||          Comments          |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | clusterEnergy            ||                            |                            | clusterEnergy              | clusterEnergy              | clusterEnergy              | clusterEnergy              | clusterEnergy              | clusterEnergy              | clusterEnergy              | clusterEnergy              | clusterEnergy              ||                            |
    // | clusterEt                ||                            |                            |                            |                            | clusterEt                  | clusterEt                  | clusterEt                  | clusterEt                  | clusterEt                  | clusterEt                  | clusterEt                  || Not stored as such by CPU. |
    // | clusterEta               ||                            |                            | clusterEta                 | clusterEta                 | clusterEta                 | clusterEta                 | clusterEta                 | clusterEta                 | clusterEta                 | clusterEta                 | clusterEta                 ||                            |
    // | clusterPhi               ||                            |                            | clusterPhi                 | clusterPhi                 | clusterPhi                 | clusterPhi                 | clusterPhi                 | clusterPhi                 | clusterPhi                 | clusterPhi                 | clusterPhi                 ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | energyPerSample     [00] || energyPerSample       [00] | energyPerSample       [00] |ePS>absolEnrgPrSmpleAux[00] |AbsolutEnergyPrSmpleAux[00] |AbsolutEnergyPrSmpleAux[00] |                            |                            | energyPerSample       [00] | energyPerSample       [00] | energyPerSample       [00] | energyPerSample       [00] ||                            |
    // | energyPerSample     [01] || energyPerSample       [01] | energyPerSample       [01] |ePS>absolEnrgPrSmpleAux[01] |AbsolutEnergyPrSmpleAux[01] |AbsolutEnergyPrSmpleAux[01] |                            |                            | energyPerSample       [01] | energyPerSample       [01] | energyPerSample       [01] | energyPerSample       [01] ||                            |
    // | energyPerSample     [02] || energyPerSample       [02] | energyPerSample       [02] |ePS>absolEnrgPrSmpleAux[02] |AbsolutEnergyPrSmpleAux[02] |AbsolutEnergyPrSmpleAux[02] |                            |                            | energyPerSample       [02] | energyPerSample       [02] | energyPerSample       [02] | energyPerSample       [02] ||                            |
    // | energyPerSample     [03] || energyPerSample       [03] | energyPerSample       [03] |ePS>absolEnrgPrSmpleAux[03] |AbsolutEnergyPrSmpleAux[03] |AbsolutEnergyPrSmpleAux[03] |                            |                            | energyPerSample       [03] | energyPerSample       [03] | energyPerSample       [03] | energyPerSample       [03] ||                            |
    // | energyPerSample     [04] || energyPerSample       [04] | energyPerSample       [04] |ePS>absolEnrgPrSmpleAux[04] |AbsolutEnergyPrSmpleAux[04] |AbsolutEnergyPrSmpleAux[04] |                            |                            | energyPerSample       [04] | energyPerSample       [04] | energyPerSample       [04] | energyPerSample       [04] ||                            |
    // | energyPerSample     [05] || energyPerSample       [05] | energyPerSample       [05] |ePS>absolEnrgPrSmpleAux[05] |AbsolutEnergyPrSmpleAux[05] |AbsolutEnergyPrSmpleAux[05] |                            |                            | energyPerSample       [05] | energyPerSample       [05] | energyPerSample       [05] | energyPerSample       [05] ||                            |
    // | energyPerSample     [06] || energyPerSample       [06] | energyPerSample       [06] |ePS>absolEnrgPrSmpleAux[06] |AbsolutEnergyPrSmpleAux[06] |AbsolutEnergyPrSmpleAux[06] |                            |                            | energyPerSample       [06] | energyPerSample       [06] | energyPerSample       [06] | energyPerSample       [06] ||                            |
    // | energyPerSample     [07] || energyPerSample       [07] | energyPerSample       [07] |ePS>absolEnrgPrSmpleAux[07] |AbsolutEnergyPrSmpleAux[07] |AbsolutEnergyPrSmpleAux[07] |                            |                            | energyPerSample       [07] | energyPerSample       [07] | energyPerSample       [07] | energyPerSample       [07] ||                            |
    // | energyPerSample     [08] || energyPerSample       [08] | energyPerSample       [08] |ePS>absolEnrgPrSmpleAux[08] |AbsolutEnergyPrSmpleAux[08] |AbsolutEnergyPrSmpleAux[08] |                            |                            | energyPerSample       [08] | energyPerSample       [08] | energyPerSample       [08] | energyPerSample       [08] ||                            |
    // | energyPerSample     [09] || energyPerSample       [09] | energyPerSample       [09] |ePS>absolEnrgPrSmpleAux[09] |AbsolutEnergyPrSmpleAux[09] |AbsolutEnergyPrSmpleAux[09] |                            |                            | energyPerSample       [09] | energyPerSample       [09] | energyPerSample       [09] | energyPerSample       [09] ||                            |
    // | energyPerSample     [10] || energyPerSample       [10] | energyPerSample       [10] |ePS>absolEnrgPrSmpleAux[10] |AbsolutEnergyPrSmpleAux[10] |AbsolutEnergyPrSmpleAux[10] |                            |                            | energyPerSample       [10] | energyPerSample       [10] | energyPerSample       [10] | energyPerSample       [10] ||                            |
    // | energyPerSample     [11] || energyPerSample       [11] | energyPerSample       [11] |ePS>absolEnrgPrSmpleAux[11] |AbsolutEnergyPrSmpleAux[11] |AbsolutEnergyPrSmpleAux[11] |                            |                            | energyPerSample       [11] | energyPerSample       [11] | energyPerSample       [11] | energyPerSample       [11] ||                            |
    // | energyPerSample     [12] || energyPerSample       [12] | energyPerSample       [12] |ePS>absolEnrgPrSmpleAux[12] |AbsolutEnergyPrSmpleAux[12] |AbsolutEnergyPrSmpleAux[12] |                            |                            | energyPerSample       [12] | energyPerSample       [12] | energyPerSample       [12] | energyPerSample       [12] ||                            |
    // | energyPerSample     [13] || energyPerSample       [13] | energyPerSample       [13] |ePS>absolEnrgPrSmpleAux[13] |AbsolutEnergyPrSmpleAux[13] |AbsolutEnergyPrSmpleAux[13] |                            |                            | energyPerSample       [13] | energyPerSample       [13] | energyPerSample       [13] | energyPerSample       [13] ||                            |
    // | energyPerSample     [14] || energyPerSample       [14] | energyPerSample       [14] |ePS>absolEnrgPrSmpleAux[14] |AbsolutEnergyPrSmpleAux[14] |AbsolutEnergyPrSmpleAux[14] |                            |                            | energyPerSample       [14] | energyPerSample       [14] | energyPerSample       [14] | energyPerSample       [14] ||                            |
    // | energyPerSample     [15] || energyPerSample       [15] | energyPerSample       [15] |ePS>absolEnrgPrSmpleAux[15] |AbsolutEnergyPrSmpleAux[15] |AbsolutEnergyPrSmpleAux[15] |                            |                            | energyPerSample       [15] | energyPerSample       [15] | energyPerSample       [15] | energyPerSample       [15] ||                            |
    // | energyPerSample     [16] || energyPerSample       [16] | energyPerSample       [16] |ePS>absolEnrgPrSmpleAux[16] |AbsolutEnergyPrSmpleAux[16] |AbsolutEnergyPrSmpleAux[16] |                            |                            | energyPerSample       [16] | energyPerSample       [16] | energyPerSample       [16] | energyPerSample       [16] ||                            |
    // | energyPerSample     [17] || energyPerSample       [17] | energyPerSample       [17] |ePS>absolEnrgPrSmpleAux[17] |AbsolutEnergyPrSmpleAux[17] |AbsolutEnergyPrSmpleAux[17] |                            |                            | energyPerSample       [17] | energyPerSample       [17] | energyPerSample       [17] | energyPerSample       [17] ||                            |
    // | energyPerSample     [18] || energyPerSample       [18] | energyPerSample       [18] |ePS>absolEnrgPrSmpleAux[18] |AbsolutEnergyPrSmpleAux[18] |AbsolutEnergyPrSmpleAux[18] |                            |                            | energyPerSample       [18] | energyPerSample       [18] | energyPerSample       [18] | energyPerSample       [18] ||                            |
    // | energyPerSample     [19] || energyPerSample       [19] | energyPerSample       [19] |ePS>absolEnrgPrSmpleAux[19] |AbsolutEnergyPrSmpleAux[19] |AbsolutEnergyPrSmpleAux[19] |                            |                            | energyPerSample       [19] | energyPerSample       [19] | energyPerSample       [19] | energyPerSample       [19] ||                            |
    // | energyPerSample     [20] || energyPerSample       [20] | energyPerSample       [20] |ePS>absolEnrgPrSmpleAux[20] |AbsolutEnergyPrSmpleAux[20] |AbsolutEnergyPrSmpleAux[20] |                            |                            | energyPerSample       [20] | energyPerSample       [20] | energyPerSample       [20] | energyPerSample       [20] ||                            |
    // | energyPerSample     [21] || energyPerSample       [21] | energyPerSample       [21] |ePS>absolEnrgPrSmpleAux[21] |AbsolutEnergyPrSmpleAux[21] |AbsolutEnergyPrSmpleAux[21] |                            |                            | energyPerSample       [21] | energyPerSample       [21] | energyPerSample       [21] | energyPerSample       [21] ||                            |
    // | energyPerSample     [22] || energyPerSample       [22] | energyPerSample       [22] |ePS>absolEnrgPrSmpleAux[22] |AbsolutEnergyPrSmpleAux[22] |AbsolutEnergyPrSmpleAux[22] |                            |                            | energyPerSample       [22] | energyPerSample       [22] | energyPerSample       [22] | energyPerSample       [22] ||                            |
    // | energyPerSample     [23] || energyPerSample       [23] | energyPerSample       [23] |ePS>absolEnrgPrSmpleAux[23] |AbsolutEnergyPrSmpleAux[23] |AbsolutEnergyPrSmpleAux[23] |                            |                            | energyPerSample       [23] | energyPerSample       [23] | energyPerSample       [23] | energyPerSample       [23] ||                            |
    // | energyPerSample     [24] || energyPerSample       [24] | energyPerSample       [24] |ePS>absolEnrgPrSmpleAux[24] |AbsolutEnergyPrSmpleAux[24] |AbsolutEnergyPrSmpleAux[24] |                            |                            | energyPerSample       [24] | energyPerSample       [24] | energyPerSample       [24] | energyPerSample       [24] ||                            |
    // | energyPerSample     [25] || energyPerSample       [25] | energyPerSample       [25] |ePS>absolEnrgPrSmpleAux[25] |AbsolutEnergyPrSmpleAux[25] |AbsolutEnergyPrSmpleAux[25] |                            |                            | energyPerSample       [25] | energyPerSample       [25] | energyPerSample       [25] | energyPerSample       [25] ||                            |
    // | energyPerSample     [26] || energyPerSample       [26] | energyPerSample       [26] |ePS>absolEnrgPrSmpleAux[26] |AbsolutEnergyPrSmpleAux[26] |AbsolutEnergyPrSmpleAux[26] |                            |                            | energyPerSample       [26] | energyPerSample       [26] | energyPerSample       [26] | energyPerSample       [26] ||                            |
    // | energyPerSample     [27] || energyPerSample       [27] | energyPerSample       [27] |ePS>absolEnrgPrSmpleAux[27] |AbsolutEnergyPrSmpleAux[27] |AbsolutEnergyPrSmpleAux[27] |                            |                            | energyPerSample       [27] | energyPerSample       [27] | energyPerSample       [27] | energyPerSample       [27] ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | maxEPerSample       [00] || energyPerSampleAux    [00] | energyPerSampleAux    [00] |ePSA>absolutEnrgyPrSmpl[00] | absolutEnergyPerSample[00] | absolutEnergyPerSample[00] |                            |                            | energyPerSampleAux    [00] | energyPerSampleAux    [00] | energyPerSampleAux    [00] | maxEPerSample         [00] ||                            |
    // | maxEPerSample       [01] || energyPerSampleAux    [01] | energyPerSampleAux    [01] |ePSA>absolutEnrgyPrSmpl[01] | absolutEnergyPerSample[01] | absolutEnergyPerSample[01] |                            |                            | energyPerSampleAux    [01] | energyPerSampleAux    [01] | energyPerSampleAux    [01] | maxEPerSample         [01] ||                            |
    // | maxEPerSample       [02] || energyPerSampleAux    [02] | energyPerSampleAux    [02] |ePSA>absolutEnrgyPrSmpl[02] | absolutEnergyPerSample[02] | absolutEnergyPerSample[02] |                            |                            | energyPerSampleAux    [02] | energyPerSampleAux    [02] | energyPerSampleAux    [02] | maxEPerSample         [02] ||                            |
    // | maxEPerSample       [03] || energyPerSampleAux    [03] | energyPerSampleAux    [03] |ePSA>absolutEnrgyPrSmpl[03] | absolutEnergyPerSample[03] | absolutEnergyPerSample[03] |                            |                            | energyPerSampleAux    [03] | energyPerSampleAux    [03] | energyPerSampleAux    [03] | maxEPerSample         [03] ||                            |
    // | maxEPerSample       [04] || energyPerSampleAux    [04] | energyPerSampleAux    [04] |ePSA>absolutEnrgyPrSmpl[04] | absolutEnergyPerSample[04] | absolutEnergyPerSample[04] |                            |                            | energyPerSampleAux    [04] | energyPerSampleAux    [04] | energyPerSampleAux    [04] | maxEPerSample         [04] ||                            |
    // | maxEPerSample       [05] || energyPerSampleAux    [05] | energyPerSampleAux    [05] |ePSA>absolutEnrgyPrSmpl[05] | absolutEnergyPerSample[05] | absolutEnergyPerSample[05] |                            |                            | energyPerSampleAux    [05] | energyPerSampleAux    [05] | energyPerSampleAux    [05] | maxEPerSample         [05] ||                            |
    // | maxEPerSample       [06] || energyPerSampleAux    [06] | energyPerSampleAux    [06] |ePSA>absolutEnrgyPrSmpl[06] | absolutEnergyPerSample[06] | absolutEnergyPerSample[06] |                            |                            | energyPerSampleAux    [06] | energyPerSampleAux    [06] | energyPerSampleAux    [06] | maxEPerSample         [06] ||                            |
    // | maxEPerSample       [07] || energyPerSampleAux    [07] | energyPerSampleAux    [07] |ePSA>absolutEnrgyPrSmpl[07] | absolutEnergyPerSample[07] | absolutEnergyPerSample[07] |                            |                            | energyPerSampleAux    [07] | energyPerSampleAux    [07] | energyPerSampleAux    [07] | maxEPerSample         [07] ||                            |
    // | maxEPerSample       [08] || energyPerSampleAux    [08] | energyPerSampleAux    [08] |ePSA>absolutEnrgyPrSmpl[08] | absolutEnergyPerSample[08] | absolutEnergyPerSample[08] |                            |                            | energyPerSampleAux    [08] | energyPerSampleAux    [08] | energyPerSampleAux    [08] | maxEPerSample         [08] ||                            |
    // | maxEPerSample       [09] || energyPerSampleAux    [09] | energyPerSampleAux    [09] |ePSA>absolutEnrgyPrSmpl[09] | absolutEnergyPerSample[09] | absolutEnergyPerSample[09] |                            |                            | energyPerSampleAux    [09] | energyPerSampleAux    [09] | energyPerSampleAux    [09] | maxEPerSample         [09] ||                            |
    // | maxEPerSample       [10] || energyPerSampleAux    [10] | energyPerSampleAux    [10] |ePSA>absolutEnrgyPrSmpl[10] | absolutEnergyPerSample[10] | absolutEnergyPerSample[10] |                            |                            | energyPerSampleAux    [10] | energyPerSampleAux    [10] | energyPerSampleAux    [10] | maxEPerSample         [10] ||                            |
    // | maxEPerSample       [11] || energyPerSampleAux    [11] | energyPerSampleAux    [11] |ePSA>absolutEnrgyPrSmpl[11] | absolutEnergyPerSample[11] | absolutEnergyPerSample[11] |                            |                            | energyPerSampleAux    [11] | energyPerSampleAux    [11] | energyPerSampleAux    [11] | maxEPerSample         [11] ||                            |
    // | maxEPerSample       [12] || energyPerSampleAux    [12] | energyPerSampleAux    [12] |ePSA>absolutEnrgyPrSmpl[12] | absolutEnergyPerSample[12] | absolutEnergyPerSample[12] |                            |                            | energyPerSampleAux    [12] | energyPerSampleAux    [12] | energyPerSampleAux    [12] | maxEPerSample         [12] ||                            |
    // | maxEPerSample       [13] || energyPerSampleAux    [13] | energyPerSampleAux    [13] |ePSA>absolutEnrgyPrSmpl[13] | absolutEnergyPerSample[13] | absolutEnergyPerSample[13] |                            |                            | energyPerSampleAux    [13] | energyPerSampleAux    [13] | energyPerSampleAux    [13] | maxEPerSample         [13] ||                            |
    // | maxEPerSample       [14] || energyPerSampleAux    [14] | energyPerSampleAux    [14] |ePSA>absolutEnrgyPrSmpl[14] | absolutEnergyPerSample[14] | absolutEnergyPerSample[14] |                            |                            | energyPerSampleAux    [14] | energyPerSampleAux    [14] | energyPerSampleAux    [14] | maxEPerSample         [14] ||                            |
    // | maxEPerSample       [15] || energyPerSampleAux    [15] | energyPerSampleAux    [15] |ePSA>absolutEnrgyPrSmpl[15] | absolutEnergyPerSample[15] | absolutEnergyPerSample[15] |                            |                            | energyPerSampleAux    [15] | energyPerSampleAux    [15] | energyPerSampleAux    [15] | maxEPerSample         [15] ||                            |
    // | maxEPerSample       [16] || energyPerSampleAux    [16] | energyPerSampleAux    [16] |ePSA>absolutEnrgyPrSmpl[16] | absolutEnergyPerSample[16] | absolutEnergyPerSample[16] |                            |                            | energyPerSampleAux    [16] | energyPerSampleAux    [16] | energyPerSampleAux    [16] | maxEPerSample         [16] ||                            |
    // | maxEPerSample       [17] || energyPerSampleAux    [17] | energyPerSampleAux    [17] |ePSA>absolutEnrgyPrSmpl[17] | absolutEnergyPerSample[17] | absolutEnergyPerSample[17] |                            |                            | energyPerSampleAux    [17] | energyPerSampleAux    [17] | energyPerSampleAux    [17] | maxEPerSample         [17] ||                            |
    // | maxEPerSample       [18] || energyPerSampleAux    [18] | energyPerSampleAux    [18] |ePSA>absolutEnrgyPrSmpl[18] | absolutEnergyPerSample[18] | absolutEnergyPerSample[18] |                            |                            | energyPerSampleAux    [18] | energyPerSampleAux    [18] | energyPerSampleAux    [18] | maxEPerSample         [18] ||                            |
    // | maxEPerSample       [19] || energyPerSampleAux    [19] | energyPerSampleAux    [19] |ePSA>absolutEnrgyPrSmpl[19] | absolutEnergyPerSample[19] | absolutEnergyPerSample[19] |                            |                            | energyPerSampleAux    [19] | energyPerSampleAux    [19] | energyPerSampleAux    [19] | maxEPerSample         [19] ||                            |
    // | maxEPerSample       [20] || energyPerSampleAux    [20] | energyPerSampleAux    [20] |ePSA>absolutEnrgyPrSmpl[20] | absolutEnergyPerSample[20] | absolutEnergyPerSample[20] |                            |                            | energyPerSampleAux    [20] | energyPerSampleAux    [20] | energyPerSampleAux    [20] | maxEPerSample         [20] ||                            |
    // | maxEPerSample       [21] || energyPerSampleAux    [21] | energyPerSampleAux    [21] |ePSA>absolutEnrgyPrSmpl[21] | absolutEnergyPerSample[21] | absolutEnergyPerSample[21] |                            |                            | energyPerSampleAux    [21] | energyPerSampleAux    [21] | energyPerSampleAux    [21] | maxEPerSample         [21] ||                            |
    // | maxEPerSample       [22] || energyPerSampleAux    [22] | energyPerSampleAux    [22] |ePSA>absolutEnrgyPrSmpl[22] | absolutEnergyPerSample[22] | absolutEnergyPerSample[22] |                            |                            | energyPerSampleAux    [22] | energyPerSampleAux    [22] | energyPerSampleAux    [22] | maxEPerSample         [22] ||                            |
    // | maxEPerSample       [23] || energyPerSampleAux    [23] | energyPerSampleAux    [23] |ePSA>absolutEnrgyPrSmpl[23] | absolutEnergyPerSample[23] | absolutEnergyPerSample[23] |                            |                            | energyPerSampleAux    [23] | energyPerSampleAux    [23] | energyPerSampleAux    [23] | maxEPerSample         [23] ||                            |
    // | maxEPerSample       [24] || energyPerSampleAux    [24] | energyPerSampleAux    [24] |ePSA>absolutEnrgyPrSmpl[24] | absolutEnergyPerSample[24] | absolutEnergyPerSample[24] |                            |                            | energyPerSampleAux    [24] | energyPerSampleAux    [24] | energyPerSampleAux    [24] | maxEPerSample         [24] ||                            |
    // | maxEPerSample       [25] || energyPerSampleAux    [25] | energyPerSampleAux    [25] |ePSA>absolutEnrgyPrSmpl[25] | absolutEnergyPerSample[25] | absolutEnergyPerSample[25] |                            |                            | energyPerSampleAux    [25] | energyPerSampleAux    [25] | energyPerSampleAux    [25] | maxEPerSample         [25] ||                            |
    // | maxEPerSample       [26] || energyPerSampleAux    [26] | energyPerSampleAux    [26] |ePSA>absolutEnrgyPrSmpl[26] | absolutEnergyPerSample[26] | absolutEnergyPerSample[26] |                            |                            | energyPerSampleAux    [26] | energyPerSampleAux    [26] | energyPerSampleAux    [26] | maxEPerSample         [26] ||                            |
    // | maxEPerSample       [27] || energyPerSampleAux    [27] | energyPerSampleAux    [27] |ePSA>absolutEnrgyPrSmpl[27] | absolutEnergyPerSample[27] | absolutEnergyPerSample[27] |                            |                            | energyPerSampleAux    [27] | energyPerSampleAux    [27] | energyPerSampleAux    [27] | maxEPerSample         [27] ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | maxPhiPerSample     [00] ||numberNonEmptySamplings[00] |numberNonEmptySamplings[00] | nNES>phiPerSampleAux  [00] | phiPerSampleAux       [00] | phiPerSampleAux       [00] |                            |                            |maxEnrgyAndCellPrSampl2[00] |maxEnrgyAndCellPrSampl2[00] |maxEnrgyAndCellPrSampl2[00] | maxPhiPerSample       [00] ||                            |
    // | maxPhiPerSample     [01] ||numberNonEmptySamplings[01] |numberNonEmptySamplings[01] | nNES>phiPerSampleAux  [01] | phiPerSampleAux       [01] | phiPerSampleAux       [01] |                            |                            |maxEnrgyAndCellPrSampl2[01] |maxEnrgyAndCellPrSampl2[01] |maxEnrgyAndCellPrSampl2[01] | maxPhiPerSample       [01] ||                            |
    // | maxPhiPerSample     [02] ||numberNonEmptySamplings[02] |numberNonEmptySamplings[02] | nNES>phiPerSampleAux  [02] | phiPerSampleAux       [02] | phiPerSampleAux       [02] |                            |                            |maxEnrgyAndCellPrSampl2[02] |maxEnrgyAndCellPrSampl2[02] |maxEnrgyAndCellPrSampl2[02] | maxPhiPerSample       [02] ||                            |
    // | maxPhiPerSample     [03] ||numberNonEmptySamplings[03] |numberNonEmptySamplings[03] | nNES>phiPerSampleAux  [03] | phiPerSampleAux       [03] | phiPerSampleAux       [03] |                            |                            |maxEnrgyAndCellPrSampl2[03] |maxEnrgyAndCellPrSampl2[03] |maxEnrgyAndCellPrSampl2[03] | maxPhiPerSample       [03] ||                            |
    // | maxPhiPerSample     [04] ||numberNonEmptySamplings[04] |numberNonEmptySamplings[04] | nNES>phiPerSampleAux  [04] | phiPerSampleAux       [04] | phiPerSampleAux       [04] |                            |                            |maxEnrgyAndCellPrSampl2[04] |maxEnrgyAndCellPrSampl2[04] |maxEnrgyAndCellPrSampl2[04] | maxPhiPerSample       [04] ||                            |
    // | maxPhiPerSample     [05] ||numberNonEmptySamplings[05] |numberNonEmptySamplings[05] | nNES>phiPerSampleAux  [05] | phiPerSampleAux       [05] | phiPerSampleAux       [05] |                            |                            |maxEnrgyAndCellPrSampl2[05] |maxEnrgyAndCellPrSampl2[05] |maxEnrgyAndCellPrSampl2[05] | maxPhiPerSample       [05] ||                            |
    // | maxPhiPerSample     [06] ||numberNonEmptySamplings[06] |numberNonEmptySamplings[06] | nNES>phiPerSampleAux  [06] | phiPerSampleAux       [06] | phiPerSampleAux       [06] |                            |                            |maxEnrgyAndCellPrSampl2[06] |maxEnrgyAndCellPrSampl2[06] |maxEnrgyAndCellPrSampl2[06] | maxPhiPerSample       [06] ||                            |
    // | maxPhiPerSample     [07] ||numberNonEmptySamplings[07] |numberNonEmptySamplings[07] | nNES>phiPerSampleAux  [07] | phiPerSampleAux       [07] | phiPerSampleAux       [07] |                            |                            |maxEnrgyAndCellPrSampl2[07] |maxEnrgyAndCellPrSampl2[07] |maxEnrgyAndCellPrSampl2[07] | maxPhiPerSample       [07] ||                            |
    // | maxPhiPerSample     [08] ||numberNonEmptySamplings[08] |numberNonEmptySamplings[08] | nNES>phiPerSampleAux  [08] | phiPerSampleAux       [08] | phiPerSampleAux       [08] |                            |                            |maxEnrgyAndCellPrSampl2[08] |maxEnrgyAndCellPrSampl2[08] |maxEnrgyAndCellPrSampl2[08] | maxPhiPerSample       [08] ||                            |
    // | maxPhiPerSample     [09] ||numberNonEmptySamplings[09] |numberNonEmptySamplings[09] | nNES>phiPerSampleAux  [09] | phiPerSampleAux       [09] | phiPerSampleAux       [09] |                            |                            |maxEnrgyAndCellPrSampl2[09] |maxEnrgyAndCellPrSampl2[09] |maxEnrgyAndCellPrSampl2[09] | maxPhiPerSample       [09] ||                            |
    // | maxPhiPerSample     [10] ||numberNonEmptySamplings[10] |numberNonEmptySamplings[10] | nNES>phiPerSampleAux  [10] | phiPerSampleAux       [10] | phiPerSampleAux       [10] |                            |                            |maxEnrgyAndCellPrSampl2[10] |maxEnrgyAndCellPrSampl2[10] |maxEnrgyAndCellPrSampl2[10] | maxPhiPerSample       [10] ||                            |
    // | maxPhiPerSample     [11] ||numberNonEmptySamplings[11] |numberNonEmptySamplings[11] | nNES>phiPerSampleAux  [11] | phiPerSampleAux       [11] | phiPerSampleAux       [11] |                            |                            |maxEnrgyAndCellPrSampl2[11] |maxEnrgyAndCellPrSampl2[11] |maxEnrgyAndCellPrSampl2[11] | maxPhiPerSample       [11] ||                            |
    // | maxPhiPerSample     [12] ||numberNonEmptySamplings[12] |numberNonEmptySamplings[12] | nNES>phiPerSampleAux  [12] | phiPerSampleAux       [12] | phiPerSampleAux       [12] |                            |                            |maxEnrgyAndCellPrSampl2[12] |maxEnrgyAndCellPrSampl2[12] |maxEnrgyAndCellPrSampl2[12] | maxPhiPerSample       [12] ||                            |
    // | maxPhiPerSample     [13] ||numberNonEmptySamplings[13] |numberNonEmptySamplings[13] | nNES>phiPerSampleAux  [13] | phiPerSampleAux       [13] | phiPerSampleAux       [13] |                            |                            |maxEnrgyAndCellPrSampl2[13] |maxEnrgyAndCellPrSampl2[13] |maxEnrgyAndCellPrSampl2[13] | maxPhiPerSample       [13] ||                            |
    // | maxPhiPerSample     [14] ||numberNonEmptySamplings[14] |numberNonEmptySamplings[14] | nNES>phiPerSampleAux  [14] | phiPerSampleAux       [14] | phiPerSampleAux       [14] |                            |                            |maxEnrgyAndCellPrSampl2[14] |maxEnrgyAndCellPrSampl2[14] |maxEnrgyAndCellPrSampl2[14] | maxPhiPerSample       [14] ||                            |
    // | maxPhiPerSample     [15] ||numberNonEmptySamplings[15] |numberNonEmptySamplings[15] | nNES>phiPerSampleAux  [15] | phiPerSampleAux       [15] | phiPerSampleAux       [15] |                            |                            |maxEnrgyAndCellPrSampl2[15] |maxEnrgyAndCellPrSampl2[15] |maxEnrgyAndCellPrSampl2[15] | maxPhiPerSample       [15] ||                            |
    // | maxPhiPerSample     [16] ||numberNonEmptySamplings[16] |numberNonEmptySamplings[16] | nNES>phiPerSampleAux  [16] | phiPerSampleAux       [16] | phiPerSampleAux       [16] |                            |                            |maxEnrgyAndCellPrSampl2[16] |maxEnrgyAndCellPrSampl2[16] |maxEnrgyAndCellPrSampl2[16] | maxPhiPerSample       [16] ||                            |
    // | maxPhiPerSample     [17] ||numberNonEmptySamplings[17] |numberNonEmptySamplings[17] | nNES>phiPerSampleAux  [17] | phiPerSampleAux       [17] | phiPerSampleAux       [17] |                            |                            |maxEnrgyAndCellPrSampl2[17] |maxEnrgyAndCellPrSampl2[17] |maxEnrgyAndCellPrSampl2[17] | maxPhiPerSample       [17] ||                            |
    // | maxPhiPerSample     [18] ||numberNonEmptySamplings[18] |numberNonEmptySamplings[18] | nNES>phiPerSampleAux  [18] | phiPerSampleAux       [18] | phiPerSampleAux       [18] |                            |                            |maxEnrgyAndCellPrSampl2[18] |maxEnrgyAndCellPrSampl2[18] |maxEnrgyAndCellPrSampl2[18] | maxPhiPerSample       [18] ||                            |
    // | maxPhiPerSample     [19] ||numberNonEmptySamplings[19] |numberNonEmptySamplings[19] | nNES>phiPerSampleAux  [19] | phiPerSampleAux       [19] | phiPerSampleAux       [19] |                            |                            |maxEnrgyAndCellPrSampl2[19] |maxEnrgyAndCellPrSampl2[19] |maxEnrgyAndCellPrSampl2[19] | maxPhiPerSample       [19] ||                            |
    // | maxPhiPerSample     [20] ||numberNonEmptySamplings[20] |numberNonEmptySamplings[20] | nNES>phiPerSampleAux  [20] | phiPerSampleAux       [20] | phiPerSampleAux       [20] |                            |                            |maxEnrgyAndCellPrSampl2[20] |maxEnrgyAndCellPrSampl2[20] |maxEnrgyAndCellPrSampl2[20] | maxPhiPerSample       [20] ||                            |
    // | maxPhiPerSample     [21] ||numberNonEmptySamplings[21] |numberNonEmptySamplings[21] | nNES>phiPerSampleAux  [21] | phiPerSampleAux       [21] | phiPerSampleAux       [21] |                            |                            |maxEnrgyAndCellPrSampl2[21] |maxEnrgyAndCellPrSampl2[21] |maxEnrgyAndCellPrSampl2[21] | maxPhiPerSample       [21] ||                            |
    // | maxPhiPerSample     [22] ||numberNonEmptySamplings[22] |numberNonEmptySamplings[22] | nNES>phiPerSampleAux  [22] | phiPerSampleAux       [22] | phiPerSampleAux       [22] |                            |                            |maxEnrgyAndCellPrSampl2[22] |maxEnrgyAndCellPrSampl2[22] |maxEnrgyAndCellPrSampl2[22] | maxPhiPerSample       [22] ||                            |
    // | maxPhiPerSample     [23] ||numberNonEmptySamplings[23] |numberNonEmptySamplings[23] | nNES>phiPerSampleAux  [23] | phiPerSampleAux       [23] | phiPerSampleAux       [23] |                            |                            |maxEnrgyAndCellPrSampl2[23] |maxEnrgyAndCellPrSampl2[23] |maxEnrgyAndCellPrSampl2[23] | maxPhiPerSample       [23] ||                            |
    // | maxPhiPerSample     [24] ||numberNonEmptySamplings[24] |numberNonEmptySamplings[24] | nNES>phiPerSampleAux  [24] | phiPerSampleAux       [24] | phiPerSampleAux       [24] |                            |                            |maxEnrgyAndCellPrSampl2[24] |maxEnrgyAndCellPrSampl2[24] |maxEnrgyAndCellPrSampl2[24] | maxPhiPerSample       [24] ||                            |
    // | maxPhiPerSample     [25] ||numberNonEmptySamplings[25] |numberNonEmptySamplings[25] | nNES>phiPerSampleAux  [25] | phiPerSampleAux       [25] | phiPerSampleAux       [25] |                            |                            |maxEnrgyAndCellPrSampl2[25] |maxEnrgyAndCellPrSampl2[25] |maxEnrgyAndCellPrSampl2[25] | maxPhiPerSample       [25] ||                            |
    // | maxPhiPerSample     [26] ||numberNonEmptySamplings[26] |numberNonEmptySamplings[26] | nNES>phiPerSampleAux  [26] | phiPerSampleAux       [26] | phiPerSampleAux       [26] |                            |                            |maxEnrgyAndCellPrSampl2[26] |maxEnrgyAndCellPrSampl2[26] |maxEnrgyAndCellPrSampl2[26] | maxPhiPerSample       [26] ||                            |
    // | maxPhiPerSample     [27] ||numberNonEmptySamplings[27] |numberNonEmptySamplings[27] | nNES>phiPerSampleAux  [27] | phiPerSampleAux       [27] | phiPerSampleAux       [27] |                            |                            |maxEnrgyAndCellPrSampl2[27] |maxEnrgyAndCellPrSampl2[27] |maxEnrgyAndCellPrSampl2[27] | maxPhiPerSample       [27] ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | maxEtaPerSample     [00] ||maxMomentsEnergPrSample[00] |maxMomentsEnergPrSample[00] | mMEPS>etaPerSampleAux [00] | etaPerSampleAux       [00] | etaPerSampleAux       [00] |                            |                            |maxEnrgyAndCellPrSampl1[00] |maxEnrgyAndCellPrSampl1[00] |maxEnrgyAndCellPrSampl1[00] | maxEtaPerSample       [00] ||                            |
    // | maxEtaPerSample     [01] ||maxMomentsEnergPrSample[01] |maxMomentsEnergPrSample[01] | mMEPS>etaPerSampleAux [01] | etaPerSampleAux       [01] | etaPerSampleAux       [01] |                            |                            |maxEnrgyAndCellPrSampl1[01] |maxEnrgyAndCellPrSampl1[01] |maxEnrgyAndCellPrSampl1[01] | maxEtaPerSample       [01] ||                            |
    // | maxEtaPerSample     [02] ||maxMomentsEnergPrSample[02] |maxMomentsEnergPrSample[02] | mMEPS>etaPerSampleAux [02] | etaPerSampleAux       [02] | etaPerSampleAux       [02] |                            |                            |maxEnrgyAndCellPrSampl1[02] |maxEnrgyAndCellPrSampl1[02] |maxEnrgyAndCellPrSampl1[02] | maxEtaPerSample       [02] ||                            |
    // | maxEtaPerSample     [03] ||maxMomentsEnergPrSample[03] |maxMomentsEnergPrSample[03] | mMEPS>etaPerSampleAux [03] | etaPerSampleAux       [03] | etaPerSampleAux       [03] |                            |                            |maxEnrgyAndCellPrSampl1[03] |maxEnrgyAndCellPrSampl1[03] |maxEnrgyAndCellPrSampl1[03] | maxEtaPerSample       [03] ||                            |
    // | maxEtaPerSample     [04] ||maxMomentsEnergPrSample[04] |maxMomentsEnergPrSample[04] | mMEPS>etaPerSampleAux [04] | etaPerSampleAux       [04] | etaPerSampleAux       [04] |                            |                            |maxEnrgyAndCellPrSampl1[04] |maxEnrgyAndCellPrSampl1[04] |maxEnrgyAndCellPrSampl1[04] | maxEtaPerSample       [04] ||                            |
    // | maxEtaPerSample     [05] ||maxMomentsEnergPrSample[05] |maxMomentsEnergPrSample[05] | mMEPS>etaPerSampleAux [05] | etaPerSampleAux       [05] | etaPerSampleAux       [05] |                            |                            |maxEnrgyAndCellPrSampl1[05] |maxEnrgyAndCellPrSampl1[05] |maxEnrgyAndCellPrSampl1[05] | maxEtaPerSample       [05] ||                            |
    // | maxEtaPerSample     [06] ||maxMomentsEnergPrSample[06] |maxMomentsEnergPrSample[06] | mMEPS>etaPerSampleAux [06] | etaPerSampleAux       [06] | etaPerSampleAux       [06] |                            |                            |maxEnrgyAndCellPrSampl1[06] |maxEnrgyAndCellPrSampl1[06] |maxEnrgyAndCellPrSampl1[06] | maxEtaPerSample       [06] ||                            |
    // | maxEtaPerSample     [07] ||maxMomentsEnergPrSample[07] |maxMomentsEnergPrSample[07] | mMEPS>etaPerSampleAux [07] | etaPerSampleAux       [07] | etaPerSampleAux       [07] |                            |                            |maxEnrgyAndCellPrSampl1[07] |maxEnrgyAndCellPrSampl1[07] |maxEnrgyAndCellPrSampl1[07] | maxEtaPerSample       [07] ||                            |
    // | maxEtaPerSample     [08] ||maxMomentsEnergPrSample[08] |maxMomentsEnergPrSample[08] | mMEPS>etaPerSampleAux [08] | etaPerSampleAux       [08] | etaPerSampleAux       [08] |                            |                            |maxEnrgyAndCellPrSampl1[08] |maxEnrgyAndCellPrSampl1[08] |maxEnrgyAndCellPrSampl1[08] | maxEtaPerSample       [08] ||                            |
    // | maxEtaPerSample     [09] ||maxMomentsEnergPrSample[09] |maxMomentsEnergPrSample[09] | mMEPS>etaPerSampleAux [09] | etaPerSampleAux       [09] | etaPerSampleAux       [09] |                            |                            |maxEnrgyAndCellPrSampl1[09] |maxEnrgyAndCellPrSampl1[09] |maxEnrgyAndCellPrSampl1[09] | maxEtaPerSample       [09] ||                            |
    // | maxEtaPerSample     [10] ||maxMomentsEnergPrSample[10] |maxMomentsEnergPrSample[10] | mMEPS>etaPerSampleAux [10] | etaPerSampleAux       [10] | etaPerSampleAux       [10] |                            |                            |maxEnrgyAndCellPrSampl1[10] |maxEnrgyAndCellPrSampl1[10] |maxEnrgyAndCellPrSampl1[10] | maxEtaPerSample       [10] ||                            |
    // | maxEtaPerSample     [11] ||maxMomentsEnergPrSample[11] |maxMomentsEnergPrSample[11] | mMEPS>etaPerSampleAux [11] | etaPerSampleAux       [11] | etaPerSampleAux       [11] |                            |                            |maxEnrgyAndCellPrSampl1[11] |maxEnrgyAndCellPrSampl1[11] |maxEnrgyAndCellPrSampl1[11] | maxEtaPerSample       [11] ||                            |
    // | maxEtaPerSample     [12] ||maxMomentsEnergPrSample[12] |maxMomentsEnergPrSample[12] | mMEPS>etaPerSampleAux [12] | etaPerSampleAux       [12] | etaPerSampleAux       [12] |                            |                            |maxEnrgyAndCellPrSampl1[12] |maxEnrgyAndCellPrSampl1[12] |maxEnrgyAndCellPrSampl1[12] | maxEtaPerSample       [12] ||                            |
    // | maxEtaPerSample     [13] ||maxMomentsEnergPrSample[13] |maxMomentsEnergPrSample[13] | mMEPS>etaPerSampleAux [13] | etaPerSampleAux       [13] | etaPerSampleAux       [13] |                            |                            |maxEnrgyAndCellPrSampl1[13] |maxEnrgyAndCellPrSampl1[13] |maxEnrgyAndCellPrSampl1[13] | maxEtaPerSample       [13] ||                            |
    // | maxEtaPerSample     [14] ||maxMomentsEnergPrSample[14] |maxMomentsEnergPrSample[14] | mMEPS>etaPerSampleAux [14] | etaPerSampleAux       [14] | etaPerSampleAux       [14] |                            |                            |maxEnrgyAndCellPrSampl1[14] |maxEnrgyAndCellPrSampl1[14] |maxEnrgyAndCellPrSampl1[14] | maxEtaPerSample       [14] ||                            |
    // | maxEtaPerSample     [15] ||maxMomentsEnergPrSample[15] |maxMomentsEnergPrSample[15] | mMEPS>etaPerSampleAux [15] | etaPerSampleAux       [15] | etaPerSampleAux       [15] |                            |                            |maxEnrgyAndCellPrSampl1[15] |maxEnrgyAndCellPrSampl1[15] |maxEnrgyAndCellPrSampl1[15] | maxEtaPerSample       [15] ||                            |
    // | maxEtaPerSample     [16] ||maxMomentsEnergPrSample[16] |maxMomentsEnergPrSample[16] | mMEPS>etaPerSampleAux [16] | etaPerSampleAux       [16] | etaPerSampleAux       [16] |                            |                            |maxEnrgyAndCellPrSampl1[16] |maxEnrgyAndCellPrSampl1[16] |maxEnrgyAndCellPrSampl1[16] | maxEtaPerSample       [16] ||                            |
    // | maxEtaPerSample     [17] ||maxMomentsEnergPrSample[17] |maxMomentsEnergPrSample[17] | mMEPS>etaPerSampleAux [17] | etaPerSampleAux       [17] | etaPerSampleAux       [17] |                            |                            |maxEnrgyAndCellPrSampl1[17] |maxEnrgyAndCellPrSampl1[17] |maxEnrgyAndCellPrSampl1[17] | maxEtaPerSample       [17] ||                            |
    // | maxEtaPerSample     [18] ||maxMomentsEnergPrSample[18] |maxMomentsEnergPrSample[18] | mMEPS>etaPerSampleAux [18] | etaPerSampleAux       [18] | etaPerSampleAux       [18] |                            |                            |maxEnrgyAndCellPrSampl1[18] |maxEnrgyAndCellPrSampl1[18] |maxEnrgyAndCellPrSampl1[18] | maxEtaPerSample       [18] ||                            |
    // | maxEtaPerSample     [19] ||maxMomentsEnergPrSample[19] |maxMomentsEnergPrSample[19] | mMEPS>etaPerSampleAux [19] | etaPerSampleAux       [19] | etaPerSampleAux       [19] |                            |                            |maxEnrgyAndCellPrSampl1[19] |maxEnrgyAndCellPrSampl1[19] |maxEnrgyAndCellPrSampl1[19] | maxEtaPerSample       [19] ||                            |
    // | maxEtaPerSample     [20] ||maxMomentsEnergPrSample[20] |maxMomentsEnergPrSample[20] | mMEPS>etaPerSampleAux [20] | etaPerSampleAux       [20] | etaPerSampleAux       [20] |                            |                            |maxEnrgyAndCellPrSampl1[20] |maxEnrgyAndCellPrSampl1[20] |maxEnrgyAndCellPrSampl1[20] | maxEtaPerSample       [20] ||                            |
    // | maxEtaPerSample     [21] ||maxMomentsEnergPrSample[21] |maxMomentsEnergPrSample[21] | mMEPS>etaPerSampleAux [21] | etaPerSampleAux       [21] | etaPerSampleAux       [21] |                            |                            |maxEnrgyAndCellPrSampl1[21] |maxEnrgyAndCellPrSampl1[21] |maxEnrgyAndCellPrSampl1[21] | maxEtaPerSample       [21] ||                            |
    // | maxEtaPerSample     [22] ||maxMomentsEnergPrSample[22] |maxMomentsEnergPrSample[22] | mMEPS>etaPerSampleAux [22] | etaPerSampleAux       [22] | etaPerSampleAux       [22] |                            |                            |maxEnrgyAndCellPrSampl1[22] |maxEnrgyAndCellPrSampl1[22] |maxEnrgyAndCellPrSampl1[22] | maxEtaPerSample       [22] ||                            |
    // | maxEtaPerSample     [23] ||maxMomentsEnergPrSample[23] |maxMomentsEnergPrSample[23] | mMEPS>etaPerSampleAux [23] | etaPerSampleAux       [23] | etaPerSampleAux       [23] |                            |                            |maxEnrgyAndCellPrSampl1[23] |maxEnrgyAndCellPrSampl1[23] |maxEnrgyAndCellPrSampl1[23] | maxEtaPerSample       [23] ||                            |
    // | maxEtaPerSample     [24] ||maxMomentsEnergPrSample[24] |maxMomentsEnergPrSample[24] | mMEPS>etaPerSampleAux [24] | etaPerSampleAux       [24] | etaPerSampleAux       [24] |                            |                            |maxEnrgyAndCellPrSampl1[24] |maxEnrgyAndCellPrSampl1[24] |maxEnrgyAndCellPrSampl1[24] | maxEtaPerSample       [24] ||                            |
    // | maxEtaPerSample     [25] ||maxMomentsEnergPrSample[25] |maxMomentsEnergPrSample[25] | mMEPS>etaPerSampleAux [25] | etaPerSampleAux       [25] | etaPerSampleAux       [25] |                            |                            |maxEnrgyAndCellPrSampl1[25] |maxEnrgyAndCellPrSampl1[25] |maxEnrgyAndCellPrSampl1[25] | maxEtaPerSample       [25] ||                            |
    // | maxEtaPerSample     [26] ||maxMomentsEnergPrSample[26] |maxMomentsEnergPrSample[26] | mMEPS>etaPerSampleAux [26] | etaPerSampleAux       [26] | etaPerSampleAux       [26] |                            |                            |maxEnrgyAndCellPrSampl1[26] |maxEnrgyAndCellPrSampl1[26] |maxEnrgyAndCellPrSampl1[26] | maxEtaPerSample       [26] ||                            |
    // | maxEtaPerSample     [27] ||maxMomentsEnergPrSample[27] |maxMomentsEnergPrSample[27] | mMEPS>etaPerSampleAux [27] | etaPerSampleAux       [27] | etaPerSampleAux       [27] |                            |                            |maxEnrgyAndCellPrSampl1[27] |maxEnrgyAndCellPrSampl1[27] |maxEnrgyAndCellPrSampl1[27] | maxEtaPerSample       [27] ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | etaPerSample        [00] || numberEmptySamplings  [00] | numberEmptySamplings  [00] | nuES>etaPerSample     [00] | etaPerSample          [00] | etaPerSample          [00] | etaPerSample          [00] | etaPerSample          [00] | etaPerSample          [00] | etaPerSample          [00] | etaPerSample          [00] | etaPerSample          [00] ||                            |
    // | etaPerSample        [01] || numberEmptySamplings  [01] | numberEmptySamplings  [01] | nuES>etaPerSample     [01] | etaPerSample          [01] | etaPerSample          [01] | etaPerSample          [01] | etaPerSample          [01] | etaPerSample          [01] | etaPerSample          [01] | etaPerSample          [01] | etaPerSample          [01] ||                            |
    // | etaPerSample        [02] || numberEmptySamplings  [02] | numberEmptySamplings  [02] | nuES>etaPerSample     [02] | etaPerSample          [02] | etaPerSample          [02] | etaPerSample          [02] | etaPerSample          [02] | etaPerSample          [02] | etaPerSample          [02] | etaPerSample          [02] | etaPerSample          [02] ||                            |
    // | etaPerSample        [03] || numberEmptySamplings  [03] | numberEmptySamplings  [03] | nuES>etaPerSample     [03] | etaPerSample          [03] | etaPerSample          [03] | etaPerSample          [03] | etaPerSample          [03] | etaPerSample          [03] | etaPerSample          [03] | etaPerSample          [03] | etaPerSample          [03] ||                            |
    // | etaPerSample        [04] || numberEmptySamplings  [04] | numberEmptySamplings  [04] | nuES>etaPerSample     [04] | etaPerSample          [04] | etaPerSample          [04] | etaPerSample          [04] | etaPerSample          [04] | etaPerSample          [04] | etaPerSample          [04] | etaPerSample          [04] | etaPerSample          [04] ||                            |
    // | etaPerSample        [05] || numberEmptySamplings  [05] | numberEmptySamplings  [05] | nuES>etaPerSample     [05] | etaPerSample          [05] | etaPerSample          [05] | etaPerSample          [05] | etaPerSample          [05] | etaPerSample          [05] | etaPerSample          [05] | etaPerSample          [05] | etaPerSample          [05] ||                            |
    // | etaPerSample        [06] || numberEmptySamplings  [06] | numberEmptySamplings  [06] | nuES>etaPerSample     [06] | etaPerSample          [06] | etaPerSample          [06] | etaPerSample          [06] | etaPerSample          [06] | etaPerSample          [06] | etaPerSample          [06] | etaPerSample          [06] | etaPerSample          [06] ||                            |
    // | etaPerSample        [07] || numberEmptySamplings  [07] | numberEmptySamplings  [07] | nuES>etaPerSample     [07] | etaPerSample          [07] | etaPerSample          [07] | etaPerSample          [07] | etaPerSample          [07] | etaPerSample          [07] | etaPerSample          [07] | etaPerSample          [07] | etaPerSample          [07] ||                            |
    // | etaPerSample        [08] || numberEmptySamplings  [08] | numberEmptySamplings  [08] | nuES>etaPerSample     [08] | etaPerSample          [08] | etaPerSample          [08] | etaPerSample          [08] | etaPerSample          [08] | etaPerSample          [08] | etaPerSample          [08] | etaPerSample          [08] | etaPerSample          [08] ||                            |
    // | etaPerSample        [09] || numberEmptySamplings  [09] | numberEmptySamplings  [09] | nuES>etaPerSample     [09] | etaPerSample          [09] | etaPerSample          [09] | etaPerSample          [09] | etaPerSample          [09] | etaPerSample          [09] | etaPerSample          [09] | etaPerSample          [09] | etaPerSample          [09] ||                            |
    // | etaPerSample        [10] || numberEmptySamplings  [10] | numberEmptySamplings  [10] | nuES>etaPerSample     [10] | etaPerSample          [10] | etaPerSample          [10] | etaPerSample          [10] | etaPerSample          [10] | etaPerSample          [10] | etaPerSample          [10] | etaPerSample          [10] | etaPerSample          [10] ||                            |
    // | etaPerSample        [11] || numberEmptySamplings  [11] | numberEmptySamplings  [11] | nuES>etaPerSample     [11] | etaPerSample          [11] | etaPerSample          [11] | etaPerSample          [11] | etaPerSample          [11] | etaPerSample          [11] | etaPerSample          [11] | etaPerSample          [11] | etaPerSample          [11] ||                            |
    // | etaPerSample        [12] || numberEmptySamplings  [12] | numberEmptySamplings  [12] | nuES>etaPerSample     [12] | etaPerSample          [12] | etaPerSample          [12] | etaPerSample          [12] | etaPerSample          [12] | etaPerSample          [12] | etaPerSample          [12] | etaPerSample          [12] | etaPerSample          [12] ||                            |
    // | etaPerSample        [13] || numberEmptySamplings  [13] | numberEmptySamplings  [13] | nuES>etaPerSample     [13] | etaPerSample          [13] | etaPerSample          [13] | etaPerSample          [13] | etaPerSample          [13] | etaPerSample          [13] | etaPerSample          [13] | etaPerSample          [13] | etaPerSample          [13] ||                            |
    // | etaPerSample        [14] || numberEmptySamplings  [14] | numberEmptySamplings  [14] | nuES>etaPerSample     [14] | etaPerSample          [14] | etaPerSample          [14] | etaPerSample          [14] | etaPerSample          [14] | etaPerSample          [14] | etaPerSample          [14] | etaPerSample          [14] | etaPerSample          [14] ||                            |
    // | etaPerSample        [15] || numberEmptySamplings  [15] | numberEmptySamplings  [15] | nuES>etaPerSample     [15] | etaPerSample          [15] | etaPerSample          [15] | etaPerSample          [15] | etaPerSample          [15] | etaPerSample          [15] | etaPerSample          [15] | etaPerSample          [15] | etaPerSample          [15] ||                            |
    // | etaPerSample        [16] || numberEmptySamplings  [16] | numberEmptySamplings  [16] | nuES>etaPerSample     [16] | etaPerSample          [16] | etaPerSample          [16] | etaPerSample          [16] | etaPerSample          [16] | etaPerSample          [16] | etaPerSample          [16] | etaPerSample          [16] | etaPerSample          [16] ||                            |
    // | etaPerSample        [17] || numberEmptySamplings  [17] | numberEmptySamplings  [17] | nuES>etaPerSample     [17] | etaPerSample          [17] | etaPerSample          [17] | etaPerSample          [17] | etaPerSample          [17] | etaPerSample          [17] | etaPerSample          [17] | etaPerSample          [17] | etaPerSample          [17] ||                            |
    // | etaPerSample        [18] || numberEmptySamplings  [18] | numberEmptySamplings  [18] | nuES>etaPerSample     [18] | etaPerSample          [18] | etaPerSample          [18] | etaPerSample          [18] | etaPerSample          [18] | etaPerSample          [18] | etaPerSample          [18] | etaPerSample          [18] | etaPerSample          [18] ||                            |
    // | etaPerSample        [19] || numberEmptySamplings  [19] | numberEmptySamplings  [19] | nuES>etaPerSample     [19] | etaPerSample          [19] | etaPerSample          [19] | etaPerSample          [19] | etaPerSample          [19] | etaPerSample          [19] | etaPerSample          [19] | etaPerSample          [19] | etaPerSample          [19] ||                            |
    // | etaPerSample        [20] || numberEmptySamplings  [20] | numberEmptySamplings  [20] | nuES>etaPerSample     [20] | etaPerSample          [20] | etaPerSample          [20] | etaPerSample          [20] | etaPerSample          [20] | etaPerSample          [20] | etaPerSample          [20] | etaPerSample          [20] | etaPerSample          [20] ||                            |
    // | etaPerSample        [21] || numberEmptySamplings  [21] | numberEmptySamplings  [21] | nuES>etaPerSample     [21] | etaPerSample          [21] | etaPerSample          [21] | etaPerSample          [21] | etaPerSample          [21] | etaPerSample          [21] | etaPerSample          [21] | etaPerSample          [21] | etaPerSample          [21] ||                            |
    // | etaPerSample        [22] || numberEmptySamplings  [22] | numberEmptySamplings  [22] | nuES>etaPerSample     [22] | etaPerSample          [22] | etaPerSample          [22] | etaPerSample          [22] | etaPerSample          [22] | etaPerSample          [22] | etaPerSample          [22] | etaPerSample          [22] | etaPerSample          [22] ||                            |
    // | etaPerSample        [23] || numberEmptySamplings  [23] | numberEmptySamplings  [23] | nuES>etaPerSample     [23] | etaPerSample          [23] | etaPerSample          [23] | etaPerSample          [23] | etaPerSample          [23] | etaPerSample          [23] | etaPerSample          [23] | etaPerSample          [23] | etaPerSample          [23] ||                            |
    // | etaPerSample        [24] || numberEmptySamplings  [24] | numberEmptySamplings  [24] | nuES>etaPerSample     [24] | etaPerSample          [24] | etaPerSample          [24] | etaPerSample          [24] | etaPerSample          [24] | etaPerSample          [24] | etaPerSample          [24] | etaPerSample          [24] | etaPerSample          [24] ||                            |
    // | etaPerSample        [25] || numberEmptySamplings  [25] | numberEmptySamplings  [25] | nuES>etaPerSample     [25] | etaPerSample          [25] | etaPerSample          [25] | etaPerSample          [25] | etaPerSample          [25] | etaPerSample          [25] | etaPerSample          [25] | etaPerSample          [25] | etaPerSample          [25] ||                            |
    // | etaPerSample        [26] || numberEmptySamplings  [26] | numberEmptySamplings  [26] | nuES>etaPerSample     [26] | etaPerSample          [26] | etaPerSample          [26] | etaPerSample          [26] | etaPerSample          [26] | etaPerSample          [26] | etaPerSample          [26] | etaPerSample          [26] | etaPerSample          [26] ||                            |
    // | etaPerSample        [27] || numberEmptySamplings  [27] | numberEmptySamplings  [27] | nuES>etaPerSample     [27] | etaPerSample          [27] | etaPerSample          [27] | etaPerSample          [27] | etaPerSample          [27] | etaPerSample          [27] | etaPerSample          [27] | etaPerSample          [27] | etaPerSample          [27] ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | phiPerSample        [00] ||                            |                            | phiPerSample          [00] | phiPerSample          [00] | phiPerSample          [00] | phiPerSample          [00] | phiPerSample          [00] | phiPerSample          [00] | phiPerSample          [00] | phiPerSample          [00] | phiPerSample          [00] ||                            |
    // | phiPerSample        [01] ||                            |                            | phiPerSample          [01] | phiPerSample          [01] | phiPerSample          [01] | phiPerSample          [01] | phiPerSample          [01] | phiPerSample          [01] | phiPerSample          [01] | phiPerSample          [01] | phiPerSample          [01] ||                            |
    // | phiPerSample        [02] ||                            |                            | phiPerSample          [02] | phiPerSample          [02] | phiPerSample          [02] | phiPerSample          [02] | phiPerSample          [02] | phiPerSample          [02] | phiPerSample          [02] | phiPerSample          [02] | phiPerSample          [02] ||                            |
    // | phiPerSample        [03] ||                            |                            | phiPerSample          [03] | phiPerSample          [03] | phiPerSample          [03] | phiPerSample          [03] | phiPerSample          [03] | phiPerSample          [03] | phiPerSample          [03] | phiPerSample          [03] | phiPerSample          [03] ||                            |
    // | phiPerSample        [04] ||                            |                            | phiPerSample          [04] | phiPerSample          [04] | phiPerSample          [04] | phiPerSample          [04] | phiPerSample          [04] | phiPerSample          [04] | phiPerSample          [04] | phiPerSample          [04] | phiPerSample          [04] ||                            |
    // | phiPerSample        [05] ||                            |                            | phiPerSample          [05] | phiPerSample          [05] | phiPerSample          [05] | phiPerSample          [05] | phiPerSample          [05] | phiPerSample          [05] | phiPerSample          [05] | phiPerSample          [05] | phiPerSample          [05] ||                            |
    // | phiPerSample        [06] ||                            |                            | phiPerSample          [06] | phiPerSample          [06] | phiPerSample          [06] | phiPerSample          [06] | phiPerSample          [06] | phiPerSample          [06] | phiPerSample          [06] | phiPerSample          [06] | phiPerSample          [06] ||                            |
    // | phiPerSample        [07] ||                            |                            | phiPerSample          [07] | phiPerSample          [07] | phiPerSample          [07] | phiPerSample          [07] | phiPerSample          [07] | phiPerSample          [07] | phiPerSample          [07] | phiPerSample          [07] | phiPerSample          [07] ||                            |
    // | phiPerSample        [08] ||                            |                            | phiPerSample          [08] | phiPerSample          [08] | phiPerSample          [08] | phiPerSample          [08] | phiPerSample          [08] | phiPerSample          [08] | phiPerSample          [08] | phiPerSample          [08] | phiPerSample          [08] ||                            |
    // | phiPerSample        [09] ||                            |                            | phiPerSample          [09] | phiPerSample          [09] | phiPerSample          [09] | phiPerSample          [09] | phiPerSample          [09] | phiPerSample          [09] | phiPerSample          [09] | phiPerSample          [09] | phiPerSample          [09] ||                            |
    // | phiPerSample        [10] ||                            |                            | phiPerSample          [10] | phiPerSample          [10] | phiPerSample          [10] | phiPerSample          [10] | phiPerSample          [10] | phiPerSample          [10] | phiPerSample          [10] | phiPerSample          [10] | phiPerSample          [10] ||                            |
    // | phiPerSample        [11] ||                            |                            | phiPerSample          [11] | phiPerSample          [11] | phiPerSample          [11] | phiPerSample          [11] | phiPerSample          [11] | phiPerSample          [11] | phiPerSample          [11] | phiPerSample          [11] | phiPerSample          [11] ||                            |
    // | phiPerSample        [12] ||                            |                            | phiPerSample          [12] | phiPerSample          [12] | phiPerSample          [12] | phiPerSample          [12] | phiPerSample          [12] | phiPerSample          [12] | phiPerSample          [12] | phiPerSample          [12] | phiPerSample          [12] ||                            |
    // | phiPerSample        [13] ||                            |                            | phiPerSample          [13] | phiPerSample          [13] | phiPerSample          [13] | phiPerSample          [13] | phiPerSample          [13] | phiPerSample          [13] | phiPerSample          [13] | phiPerSample          [13] | phiPerSample          [13] ||                            |
    // | phiPerSample        [14] ||                            |                            | phiPerSample          [14] | phiPerSample          [14] | phiPerSample          [14] | phiPerSample          [14] | phiPerSample          [14] | phiPerSample          [14] | phiPerSample          [14] | phiPerSample          [14] | phiPerSample          [14] ||                            |
    // | phiPerSample        [15] ||                            |                            | phiPerSample          [15] | phiPerSample          [15] | phiPerSample          [15] | phiPerSample          [15] | phiPerSample          [15] | phiPerSample          [15] | phiPerSample          [15] | phiPerSample          [15] | phiPerSample          [15] ||                            |
    // | phiPerSample        [16] ||                            |                            | phiPerSample          [16] | phiPerSample          [16] | phiPerSample          [16] | phiPerSample          [16] | phiPerSample          [16] | phiPerSample          [16] | phiPerSample          [16] | phiPerSample          [16] | phiPerSample          [16] ||                            |
    // | phiPerSample        [17] ||                            |                            | phiPerSample          [17] | phiPerSample          [17] | phiPerSample          [17] | phiPerSample          [17] | phiPerSample          [17] | phiPerSample          [17] | phiPerSample          [17] | phiPerSample          [17] | phiPerSample          [17] ||                            |
    // | phiPerSample        [18] ||                            |                            | phiPerSample          [18] | phiPerSample          [18] | phiPerSample          [18] | phiPerSample          [18] | phiPerSample          [18] | phiPerSample          [18] | phiPerSample          [18] | phiPerSample          [18] | phiPerSample          [18] ||                            |
    // | phiPerSample        [19] ||                            |                            | phiPerSample          [19] | phiPerSample          [19] | phiPerSample          [19] | phiPerSample          [19] | phiPerSample          [19] | phiPerSample          [19] | phiPerSample          [19] | phiPerSample          [19] | phiPerSample          [19] ||                            |
    // | phiPerSample        [20] ||                            |                            | phiPerSample          [20] | phiPerSample          [20] | phiPerSample          [20] | phiPerSample          [20] | phiPerSample          [20] | phiPerSample          [20] | phiPerSample          [20] | phiPerSample          [20] | phiPerSample          [20] ||                            |
    // | phiPerSample        [21] ||                            |                            | phiPerSample          [21] | phiPerSample          [21] | phiPerSample          [21] | phiPerSample          [21] | phiPerSample          [21] | phiPerSample          [21] | phiPerSample          [21] | phiPerSample          [21] | phiPerSample          [21] ||                            |
    // | phiPerSample        [22] ||                            |                            | phiPerSample          [22] | phiPerSample          [22] | phiPerSample          [22] | phiPerSample          [22] | phiPerSample          [22] | phiPerSample          [22] | phiPerSample          [22] | phiPerSample          [22] | phiPerSample          [22] ||                            |
    // | phiPerSample        [23] ||                            |                            | phiPerSample          [23] | phiPerSample          [23] | phiPerSample          [23] | phiPerSample          [23] | phiPerSample          [23] | phiPerSample          [23] | phiPerSample          [23] | phiPerSample          [23] | phiPerSample          [23] ||                            |
    // | phiPerSample        [24] ||                            |                            | phiPerSample          [24] | phiPerSample          [24] | phiPerSample          [24] | phiPerSample          [24] | phiPerSample          [24] | phiPerSample          [24] | phiPerSample          [24] | phiPerSample          [24] | phiPerSample          [24] ||                            |
    // | phiPerSample        [25] ||                            |                            | phiPerSample          [25] | phiPerSample          [25] | phiPerSample          [25] | phiPerSample          [25] | phiPerSample          [25] | phiPerSample          [25] | phiPerSample          [25] | phiPerSample          [25] | phiPerSample          [25] ||                            |
    // | phiPerSample        [26] ||                            |                            | phiPerSample          [26] | phiPerSample          [26] | phiPerSample          [26] | phiPerSample          [26] | phiPerSample          [26] | phiPerSample          [26] | phiPerSample          [26] | phiPerSample          [26] | phiPerSample          [26] ||                            |
    // | phiPerSample        [27] ||                            |                            | phiPerSample          [27] | phiPerSample          [27] | phiPerSample          [27] | phiPerSample          [27] | phiPerSample          [27] | phiPerSample          [27] | phiPerSample          [27] | phiPerSample          [27] | phiPerSample          [27] ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | time                     ||                            |                            |                            |                            | time                       | time                       | time                       | time                       | time                       | time                       | time                       ||                            |
    // | firstPhi                 ||                            |                            | firstPhi                   | firstPhi                   | firstPhi                   | firstPhi                   | firstPhi                   | firstPhi                   | firstPhi                   | firstPhi                   | firstPhi                   ||                            |
    // | firstEta                 ||                            |                            | firstEta                   | firstEta                   | firstEta                   | firstEta                   | firstEta                   | firstEta                   | firstEta                   | firstEta                   | firstEta                   ||                            |
    // | secondR                  ||                            |                            | firstPhiAux                | firstPhiAux                | firstPhiAux -> matrix11    | matrix11                   | matrix11                   | secondR                    | secondR                    | secondR                    | secondR                    ||                            |
    // | secondLambda             ||                            |                            |energyDensityNormlizationAux|energyDensityNormlizationAux|EDnstNormAux>avgLArQNormAux | averageLArQNormAux         | averageLArQNormAux         |avgLArQNormAux>secondLambda | secondLambda               | secondLambda               | secondLambda               ||                            |
    // | deltaPhi                 ||                            |                            |                            |                            | matrix10Aux                | matrix10Aux                | matrix10Aux -> deltaPhi    | deltaPhi                   | deltaPhi                   | deltaPhi                   | deltaPhi                   ||                            |
    // | deltaTheta               ||                            |                            |                            |                            | matrix20Aux                | matrix20Aux                | matrix20Aux -> deltaTheta  | deltaTheta                 | deltaTheta                 | deltaTheta                 | deltaTheta                 ||                            |
    // | deltaAlpha               ||                            |                            |                            |                            | matrix21Aux                | matrix21Aux                | matrix21Aux -> deltaAlpha  | deltaAlpha                 | deltaAlpha                 | deltaAlpha                 | deltaAlpha                 ||                            |
    // | centerX                  ||                            |                            | centerX                    | centerX                    | centerX                    | centerX                    | centerX                    | centerX                    | centerX                    | centerX                    | centerX                    ||                            |
    // | centerY                  ||                            |                            | centerY                    | centerY                    | centerY                    | centerY                    | centerY                    | centerY                    | centerY                    | centerY                    | centerY                    ||                            |
    // | centerZ                  ||                            |                            | centerZ                    | centerZ                    | centerZ                    | centerZ                    | centerZ                    | centerZ                    | centerZ                    | centerZ                    | centerZ                    ||                            |
    // | centerMag                ||                            |                            | engFracEMAux               | engFracEMAux               | engFracEMAux               |                            | centerMag                  | centerMag                  | centerMag                  | centerMag                  | centerMag                  ||                            |
    // | centerLambda             ||                            |                            |                            |                            | badCellsCorrEAux           | badCellsCorrEAux           | badCellsCorrEAux           | badCellsCorrEAux           |                            |                            | centerLambda               ||                            |
    // | lateral                  ||                            |                            | secondEngDensAux           | secondEngDensAux           | secondEngDensAux           |                            |                            | lateral                    | lateral                    | lateral                    | lateral                    ||                            |
    // | longitudinal             ||                            |                            | firstEtaAux                | firstEtaAux                | firstEtaAux -> matrix00    | matrix00                   | matrix00                   | longitudinal               | longitudinal               | longitudinal               | longitudinal               ||                            |
    // | engFracEM                ||                            |                            | engFracEM                  | engFracEM                  | engFracEM                  | engFracEM                  | engFracEM                  | engFracEM                  | engFracEM                  | engFracEM                  | engFracEM                  ||                            |
    // | engFracMax               ||                            |                            |                            |                            | engFracMax                 | engFracMax                 | engFracMax                 | engFracMax                 | engFracMax                 | engFracMax                 | engFracMax                 ||                            |
    // | engFracCore              || engFracCore                | engFracCore                | engFracCore                | engFracCore                | engFracCore                | engFracCore                | engFracCore                | engFracCore                | engFracCore                | engFracCore                | engFracCore                ||                            |
    // | firstEngDens             ||                            |                            | firstEngDens               | firstEngDens               | firstEngDens               | firstEngDens               | firstEngDens               | firstEngDens               | firstEngDens               | firstEngDens               | firstEngDens               ||                            |
    // | secondEngDens            ||                            |                            | secondEngDens              | secondEngDens              | secondEngDens              | secondEngDens              | secondEngDens              | secondEngDens              | secondEngDens              | secondEngDens              | secondEngDens              ||                            |
    // | isolation                ||                            |                            | isolation                  | isolation                  | isolation                  | isolation                  | isolation                  | isolation                  | isolation                  | isolation                  | isolation                  ||                            |
    // | engBadCells              ||                            |                            |                            |                            | engBadCells                | engBadCells                | engBadCells                | engBadCells                | engBadCells                | engBadCells                | engBadCells                ||                            |
    // | nBadCells                ||                            |                            |                            |                            | nBadCells                  | nBadCells                  | nBadCells                  | nBadCells                  | nBadCells                  | nBadCells                  | nBadCells                  ||                            |
    // | nBadCellsCorr            ||                            |                            |                            |                            | nBadCellsCorr              | nBadCellsCorr              | nBadCellsCorr              | nBadCellsCorr              | nBadCellsCorr              | nBadCellsCorr              | nBadCellsCorr              ||                            |
    // | badCellsCorrE            ||                            |                            |                            |                            | badCellsCorrE              | badCellsCorrE              | badCellsCorrE              | badCellsCorrE              | badCellsCorrE              | badCellsCorrE              | badCellsCorrE              ||                            |
    // | badLArQFrac              ||                            |                            |                            |                            | badLArQFrac                | badLArQFrac                | badLArQFrac                | badLArQFrac                | badLArQFrac                | badLArQFrac                | badLArQFrac                ||                            |
    // | engPos                   || engPos                     | engPos                     | engPos                     | engPos                     | engPos                     | engPos                     | engPos                     | engPos                     | engPos                     | engPos                     | engPos                     ||                            |
    // | significance             ||                            |                            |                            |                            | significance               | significance               | significance               | significance               | significance               | significance               | significance               ||                            |
    // | cellSignificance         ||                            |                            | mYAux                      | mYAux                      | mYAux                      |                            |                            | cellSignificance           | cellSignificance           | cellSignificance           | cellSignificance           ||                            |
    // | cellSigSampling          ||                            |                            | mZAux                      | mZAux                      | mZAux                      |                            |                            | cellSigSampling            | cellSigSampling            | cellSigSampling            | cellSigSampling            ||                            |
    // | avgLArQ                  ||                            |                            |                            |                            | avgLArQ                    | avgLArQ                    | avgLArQ                    | avgLArQ                    | avgLArQ                    | avgLArQ                    | avgLArQ                    ||                            |
    // | avgTileQ                 ||                            |                            |                            |                            | avgTileQ                   | avgTileQ                   | avgTileQ                   | avgTileQ                   | avgTileQ                   | avgTileQ                   | avgTileQ                   ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | engBadHVCells            ||                            | engPosAux                  | engPosAux                  |                            | timeNormalizationAux       | timeNormalizationAux       | timeNormalizationAux       | timeNormalizationAux       |                            |                            |                            || calculated on the CPU...   |
    // | nBadHVCells              ||                            |                            |                            |                            | significanceAux            | significanceAux            | significanceAux            | significanceAux            |                            |                            |                            || calculated on the CPU...   |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | PTD                      ||                            |                            | mX                         | mX                         | mX -> PTD                  | PTD                        | PTD                        | PTD                        | PTD                        | PTD                        | PTD                        ||                            |
    // | mass                     ||                            |                            | mXAux                      | mXAux                      | mXAux -> mass              | mass                       | mass                       | mass                       | mass                       | mass                       | mass                       ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | EMProbability            ||                            |                            | centerXAux                 | centerXAux                 | centerXAux -> matrix00Aux  | matrix00Aux                | matrix00Aux -> showerAxisX | showerAxisX                | showerAxisX                | showerAxisX                | showerAxisX                ||                            |
    // | hadWeight                ||                            |                            | centerYAux                 | centerYAux                 | centerYAux -> matrix11Aux  | matrix11Aux                | matrix11Aux -> showerAxisY | showerAxisY                | showerAxisY                | showerAxisY                | showerAxisY                ||                            |
    // | OOCweight                ||                            |                            | centerZAux                 | centerZAux                 | centerZAux -> matrix22Aux  | matrix22Aux                | matrix22Aux -> showerAxisZ | showerAxisZ                | showerAxisZ                | showerAxisZ                | showerAxisZ                ||                            |
    // | DMweight                 ||                            |                            | clusterEnergyAux           | clusterEnergyAux           | clusterEnergyAux           |                            |                            | lateralNormalization       | lateralNormalization       | lateralNormalization       |                            ||                            |
    // | tileConfidenceLevel      ||                            |                            | clusterPhiAux              | clusterPhiAux              | clusterPhiAux -> matrix22  | matrix22                   | matrix22                   | lateralNormalizationAux    | lateralNormalizationAux    | lateralNormalizationAux    |                            ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | secondTime               ||                            |                            |                            |                            | secondTime                 | secondTime                 | secondTime                 | secondTime                 | secondTime                 | secondTime                 | secondTime                 ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | nCellSampling       [00] ||                            |                            | nCellSampling         [00] | nCellSampling         [00] | nCellSampling         [00] | nCellSampling         [00] | nCellSampling         [00] | nCellSampling         [00] | nCellSampling         [00] | nCellSampling         [00] | nCellSampling         [00] ||                            |
    // | nCellSampling       [01] ||                            |                            | nCellSampling         [01] | nCellSampling         [01] | nCellSampling         [01] | nCellSampling         [01] | nCellSampling         [01] | nCellSampling         [01] | nCellSampling         [01] | nCellSampling         [01] | nCellSampling         [01] ||                            |
    // | nCellSampling       [02] ||                            |                            | nCellSampling         [02] | nCellSampling         [02] | nCellSampling         [02] | nCellSampling         [02] | nCellSampling         [02] | nCellSampling         [02] | nCellSampling         [02] | nCellSampling         [02] | nCellSampling         [02] ||                            |
    // | nCellSampling       [03] ||                            |                            | nCellSampling         [03] | nCellSampling         [03] | nCellSampling         [03] | nCellSampling         [03] | nCellSampling         [03] | nCellSampling         [03] | nCellSampling         [03] | nCellSampling         [03] | nCellSampling         [03] ||                            |
    // | nCellSampling       [04] ||                            |                            | nCellSampling         [04] | nCellSampling         [04] | nCellSampling         [04] | nCellSampling         [04] | nCellSampling         [04] | nCellSampling         [04] | nCellSampling         [04] | nCellSampling         [04] | nCellSampling         [04] ||                            |
    // | nCellSampling       [05] ||                            |                            | nCellSampling         [05] | nCellSampling         [05] | nCellSampling         [05] | nCellSampling         [05] | nCellSampling         [05] | nCellSampling         [05] | nCellSampling         [05] | nCellSampling         [05] | nCellSampling         [05] ||                            |
    // | nCellSampling       [06] ||                            |                            | nCellSampling         [06] | nCellSampling         [06] | nCellSampling         [06] | nCellSampling         [06] | nCellSampling         [06] | nCellSampling         [06] | nCellSampling         [06] | nCellSampling         [06] | nCellSampling         [06] ||                            |
    // | nCellSampling       [07] ||                            |                            | nCellSampling         [07] | nCellSampling         [07] | nCellSampling         [07] | nCellSampling         [07] | nCellSampling         [07] | nCellSampling         [07] | nCellSampling         [07] | nCellSampling         [07] | nCellSampling         [07] ||                            |
    // | nCellSampling       [08] ||                            |                            | nCellSampling         [08] | nCellSampling         [08] | nCellSampling         [08] | nCellSampling         [08] | nCellSampling         [08] | nCellSampling         [08] | nCellSampling         [08] | nCellSampling         [08] | nCellSampling         [08] ||                            |
    // | nCellSampling       [09] ||                            |                            | nCellSampling         [09] | nCellSampling         [09] | nCellSampling         [09] | nCellSampling         [09] | nCellSampling         [09] | nCellSampling         [09] | nCellSampling         [09] | nCellSampling         [09] | nCellSampling         [09] ||                            |
    // | nCellSampling       [10] ||                            |                            | nCellSampling         [10] | nCellSampling         [10] | nCellSampling         [10] | nCellSampling         [10] | nCellSampling         [10] | nCellSampling         [10] | nCellSampling         [10] | nCellSampling         [10] | nCellSampling         [10] ||                            |
    // | nCellSampling       [11] ||                            |                            | nCellSampling         [11] | nCellSampling         [11] | nCellSampling         [11] | nCellSampling         [11] | nCellSampling         [11] | nCellSampling         [11] | nCellSampling         [11] | nCellSampling         [11] | nCellSampling         [11] ||                            |
    // | nCellSampling       [12] ||                            |                            | nCellSampling         [12] | nCellSampling         [12] | nCellSampling         [12] | nCellSampling         [12] | nCellSampling         [12] | nCellSampling         [12] | nCellSampling         [12] | nCellSampling         [12] | nCellSampling         [12] ||                            |
    // | nCellSampling       [13] ||                            |                            | nCellSampling         [13] | nCellSampling         [13] | nCellSampling         [13] | nCellSampling         [13] | nCellSampling         [13] | nCellSampling         [13] | nCellSampling         [13] | nCellSampling         [13] | nCellSampling         [13] ||                            |
    // | nCellSampling       [14] ||                            |                            | nCellSampling         [14] | nCellSampling         [14] | nCellSampling         [14] | nCellSampling         [14] | nCellSampling         [14] | nCellSampling         [14] | nCellSampling         [14] | nCellSampling         [14] | nCellSampling         [14] ||                            |
    // | nCellSampling       [15] ||                            |                            | nCellSampling         [15] | nCellSampling         [15] | nCellSampling         [15] | nCellSampling         [15] | nCellSampling         [15] | nCellSampling         [15] | nCellSampling         [15] | nCellSampling         [15] | nCellSampling         [15] ||                            |
    // | nCellSampling       [16] ||                            |                            | nCellSampling         [16] | nCellSampling         [16] | nCellSampling         [16] | nCellSampling         [16] | nCellSampling         [16] | nCellSampling         [16] | nCellSampling         [16] | nCellSampling         [16] | nCellSampling         [16] ||                            |
    // | nCellSampling       [17] ||                            |                            | nCellSampling         [17] | nCellSampling         [17] | nCellSampling         [17] | nCellSampling         [17] | nCellSampling         [17] | nCellSampling         [17] | nCellSampling         [17] | nCellSampling         [17] | nCellSampling         [17] ||                            |
    // | nCellSampling       [18] ||                            |                            | nCellSampling         [18] | nCellSampling         [18] | nCellSampling         [18] | nCellSampling         [18] | nCellSampling         [18] | nCellSampling         [18] | nCellSampling         [18] | nCellSampling         [18] | nCellSampling         [18] ||                            |
    // | nCellSampling       [19] ||                            |                            | nCellSampling         [19] | nCellSampling         [19] | nCellSampling         [19] | nCellSampling         [19] | nCellSampling         [19] | nCellSampling         [19] | nCellSampling         [19] | nCellSampling         [19] | nCellSampling         [19] ||                            |
    // | nCellSampling       [20] ||                            |                            | nCellSampling         [20] | nCellSampling         [20] | nCellSampling         [20] | nCellSampling         [20] | nCellSampling         [20] | nCellSampling         [20] | nCellSampling         [20] | nCellSampling         [20] | nCellSampling         [20] ||                            |
    // | nCellSampling       [21] ||                            |                            | nCellSampling         [21] | nCellSampling         [21] | nCellSampling         [21] | nCellSampling         [21] | nCellSampling         [21] | nCellSampling         [21] | nCellSampling         [21] | nCellSampling         [21] | nCellSampling         [21] ||                            |
    // | nCellSampling       [22] ||                            |                            | nCellSampling         [22] | nCellSampling         [22] | nCellSampling         [22] | nCellSampling         [22] | nCellSampling         [22] | nCellSampling         [22] | nCellSampling         [22] | nCellSampling         [22] | nCellSampling         [22] ||                            |
    // | nCellSampling       [23] ||                            |                            | nCellSampling         [23] | nCellSampling         [23] | nCellSampling         [23] | nCellSampling         [23] | nCellSampling         [23] | nCellSampling         [23] | nCellSampling         [23] | nCellSampling         [23] | nCellSampling         [23] ||                            |
    // | nCellSampling       [24] ||                            |                            | nCellSampling         [24] | nCellSampling         [24] | nCellSampling         [24] | nCellSampling         [24] | nCellSampling         [24] | nCellSampling         [24] | nCellSampling         [24] | nCellSampling         [24] | nCellSampling         [24] ||                            |
    // | nCellSampling       [25] ||                            |                            | nCellSampling         [25] | nCellSampling         [25] | nCellSampling         [25] | nCellSampling         [25] | nCellSampling         [25] | nCellSampling         [25] | nCellSampling         [25] | nCellSampling         [25] | nCellSampling         [25] ||                            |
    // | nCellSampling       [26] ||                            |                            | nCellSampling         [26] | nCellSampling         [26] | nCellSampling         [26] | nCellSampling         [26] | nCellSampling         [26] | nCellSampling         [26] | nCellSampling         [26] | nCellSampling         [26] | nCellSampling         [26] ||                            |
    // | nCellSampling       [27] ||                            |                            | nCellSampling         [27] | nCellSampling         [27] | nCellSampling         [27] | nCellSampling         [27] | nCellSampling         [27] | nCellSampling         [27] | nCellSampling         [27] | nCellSampling         [27] | nCellSampling         [27] ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | nExtraCellSampling       ||                            |                            | sumAbsEnergyNonMomentsAux  | sumAbsEnergyNonMomentsAux  | sumAbsEnergyNonMomentsAux  |                            |                            | nExtraCellSampling         | nExtraCellSampling         | nExtraCellSampling         | nExtraCellSampling         ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | vertexFraction           ||                            |                            |                            |                            | engBadCellsAux             | engBadCellsAux             | engBadCellsAux             | engBadCellsAux             |                            |                            |                            ||                            |
    // | nVertexFraction          ||                            |                            |                            |                            | badLArQFracAux             | badLArQFracAux             | badLArQFracAux             | badLArQFracAux             |                            |                            |                            ||                            |
    // | etaCaloFrame             ||                            |                            | energyDensityNormalization | energyDensityNormalization |enrgyDnstyNorm > avgLArQAux | avgLArQAux                 | avgLArQAux                 | avgLArQAux                 |                            |                            |                            ||                            |
    // | phiCaloFrame             ||                            |                            |                            |                            | timeAux                    | timeAux                    | timeAux                    | timeAux                    |                            |                            |                            ||                            |
    // | eta1CaloFrame            ||                            |                            |                            |                            | secondTimeAux              | secondTimeAux              | secondTimeAux              | secondTimeAux              |                            |                            |                            ||                            |
    // | phi1CaloFrame            ||                            |                            |                            |                            | averageTileQNorm           | averageTileQNorm           | averageTileQNorm           | averageTileQNorm           |                            |                            |                            ||                            |
    // | eta2CaloFrame            ||                            |                            |                            |                            | averageTileQNormAux        | averageTileQNormAux        | averageTileQNormAux        | averageTileQNormAux        |                            |                            |                            ||                            |
    // | phi2CaloFrame            ||                            |                            |                            |                            | timeNormalization          | timeNormalization          | timeNormalization          | timeNormalization          |                            |                            |                            ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | engCalibTot              || <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> || <struct's extra cell info> |
    // | engCalibOutL             || <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> || <struct's extra cell info> |
    // | engCalibOutM             || <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> || <struct's extra cell info> |
    // | engCalibOutT             || <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> || <struct's extra cell info> |
    // | engCalibDeadL            || <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> || <struct's extra cell info> |
    // | engCalibDeadM            || <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> | <struct's extra cell info> || <struct's extra cell info> |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | engCalibDeadT            ||                            |                            |                            |                            | matrix10                   | matrix10                   | matrix10                   |                            |                            |                            |                            ||                            |
    // | engCalibEMB0             ||                            |                            |                            |                            | matrix20                   | matrix20                   | matrix20                   | lateralAux                 | lateralAux                 | lateralAux                 |                            ||                            |
    // | engCalibEME0             ||                            |                            |                            |                            | matrix21                   | matrix21                   | matrix21                   | longitudinalAux            | longitudinalAux            | longitudinalAux            |                            ||                            |
    // | engCalibTileG3           ||                            |                            | firstEngDensAux            | firstEngDensAux            |firstEngDensAux>avgLArQNorm | averageLArQNorm            | averageLArQNorm            |avgLArQNorm>secondLambdaAux | secondLambdaAux            | secondLambdaAux            |                            ||                            |
    // | engCalibDeadTot          ||                            |                            |                            |                            | sumSquareEnergies          | sumSquareEnergies          | sumSquareEnergies          | secondRAux                 | secondRAux                 | secondRAux                 |                            ||                            |
    // | engCalibDeadEMB0         ||                            |                            |                            |                            | sumSquareEnergiesAux       | sumSquareEnergiesAux       | sumSquareEnergiesAux       |                            |                            |                            |                            ||                            |
    // | engCalibDeadTile0        ||                            |                            | maxCellEnergyAndCell_2     | maxCellEnergyAndCell_2     |mCEAC2>mxSgnificanceAndSmpl2|maxSignificanceAndSampling_2|maxSignificanceAndSampling_2|maxSignificanceAndSampling_2|                            |                            |                            ||                            |
    // | engCalibDeadTileG3       ||                            |                            | maxCellEnergyAndCell_1     | maxCellEnergyAndCell_1     |mCEAC1>mxSgnificanceAndSmpl1|maxSignificanceAndSampling_1|maxSignificanceAndSampling_1|maxSignificanceAndSampling_1|                            |                            |                            ||                            |
    // | engCalibDeadEME0         ||                            |                            |secondMaxCellEnergyAndCell_2|secondMaxCellEnergyAndCell_2|sMCEAC2>maxAndScondMaxCells2| maxAndSecondMaxCells_2     | maxAndSecondMaxCells_2     | maxAndSecondMaxCells_2     | maxAndSecondMaxCells_2     |                            |                            ||                            |
    // | engCalibDeadHEC0         ||                            |                            |secondMaxCellEnergyAndCell_1|secondMaxCellEnergyAndCell_1|sMCEAC2>maxAndScondMaxCells1| maxAndSecondMaxCells_1     | maxAndSecondMaxCells_1     | maxAndSecondMaxCells_1     | maxAndSecondMaxCells_1     |                            |                            ||                            |
    // | engCalibDeadFCAL         ||                            |                            | clusterEtaAux              | clusterEtaAux              | clusterEtaAux              |                            |                            |                            |                            |                            |                            ||                            |
    // | engCalibDeadLeakage      ||                            |                            | mY                         | mY                         | mY -> PTDAux               | PTDAux                     | PTDAux                     | PTDAux                     |                            |                            |                            ||                            |
    // | engCalibDeadUnclass      ||                            |                            | mZ                         | mZ                         | mZ > numPositiveEnergyCells| numPositiveEnergyCells     | numPositiveEnergyCells     |                            |                            |                            |                            ||                            |
    // | engCalibFracEM           ||                            |                            |                            |                            | avgTileQAux                | avgTileQAux                | avgTileQAux                | avgTileQAux                |                            |                            |                            ||                            |
    // | engCalibFracHad          ||                            |                            | sumAbsEnergyNonMoments     | sumAbsEnergyNonMoments     | sumAbsEnergyNonMoments     |                            |                            | longitudinalNormalization  | longitudinalNormalization  | longitudinalNormalization  |                            ||                            |
    // | engCalibFracRest         ||                            |                            | seedCellPhi                | seedCellPhi                |                            |                            |                            |longitudinalNormalizationAux|longitudinalNormalizationAux|longitudinalNormalizationAux|                            ||                            |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    // | CLUSTER STORAGE VARIABLE ||   Isolation Cluster Pass   |    Isolation Cell Pass     |    Zeroth Cluster Pass     |      First Cell Pass       |     First Cluster Pass     |      Second Cell Pass      |      Shower Axis Pass      |    Second Cluster Pass     |      Third Cell Pass       |     Third Cluster Pass     |     Final Cluster Pass     ||          Comments          |
    // +--------------------------++----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------+----------------------------++----------------------------+
    //


    CALORECGPU_TEMP2DARR_1 (          numberEmptySamplings,         etaPerSample,                                       int);
    CALORECGPU_TEMP2DARR_1 (       numberNonEmptySamplings,      maxPhiPerSample,                                       int);
    CALORECGPU_TEMP2DARR_1 (     maxMomentsEnergyPerSample,      maxEtaPerSample,                              unsigned int);


    CALORECGPU_TEMP2DARR_1 (       absoluteEnergyPerSample,        maxEPerSample,                                     float);
    CALORECGPU_TEMP2DARR_1 (    absoluteEnergyPerSampleAux,      energyPerSample,                                     float);
    CALORECGPU_TEMP2DARR_1 (               phiPerSampleAux,      maxPhiPerSample,                                     float);
    CALORECGPU_TEMP2DARR_1 (               etaPerSampleAux,      maxEtaPerSample,                                     float);
    CALORECGPU_TEMPARR_1   (                   seedCellPhi,     engCalibFracRest,                                     float);
    CALORECGPU_TEMPARR_1   (              clusterEnergyAux,             DMweight,                                     float);
    CALORECGPU_TEMPARR_1   (                 clusterPhiAux,  tileConfidenceLevel,                                     float);
    CALORECGPU_TEMPARR_1   (                 clusterEtaAux,     engCalibDeadFCAL,                                     float);
    CALORECGPU_TEMPARR_1   (                    centerXAux,        EMProbability,                                     float);
    CALORECGPU_TEMPARR_1   (                    centerYAux,            hadWeight,                                     float);
    CALORECGPU_TEMPARR_1   (                    centerZAux,            OOCweight,                                     float);
    CALORECGPU_TEMPARR_1   (                   firstPhiAux,              secondR,                                     float);
    CALORECGPU_TEMPARR_1   (                   firstEtaAux,         longitudinal,                                     float);
    CALORECGPU_TEMPARR_1   (                     engPosAux,        engBadHVCells,                                     float);
    CALORECGPU_TEMPARR_1   (                  engFracEMAux,            centerMag,                                     float);
    CALORECGPU_TEMPARR_1   (               firstEngDensAux,       engCalibTileG3,                                     float);
    CALORECGPU_TEMPARR_1   (              secondEngDensAux,              lateral,                                     float);
    CALORECGPU_TEMPARR_1   (    energyDensityNormalization,         etaCaloFrame,                                     float);
    CALORECGPU_TEMPARR_1   ( energyDensityNormalizationAux,         secondLambda,                                     float);
    CALORECGPU_TEMPARR_1   (        sumAbsEnergyNonMoments,      engCalibFracHad,                                     float);
    CALORECGPU_TEMPARR_1   (     sumAbsEnergyNonMomentsAux,   nExtraCellSampling,                                     float);
    CALORECGPU_TEMPARR_1   (                            mX,                  PTD,                                     float);
    CALORECGPU_TEMPARR_1   (                         mXAux,                 mass,                                     float);
    CALORECGPU_TEMPARR_1   (                            mY,  engCalibDeadLeakage,                                     float);
    CALORECGPU_TEMPARR_1   (                         mYAux,     cellSignificance,                                     float);
    CALORECGPU_TEMPARR_1   (                            mZ,  engCalibDeadUnclass,                                     float);
    CALORECGPU_TEMPARR_1   (                         mZAux,      cellSigSampling,                                     float);
    CALORECGPU_TEMPARR_2   (          maxCellEnergyAndCell,   engCalibDeadTileG3,    engCalibDeadTile0,  unsigned long long);
    CALORECGPU_TEMPARR_2   (    secondMaxCellEnergyAndCell,     engCalibDeadHEC0,     engCalibDeadEME0,  unsigned long long);
    CALORECGPU_TEMPARR_2   (          maxAndSecondMaxCells,     engCalibDeadHEC0,     engCalibDeadEME0,  unsigned long long);

    CALORECGPU_TEMPARR_1   (                       timeAux,         phiCaloFrame,                                     float);
    CALORECGPU_TEMPARR_1   (                 secondTimeAux,        eta1CaloFrame,                                     float);
    CALORECGPU_TEMPARR_1   (               significanceAux,          nBadHVCells,                                     float);
    CALORECGPU_TEMPARR_1   (                        PTDAux,  engCalibDeadLeakage,                                     float);
    CALORECGPU_TEMPARR_1   (                engBadCellsAux,       vertexFraction,                                     float);
    CALORECGPU_TEMPARR_1   (              badCellsCorrEAux,         centerLambda,                                     float);
    CALORECGPU_TEMPARR_1   (                badLArQFracAux,      nVertexFraction,                                     float);
    CALORECGPU_TEMPARR_1   (                    avgLArQAux,         etaCaloFrame,                                     float);
    CALORECGPU_TEMPARR_1   (                   avgTileQAux,       engCalibFracEM,                                     float);
    CALORECGPU_TEMPARR_1   (        numPositiveEnergyCells,  engCalibDeadUnclass,                                       int);
    CALORECGPU_TEMPARR_1   (             sumSquareEnergies,      engCalibDeadTot,                                     float);
    CALORECGPU_TEMPARR_1   (          sumSquareEnergiesAux,     engCalibDeadEMB0,                                     float);
    CALORECGPU_TEMPARR_1   (                      matrix00,         longitudinal,                                     float);
    CALORECGPU_TEMPARR_1   (                      matrix10,        engCalibDeadT,                                     float);
    CALORECGPU_TEMPARR_1   (                      matrix20,         engCalibEMB0,                                     float);
    CALORECGPU_TEMPARR_1   (                      matrix11,              secondR,                                     float);
    CALORECGPU_TEMPARR_1   (                      matrix21,         engCalibEME0,                                     float);
    CALORECGPU_TEMPARR_1   (                      matrix22,  tileConfidenceLevel,                                     float);
    CALORECGPU_TEMPARR_1   (                   matrix00Aux,        EMProbability,                                     float);
    CALORECGPU_TEMPARR_1   (                   matrix10Aux,             deltaPhi,                                     float);
    CALORECGPU_TEMPARR_1   (                   matrix20Aux,           deltaTheta,                                     float);
    CALORECGPU_TEMPARR_1   (                   matrix11Aux,            hadWeight,                                     float);
    CALORECGPU_TEMPARR_1   (                   matrix21Aux,           deltaAlpha,                                     float);
    CALORECGPU_TEMPARR_1   (                   matrix22Aux,            OOCweight,                                     float);
    CALORECGPU_TEMPARR_1   (             timeNormalization,        phi2CaloFrame,                                     float);
    CALORECGPU_TEMPARR_1   (          timeNormalizationAux,        engBadHVCells,                                     float);
    CALORECGPU_TEMPARR_1   (               averageLArQNorm,       engCalibTileG3,                                     float);
    CALORECGPU_TEMPARR_1   (            averageLArQNormAux,         secondLambda,                                     float);
    CALORECGPU_TEMPARR_1   (              averageTileQNorm,        phi1CaloFrame,                                     float);
    CALORECGPU_TEMPARR_1   (           averageTileQNormAux,        eta2CaloFrame,                                     float);
    CALORECGPU_TEMPARR_2   (    maxSignificanceAndSampling,   engCalibDeadTileG3,    engCalibDeadTile0,  unsigned long long);
    CALORECGPU_TEMPARR_1   (                   showerAxisX,        EMProbability,                                     float);
    CALORECGPU_TEMPARR_1   (                   showerAxisY,            hadWeight,                                     float);
    CALORECGPU_TEMPARR_1   (                   showerAxisZ,            OOCweight,                                     float);

    CALORECGPU_TEMP2DARR_2 (     maxEnergyAndCellPerSample,      maxEtaPerSample,      maxPhiPerSample,  unsigned long long);
    CALORECGPU_TEMP2DARR_1 (            energyPerSampleAux,        maxEPerSample,                                     float);
    CALORECGPU_TEMPARR_1   (                    lateralAux,         engCalibEMB0,                                     float);
    CALORECGPU_TEMPARR_1   (               longitudinalAux,         engCalibEME0,                                     float);
    CALORECGPU_TEMPARR_1   (               secondLambdaAux,       engCalibTileG3,                                     float);
    CALORECGPU_TEMPARR_1   (                    secondRAux,      engCalibDeadTot,                                     float);
    CALORECGPU_TEMPARR_1   (          lateralNormalization,             DMweight,                                     float);
    CALORECGPU_TEMPARR_1   (       lateralNormalizationAux,  tileConfidenceLevel,                                     float);
    CALORECGPU_TEMPARR_1   (     longitudinalNormalization,      engCalibFracHad,                                     float);
    CALORECGPU_TEMPARR_1   (  longitudinalNormalizationAux,     engCalibFracRest,                                     float);

    CALORECGPU_TEMP2DARR_1 (             maxECellPerSample,        maxEPerSample,                                       int);

  }

  template <class ... Ts>
  struct TypeList
  {
    static constexpr unsigned int number = static_cast<unsigned int>(sizeof...(Ts));
  };

  template <class ... Ts, class T>
  constexpr __host__ __device__ auto operator| (const TypeList<Ts...> & list, const TypeList<T> &)
  {
    if constexpr ((false || ... || std::is_same_v<T, Ts>))
      {
        return list;
      }
    else
      {
        return TypeList<Ts..., T> {};
      }
  }

  template <class ... As, class ... Bs>
  constexpr __host__ __device__ auto operator | (const TypeList<As...> & list, const TypeList<Bs...> &)
  {
    return (list | ... | TypeList<Bs> {});
  }

  template <class ... As, class ... Bs>
  constexpr __host__ __device__ auto operator + (const TypeList<As...> & list, const TypeList<Bs...> &)
  {
    return TypeList<As..., Bs...> {};
  }

  template <class T>
  struct is_type_list
  {
    static constexpr bool value = false;
  };

  template <class ... Ts>
  struct is_type_list<TypeList<Ts...>>
  {
    static constexpr bool value = true;
  };

  template <class T>
  inline constexpr bool is_type_list_v = is_type_list<T>::value;

  struct Parameters
  {
    bool                                                                                                     assume_complete_cells;
    int                                                                                                      moments_index;
    CaloRecGPU::Helpers::CUDA_kernel_object<CaloRecGPU::ClusterInfoArr>                                      clusters_arr;
    const CaloRecGPU::Helpers::CUDA_kernel_object<CaloRecGPU::CellInfoArr>                                   cell_info_arr;
    const CaloRecGPU::Helpers::CUDA_kernel_object<CaloRecGPU::GeometryArr>                                   geometry;
    const CaloRecGPU::Helpers::CUDA_kernel_object<CaloRecGPU::CellNoiseArr>                                  noise_arr;
    const CaloRecGPU::Helpers::CUDA_kernel_object<ClusterMomentsCalculator::ClusterMomentCalculationOptions> opts;
  };

  template <class T>
  struct OnePassLoadHelper
  {
    static_assert(is_type_list_v<T>);

    using ToLoad = T;
  };

  //Add property if unique
  template <class ... Ts, class T>
  constexpr __host__ __device__ auto operator + (const OnePassLoadHelper<TypeList<Ts...>> &, const TypeList<T> &)
  {
    constexpr auto combined = TypeList<Ts...> {} | TypeList<T> {};

    return OnePassLoadHelper<std::decay_t<decltype(combined)>> {};
  }

  //Add any dependencies and then the desired property
  template <class T, class NewLoad>
  constexpr __host__ __device__ auto operator | (const OnePassLoadHelper<T> & c, const TypeList<NewLoad> & n)
  {
    auto combine = c | typename NewLoad::AssumedList{};

    return combine + n;
  }

  //Add all properties with potential dependencies
  template <class T, class ... Ts>
  constexpr __host__ __device__ auto operator | (const OnePassLoadHelper<T> & c, const TypeList<Ts...> &)
  {
    return (c | ... | TypeList<Ts> {});
  }

  template <class T>
  using ClusterPassLoadHelper = OnePassLoadHelper<T>;

  template <class T1, class T2>
  struct CellAndClusterPassLoadHelper
  {
    static_assert(is_type_list_v<T1> && is_type_list_v<T2>);

    using ToLoadCells    = T1;
    using ToLoadClusters = T2;
  };

  //Use | to add cell loading with dependencies
  //Use ^ to add cluster loading with dependencies

  template <class T1, class T2, class NewCellLoad>
  constexpr __host__ __device__ auto operator | (const CellAndClusterPassLoadHelper<T1, T2> & c, const TypeList<NewCellLoad> & n)
  {
    auto combine_cell = OnePassLoadHelper<T1> {} | n;

    return CellAndClusterPassLoadHelper<typename std::decay_t<decltype(combine_cell)>::ToLoad, T2> {};
  }

  template <class T1, class T2, class ... Ts>
  constexpr __host__ __device__ auto operator | (const CellAndClusterPassLoadHelper<T1, T2> & c, const TypeList<Ts...> &)
  {
    return (c | ... | TypeList<Ts> {});
  }

  template <class T1, class T2, class NewClusterLoad>
  constexpr __host__ __device__  auto operator ^ (const CellAndClusterPassLoadHelper<T1, T2> & c, const TypeList<NewClusterLoad> & n)
  {
    auto combine_dependencies = (c | typename NewClusterLoad::AssumedPreviousList{}) ^ typename NewClusterLoad::AssumedList{};

    using DepType = std::decay_t<decltype(combine_dependencies)>;

    auto combine_clusters = typename DepType::ToLoadClusters{} | n;

    return CellAndClusterPassLoadHelper<typename DepType::ToLoadCells, std::decay_t<decltype(combine_clusters)>> {};
  }

  template <class T1, class T2, class ... Ts>
  constexpr __host__ __device__  auto operator ^ (const CellAndClusterPassLoadHelper<T1, T2> & c, const TypeList<Ts...> &)
  {
    return (c ^ ... ^ TypeList<Ts> {});
  }

  template <class T>
  struct ClusterLoader;

  template <class ... Ts>
  struct ClusterLoader<ClusterPassLoadHelper<TypeList<Ts...>>> : Ts...
  {
    __host__ __device__ ClusterLoader(Parameters p, const int idx):
      Ts(*this, p, idx)...
    {
    }
  };

  template <class T>
  struct CellClusterLoader;

  struct WeightCarrier
  {
    float weight;

    __host__ __device__ WeightCarrier(const float w): weight(w)
    {
    }
  };

  template <class ... CellLoad, class ... ClusterLoad>
  struct CellClusterLoader<CellAndClusterPassLoadHelper<TypeList<CellLoad...>, TypeList<ClusterLoad...>>>: WeightCarrier, CellLoad..., ClusterLoad...
  {
    __host__ __device__ CellClusterLoader(Parameters p, const int cell, const int cluster, const float weight):
      WeightCarrier(weight), CellLoad(*this, p, cell)..., ClusterLoad(*this, p, cluster)...
    {
    }
  };

  template <class ... Moments>
  __host__ __device__ void one_cluster_before_pass(const TypeList<Moments...> &, const int cluster, Parameters p)
  {
    constexpr auto to_load = (ClusterPassLoadHelper<TypeList<>> {} | ... | typename Moments::BeforeLoading{});

    ClusterLoader<std::decay_t<decltype(to_load)>> data{p, cluster};

    (Moments::before(p, data, cluster), ...);
  }

  template <class ... Moments>
  __host__ __device__ void one_cluster_after_pass(const TypeList<Moments...> &, const int cluster, Parameters p)
  {
    constexpr auto to_load = (ClusterPassLoadHelper<TypeList<>> {} | ... | typename Moments::AfterLoading{});

    ClusterLoader<std::decay_t<decltype(to_load)>> data{p, cluster};

    (Moments::after(p, data, cluster), ...);
  }

  template <class ... Moments>
  __host__ __device__ void one_cell_pass(const TypeList<Moments ...> &, const int cell, const int cluster, const float weight, Parameters p)
  {
    constexpr auto to_load = ((CellAndClusterPassLoadHelper<TypeList<>, TypeList<>> {} | ... | typename Moments::CellLoading{}) ^ ... ^ typename Moments::ClusterLoading{});

    CellClusterLoader<std::decay_t<decltype(to_load)>> data{p, cell, cluster, weight};

    (Moments::per_cell(p, data, cell, cluster), ...);

  }

  ///Receives two `TypeList`s containing the `TypeList`s that correspond to the moments
  ///to be calculated together (in the same thread), both for the current step (finalizing)
  ///and the next step (initializing).
  template <class ... ThisStepMomentLists, class ... NextStepMomentLists>
  __host__ __device__ void do_cluster_pass(const TypeList<ThisStepMomentLists...> &, const TypeList<NextStepMomentLists...> &, const int cluster, Parameters p)
  {
    auto helper_1 = [&](const auto & list, const int count)
    {
      if (p.moments_index == count)
        {
          one_cluster_after_pass(list, cluster, p);
        }
    };
    auto helper_2 = [&](const auto & list, const int count)
    {
      if (p.moments_index == count)
        {
          one_cluster_before_pass(list, cluster, p);
        }
    };

    int count = 0;

    (helper_1(ThisStepMomentLists{}, count++), ...);

    count = 0;

    (helper_2(NextStepMomentLists{}, count++), ...);
  }

  template <class... MomentLists>
  __host__ __device__ void do_cell_pass(const TypeList<MomentLists...> &, const int cell, const int cluster, const float weight, Parameters p)
  {
    auto helper = [&](const auto & list, const int count)
    {
      if (p.moments_index == count)
        {
          one_cell_pass(list, cell, cluster, weight, p);
        }
    };

    int count = 0;

    (helper(MomentLists{}, count++), ...);
  }


  //-------------------------------

  namespace ToLoad
  {

#define CALORECGPU_CMC_EXPAND(...) __VA_ARGS__

    
#define CALORECGPU_CMC_LOAD(NAME, NEEDED, PREVNEEDED, VARS, INIT)                                                                      \
  struct NAME                                                                                                                          \
  {                                                                                                                                    \
    using AssumedList = TypeList<CALORECGPU_CMC_EXPAND NEEDED  >;                                                                          \
    using AssumedPreviousList = TypeList<CALORECGPU_CMC_EXPAND PREVNEEDED  >;                                                              \
    CALORECGPU_CMC_EXPAND VARS                                                                                                             \
    template <class Final> __device__ NAME(const Final & f, Parameters p, const int idx) { CALORECGPU_CMC_EXPAND INIT }                    \
  }
    //Of course, any circular dependencies will lead to infinite loops during compilation!


#define CALORECGPU_CMC_LOAD_SIMPLE_CELL_INFO(NAME, VARNAME, PROPNAME)                                                                  \
  CALORECGPU_CMC_LOAD(NAME,                                                                                                            \
                      (),                                                                                                              \
                      (),                                                                                                              \
                      (std::decay_t<decltype(std::declval<CaloRecGPU::CellInfoArr>().PROPNAME[0])> VARNAME;),                          \
                      (VARNAME = p.cell_info_arr->PROPNAME[idx];)                                                                      \
                     );

#define CALORECGPU_CMC_LOAD_SIMPLE_GEOMETRY_INFO(NAME, VARNAME, PROPNAME)                                                              \
  CALORECGPU_CMC_LOAD(NAME,                                                                                                            \
                      (CellHashID),                                                                                                    \
                      (),                                                                                                              \
                      (std::decay_t<decltype(std::declval<CaloRecGPU::GeometryArr>().PROPNAME[0])> VARNAME;),                          \
                      (VARNAME = p.geometry->PROPNAME[f.hash_ID];)                                                                     \
                     );

#define CALORECGPU_CMC_LOAD_SIMPLE_CLUSTER_INFO(NAME, VARNAME, PROPNAME)                                                               \
  CALORECGPU_CMC_LOAD(NAME,                                                                                                            \
                      (),                                                                                                              \
                      (),                                                                                                              \
                      (std::decay_t<decltype(std::declval<CaloRecGPU::ClusterInfoArr>().PROPNAME[0])> VARNAME;),                       \
                      (VARNAME = p.clusters_arr->PROPNAME[idx];)                                                                       \
                     );

#define CALORECGPU_CMC_LOAD_SIMPLE_MOMENT_INFO(NAME, VARNAME, PROPNAME)                                                                \
  CALORECGPU_CMC_LOAD(NAME,                                                                                                            \
                      (),                                                                                                              \
                      (),                                                                                                              \
                      (std::decay_t<decltype(std::declval<CaloRecGPU::ClusterInfoArr>().moments.PROPNAME[0])> VARNAME;),               \
                      (VARNAME = p.clusters_arr->moments.PROPNAME[idx];)                                                               \
                     );

    //Warning! These require either a CellSampling or a SamplingFromMomentIndex
    //to be calculated!
#define CALORECGPU_CMC_LOAD_SIMPLE_PER_SAMPLING_MOMENT_INFO(NAME, VARNAME, PROPNAME)                                                   \
  CALORECGPU_CMC_LOAD(NAME,                                                                                                            \
                      (),                                                                                                              \
                      (),                                                                                                              \
                      (std::decay_t<decltype(std::declval<CaloRecGPU::ClusterInfoArr>().moments.PROPNAME[0][0])> VARNAME;),            \
                      (VARNAME = p.clusters_arr->moments.PROPNAME[f.sampling][idx];)                                                   \
                     );

#define CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(NAME, VARNAME, PROPNAME)                                                             \
  CALORECGPU_CMC_LOAD(NAME,                                                                                                            \
                      (),                                                                                                              \
                      (),                                                                                                              \
                      (std::decay_t<decltype(CMCTemporaries::PROPNAME(std::declval<CaloRecGPU::ClusterInfoArr *>(),0))> VARNAME;),     \
                      (VARNAME = CMCTemporaries::PROPNAME(p.clusters_arr, idx);)                                                       \
                     );

    //Warning! These require either a CellSampling or a SamplingFromMomentIndex
    //to be calculated!
#define CALORECGPU_CMC_LOAD_SIMPLE_PER_SAMPLING_TEMPORARY_INFO(NAME, VARNAME, PROPNAME)                                                \
  CALORECGPU_CMC_LOAD(NAME,                                                                                                            \
                      (),                                                                                                              \
                      (),                                                                                                              \
                      (std::decay_t<decltype(CMCTemporaries::PROPNAME(std::declval<CaloRecGPU::ClusterInfoArr *>(),0,0))> VARNAME;),   \
                      (VARNAME = CMCTemporaries::PROPNAME(p.clusters_arr, f.sampling, idx);)                                           \
                     );

    //+---------------------------------------------------+
    //|                Sampling Variables:                |
    //+---------------------------------------------------+

    CALORECGPU_CMC_LOAD
    (SamplingFromMomentIndex,
     (),
     (),
     (int sampling;),
     (sampling = p.moments_index;)
    );

    CALORECGPU_CMC_LOAD
    (CellHashID,
     (),
     (),
     (int hash_ID;),
     (hash_ID = p.cell_info_arr->get_hash_ID(idx, p.assume_complete_cells);)
    );
    
    CALORECGPU_CMC_LOAD
    (CellSampling,
     (CellHashID),
     (),
     (int sampling;),
     (sampling = p.geometry->sampling(f.hash_ID);)
    );

    //+---------------------------------------------------+
    //|                  Cell Only Info:                  |
    //+---------------------------------------------------+

    CALORECGPU_CMC_LOAD_SIMPLE_GEOMETRY_INFO(CellX, x, x);
    CALORECGPU_CMC_LOAD_SIMPLE_GEOMETRY_INFO(CellY, y, y);
    CALORECGPU_CMC_LOAD_SIMPLE_GEOMETRY_INFO(CellZ, z, z);
    CALORECGPU_CMC_LOAD_SIMPLE_GEOMETRY_INFO(CellEta, eta, eta);
    CALORECGPU_CMC_LOAD_SIMPLE_GEOMETRY_INFO(CellPhi, phi, phi);
    CALORECGPU_CMC_LOAD_SIMPLE_GEOMETRY_INFO(CellVolume, volume, volume);

    CALORECGPU_CMC_LOAD_SIMPLE_CELL_INFO(CellEnergy, energy, energy);

    CALORECGPU_CMC_LOAD
    (CellAbsEnergy,
     (CellEnergy),
     (),
     (float abs_energy;),
     (abs_energy = fabsf(f.energy);)
    );

    CALORECGPU_CMC_LOAD
    (CellMomentsEnergy,
     (CellEnergy, CellAbsEnergy),
     (),
     (float moments_energy;),
     (moments_energy = (p.opts->use_abs_energy || f.energy > 0.f) ? f.abs_energy : 0.f;)
    );

    CALORECGPU_CMC_LOAD_SIMPLE_CELL_INFO(CellTime, time, time);
    CALORECGPU_CMC_LOAD_SIMPLE_CELL_INFO(CellGain, gain, gain);

    CALORECGPU_CMC_LOAD
    (CellQualityProvenance,
     (),
     (),
     (CaloRecGPU::QualityProvenance qp{0};),
     (qp = p.cell_info_arr->qualityProvenance[idx];)
    );

    CALORECGPU_CMC_LOAD
    (CellIsTile,
     (CellHashID),
     (),
     (bool is_tile;),
     (is_tile = p.geometry->is_tile(f.hash_ID);)
    );

    CALORECGPU_CMC_LOAD
    (CellIsBad,
     (CellIsTile, CellQualityProvenance),
     (),
     (bool is_bad;),
     (is_bad = p.cell_info_arr->is_bad_general(f.is_tile, f.qp, false);)
    );

    CALORECGPU_CMC_LOAD
    (CellNoise,
     (CellHashID, CellIsTile, CellGain, CellEnergy),
     (),
     (float noise;),
     (noise = ( f.is_tile && p.opts->use_two_gaussian_noise                   ?
                p.noise_arr->get_double_gaussian_noise(f.hash_ID, f.gain, f.energy) :
                p.noise_arr->get_noise(f.hash_ID, f.gain)                             );)
    );

    CALORECGPU_CMC_LOAD
    (CellLArQCheck,
     (CellIsBad, CellIsTile, CellQualityProvenance),
     (),
     (bool LArQ_cell_check;),
     (LArQ_cell_check = !f.is_bad && !f.is_tile && ((f.qp.provenance() & 0x2800U) == 0x2000U);)
    );

    CALORECGPU_CMC_LOAD
    (CellTileQCheck,
     (CellIsBad, CellIsTile, CellQualityProvenance),
     (),
     (bool TileQ_cell_check;),
     (TileQ_cell_check = !f.is_bad && f.is_tile && f.qp.tile_qual1() != 0xFFU && f.qp.tile_qual2() != 0xFFU;)
    );


    CALORECGPU_CMC_LOAD
    (CellTimeMomentsCheck,
     (CellIsTile, CellQualityProvenance, CellSampling),
     (),
     (bool time_moments_check;),
     (time_moments_check = ( (f.is_tile && (f.qp.provenance() & 0x8080U)) ||
                             (!f.is_tile && (f.qp.provenance() & 0x2000U))   ) &&
                           f.sampling != CaloSampling::PreSamplerB && f.sampling != CaloSampling::PreSamplerE; )
    );



    //+---------------------------------------------------+
    //|          Cell-In-Cluster-Dependent Info:          |
    //+---------------------------------------------------+

    CALORECGPU_CMC_LOAD
    (WeightedEnergy,
     (),
     (CellMomentsEnergy),
     (float weighted_energy;),
     (weighted_energy = f.moments_energy * f.weight;)
    );

    CALORECGPU_CMC_LOAD
    (SquareWeightedEnergy,
     (WeightedEnergy),
     (),
     (float square_w_E;),
     (square_w_E = f.weighted_energy * f.weighted_energy;)
    );

    CALORECGPU_CMC_LOAD
    (WeightedEnergyOverVolume,
     (WeightedEnergy),
     (CellVolume),
     (float w_E_over_V;),
     (w_E_over_V = (f.volume > 0.f ? f.weighted_energy / f.volume : 1.f);)
    );

    CALORECGPU_CMC_LOAD
    (WeightedCellPositionNormalization,
     (WeightedEnergy),
     (CellX, CellY, CellZ),
     (float r_dir;
      float w_E_r_dir;
     ),
     (const float r_dir_base = rnorm3df(f.x, f.y, f.z);
      r_dir = isinf(r_dir_base) ? 0.f : r_dir_base;
      w_E_r_dir = f.weighted_energy * f.r_dir;
     )
    );

    CALORECGPU_CMC_LOAD
    (WeightedEnergyOrNegative,
     (),
     (CellEnergy),
     (float weighted_energy_or_negative;),
     (weighted_energy_or_negative = (p.opts->use_abs_energy ? fabsf(f.energy) : f.energy) * f.weight;)
    );

    CALORECGPU_CMC_LOAD
    (SquareWeightedEnergyOrNegative,
     (WeightedEnergyOrNegative),
     (),
     (float square_w_E_or_neg;),
     (square_w_E_or_neg = f.weighted_energy_or_negative * f.weighted_energy_or_negative;)
    );

    CALORECGPU_CMC_LOAD
    (WeightedNonMomentsEnergy,
     (),
     (CellEnergy),
     (float normE;),
     (normE = f.weight * f.energy;)
    );

    CALORECGPU_CMC_LOAD
    (SquaredWeightedNonMomentsEnergy,
     (WeightedNonMomentsEnergy),
     (),
     (float squared_normE;),
     (squared_normE = f.normE * f.normE;)
    );

    struct CenterX;
    struct CenterY;
    struct CenterZ;
    struct ShowerAxisX;
    struct ShowerAxisY;
    struct ShowerAxisZ;

    // Possible simplification if we need more performance
    // when calculating both Lambda and R (but probably worse accuracy):
    //
    //d\vec{v} = \vec{r}_{cell} - \vec{r}_{center}
    //
    //  r      = ||d\vec{v} \cross \vec{axis}||
    //  lambda = d\vec{v} \dot \vec{axis}
    //
    //  lambda = ||d\vec{v}||  cos (\theta)
    //  r      = ||d\vec{v}|| |sin(\theta)|
    //
    //  |sin(\theta)| = sqrt(1 - cos(\theta)^2)

    CALORECGPU_CMC_LOAD
    (Deltas,
     (CenterX, CenterY, CenterZ),
     (CellX, CellY, CellZ),
     (float dx, dy, dz;),
     (dx = f.x - f.center_x;
      dy = f.y - f.center_y;
      dz = f.z - f.center_z;
     )
    );

    CALORECGPU_CMC_LOAD
    (Lambda,
     (Deltas, ShowerAxisX, ShowerAxisY, ShowerAxisZ),
     (),
     (float lambda;),
     (lambda = CaloRecGPU::Helpers::corrected_dot_product(f.dx, f.dy, f.dz, f.axis_x, f.axis_y, f.axis_z);)
    );

    CALORECGPU_CMC_LOAD
    (R,
     (Deltas, ShowerAxisX, ShowerAxisY, ShowerAxisZ),
     (),
     (float r;),
     (r = CaloRecGPU::Helpers::corrected_magn_cross_product(f.dx, f.dy, f.dz, f.axis_x, f.axis_y, f.axis_z);)
    );

    //+---------------------------------------------------+
    //|                   Cluster Info:                   |
    //+---------------------------------------------------+

    CALORECGPU_CMC_LOAD_SIMPLE_CLUSTER_INFO(SeedCellIndex, seed_cell, seedCellIndex);

    CALORECGPU_CMC_LOAD_SIMPLE_MOMENT_INFO(SumEnergies, sum_energies, engPos);

    CALORECGPU_CMC_LOAD
    (ReverseSumEnergies,
     (SumEnergies),
     (),
     (float rev_sum_energies;),
     (rev_sum_energies =  1.0f / (f.sum_energies > 0.f ? f.sum_energies : 1.f);)
    );

    CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(SeedCellPhi, phi_0, seedCellPhi);

    CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(EnergyDensityNormalizationBase, energy_density_norm_base, energyDensityNormalization);
    CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(EnergyDensityNormalizationCorr, energy_density_norm_corr, energyDensityNormalizationAux);

    CALORECGPU_CMC_LOAD
    (EnergyDensityNormalization,
     (EnergyDensityNormalizationBase, EnergyDensityNormalizationCorr),
     (),
     (float energy_density_norm;),
     (energy_density_norm =  f.energy_density_norm_base + f.energy_density_norm_corr;)
    );

    CALORECGPU_CMC_LOAD
    (ReverseEnergyDensityNormalization,
     (EnergyDensityNormalization),
     (),
     (float rev_energy_density_norm;),
     (rev_energy_density_norm =  1.0f / (f.energy_density_norm > 0.f ? f.energy_density_norm : 1.f);)
    );

    CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(ClusterMaxCellEnergyAndCell, max_E_and_cell, maxCellEnergyAndCell);

    CALORECGPU_CMC_LOAD
    (ClusterCellWithMaxEnergy,
     (ClusterMaxCellEnergyAndCell),
     (),
     (int max_E_cell;),
     (max_E_cell = (f.max_E_and_cell & 0x7FFFFFFFU) - 1;)
    );

    CALORECGPU_CMC_LOAD
    (ClusterMaxCellEnergy,
     (ClusterMaxCellEnergyAndCell),
     (),
     (float max_E;),
     (max_E = __uint_as_float(f.max_E_and_cell >> 32U);)
    );

    CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(ClusterSecondMaxCellEnergyAndCell, second_max_E_and_cell, secondMaxCellEnergyAndCell);

    CALORECGPU_CMC_LOAD
    (ClusterCellWithSecondMaxEnergy,
     (ClusterSecondMaxCellEnergyAndCell),
     (),
     (int second_max_E_cell;),
     (second_max_E_cell = (f.second_max_E_and_cell & 0x7FFFFFFFU) - 1;)
    );

    CALORECGPU_CMC_LOAD
    (ClusterSecondMaxCellEnergy,
     (ClusterSecondMaxCellEnergyAndCell),
     (),
     (float second_max_E;),
     (second_max_E = __uint_as_float(f.second_max_E_and_cell >> 32U);)
    );

    CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(SumAbsEnergyNonMomentsBase, abs_energy_non_moments_base, sumAbsEnergyNonMoments);
    CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(SumAbsEnergyNonMomentsCorr, abs_energy_non_moments_corr, sumAbsEnergyNonMomentsAux);

    CALORECGPU_CMC_LOAD
    (SumAbsEnergyNonMoments,
     (SumAbsEnergyNonMomentsBase, SumAbsEnergyNonMomentsCorr),
     (),
     (float abs_energy_non_moments;),
     (abs_energy_non_moments = f.abs_energy_non_moments_base + f.abs_energy_non_moments_corr;)
    );

    CALORECGPU_CMC_LOAD
    (ReverseSumAbsEnergyNonMoments,
     (SumAbsEnergyNonMoments),
     (),
     (float rev_abs_energy_non_moments;),
     (rev_abs_energy_non_moments = 1.f / (f.abs_energy_non_moments != 0.f ? f.abs_energy_non_moments : 1.f);)
    );

    CALORECGPU_CMC_LOAD_SIMPLE_MOMENT_INFO(CenterX, center_x, centerX);
    CALORECGPU_CMC_LOAD_SIMPLE_MOMENT_INFO(CenterY, center_y, centerY);
    CALORECGPU_CMC_LOAD_SIMPLE_MOMENT_INFO(CenterZ, center_z, centerZ);

    CALORECGPU_CMC_LOAD_SIMPLE_CLUSTER_INFO(ClusterEnergy, cluster_energy, clusterEnergy);

    CALORECGPU_CMC_LOAD
    (ReverseClusterEnergy,
     (ClusterEnergy),
     (),
     (float rev_cluster_energy;),
     (rev_cluster_energy = (f.cluster_energy != 0.f ? 1.f / f.cluster_energy : 1.f);)
    );

    CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(MaxSignificanceAndSampling, max_sig_and_samp, maxSignificanceAndSampling);

    CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(TimeNormalizationBase, time_norm_base, timeNormalization);
    CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(TimeNormalizationCorr, time_norm_corr, timeNormalizationAux);

    CALORECGPU_CMC_LOAD
    (TimeNormalization,
     (TimeNormalizationBase, TimeNormalizationCorr),
     (),
     (float time_norm;),
     (time_norm = f.time_norm_base + f.time_norm_corr;)
    );

    CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(ShowerAxisX, axis_x, showerAxisX);
    CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(ShowerAxisY, axis_y, showerAxisY);
    CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(ShowerAxisZ, axis_z, showerAxisZ);


    CALORECGPU_CMC_LOAD_SIMPLE_TEMPORARY_INFO(ClusterMaxAndSecondMaxCellTogether, stored_max_and_second_max, maxAndSecondMaxCells);


    CALORECGPU_CMC_LOAD
    (ClusterMaxAndSecondMaxCell,
     (ClusterMaxAndSecondMaxCellTogether),
     (),
     (int max_cell, second_max_cell;),
     ( max_cell = f.stored_max_and_second_max >> 32U;
       second_max_cell = f.stored_max_and_second_max;
     )
    );

    CALORECGPU_CMC_LOAD_SIMPLE_PER_SAMPLING_TEMPORARY_INFO(AbsoluteEnergyPerSample, sampling_normalization, absoluteEnergyPerSample);

    CALORECGPU_CMC_LOAD
    (ReverseAbsoluteEnergyPerSample,
     (AbsoluteEnergyPerSample),
     (),
     (float rev_sampling_normalization;),
     (rev_sampling_normalization = 1.0f / (f.sampling_normalization != 0.f ? f.sampling_normalization : 1.0f);)
    );


    CALORECGPU_CMC_LOAD_SIMPLE_PER_SAMPLING_TEMPORARY_INFO(MaxEnergyAndCellPerSample, max_energy_and_cell, maxEnergyAndCellPerSample);

    //+---------------------------------------------------+
    //|                      Others:                      |
    //+---------------------------------------------------+

    //This one reads directly from the geometry array,
    //the calculation of the moments themselves probably benefits
    //from using the seedCellPhi temporary instead!
    CALORECGPU_CMC_LOAD
    (SeedCellGeometryPhi,
     (SeedCellIndex),
     (),
     (float seed_cell_phi_coordinate;),
     ( const int seed_cell_hash_ID = ( f.seed_cell >= 0 && f.seed_cell < CaloRecGPU::NCaloCells ?
                                       p.cell_info_arr->get_hash_ID(f.seed_cell, p.assume_complete_cells) : -1);
        seed_cell_phi_coordinate = (seed_cell_hash_ID >= 0 ? p.geometry->phi[seed_cell_hash_ID] : -999);
     )
    );
  }

  //-------------------------------
  namespace ToCalculate
  {

    struct Template
    {
      //A list of things to load (from memory)
      //so that we can load only once if we combine
      //several moments.
      using BeforeLoading = TypeList<>;

      //T is a type that has at least
      //the variables specified in BeforeLoading
      template <class T>
      __device__ static void before(Parameters p,
                                    const T & data,
                                    const int cluster)
      {
      }

      //CellLoading has access just to the cell index
      using CellLoading    = TypeList<>;
      //ClusterLoading will have in the object passed to it
      //the cluster weight too.
      using ClusterLoading = TypeList<>;

      //T is a type that has at least
      //the variables specified in CellLoading and ClusterLoading,
      //as well as the cluster weight.
      template <class T>
      __device__ static  void per_cell(Parameters p,
                                       const T & data,
                                       const int cell,
                                       const int cluster)
      {
      }

      using AfterLoading = TypeList<>;

      //T is a type that has at least
      //the variables specified in AfterLoading
      template <class T>
      __device__ static void after(Parameters p,
                                   const T & data,
                                   const int cluster)
      {
      }
    };

#define CALORECGPU_CMC_MOMENT_CALC(NAME, BEFORELOAD, BEFOREEXEC, CELLLOAD, CLUSTERLOAD, CELLEXEC, AFTERLOAD, AFTEREXEC)                \
  struct NAME                                                                                                                          \
  {                                                                                                                                    \
    using BeforeLoading = TypeList<CALORECGPU_CMC_EXPAND BEFORELOAD>;                                                                      \
    template <class T>  __device__ static void before(Parameters p,                                                                    \
                                                      const T & data,                                                                  \
                                                      const int cluster)                                                               \
    { CALORECGPU_CMC_EXPAND BEFOREEXEC }                                                                                                   \
    using CellLoading    = TypeList<CALORECGPU_CMC_EXPAND CELLLOAD>;                                                                       \
    using ClusterLoading = TypeList<CALORECGPU_CMC_EXPAND CLUSTERLOAD>;                                                                    \
    template <class T> __device__ static  void per_cell(Parameters p,                                                                  \
                                                        const T & data,                                                                \
                                                        const int cell,                                                                \
                                                        const int cluster)                                                             \
    { CALORECGPU_CMC_EXPAND CELLEXEC }                                                                                                     \
    using AfterLoading = TypeList<CALORECGPU_CMC_EXPAND AFTERLOAD>;                                                                        \
    template <class T> __device__ static void after(Parameters p,                                                                      \
                                                    const T & data,                                                                    \
                                                    const int cluster)                                                                 \
    { CALORECGPU_CMC_EXPAND AFTEREXEC }                                                                                                    \
  }

    //-------------------------------

    //ISOLATION & ENG_FRAC_CORE SPECIAL CASING TO BE CONSIDERED LATER.
    //ALSO CENTER LAMBDA!

    //+---------------------------------------------------+
    //|            Regular cluster properties:            |
    //+---------------------------------------------------+

    CALORECGPU_CMC_MOMENT_CALC
    (ClusterEnergyEtaAndEt,
     (),
     (p.clusters_arr->clusterEnergy[cluster] = 0.f;
      CMCTemporaries::clusterEnergyAux(p.clusters_arr, cluster) = 0.f;
      p.clusters_arr->clusterEta[cluster] = 0.f;
      CMCTemporaries::clusterEtaAux(p.clusters_arr, cluster) = 0.f;),
     (ToLoad::CellEnergy, ToLoad::CellAbsEnergy, ToLoad::CellEta),
     (),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->clusterEnergy[cluster]),
                                                          CMCTemporaries::clusterEnergyAux_ptr(p.clusters_arr, cluster),
                                                          data.energy * data.weight);
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->clusterEta[cluster]),
                                                          CMCTemporaries::clusterEtaAux_ptr(p.clusters_arr, cluster),
                                                          data.abs_energy * data.weight * data.eta);
     ),
     (ToLoad::SumAbsEnergyNonMoments, ToLoad::ReverseSumAbsEnergyNonMoments),
     (const float temp_E_main = p.clusters_arr->clusterEnergy[cluster];
      const float temp_E_corr = CMCTemporaries::clusterEnergyAux(p.clusters_arr, cluster);
      
      const float temp_eta_main = p.clusters_arr->clusterEta[cluster];
      const float temp_eta_corr = CMCTemporaries::clusterEtaAux(p.clusters_arr, cluster);
      
      const float temp_exp_1 = expf(temp_eta_main * data.rev_abs_energy_non_moments);
      const float temp_exp_2 = expf(temp_eta_corr * data.rev_abs_energy_non_moments);
      
      const float temp_exp_mult = temp_exp_1 * temp_exp_2;
      
      const float temp_numerator   = 2.f * CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(temp_E_main, temp_exp_mult,
                                                                                                 temp_E_corr, temp_exp_mult);
      const float temp_inv_denominator = 1.f/fmaf(temp_exp_mult, temp_exp_mult, 1.f);
      
      const float temp_ET = temp_numerator * temp_inv_denominator;

      const float temp_energy = temp_E_main + temp_E_corr;

      p.clusters_arr->clusterEnergy[cluster] = temp_energy;

      const float temp_eta = (temp_eta_main + temp_eta_corr) * data.rev_abs_energy_non_moments;

      p.clusters_arr->clusterEta[cluster] = temp_eta * (data.abs_energy_non_moments != 0.f);

      //const float temp_ET = temp_energy / coshf(abs(temp_eta));
      
      p.clusters_arr->clusterEt[cluster] = temp_ET * (data.abs_energy_non_moments != 0.f);
     )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (ClusterPhi,
     (),
     (p.clusters_arr->clusterPhi[cluster] = 0.f;
      CMCTemporaries::clusterPhiAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellAbsEnergy, ToLoad::CellPhi),
     (ToLoad::SeedCellPhi),
     (const float phi_real = CaloRecGPU::Helpers::regularize_angle(data.phi, data.phi_0);
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->clusterPhi[cluster]),
                                                          CMCTemporaries::clusterPhiAux_ptr(p.clusters_arr, cluster),
                                                          phi_real * data.abs_energy * data.weight);
     ),
     (ToLoad::SumAbsEnergyNonMoments, ToLoad::ReverseSumAbsEnergyNonMoments),
     (const float old_phi = p.clusters_arr->clusterPhi[cluster] + CMCTemporaries::clusterPhiAux(p.clusters_arr, cluster);
      p.clusters_arr->clusterPhi[cluster] = CaloRecGPU::Helpers::regularize_angle(old_phi *
                                                                                  data.rev_abs_energy_non_moments, 0.f) *
                                            (data.abs_energy_non_moments != 0.f);
     )
    );

    //+---------------------------------------------------+
    //|              Cluster moments proper:              |
    //+---------------------------------------------------+

    CALORECGPU_CMC_MOMENT_CALC
    (AvgLArQ,
     (),
     (p.clusters_arr->moments.avgLArQ[cluster] = 0.f;
      CMCTemporaries::avgLArQAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellQualityProvenance, ToLoad::CellLArQCheck),
     (ToLoad::SquareWeightedEnergyOrNegative),
     (if (data.LArQ_cell_check)
    {
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.avgLArQ[cluster]),
                                                          CMCTemporaries::avgLArQAux_ptr(p.clusters_arr, cluster),
                                                          data.square_w_E_or_neg * data.qp.quality());
      }
     ),
    (),
    (const float norm_LAr = CMCTemporaries::averageLArQNorm(p.clusters_arr, cluster) + CMCTemporaries::averageLArQNormAux(p.clusters_arr, cluster);
     const float rev_norm_LAr = 1.0f / (norm_LAr > 0.f ? norm_LAr : 1.0f);
     const float new_LArQ = p.clusters_arr->moments.avgLArQ[cluster] + CMCTemporaries::avgLArQAux(p.clusters_arr, cluster);
     p.clusters_arr->moments.avgLArQ[cluster] = new_LArQ * rev_norm_LAr;
    )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (AvgTileQ,
     (),
     (p.clusters_arr->moments.avgTileQ[cluster] = 0.f;
      CMCTemporaries::avgTileQAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellQualityProvenance, ToLoad::CellTileQCheck),
     (ToLoad::SquareWeightedEnergyOrNegative),
     (if (data.TileQ_cell_check)
    {
      const float max_quality = max((unsigned int) data.qp.tile_qual1(), (unsigned int) data.qp.tile_qual2());

        CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.avgTileQ[cluster]),
                                                            CMCTemporaries::avgTileQAux_ptr(p.clusters_arr, cluster),
                                                            data.square_w_E_or_neg * max_quality);
      }
     ),
    (),
    (const float norm_Tile = CMCTemporaries::averageTileQNorm(p.clusters_arr, cluster) + CMCTemporaries::averageTileQNormAux(p.clusters_arr, cluster);
     const float rev_norm_Tile = 1.0f / (norm_Tile > 0.f ? norm_Tile : 1.0f);
     const float new_TileQ = p.clusters_arr->moments.avgTileQ[cluster] + CMCTemporaries::avgTileQAux(p.clusters_arr, cluster);
     p.clusters_arr->moments.avgTileQ[cluster] = new_TileQ * rev_norm_Tile;
    )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (BadCellsCorrE,
     (),
     (p.clusters_arr->moments.badCellsCorrE[cluster] = 0.f;
      CMCTemporaries::badCellsCorrEAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellIsBad, ToLoad::CellEnergy),
     (ToLoad::WeightedEnergyOrNegative),
     (if (data.is_bad && data.energy != 0.f)
    {
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.badCellsCorrE[cluster]),
                                                          CMCTemporaries::badCellsCorrEAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy_or_negative);
      }
     ),
    (),
    (p.clusters_arr->moments.badCellsCorrE[cluster] += CMCTemporaries::badCellsCorrEAux(p.clusters_arr, cluster);
    )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (BadLArQFrac,
     (),
     (p.clusters_arr->moments.badLArQFrac[cluster] = 0.f;
      CMCTemporaries::badLArQFracAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellQualityProvenance, ToLoad::CellLArQCheck),
     (ToLoad::WeightedEnergyOrNegative),
     (if (data.LArQ_cell_check && data.qp.quality() > p.opts->min_LAr_quality)
    {
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.badLArQFrac[cluster]),
                                                          CMCTemporaries::badLArQFracAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy_or_negative);
      }
     ),
    (ToLoad::ReverseClusterEnergy),
    (const float new_badLArQFrac = p.clusters_arr->moments.badLArQFrac[cluster] + CMCTemporaries::badLArQFracAux(p.clusters_arr, cluster);
     p.clusters_arr->moments.badLArQFrac[cluster] = new_badLArQFrac * data.rev_cluster_energy;
    )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (CellSignificance,
     (),
     (),
     (),
     (),
     (),
     (ToLoad::MaxSignificanceAndSampling),
     (const float max_sig = __uint_as_float(data.max_sig_and_samp >> 32);
      p.clusters_arr->moments.cellSignificance[cluster] = max_sig * (data.max_sig_and_samp & 1 ? 1.f : -1.f);
     )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (CellSigSampling,
     (),
     (),
     (),
     (),
     (),
     (ToLoad::MaxSignificanceAndSampling),
     (const int max_samp = (data.max_sig_and_samp & 0xFFFFFFFEU) >> 1;
      p.clusters_arr->moments.cellSigSampling[cluster] = max_samp;
     )
    );


    CALORECGPU_CMC_MOMENT_CALC
    (CenterX,
     (),
     (p.clusters_arr->moments.centerX[cluster] = 0.f;
      CMCTemporaries::centerXAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellX),
     (ToLoad::WeightedEnergy),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.centerX[cluster]),
                                                          CMCTemporaries::centerXAux_ptr(p.clusters_arr, cluster),
                                                          data.x * data.weighted_energy);
     ),
     (ToLoad::ReverseSumEnergies),
     (const float new_value = p.clusters_arr->moments.centerX[cluster] + CMCTemporaries::centerXAux(p.clusters_arr, cluster);
      p.clusters_arr->moments.centerX[cluster] = new_value * data.rev_sum_energies;
     )
    );
    CALORECGPU_CMC_MOMENT_CALC
    (CenterY,
     (),
     (p.clusters_arr->moments.centerY[cluster] = 0.f;
      CMCTemporaries::centerYAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellY),
     (ToLoad::WeightedEnergy),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.centerY[cluster]),
                                                          CMCTemporaries::centerYAux_ptr(p.clusters_arr, cluster),
                                                          data.y * data.weighted_energy);
     ),
     (ToLoad::ReverseSumEnergies),
     (const float new_value = p.clusters_arr->moments.centerY[cluster] + CMCTemporaries::centerYAux(p.clusters_arr, cluster);
      p.clusters_arr->moments.centerY[cluster] = new_value * data.rev_sum_energies;
     )
    );
    CALORECGPU_CMC_MOMENT_CALC
    (CenterZ,
     (),
     (p.clusters_arr->moments.centerZ[cluster] = 0.f;
      CMCTemporaries::centerZAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellZ),
     (ToLoad::WeightedEnergy),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.centerZ[cluster]),
                                                          CMCTemporaries::centerZAux_ptr(p.clusters_arr, cluster),
                                                          data.z * data.weighted_energy);
     ),
     (ToLoad::ReverseSumEnergies),
     (const float new_value = p.clusters_arr->moments.centerZ[cluster] + CMCTemporaries::centerZAux(p.clusters_arr, cluster);
      p.clusters_arr->moments.centerZ[cluster] = new_value * data.rev_sum_energies;
     )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (EngFracEM,
     (),
     (p.clusters_arr->moments.engFracEM[cluster] = 0.f;
      CMCTemporaries::engFracEMAux(p.clusters_arr, cluster) = 0.f;),
     (ToLoad::CellSampling),
     (ToLoad::WeightedEnergy),
     ( if ( data.sampling == CaloSampling::EMB1   ||
            data.sampling == CaloSampling::EMB2   ||
            data.sampling == CaloSampling::EMB3   ||
            data.sampling == CaloSampling::EME1   ||
            data.sampling == CaloSampling::EME2   ||
            data.sampling == CaloSampling::EME3   ||
            data.sampling == CaloSampling::FCAL0     )
    {
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.engFracEM[cluster]),
                                                          CMCTemporaries::engFracEMAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy);
      }
     ),
    (ToLoad::ReverseSumEnergies),
    (const float new_engFracEM = p.clusters_arr->moments.engFracEM[cluster] + CMCTemporaries::engFracEMAux(p.clusters_arr, cluster);
     p.clusters_arr->moments.engFracEM[cluster] = new_engFracEM * data.rev_sum_energies;
    )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (EngBadCells,
     (),
     (p.clusters_arr->moments.engBadCells[cluster] = 0.f;
      CMCTemporaries::engBadCellsAux(p.clusters_arr, cluster) = 0.f;),
     (ToLoad::CellIsBad),
     (ToLoad::WeightedEnergyOrNegative),
     (if (data.is_bad)
    {
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.engBadCells[cluster]),
                                                          CMCTemporaries::engBadCellsAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy_or_negative);
      }
     ),
    (),
    (p.clusters_arr->moments.engBadCells[cluster] += CMCTemporaries::engBadCellsAux(p.clusters_arr, cluster);)
    );

    CALORECGPU_CMC_MOMENT_CALC
    (EngFracMax,
     (),
     (),
     (),
     (),
     (),
     (ToLoad::ReverseSumEnergies, ToLoad::ClusterMaxCellEnergy),
     (p.clusters_arr->moments.engFracMax[cluster] = data.max_E * data.rev_sum_energies;)
    );

    CALORECGPU_CMC_MOMENT_CALC
    (EngPosAndEngFracCore,
     (),
     (const float sum_energies = p.clusters_arr->moments.engPos[cluster] + CMCTemporaries::engPosAux(p.clusters_arr, cluster);
      p.clusters_arr->moments.engPos[cluster] = sum_energies;
      p.clusters_arr->moments.engFracCore[cluster] *= (sum_energies != 0.f ? 1.0f / sum_energies : 0.f);
     ),
     (),
     (),
     (),
     (),
     ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (FirstEngDens,
     (),
     (p.clusters_arr->moments.firstEngDens[cluster] = 0.f;
      CMCTemporaries::firstEngDensAux(p.clusters_arr, cluster) = 0.f;),
     (ToLoad::CellVolume),
     (ToLoad::WeightedEnergy, ToLoad::WeightedEnergyOverVolume),
     (if (data.volume > 0.f)
    {
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.firstEngDens[cluster]),
                                                          CMCTemporaries::firstEngDensAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy * data.w_E_over_V);
      }
     ),
    (ToLoad::ReverseEnergyDensityNormalization),
    (const float new_firstEngDens = p.clusters_arr->moments.firstEngDens[cluster] + CMCTemporaries::firstEngDensAux(p.clusters_arr, cluster);
     p.clusters_arr->moments.firstEngDens[cluster] = new_firstEngDens * data.rev_energy_density_norm;
    )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (FirstEta,
     (),
     (p.clusters_arr->moments.firstEta[cluster] = 0.f;
      CMCTemporaries::firstEtaAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellEta),
     (ToLoad::WeightedEnergy),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.firstEta[cluster]),
                                                          CMCTemporaries::firstEtaAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy * data.eta);
     ),
     (ToLoad::ReverseSumEnergies),
     (const float new_firstEta = p.clusters_arr->moments.firstEta[cluster] + CMCTemporaries::firstEtaAux(p.clusters_arr, cluster);
      p.clusters_arr->moments.firstEta[cluster] = new_firstEta * data.rev_sum_energies;
     )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (FirstPhi,
     (),
     (p.clusters_arr->moments.firstPhi[cluster] = 0.f;
      CMCTemporaries::firstPhiAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellPhi),
     (ToLoad::SeedCellPhi, ToLoad::WeightedEnergy),
     (const float phi_real = CaloRecGPU::Helpers::regularize_angle(data.phi, data.phi_0);
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.firstPhi[cluster]),
                                                          CMCTemporaries::firstPhiAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy * phi_real);
     ),
     (ToLoad::ReverseSumEnergies),
     (const float new_firstPhi = p.clusters_arr->moments.firstPhi[cluster] + CMCTemporaries::firstPhiAux(p.clusters_arr, cluster);
      p.clusters_arr->moments.firstPhi[cluster] = CaloRecGPU::Helpers::regularize_angle(new_firstPhi * data.rev_sum_energies);
     )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (Lateral,
     (),
     (p.clusters_arr->moments.lateral[cluster] = 0.f;
      CMCTemporaries::lateralAux(p.clusters_arr, cluster) = 0.f;
     ),
     (),
     (ToLoad::WeightedEnergy, ToLoad::ClusterMaxAndSecondMaxCell, ToLoad::R),
     (if (cell != data.max_cell && cell != data.second_max_cell)
    {
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.lateral[cluster]),
                                                          CMCTemporaries::lateralAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy * data.r * data.r);
      }
     ),
    (),
    (const float new_lateral = p.clusters_arr->moments.lateral[cluster] + CMCTemporaries::lateralAux(p.clusters_arr, cluster);
     const float new_norm    = CMCTemporaries::lateralNormalization(p.clusters_arr, cluster) +
                               CMCTemporaries::lateralNormalizationAux(p.clusters_arr, cluster);
     p.clusters_arr->moments.lateral[cluster] = new_lateral / (new_norm != 0.f ? new_norm : 1.f);
    )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (Longitudinal,
     (),
     (p.clusters_arr->moments.longitudinal[cluster] = 0.f;
      CMCTemporaries::longitudinalAux(p.clusters_arr, cluster) = 0.f;
     ),
     (),
     (ToLoad::WeightedEnergy, ToLoad::ClusterMaxAndSecondMaxCell, ToLoad::Lambda),
     (if (cell != data.max_cell && cell != data.second_max_cell)
    {
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.longitudinal[cluster]),
                                                          CMCTemporaries::longitudinalAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy * data.lambda * data.lambda);
      }
     ),
    (),
    (const float new_longitudinal = p.clusters_arr->moments.longitudinal[cluster] + CMCTemporaries::longitudinalAux(p.clusters_arr, cluster);
     const float new_norm          = CMCTemporaries::longitudinalNormalization(p.clusters_arr, cluster) +
                                     CMCTemporaries::longitudinalNormalizationAux(p.clusters_arr, cluster);
     p.clusters_arr->moments.longitudinal[cluster] = new_longitudinal / (new_norm != 0.f ? new_norm : 1.f);
    )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (Mass,
     (),
     (),
     (),
     (),
     (),
     (ToLoad::SumEnergies),
     (const float mx = CMCTemporaries::mX(p.clusters_arr, cluster) + CMCTemporaries::mXAux(p.clusters_arr, cluster);
      const float my = CMCTemporaries::mY(p.clusters_arr, cluster) + CMCTemporaries::mYAux(p.clusters_arr, cluster);
      const float mz = CMCTemporaries::mZ(p.clusters_arr, cluster) + CMCTemporaries::mZAux(p.clusters_arr, cluster);

      const float v_1 = mx * mx;
      const float v_2 = my * my;
      const float v_3 = mz * mz;
      const float v_4 = data.sum_energies * data.sum_energies;

      const float c_1 = fmaf(mx, mx, -v_1);
      const float c_2 = fmaf(my, my, -v_2);
      const float c_3 = fmaf(mz, mz, -v_3);
      const float c_4 = fmaf(data.sum_energies, data.sum_energies, -v_4);

      const float sq_mass = CaloRecGPU::Helpers::sum_kahan_babushka_neumaier(v_4, -v_1, -v_2, -v_3, c_4, -c_1, -c_2, -c_3);

      p.clusters_arr->moments.mass[cluster] = sqrtf(fabsf(sq_mass)) * ((sq_mass > 0.f) - (sq_mass < 0.f));
     )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (NBadCells,
     (),
     (p.clusters_arr->moments.nBadCells[cluster] = 0;),
     (ToLoad::CellIsBad),
     (),
     (if (data.is_bad)
    {
      atomicAdd(&(p.clusters_arr->moments.nBadCells[cluster]), 1);
      }),
    (),
    ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (NBadCellsCorr,
     (),
     (p.clusters_arr->moments.nBadCellsCorr[cluster] = 0;),
     (ToLoad::CellIsBad, ToLoad::CellEnergy),
     (),
     (if (data.is_bad && data.energy != 0.f)
    {
      atomicAdd(&(p.clusters_arr->moments.nBadCellsCorr[cluster]), 1);
      }
     ),
    (),
    ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (NExtraCellSampling,
     (),
     (p.clusters_arr->moments.nExtraCellSampling[cluster] = 0;),
     (ToLoad::CellSampling, ToLoad::CellEta),
     (),
     (if (data.sampling == CaloSampling::EME2 && fabsf(data.eta) > p.opts->eta_inner_wheel)
    {
      atomicAdd(&(p.clusters_arr->moments.nExtraCellSampling[cluster]), 1);
      }
     ),
    (),
    ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (PTD,
     (),
     (p.clusters_arr->moments.PTD[cluster] = 0.f;
      CMCTemporaries::PTDAux(p.clusters_arr, cluster) = 0.f;
     ),
     (),
     (ToLoad::SquareWeightedEnergy),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.PTD[cluster]),
                                                          CMCTemporaries::PTDAux_ptr(p.clusters_arr, cluster),
                                                          data.square_w_E);

      //Comment on there:
      //
      //  +--------------- begin comment on there ---------------+
      //  |                                                      |
      //  | do not convert to pT since clusters are small and    |
      //  | there is virtually no difference and cosh just costs |
      //  | time ...                                             |
      //  |                                                      |
      //  +---------------- end comment on there ----------------+
      //
      //So maybe we could change this here?
     ),
     (ToLoad::SumEnergies),
     (const float new_PTD = p.clusters_arr->moments.PTD[cluster] + CMCTemporaries::PTDAux(p.clusters_arr, cluster);
      p.clusters_arr->moments.PTD[cluster] = 1.0f / ((data.sum_energies > 0.f ? data.sum_energies : 1.f) * rsqrtf(new_PTD));
      //See before: maybe to be revised?
     )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (SecondEngDens,
     (),
     (p.clusters_arr->moments.secondEngDens[cluster] = 0.f;
      CMCTemporaries::secondEngDensAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellVolume),
     (ToLoad::WeightedEnergy, ToLoad::WeightedEnergyOverVolume),
     (if (data.volume > 0.f)
    {
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.secondEngDens[cluster]),
                                                          CMCTemporaries::secondEngDensAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy * data.w_E_over_V * data.w_E_over_V);
      }
     ),
    (ToLoad::ReverseEnergyDensityNormalization),
    (const float new_secondEngDens = p.clusters_arr->moments.secondEngDens[cluster] + CMCTemporaries::secondEngDensAux(p.clusters_arr, cluster);
     p.clusters_arr->moments.secondEngDens[cluster] = new_secondEngDens * data.rev_energy_density_norm;)
    );

    CALORECGPU_CMC_MOMENT_CALC
    (SecondLambda,
     (),
     (p.clusters_arr->moments.secondLambda[cluster] = 0.f;
      CMCTemporaries::secondLambdaAux(p.clusters_arr, cluster) = 0.f;
     ),
     (),
     (ToLoad::WeightedEnergy, ToLoad::Lambda),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.secondLambda[cluster]),
                                                          CMCTemporaries::secondLambdaAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy * data.lambda * data.lambda);
     ),
     (ToLoad::ReverseSumEnergies),
     (const float new_secondLambda = p.clusters_arr->moments.secondLambda[cluster] + CMCTemporaries::secondLambdaAux(p.clusters_arr, cluster);
      p.clusters_arr->moments.secondLambda[cluster] = new_secondLambda * data.rev_sum_energies;
     )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (SecondR,
     (),
     (p.clusters_arr->moments.secondR[cluster] = 0.f;
      CMCTemporaries::secondRAux(p.clusters_arr, cluster) = 0.f;
     ),
     (),
     (ToLoad::WeightedEnergy, ToLoad::R),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.secondR[cluster]),
                                                          CMCTemporaries::secondRAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy * data.r * data.r);
     ),
     (ToLoad::ReverseSumEnergies),
     (const float new_secondR = p.clusters_arr->moments.secondR[cluster] + CMCTemporaries::secondRAux(p.clusters_arr, cluster);
      p.clusters_arr->moments.secondR[cluster] = new_secondR * data.rev_sum_energies;
     )
    );


    CALORECGPU_CMC_MOMENT_CALC
    (Significance,
     (),
     (p.clusters_arr->moments.significance[cluster] = 0.f;
      CMCTemporaries::significanceAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellNoise),
     (),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.significance[cluster]),
                                                          CMCTemporaries::significanceAux_ptr(p.clusters_arr, cluster),
                                                          data.noise * data.noise);
     ),
     (ToLoad::ClusterEnergy),
     (const float prev_v = p.clusters_arr->moments.significance[cluster] + CMCTemporaries::significanceAux(p.clusters_arr, cluster);
      p.clusters_arr->moments.significance[cluster] = (prev_v > 0.f ? data.cluster_energy * rsqrtf(prev_v) : 0.f);)
    );

    CALORECGPU_CMC_MOMENT_CALC
    (TimeAndSecondTime,
     (),
     (p.clusters_arr->moments.time[cluster] = 0.f;
      CMCTemporaries::timeAux(p.clusters_arr, cluster) = 0.f;
      p.clusters_arr->moments.secondTime[cluster] = 0.f;
      CMCTemporaries::secondTimeAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellTime, ToLoad::CellTimeMomentsCheck),
     (ToLoad::SquaredWeightedNonMomentsEnergy),
     (if (data.time_moments_check)
    {
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.time[cluster]),
                                                          CMCTemporaries::timeAux_ptr(p.clusters_arr, cluster),
                                                          data.time * data.squared_normE);
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.secondTime[cluster]),
                                                          CMCTemporaries::secondTimeAux_ptr(p.clusters_arr, cluster),
                                                          data.time * data.time * data.squared_normE);
      }),
    (ToLoad::TimeNormalization),
    (if (data.time_norm != 0.f)
    {
      const float real_norm = 1.0f / data.time_norm;
      const float time = (p.clusters_arr->moments.time[cluster] + CMCTemporaries::timeAux(p.clusters_arr, cluster))
                           * real_norm;
        const float second_sum = p.clusters_arr->moments.secondTime[cluster] + CMCTemporaries::secondTimeAux(p.clusters_arr, cluster);
        p.clusters_arr->moments.time[cluster] = time;
        p.clusters_arr->moments.secondTime[cluster] = CaloRecGPU::Helpers::product_sum_cornea_harrison_tang(second_sum, real_norm, -time, time);
      }
    else
      {
        p.clusters_arr->moments.time[cluster] = 0.f;
        p.clusters_arr->moments.secondTime[cluster] = 0.f;
      }
    )
    );

    //+---------------------------------------------------+
    //|           Cluster per-sampling moments:           |
    //+---------------------------------------------------+

    template <int num, int delta = 0>
    CALORECGPU_CMC_MOMENT_CALC
    (EnergyPerSampleSeveral,
     (ToLoad::SamplingFromMomentIndex),
     (const int offset = data.sampling * num + delta;
      for (int i = 0; i < num; ++i)
    {
      p.clusters_arr->moments.energyPerSample[offset + i]
        [cluster] = 0.f;
        CMCTemporaries::energyPerSampleAux(p.clusters_arr, offset + i, cluster) = 0.f;
      }
     ),
    (ToLoad::CellEnergy, ToLoad::CellSampling),
    (),
    (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.energyPerSample[data.sampling][cluster]),
                                                         CMCTemporaries::energyPerSampleAux_ptr(p.clusters_arr, data.sampling, cluster),
                                                         data.energy * data.weight);
    ),
    (ToLoad::SamplingFromMomentIndex),
    (const int offset = data.sampling * num + delta;
     for (int i = 0; i < num; ++i)
    {
      p.clusters_arr->moments.energyPerSample[offset + i]
        [cluster] += CMCTemporaries::energyPerSampleAux(p.clusters_arr, offset + i, cluster);
      }
    )
    );

    using EnergyPerSample = EnergyPerSampleSeveral<1, 0>;


    template <int num, int delta = 0>
    CALORECGPU_CMC_MOMENT_CALC
    (EtaPerSampleSeveral,
     (ToLoad::SamplingFromMomentIndex),
     (const int offset = data.sampling * num + delta;
      for (int i = 0; i < num; ++i)
    {
      p.clusters_arr->moments.etaPerSample[offset + i]
        [cluster] = 0.f;
        CMCTemporaries::etaPerSampleAux(p.clusters_arr, offset + i, cluster) = 0.f;
      }
     ),
    (ToLoad::CellSampling, ToLoad::CellAbsEnergy, ToLoad::CellEta),
    (),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.etaPerSample[data.sampling][cluster]),
                                                          CMCTemporaries::etaPerSampleAux_ptr(p.clusters_arr, data.sampling, cluster),
                                                          data.abs_energy * data.weight * data.eta);
    ),
    (ToLoad::SamplingFromMomentIndex/*, ToLoad::ReverseAbsoluteEnergyPerSample*/),
    (const int offset = data.sampling * num + delta;
     for (int i = 0; i < num; ++i)
    {
      const int idx = offset + i;
      const float normalization = CMCTemporaries::absoluteEnergyPerSample(p.clusters_arr, idx, cluster) +
                                    CMCTemporaries::absoluteEnergyPerSampleAux(p.clusters_arr, idx, cluster);
        const float rev_normalization = 1.0f / (normalization != 0.f ? normalization : 1.0f);
        const float new_eta = p.clusters_arr->moments.etaPerSample[idx][cluster] + CMCTemporaries::etaPerSampleAux(p.clusters_arr, idx, cluster);
        p.clusters_arr->moments.etaPerSample[idx][cluster] = new_eta * rev_normalization;
      }
    )
    );

    using EtaPerSample = EtaPerSampleSeveral<1, 0>;

    template <int num, int delta = 0>
    CALORECGPU_CMC_MOMENT_CALC
    (NCellSamplingSeveral,
     (ToLoad::SamplingFromMomentIndex),
     (const int offset = data.sampling * num + delta;
      for (int i = 0; i < num; ++i)
    {
      p.clusters_arr->moments.nCellSampling[offset + i]
        [cluster] = 0;
      }
     ),
    (ToLoad::CellSampling),
    (),
    (atomicAdd(&(p.clusters_arr->moments.nCellSampling[data.sampling][cluster]), 1);),
    (),
    ()
    );

    using NCellSampling = NCellSamplingSeveral<1, 0>;

    template <int num, int delta = 0>
    CALORECGPU_CMC_MOMENT_CALC
    (PhiPerSampleSeveral,
     (ToLoad::SamplingFromMomentIndex),
     (const int offset = data.sampling * num + delta;
      for (int i = 0; i < num; ++i)
    {
      p.clusters_arr->moments.phiPerSample[offset + i]
        [cluster] = 0.f;
        CMCTemporaries::phiPerSampleAux(p.clusters_arr, offset + i, cluster) = 0.f;
      }
     ),
    (ToLoad::CellSampling, ToLoad::CellAbsEnergy, ToLoad::CellPhi),
    (ToLoad::SeedCellPhi),
    (const float phi_real = CaloRecGPU::Helpers::regularize_angle(data.phi, data.phi_0);
     CaloRecGPU::Helpers::device_kahan_babushka_neumaier(&(p.clusters_arr->moments.phiPerSample[data.sampling][cluster]),
                                                         CMCTemporaries::phiPerSampleAux_ptr(p.clusters_arr, data.sampling, cluster),
                                                         data.abs_energy * data.weight * phi_real);
    ),
    (ToLoad::SamplingFromMomentIndex/*, ToLoad::ReverseAbsoluteEnergyPerSample*/),
    (const int offset = data.sampling * num + delta;
     for (int i = 0; i < num; ++i)
    {
      const int idx = offset + i;
      const float normalization = CMCTemporaries::absoluteEnergyPerSample(p.clusters_arr, idx, cluster) +
                                    CMCTemporaries::absoluteEnergyPerSampleAux(p.clusters_arr, idx, cluster);
        const float rev_normalization = 1.0f / (normalization != 0.f ? normalization : 1.0f);
        const float new_phi = p.clusters_arr->moments.phiPerSample[idx][cluster] + CMCTemporaries::phiPerSampleAux(p.clusters_arr, idx, cluster);
        p.clusters_arr->moments.phiPerSample[idx][cluster] = CaloRecGPU::Helpers::regularize_angle(new_phi * rev_normalization, 0.f);
      }
    )
    );

    using PhiPerSample = PhiPerSampleSeveral<1, 0>;

    //+---------------------------------------------------+
    //|                 Temporary Values:                 |
    //+---------------------------------------------------+


    CALORECGPU_CMC_MOMENT_CALC
    (AverageLArQNormalization,
     (),
     (CMCTemporaries::averageLArQNorm(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::averageLArQNormAux(p.clusters_arr, cluster) = 0.f;),
     (ToLoad::CellLArQCheck),
     (ToLoad::SquareWeightedEnergyOrNegative),
     (if (data.LArQ_cell_check)
    {
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::averageLArQNorm_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::averageLArQNormAux_ptr(p.clusters_arr, cluster),
                                                          data.square_w_E_or_neg);
      }
     ),
    (),
    ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (AverageTileQNormalization,
     (),
     (CMCTemporaries::averageTileQNorm(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::averageTileQNormAux(p.clusters_arr, cluster) = 0.f;),
     (ToLoad::CellQualityProvenance, ToLoad::CellTileQCheck),
     (ToLoad::SquareWeightedEnergyOrNegative),
     (if (data.TileQ_cell_check)
    {
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::averageTileQNorm_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::averageTileQNormAux_ptr(p.clusters_arr, cluster),
                                                          data.square_w_E_or_neg);
      }
     ),
    (),
    ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (EnergyDensityNormalization,
     (),
     (CMCTemporaries::energyDensityNormalization(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::energyDensityNormalizationAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellVolume),
     (ToLoad::WeightedEnergy),
     (if (data.volume > 0.f)
    {
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::energyDensityNormalization_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::energyDensityNormalizationAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy);
      }
     ),
    (),
    ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (FirstAndSecondMaxEnergyAndCell,
     (),
     (CMCTemporaries::maxCellEnergyAndCell(p.clusters_arr, cluster) = 0ULL;
      CMCTemporaries::secondMaxCellEnergyAndCell(p.clusters_arr, cluster) = 0ULL;
     ),
     (),
     (ToLoad::WeightedEnergy),
     (if (data.weighted_energy > 0)
    {
      unsigned long long int energy_and_cell = __float_as_uint(data.weighted_energy);
        //Energy is positive, so no need to switch to total ordering...
        energy_and_cell = (energy_and_cell << 32) | (cell + 1);
        const unsigned long long int old_enc = atomicMax(&(CMCTemporaries::maxCellEnergyAndCell(p.clusters_arr, cluster)), energy_and_cell);
        atomicMax(&(CMCTemporaries::secondMaxCellEnergyAndCell(p.clusters_arr, cluster)), min(old_enc, energy_and_cell));
      }
     ),
    (),
    ()
    );


    CALORECGPU_CMC_MOMENT_CALC
    (LateralNormalization,
     (),
     (CMCTemporaries::lateralNormalization(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::lateralNormalizationAux(p.clusters_arr, cluster) = 0.f;
     ),
     (),
     (ToLoad::WeightedEnergy, ToLoad::ClusterMaxAndSecondMaxCell, ToLoad::R),
     (const float real_r = (cell != data.max_cell && cell != data.second_max_cell) ?
                           data.r : max(data.r, p.opts->min_r_lateral);

      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::lateralNormalization_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::lateralNormalizationAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy * real_r * real_r);
     ),
     (),
     ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (LongitudinalNormalization,
     (),
     (CMCTemporaries::longitudinalNormalization(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::longitudinalNormalizationAux(p.clusters_arr, cluster) = 0.f;
     ),
     (),
     (ToLoad::WeightedEnergy, ToLoad::ClusterMaxAndSecondMaxCell, ToLoad::Lambda),
     (const float real_lambda = (cell != data.max_cell && cell != data.second_max_cell) ?
                                data.lambda : max(data.lambda, p.opts->min_l_longitudinal);

      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::longitudinalNormalization_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::longitudinalNormalizationAux_ptr(p.clusters_arr, cluster),
                                                          data.weighted_energy * real_lambda * real_lambda);
     ),
     (),
     ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (Matrix00,
     (),
     (CMCTemporaries::matrix00(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::matrix00Aux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellX),
     (ToLoad::CenterX, ToLoad::SquareWeightedEnergy),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::matrix00_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::matrix00Aux_ptr(p.clusters_arr, cluster),
                                                          data.square_w_E * (data.x - data.center_x) * (data.x - data.center_x));
     ),
     (),
     ()
    );
    CALORECGPU_CMC_MOMENT_CALC
    (Matrix10,
     (),
     (CMCTemporaries::matrix10(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::matrix10Aux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellX, ToLoad::CellY),
     (ToLoad::CenterX, ToLoad::CenterY, ToLoad::SquareWeightedEnergy),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::matrix10_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::matrix10Aux_ptr(p.clusters_arr, cluster),
                                                          data.square_w_E * (data.x - data.center_x) * (data.y - data.center_y));
     ),
     (),
     ()
    );
    CALORECGPU_CMC_MOMENT_CALC
    (Matrix20,
     (),
     (CMCTemporaries::matrix20(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::matrix20Aux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellX, ToLoad::CellZ),
     (ToLoad::CenterX, ToLoad::CenterZ, ToLoad::SquareWeightedEnergy),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::matrix20_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::matrix20Aux_ptr(p.clusters_arr, cluster),
                                                          data.square_w_E * (data.x - data.center_x) * (data.z - data.center_z));
     ),
     (),
     ()
    );
    CALORECGPU_CMC_MOMENT_CALC
    (Matrix11,
     (),
     (CMCTemporaries::matrix11(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::matrix11Aux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellY),
     (ToLoad::CenterY, ToLoad::SquareWeightedEnergy),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::matrix11_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::matrix11Aux_ptr(p.clusters_arr, cluster),
                                                          data.square_w_E * (data.y - data.center_y) * (data.y - data.center_y));
     ),
     (),
     ()
    );
    CALORECGPU_CMC_MOMENT_CALC
    (Matrix21,
     (),
     (CMCTemporaries::matrix21(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::matrix21Aux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellY, ToLoad::CellZ),
     (ToLoad::CenterY, ToLoad::CenterZ, ToLoad::SquareWeightedEnergy),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::matrix21_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::matrix21Aux_ptr(p.clusters_arr, cluster),
                                                          data.square_w_E * (data.y - data.center_y) * (data.z - data.center_z));
     ),
     (),
     ()
    );
    CALORECGPU_CMC_MOMENT_CALC
    (Matrix22,
     (),
     (CMCTemporaries::matrix22(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::matrix22Aux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellZ),
     (ToLoad::CenterZ, ToLoad::SquareWeightedEnergy),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::matrix22_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::matrix22Aux_ptr(p.clusters_arr, cluster),
                                                          data.square_w_E * (data.z - data.center_z) * (data.z - data.center_z));
     ),
     (),
     ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (MaxAndSecondMaxCells,
     (),
     (),
     (),
     (),
     (),
     (ToLoad::ClusterCellWithMaxEnergy, ToLoad::ClusterCellWithSecondMaxEnergy),
     (unsigned long long to_store = data.max_E_cell;
      to_store = (to_store << 32u) | static_cast<unsigned int>(data.second_max_E_cell);
      CMCTemporaries::maxAndSecondMaxCells(p.clusters_arr, cluster) = to_store;
     )
    );

    CALORECGPU_CMC_MOMENT_CALC
    (MaxSignificanceAndSampling,
     (),
     (CMCTemporaries::maxSignificanceAndSampling(p.clusters_arr, cluster) = 0ULL;),
     (ToLoad::CellSampling, ToLoad::CellNoise),
     (ToLoad::WeightedEnergyOrNegative),
     (const float max_sig = data.noise > 0.f ? data.weighted_energy_or_negative / data.noise : 0.f;
      unsigned long long int max_S_and_S = __float_as_uint(fabsf(max_sig));
      max_S_and_S = (max_S_and_S << 32) | (static_cast<unsigned long long int>(data.sampling) << 1) | (max_sig > 0.f);
      atomicMax(&(CMCTemporaries::maxSignificanceAndSampling(p.clusters_arr, cluster)), max_S_and_S);),
     (),
     ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (MX,
     (),
     (CMCTemporaries::mX(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::mXAux(p.clusters_arr, cluster) = 0.f;),
     (ToLoad::CellX),
     (ToLoad::WeightedCellPositionNormalization),
     (const float mx = data.w_E_r_dir * data.x;
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::mX_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::mXAux_ptr(p.clusters_arr, cluster),
                                                          mx);
     ),
     (),
     ()
    );
    CALORECGPU_CMC_MOMENT_CALC
    (MY,
     (),
     (CMCTemporaries::mY(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::mYAux(p.clusters_arr, cluster) = 0.f;),
     (ToLoad::CellY),
     (ToLoad::WeightedCellPositionNormalization),
     (const float my = data.w_E_r_dir * data.y;
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::mY_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::mYAux_ptr(p.clusters_arr, cluster),
                                                          my);
     ),
     (),
     ()
    );
    CALORECGPU_CMC_MOMENT_CALC
    (MZ,
     (),
     (CMCTemporaries::mZ(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::mZAux(p.clusters_arr, cluster) = 0.f;),
     (ToLoad::CellZ),
     (ToLoad::WeightedCellPositionNormalization),
     (const float mz = data.w_E_r_dir * data.z;
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::mZ_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::mZAux_ptr(p.clusters_arr, cluster),
                                                          mz);
     ),
     (),
     ()
    );


    CALORECGPU_CMC_MOMENT_CALC
    (NumPositiveEnergyCells,
     (),
     (CMCTemporaries::numPositiveEnergyCells(p.clusters_arr, cluster) = 0;),
     (),
     (ToLoad::WeightedEnergyOrNegative),
     (if (data.weighted_energy_or_negative > 0)
    {
      atomicAdd(&(CMCTemporaries::numPositiveEnergyCells(p.clusters_arr, cluster)), 1);
      }
     ),
    (),
    ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (SeedCellPhi,
     (ToLoad::SeedCellGeometryPhi),
     (CMCTemporaries::seedCellPhi(p.clusters_arr, cluster) = data.seed_cell_phi_coordinate;),
     (),
     (),
     (),
     (),
     ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (SumAbsEnergyNonMoments,
     (),
     (CMCTemporaries::sumAbsEnergyNonMoments(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::sumAbsEnergyNonMomentsAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellAbsEnergy),
     (),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::sumAbsEnergyNonMoments_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::sumAbsEnergyNonMomentsAux_ptr(p.clusters_arr, cluster),
                                                          data.abs_energy * data.weight);
     ),
     (),
     ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (SumSquareEnergies,
     (),
     (CMCTemporaries::sumSquareEnergies(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::sumSquareEnergiesAux(p.clusters_arr, cluster) = 0.f;
     ),
     (),
     (ToLoad::SquareWeightedEnergy),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::sumSquareEnergies_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::sumSquareEnergiesAux_ptr(p.clusters_arr, cluster),
                                                          data.square_w_E);
     ),
     (),
     ()
    );

    CALORECGPU_CMC_MOMENT_CALC
    (TimeNormalization,
     (),
     (CMCTemporaries::timeNormalization(p.clusters_arr, cluster) = 0.f;
      CMCTemporaries::timeNormalizationAux(p.clusters_arr, cluster) = 0.f;
     ),
     (ToLoad::CellTimeMomentsCheck),
     (ToLoad::SquaredWeightedNonMomentsEnergy),
     (if (data.time_moments_check)
    {
      CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::timeNormalization_ptr(p.clusters_arr, cluster),
                                                          CMCTemporaries::timeNormalizationAux_ptr(p.clusters_arr, cluster),
                                                          data.squared_normE);
      }
     ),
    (),
    ()
    );


    //+---------------------------------------------------+
    //|             Per-Sampling Temporaries:             |
    //+---------------------------------------------------+

    template <int num, int delta = 0>
    CALORECGPU_CMC_MOMENT_CALC
    (AbsoluteEnergyPerSampleSeveral,
     (ToLoad::SamplingFromMomentIndex),
     (const int offset = data.sampling * num + delta;
      for (int i = 0; i < num; ++i)
    {
      CMCTemporaries::absoluteEnergyPerSample(p.clusters_arr, offset + i, cluster)
          = 0.f;
        CMCTemporaries::absoluteEnergyPerSampleAux(p.clusters_arr, offset + i, cluster) = 0.f;
      }
     ),
    (ToLoad::CellSampling, ToLoad::CellAbsEnergy),
    (),
     (CaloRecGPU::Helpers::device_kahan_babushka_neumaier(CMCTemporaries::absoluteEnergyPerSample_ptr(p.clusters_arr, data.sampling, cluster),
                                                          CMCTemporaries::absoluteEnergyPerSampleAux_ptr(p.clusters_arr, data.sampling, cluster),
                                                          data.abs_energy * data.weight);
    ),
    (),
    ()
    );

    using AbsoluteEnergyPerSample = AbsoluteEnergyPerSampleSeveral<1, 0>;

    template <int num, int delta = 0>
    CALORECGPU_CMC_MOMENT_CALC
    (MaxEnergyAndCellPerSampleSeveral,
     (ToLoad::SamplingFromMomentIndex),
     (const int offset = data.sampling * num + delta;
      for (int i = 0; i < num; ++i)
    {
      CMCTemporaries::maxEnergyAndCellPerSample(p.clusters_arr, offset + i, cluster)
          = 0ULL;
      }
     ),
    (ToLoad::CellSampling, ToLoad::CellEnergy),
    (),
    (const unsigned int energy_pattern = __float_as_uint(data.energy * data.weight);
     unsigned long long int E_and_cell = FloatingPointHelpers::StandardFloat::to_total_ordering(energy_pattern);
     E_and_cell = (E_and_cell << 32) | cell;
     atomicMax(&(CMCTemporaries::maxEnergyAndCellPerSample(p.clusters_arr, data.sampling, cluster)), E_and_cell);
    ),
    (),
    ()
    );

    using MaxEnergyAndCellPerSample = MaxEnergyAndCellPerSampleSeveral<1, 0>;

    template <int num, int delta = 0>
    CALORECGPU_CMC_MOMENT_CALC
    (MaxECellPerSampleSeveral,
     (ToLoad::SamplingFromMomentIndex/*, ToLoad::MaxEnergyAndCellPerSample*/),
     (constexpr unsigned long long int total_ordering_zero = FloatingPointHelpers::StandardFloat::to_total_ordering(0x00000000U);
      //Floating point 0 is all 0.
      constexpr unsigned long long int comparison = (total_ordering_zero << 32) | 0xFFFFFFFFU;
      const int offset = data.sampling * num + delta;
      for (int i = 0; i < num; ++i)
    {
      const unsigned long long max_energy_and_cell = CMCTemporaries::maxEnergyAndCellPerSample(p.clusters_arr, offset + i, cluster);
        const int cell = (max_energy_and_cell > comparison ? ((int) (max_energy_and_cell & 0x7FFFFFFFU)) : -1);
        CMCTemporaries::maxECellPerSample(p.clusters_arr, offset + i, cluster) = cell;
      }
     ),
    (),
    (),
    (),
    (),
    ()
    );

    using MaxECellPerSample = MaxECellPerSampleSeveral<1, 0>;
  }
}

//Possible TO-DO: Write a system that would validate the dependencies
//                so we could have an extra layer of protection
//                if/when reordering the operations.

#endif
