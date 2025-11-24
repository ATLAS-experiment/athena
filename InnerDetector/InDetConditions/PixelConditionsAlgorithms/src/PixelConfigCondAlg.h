/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PixelConditionsAlgorithms/PixelConfigCondAlg.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date December, 2019
 * @brief Store pixel module parameters in PixelModuleData.
 */

#ifndef PIXELCONFIGCONDALG
#define PIXELCONFIGCONDALG

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

#include "StoreGate/WriteCondHandleKey.h"
#include "PixelConditionsData/PixelModuleData.h"

#include "Gaudi/Property.h"
#include "CLHEP/Units/SystemOfUnits.h"

class PixelConfigCondAlg : public AthReentrantAlgorithm {
  public:
    PixelConfigCondAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;
    virtual bool isReEntrant() const override final { return false; }

  private:
    // Key for basic pixel parameters
    SG::WriteCondHandleKey<PixelModuleData> m_writeKey
    {this, "WriteKey", "PixelModuleData", "Output key of pixel module data"};

    // Digitization parameters
    Gaudi::Property<double> m_bunchSpace
    {this, "BunchSpace", 25.0, "Bunch space [ns]"};

    Gaudi::Property<std::vector<int>> m_BarrelNumberOfBCID
    {this, "BarrelNumberOfBCID", {1,1,1,1}, "BCID numbers for barrel pixel layers"};

    Gaudi::Property<std::vector<int>> m_EndcapNumberOfBCID
    {this, "EndcapNumberOfBCID", {1,1,1}, "BCID numbers for endcap pixel layers"};

    Gaudi::Property<std::vector<int>> m_DBMNumberOfBCID
    {this, "DBMNumberOfBCID", {1,1,1}, "BCID numbers for DBM layers"};

    Gaudi::Property<std::vector<double>> m_BarrelTimeOffset
    {this, "BarrelTimeOffset", {5.0,5.0,5.0,5.0}, "Offset time of barrel pixel layer"};

    Gaudi::Property<std::vector<double>> m_EndcapTimeOffset
    {this, "EndcapTimeOffset", {5.0,5.0,5.0}, "Offset time of endcap pixel layer"};

    Gaudi::Property<std::vector<double>> m_DBMTimeOffset
    {this, "DBMTimeOffset", {5.0,5.0,5.0}, "Offset time of DBM layer"};

    Gaudi::Property<std::vector<double>> m_BarrelTimeJitter
    {this, "BarrelTimeJitter", {0.0,0.0,0.0,0.0}, "Time jitter of barrel pixel layer"};

    Gaudi::Property<std::vector<double>> m_EndcapTimeJitter
    {this, "EndcapTimeJitter", {0.0,0.0,0.0}, "Time jitter of endcap pixel layer"};

    Gaudi::Property<std::vector<double>> m_DBMTimeJitter
    {this, "DBMTimeJitter", {0.0,0.0,0.0}, "Time jitter of DBM layer"};

