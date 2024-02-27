/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef jFEXCoolDBDefaults_H
#define jFEXCoolDBDefaults_H

namespace LVL1::jFEXCoolDBDefaults {

    constexpr static int jJCalibParams[6][25] =
    {   //for ET based calibration : <20  <30  <40  <50  <65  <80 <110 <150 <inf  GeV
        //for eta based calibration: based on per-FPGA local eta indices
        //some non-zero default values for periods before DB based values are available (ET only, eta based introduced much later)
        { 197, 197, 197, 197, 197, 197, 197, 197, 197, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },// jFEX 0  FCal
        { 222, 222, 222, 222, 222, 222, 222, 222, 222, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },// jFEX 1  Central
        { 182, 182, 182, 182, 182, 182, 182, 182, 182, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },// jFEX 2  Central
        { 182, 182, 182, 182, 182, 182, 182, 182, 182, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },// jFEX 3  Central
        { 222, 222, 222, 222, 222, 222, 222, 222, 222, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },// jFEX 4  Central
        { 197, 197, 197, 197, 197, 197, 197, 197, 197, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 } // jFEX 5  FCal
    };

    // Apply pileup on met or jet?
    constexpr static bool PileUpCorrectionJet = false;
    constexpr static bool PileUpCorrectionMET = false;

    // upper/lower thresholds to calculate pile-up
    constexpr static int PileUpThresholdLowEm = 100;
    constexpr static int PileUpThresholdHighEm = 1000;
    constexpr static int PileUpThresholdLowHadLar = 100;
    constexpr static int PileUpThresholdHighHadLar =  1000;
    constexpr static int PileUpThresholdLowHadHecOverlap = 100;
    constexpr static int PileUpThresholdHighHadHecOverlap = 1000;
    constexpr static int PileUpThresholdLowHadTrex = 100;
    constexpr static int PileUpThresholdHighHadTrex = 1000;
    constexpr static int PileUpThresholdLowFcal = 100;
    constexpr static int PileUpThresholdHighFcal = 1000;

    // Noise values for EM and HAD
    //{CutJetEM, CutJetHad, CutMetEM, CutMetHad} in that order!
    constexpr static std::array<uint16_t,4> NoiseCuts_LATOME_TILE = {{40,  1, 40,  1}};
    constexpr static std::array<uint16_t,4> NoiseCuts_LATOME_HEC  = {{40, 40, 40, 40}};
    constexpr static std::array<uint16_t,4> NoiseCuts_FCAL        = {{ 0,  0,  0,  0}};

    // PileUp values for EM and HAD
    constexpr static std::array<uint16_t,4> PileUpWeight_default = {{0, 0, 0, 0}};

};

#endif