    //====================================================================================
    // Run-dependent SIMULATION(digitization) parameters:
    //
    //   So far, they are year-granularity (3 entries!), thus they may be still
    //   controlled via job option.
    //
    // MC Project:               RUN1            RUN2 mc16a            RUN2 mc16d            RUN2 mc16e
    // Year:                   - 2014             2015/2016                  2017                  2018
    // MC Run Number:         <222222                284500                300000                310000
    // Reference run#:            ---                303638                336506                357193
    // Luminosity(fb-1):                               17.3                  69.0                 119.4
    //
    // Barrel:
    //  ToT:         [   3,   3,   3] [  -1,   5,   5,   5] [  -1,   5,   5,   5] [  -1,   3,   5,   5]
    //  Latency:     [ 256, 256, 256] [  -1, 150, 256, 256] [  -1, 150, 256, 256] [  -1, 150, 256, 256]
    //  Duplicaiton: [   T,   T,   T] [ N/A,   F,   F,   F] [ N/A,   F,   F,   F] [ N/A,   F,   F,   F]
    //  SmallHit:    [   7,   7,   7] [ N/A,   0,   0,   0] [ N/A,   0,   0,   0] [ N/A,   0,   0,   0]
    //  TimingTune:  [2009,2009,2009] [ N/A,2015,2015,2015] [ N/A,2018,2018,2018] [ N/A,2018,2018,2018]
    //  CrossTalk:   [0.06,0.06,0.06] [0.30,0.12,0.12,0.12] [0.30,0.12,0.12,0.12] [0.30,0.12,0.12,0.12]
    //  NoiseOcc.:   [5e-8,5e-8,5e-8] [5e-8,5e-8,5e-8,5e-8] [5e-8,5e-8,5e-8,5e-8] [5e-8,5e-8,5e-8,5e-8]
    //  DisablePix:  [9e-3,9e-3,9e-3] [9e-3,9e-3,9e-3,9e-3] [9e-3,9e-3,9e-3,9e-3] [9e-3,9e-3,9e-3,9e-3]
    //  NoiseShape:  [2018,2018,2018] [2018,2018,2018,2018] [2018,2018,2018,2018] [2018,2018,2018,2018]
    //  BiasVoltage: [ 150, 150, 150] [  80, 350, 200, 150] [ 350, 350, 200, 150] [ 400, 400, 250, 250]
    //  Fluence(e14):[1.01,0.44,0.30] [0.80,1.61,0.71,0.48] [3.18,3.42,1.50,1.01] [5.50,5.19,2.28,1.53]
    //
    // Endcap:
    //  ToT:         [   3,   3,   3]      [   5,   5,   5]      [   5,   5,   5]      [   5,   5,   5]
    //  Latency:     [ 256, 256, 256]      [ 256, 256, 256]      [ 256, 256, 256]      [ 256, 256, 256]
    //  Duplicaiton: [   T,   T,   T]      [   F,   F,   F]      [   F,   F,   F]      [   F,   F,   F]
    //  SmallHit:    [   7,   7,   7]      [   0,   0,   0]      [   0,   0,   0]      [   0,   0,   0]
    //  TimingTune:  [2009,2009,2009]      [2015,2015,2015]      [2018,2018,2018]      [2018,2018,2018]
    //  CrossTalk:   [0.06,0.06,0.06]      [0.06,0.06,0.06]      [0.06,0.06,0.06]      [0.06,0.06,0.06]
    //  NoiseOcc.:   [5e-8,5e-8,5e-8]      [5e-8,5e-8,5e-8]      [5e-8,5e-8,5e-8]      [5e-8,5e-8,5e-8]
    //  DisablePix:  [9e-3,9e-3,9e-3]      [9e-3,9e-3,9e-3]      [9e-3,9e-3,9e-3]      [9e-3,9e-3,9e-3]
    //  NoiseShape:  [2018,2018,2018]      [2018,2018,2018]      [2018,2018,2018]      [2018,2018,2018]
    //  BiasVoltage: [ 150, 150, 150]      [ 150, 150, 150]      [ 150, 150, 150]      [ 250, 250, 250]
    //  Fluence(e14):[ n/a, n/a, n/a]      [ n/a, n/a, n/a]      [ n/a, n/a, n/a]      [ n/a, n/a, n/a]
    //
    // DBM:
    //  ToT:                    [N/A]      [  -1,  -1,  -1]      [  -1,  -1,  -1]      [  -1,  -1,  -1]
    //  CrossTalk:              [N/A]      [0.06,0.06,0.06]      [0.06,0.06,0.06]      [0.06,0.06,0.06]
    //  NoiseOcc.:              [N/A]      [5e-8,5e-8,5e-8]      [5e-8,5e-8,5e-8]      [5e-8,5e-8,5e-8]
    //  DisablePix:             [N/A]      [9e-3,9e-3,9e-3]      [9e-3,9e-3,9e-3]      [9e-3,9e-3,9e-3]
    //  BiasVoltage:            [N/A]      [ 500, 500, 500]      [ 500, 500, 500]      [ 500, 500, 500]
    //
    // IBL 3D:
    //  Fluence(e14):           [N/A]                [0.50]                [0.50]                [50.0]
    //
    // See  https://twiki.cern.ch/twiki/bin/view/Atlas/PixelConditionsRUN2
    // for further details.
    //
    //===============================================================================================================
    // MC Project:                    RUN3                     RUN3                     RUN3                     RUN3
    // Year:                          2022                     2023                     2024                     2025
    // MC Run Number:               410000                   450000                   470000                   495000
    // Reference run#:                 ---                      ---                      ---                      ---
    // Luminosity(fb-1):           186fb-1                  235fb-1    270(IBL) 300(PIX)fb-1    400(IBL) 430(PIX)fb-1
    //
    // Barrel:
    //  ToT:         [  -1,   3,   5,   5]    [  -1,   3,   5,   5]    [  -1,   3,   5,   5]    [  -1,   3,   5,   5]
    //  Latency:     [  -1, 150, 256, 256]    [  -1, 150, 256, 256]    [  -1, 150, 256, 256]    [  -1, 150, 256, 256]
    //  Duplicaiton: [ N/A,   F,   F,   F]    [ N/A,   F,   F,   F]    [ N/A,   F,   F,   F]    [ N/A,   F,   F,   F]
    //  SmallHit:    [ N/A,   0,   0,   0]    [ N/A,   0,   0,   0]    [ N/A,   0,   0,   0]    [ N/A,   0,   0,   0]
    //  TimingTune:  [ N/A,2022,2022,2022]    [ N/A,2022,2022,2022]    [ N/A,2022,2022,2022]    [ N/A,2022,2022,2022]    
    //  CrossTalk:   [0.30,0.12,0.12,0.12]    [0.30,0.12,0.12,0.12]    [0.30,0.12,0.12,0.12]    [0.30,0.12,0.12,0.12]
    //  NoiseOcc.:   [5e-8,5e-8,5e-8,5e-8]    [5e-8,5e-8,5e-8,5e-8]    [5e-8,5e-8,5e-8,5e-8]    [5e-8,5e-8,5e-8,5e-8]   
    //  DisablePix:  [9e-3,9e-3,9e-3,9e-3]    [9e-3,9e-3,9e-3,9e-3]    [9e-3,9e-3,9e-3,9e-3]    [9e-3,9e-3,9e-3,9e-3]
    //  NoiseShape:  [2018,2018,2018,2018]    [2018,2018,2018,2018]    [2018,2018,2018,2018]    [2018,2018,2018,2018]
    //  BiasVoltage: [ 450, 450, 300, 300]    [ 450, 450, 350, 350]    [ 450, 500, 350, 350]    [ 500, 500, 350, 350]
    //  Fluence(e14):[ 7.2, 6.8, 3.0, 2.0]    [  13, 9.2, 4.5, 3.1]    [  15,  11, 5.0, 3.6]    [  22,  15, 6.6, 4.5]
    //
    // Endcap:
    //  ToT:         [   5,   5,   5]         [   5,   5,   5]         [   5,   5,   5]         [   5,   5,   5]
    //  Latency:     [ 256, 256, 256]         [ 256, 256, 256]         [ 256, 256, 256]         [ 256, 256, 256]
    //  Duplicaiton: [   F,   F,   F]         [   F,   F,   F]         [   F,   F,   F]         [   F,   F,   F]
    //  SmallHit:    [   0,   0,   0]         [   0,   0,   0]         [   0,   0,   0]         [   0,   0,   0]
    //  TimingTune:  [2022,2022,2022]         [2022,2022,2022]         [2022,2022,2022]         [2022,2022,2022]
    //  CrossTalk:   [0.06,0.06,0.06]         [0.06,0.06,0.06]         [0.06,0.06,0.06]         [0.06,0.06,0.06]
    //  NoiseOcc.:   [5e-8,5e-8,5e-8]         [5e-8,5e-8,5e-8]         [5e-8,5e-8,5e-8]         [5e-8,5e-8,5e-8]
    //  DisablePix:  [9e-3,9e-3,9e-3]         [9e-3,9e-3,9e-3]         [9e-3,9e-3,9e-3]         [9e-3,9e-3,9e-3]
    //  NoiseShape:  [2018,2018,2018]         [2018,2018,2018]         [2018,2018,2018]         [2018,2018,2018]
    //  BiasVoltage: [ 300, 300, 300]         [ 350, 350, 350]         [ 350, 350, 350]         [ 350, 350, 350]
    //  Fluence(e14):[ n/a, n/a, n/a]         [ n/a, n/a, n/a]         [ n/a, n/a, n/a]         [ n/a, n/a, n/a]
    //
    // DBM: Terminated. All values are dummy.
    //  ToT:         [  -1,  -1,  -1]
    //  CrossTalk:   [0.06,0.06,0.06]
    //  NoiseOcc.:   [5e-8,5e-8,5e-8]
    //  DisablePix:  [9e-3,9e-3,9e-3]
    //  BiasVoltage: [ 500, 500, 500]
    //
    // IBL 3D:
    //  Fluence(e14):[ 7.5]                   [ 7.5]                   [10.2]                   [15.6]
    //
    // See  https://twiki.cern.ch/twiki/bin/view/Atlas/PixelConditionsRUN3
    // for further details.
    //======================================================================================

    Gaudi::Property<int> m_Run1IOV
    {this, "Run1IOV", 222222, "Run number for Run1 conditions"};

    Gaudi::Property<std::string> m_conditionsFolder
    {this, "PixelParameterConditionsFolder", "PixelConditionsAlgorithms/v1/", "Folder name for pixel parameter conditions"};

    Gaudi::Property<std::string> m_conditionsFileName
    {this, "PixelParameterConditionsFile", "PixelParametersList-04.dat", "File name for pixel parameter conditions"};

    Gaudi::Property<std::string> m_usePrivateFileName
    {this, "UsePrivateFileName", "", "File name for private pixel settings (default:empty)"};

    //====================================================================================
    // The following parameters are default values which will be overwritten by the one
    // from the conditions DB. Otherwise the DB is not retrieved nor available, these
    // values are used.
    //====================================================================================
    Gaudi::Property<std::vector<int>> m_BarrelAnalogThreshold
    {this, "DefaultBarrelAnalogThreshold", {-1,-1,-1,-1}, "Default analog thresholds of barrel pixel layers"};

    Gaudi::Property<std::vector<int>> m_EndcapAnalogThreshold
    {this, "DefaultEndcapAnalogThreshold", {-1,-1,-1}, "Default analog thresholds of endcap pixel layers"};

    Gaudi::Property<std::vector<int>> m_DBMAnalogThreshold
    {this, "DefaultDBMAnalogThreshold", {-1,-1,-1}, "Default analog thresholds of DBMlayers"};

    Gaudi::Property<std::vector<int>> m_BarrelAnalogThresholdSigma
    {this, "DefaultBarrelAnalogThresholdSigma", {45,35,30,30}, "Default analog threshold sigma of barrel pixel layers"};

    Gaudi::Property<std::vector<int>> m_EndcapAnalogThresholdSigma
    {this, "DefaultEndcapAnalogThresholdSigma", {30,30,30}, "Default analog threshold sigma of endcap pixel layers"};

    Gaudi::Property<std::vector<int>> m_DBMAnalogThresholdSigma
    {this, "DefaultDBMAnalogThresholdSigma", {70,70,70}, "Default analog threshold sigma of DBM pixel layers"};

    Gaudi::Property<std::vector<int>> m_BarrelAnalogThresholdNoise
    {this, "DefaultBarrelAnalogThresholdNoise", {130,150,160,160}, "Default threshold noise of barrel pixel layers"};

    Gaudi::Property<std::vector<int>> m_EndcapAnalogThresholdNoise
    {this, "DefaultEndcapAnalogThresholdNoise", {150,150,150}, "Default threshold noise of endcap pixel layers"};

    Gaudi::Property<std::vector<int>> m_DBMAnalogThresholdNoise
    {this, "DefaultDBMAnalogThresholdNoise", {190,190,190}, "Default threshold noise of DBM pixel layers"};

    Gaudi::Property<std::vector<int>> m_BarrelInTimeThreshold
    {this, "DefaultBarrelInTimeThreshold", {2000,5000,5000,5000}, "Default in-time thresholds of barrel pixel layers"};

    Gaudi::Property<std::vector<int>> m_EndcapInTimeThreshold
    {this, "DefaultEndcapInTimeThreshold", {5000,5000,5000}, "Default in-time thresholds of endcap pixel layers"};

    Gaudi::Property<std::vector<int>> m_DBMInTimeThreshold
    {this, "DefaultDBMInTimeThreshold", {1200,1200,1200}, "Default in-time thresholds of DBM pixel layers"};

    Gaudi::Property<std::vector<double>> m_BarrelThermalNoise
    {this, "BarrelThermalNoise", {160.0,160.0,160.0,160.0}, "Thermal noise of barrel pixel layers"};

    Gaudi::Property<std::vector<double>> m_EndcapThermalNoise
    {this, "EndcapThermalNoise", {160.0,160.0,160.0}, "Thermal noise of endcap pixel layers"};

    Gaudi::Property<std::vector<double>> m_DBMThermalNoise
    {this, "DBMThermalNoise", {160.0,160.0,160.0}, "Thermal noise of DBM layers"};

    Gaudi::Property<std::vector<int>> m_FEI4BarrelHitDiscConfig
    {this, "FEI4BarrelHitDiscConfig", {2,2,2}, "Set HitDiscConfig parameter for barrel pixel layers"};

    Gaudi::Property<std::vector<int>> m_FEI4EndcapHitDiscConfig
    {this, "FEI4EndcapHitDiscConfig", {2,2,2}, "Set HitDiscConfig parameter for endcap pixel layers"};

    Gaudi::Property<float> m_chargeScaleFEI4
    {this, "ChargeScaleFEI4", 1.0, "Scaling of the FEI4 charge"};

    Gaudi::Property<bool> m_UseFEI4SpecialScalingFunction
    {this, "UseFEI4SpecialScalingFunction", true, "Use FEI4 special scaling function"};

    Gaudi::Property<std::vector<double>> m_FEI4ToTSigma
    {this, "FEI4ToTSigma", {0.0,0.50,0.50,0.50,0.50,0.50,0.60,0.60,0.60,0.60,0.65,0.70,0.75,0.80,0.80,0.80,0.80}, "Set ToT sigma for FEI4"};

    // Charge calibration parameters
    Gaudi::Property<float> m_CalibrationParameterA
    {this, "DefaultCalibrationParameterA", 70.2, "Default charge calibration parameter A"};

    Gaudi::Property<float> m_CalibrationParameterE
    {this, "DefaultCalibrationParameterE", -3561.25, "Default charge calibration parameter E"};

    Gaudi::Property<float> m_CalibrationParameterC
    {this, "DefaultCalibrationParameterC", 26000.0, "Default charge calibration parameter C"};

    Gaudi::Property<bool> m_doPIXLinearExtrapolation
    {this, "doPIXLinearExtrapolation", true, "Activation for linear extrapolation for PIXEL"};

    // DCS parameters
    Gaudi::Property<float> m_biasVoltage
    {this, "DefaultBiasVoltage", 150.0, "Default bias voltage"};

    Gaudi::Property<float> m_temperature
    {this, "DefaultTemperature", -7.0, "Default temperature in Celcius"};

    // Cabling parameters
    Gaudi::Property<bool> m_cablingMapToFile
    {this, "CablingMapToFile", false, "Dump pixel cabling map into file"};

    Gaudi::Property<std::string> m_cablingMapFileName
    {this, "CablingMapFileName", "PixelCabling/Pixels_Atlas_IdMapping_2016.dat", "Read cabling map from file"};

    std::string getFileName(const int currentRunNumber) const;

};

#endif
