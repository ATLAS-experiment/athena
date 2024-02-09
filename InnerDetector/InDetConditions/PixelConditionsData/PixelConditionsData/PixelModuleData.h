/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PixelConditionsData/PixelModuleData.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date November, 2019
 * @brief Store pixel constant parameters in PixelModuleData.
 */

#ifndef PIXELMODULEDATA_H
#define PIXELMODULEDATA_H

#include <AthenaKernel/CLASS_DEF.h>
#include <AthenaKernel/CondCont.h>
#include <iosfwd>
#include <vector>
#include <string>


class PixelModuleData 
{
  public:
    // stream insertion
    friend std::ostream & operator << (std::ostream &out, const PixelModuleData &c);
    friend std::istream & operator >> (std::istream &in, PixelModuleData &c);
    
    void setDefaultBarrelAnalogThreshold(const std::vector<int> &barrelAnalogThreshold);
    void setDefaultEndcapAnalogThreshold(const std::vector<int> &endcapAnalogThreshold);
    void setDefaultDBMAnalogThreshold(const std::vector<int> &DBMAnalogThreshold);
    int getDefaultAnalogThreshold(int barrel_ec, int layer) const;

    void setDefaultBarrelAnalogThresholdSigma(const std::vector<int> &barrelAnalogThresholdSigma);
    void setDefaultEndcapAnalogThresholdSigma(const std::vector<int> &endcapAnalogThresholdSigma);
    void setDefaultDBMAnalogThresholdSigma(const std::vector<int> &DBMAnalogThresholdSigma);
    int getDefaultAnalogThresholdSigma(int barrel_ec, int layer) const;

    void setDefaultBarrelAnalogThresholdNoise(const std::vector<int> &barrelAnalogThresholdNoise);
    void setDefaultEndcapAnalogThresholdNoise(const std::vector<int> &endcapAnalogThresholdNoise);
    void setDefaultDBMAnalogThresholdNoise(const std::vector<int> &DBMAnalogThresholdNoise);
    int getDefaultAnalogThresholdNoise(int barrel_ec, int layer) const;

    void setDefaultBarrelInTimeThreshold(const std::vector<int> &barrelInTimeThreshold);
    void setDefaultEndcapInTimeThreshold(const std::vector<int> &endcapInTimeThreshold);
    void setDefaultDBMInTimeThreshold(const std::vector<int> &DBMInTimeThreshold);
    int getDefaultInTimeThreshold(int barrel_ec, int layer) const;

    void setBarrelToTThreshold(const std::vector<int> &barrelToTThreshold);
    void setEndcapToTThreshold(const std::vector<int> &endcapToTThreshold);
    void setDBMToTThreshold(const std::vector<int> &DBMToTThreshold);
    int getToTThreshold(int barrel_ec, int layer) const;

    void setBarrelCrossTalk(const std::vector<double> &barrelCrossTalk);
    void setEndcapCrossTalk(const std::vector<double> &endcapCrossTalk);
    void setDBMCrossTalk(const std::vector<double> &DBMCrossTalk);
    double getCrossTalk(int barrel_ec, int layer) const;

    void setBarrelThermalNoise(const std::vector<double> &barrelThermalNoise);
    void setEndcapThermalNoise(const std::vector<double> &endcapThermalNoise);
    void setDBMThermalNoise(const std::vector<double> &DBMThermalNoise);
    double getThermalNoise(int barrel_ec, int layer) const;

    void setBarrelNoiseOccupancy(const std::vector<double> &barrelNoiseOccupancy);
    void setEndcapNoiseOccupancy(const std::vector<double> &endcapNoiseOccupancy);
    void setDBMNoiseOccupancy(const std::vector<double> &DBMNoiseOccupancy);
    double getNoiseOccupancy(int barrel_ec, int layer) const;

    void setBarrelDisableProbability(const std::vector<double> &barrelDisableProbability);
    void setEndcapDisableProbability(const std::vector<double> &endcapDisableProbability);
    void setDBMDisableProbability(const std::vector<double> &DBMDisableProbability);
    double getDisableProbability(int barrel_ec, int layer) const;

    void setBarrelNoiseShape(const std::vector<std::vector<float>> &barrelNoiseShape);
    void setEndcapNoiseShape(const std::vector<std::vector<float>> &endcapNoiseShape);
    void setDBMNoiseShape(const std::vector<std::vector<float>> &DBMNoiseShape);
    const std::vector<float> &getNoiseShape(int barrel_ec, int layer) const;
    void setFEI3BarrelLatency(const std::vector<int> &FEI3BarrelLatency);
    void setFEI3EndcapLatency(const std::vector<int> &FEI3EndcapLatency);
    int getFEI3Latency(int barrel_ec, int layer) const;
    
    void setFEI3BarrelTimingSimTune(const std::vector<int> &FEI3BarrelTimingSimTune);
    void setFEI3EndcapTimingSimTune(const std::vector<int> &FEI3EndcapTimingSimTune);
    int getFEI3TimingSimTune(int barrel_ec, int layer) const;
    void setBLayerTimingIndex(const std::vector<float> &BLayerTimingIndex);
    void setLayer1TimingIndex(const std::vector<float> &Layer1TimingIndex);
    void setLayer2TimingIndex(const std::vector<float> &Layer2TimingIndex);
    void setEndcap1TimingIndex(const std::vector<float> &Endcap1TimingIndex);
    void setEndcap2TimingIndex(const std::vector<float> &Endcap2TimingIndex);
    void setEndcap3TimingIndex(const std::vector<float> &Endcap3TimingIndex);
    void setBLayerTimingProbability(const std::vector<float> &BLayerTimingProbability);
    void setLayer1TimingProbability(const std::vector<float> &Layer1TimingProbability);
    void setLayer2TimingProbability(const std::vector<float> &Layer2TimingProbability);
    void setEndcap1TimingProbability(const std::vector<float> &Endcap1TimingProbability);
    void setEndcap2TimingProbability(const std::vector<float> &Endcap2TimingProbability);
    void setEndcap3TimingProbability(const std::vector<float> &Endcap3TimingProbability);
    std::vector<float> getTimingIndex(int barrel_ec, int layer) const;
    std::vector<float> getTimingProbability(int barrel_ec, int layer, int eta) const;

    // Charge calibration parameters
    void setDefaultQ2TotA(float paramA);
    void setDefaultQ2TotE(float paramE);
    void setDefaultQ2TotC(float paramC);
    float getDefaultQ2TotA() const;
    float getDefaultQ2TotE() const;
    float getDefaultQ2TotC() const;

    void setPIXLinearExtrapolation(bool doLinearExtrapolation);
    bool getPIXLinearExtrapolation() const;

    // DCS parameters
    void setDefaultBiasVoltage(float biasVoltage);
    float getDefaultBiasVoltage() const;

    // Radiation damage fluence maps
    void setFluenceLayer(const std::vector<double> &fluenceLayer);
    const std::vector<double> &getFluenceLayer() const;

    void setRadSimFluenceMapList(const std::vector<std::string> &radSimFluenceMapList);
    const std::vector<std::string> &getRadSimFluenceMapList() const;

    void setFluenceLayer3D(const std::vector<double> &fluenceLayer);
    const std::vector<double> &getFluenceLayer3D() const;

    void setRadSimFluenceMapList3D(const std::vector<std::string> &radSimFluenceMapList3D);
    const std::vector<std::string> &getRadSimFluenceMapList3D() const;

    // Cabling parameters
    void setCablingMapToFile(bool cablingMapToFile);
    bool getCablingMapToFile() const;

    void setCablingMapFileName(const std::string &cablingMapFileName);
    const std::string &getCablingMapFileName() const;


  private:
    //defaults are for RUN2 2015/2016
    std::vector<int> m_defaultBarrelAnalogThreshold{-1,-1,-1,-1};
    std::vector<int> m_defaultEndcapAnalogThreshold{-1,-1,-1};
    std::vector<int> m_defaultDBMAnalogThreshold{-1,-1,-1};
    //
    std::vector<int> m_defaultBarrelAnalogThresholdSigma{45,35,30,30};
    std::vector<int> m_defaultEndcapAnalogThresholdSigma{30,30,30};
    std::vector<int> m_defaultDBMAnalogThresholdSigma{70,70,70};
    //
    std::vector<int> m_defaultBarrelAnalogThresholdNoise{130,150,160,160};
    std::vector<int> m_defaultEndcapAnalogThresholdNoise{150,150,150};
    std::vector<int> m_defaultDBMAnalogThresholdNoise{190,190,190};
    //
    std::vector<int> m_defaultBarrelInTimeThreshold{2000,5000,5000,5000};
    std::vector<int> m_defaultEndcapInTimeThreshold{5000,5000,5000};
    std::vector<int> m_defaultDBMInTimeThreshold{1200,1200,1200};
    //
    std::vector<int> m_barrelToTThreshold{-1, 5, 5, 5};
    std::vector<int> m_endcapToTThreshold{ 5, 5, 5};
    std::vector<int> m_DBMToTThreshold{-1,-1,-1};
    //
    std::vector<double> m_barrelCrossTalk{0.30,0.12,0.12,0.12}; 
    std::vector<double> m_endcapCrossTalk{0.06,0.06,0.06}; 
    std::vector<double> m_DBMCrossTalk{0.06,0.06,0.06};
    //
    std::vector<double> m_barrelThermalNoise{160.0,160.0,160.0,160.0};
    std::vector<double> m_endcapThermalNoise{160.0,160.0,160.0};
    std::vector<double> m_DBMThermalNoise{160.0,160.0,160.0};
    //
    std::vector<double> m_barrelNoiseOccupancy{5e-8,5e-8,5e-8,5e-8};
    std::vector<double> m_endcapNoiseOccupancy{5e-8,5e-8,5e-8};
    std::vector<double> m_DBMNoiseOccupancy{5e-8,5e-8,5e-8};
    //
    std::vector<double> m_barrelDisableProbability{9e-3,9e-3,9e-3,9e-3};
    std::vector<double> m_endcapDisableProbability{9e-3,9e-3,9e-3};
    std::vector<double> m_DBMDisableProbability{9e-3,9e-3,9e-3};
    //
    const  std::vector<float> m_BLayerNoiseShape{0.0, 0.0, 0.0, 0.0, 0.2204, 0.5311, 0.7493, 0.8954, 0.9980, 1.0};
    const  std::vector<float> m_PixNoiseShape{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.2418, 0.4397, 0.5858, 0.6949, 0.7737, 0.8414, 0.8959, 0.9414, 0.9828, 1.0};
    const  std::vector<float> m_IBLNoiseShape{0.0, 0.0330, 0.0, 0.3026, 0.5019, 0.6760, 0.8412, 0.9918, 0.9918, 0.9918, 0.9918, 0.9918, 0.9918, 0.9918, 0.9918, 0.9918, 1.0};
    std::vector<std::vector<float>> m_barrelNoiseShape{m_BLayerNoiseShape, m_PixNoiseShape ,m_PixNoiseShape} ;
    std::vector<std::vector<float>> m_endcapNoiseShape{m_PixNoiseShape, m_PixNoiseShape, m_PixNoiseShape};
    std::vector<std::vector<float>> m_DBMNoiseShape{m_IBLNoiseShape,m_IBLNoiseShape,m_IBLNoiseShape};
    //
    std::vector<int>  m_FEI3BarrelLatency{0,151,256,256};
    std::vector<int>  m_FEI3EndcapLatency{256,256,256};

    std::vector<int>  m_FEI3BarrelTimingSimTune{-1,2015,2015,2015};
    std::vector<int>  m_FEI3EndcapTimingSimTune{2015,2015,2015};

    std::vector<float> m_BLayerTimingIndex{0.0};
    std::vector<float> m_BLayerTimingProbability{0.0};

    std::vector<float> m_Layer1TimingIndex{0.0};
    std::vector<float> m_Layer1TimingProbability{0.0};

    std::vector<float> m_Layer2TimingIndex{0.0};
    std::vector<float> m_Layer2TimingProbability{0.0};

    std::vector<float> m_Endcap1TimingIndex{0.0};
    std::vector<float> m_Endcap1TimingProbability{0.0};

    std::vector<float> m_Endcap2TimingIndex{0.0};
    std::vector<float> m_Endcap2TimingProbability{0.0};

    std::vector<float> m_Endcap3TimingIndex{0.0};
    std::vector<float> m_Endcap3TimingProbability{0.0};

    float m_paramA{70.2f};
    float m_paramE{-3561.25f};
    float m_paramC{26000.0f};
    //default should not matter, but for ITk algorithms might cause problems
    bool m_doLinearExtrapolation{false};

    float m_biasVoltage{150.f};


    std::vector<double> m_fluenceLayer{0.80e14, 1.61e14, 0.71e14, 0.48e14};
    std::vector<std::string> m_radSimFluenceMapList{"PixelDigitization/maps_IBL_PL_80V_fl0_8e14.root",
                                    "PixelDigitization/maps_PIX_350V_fl1_61e14.root",
                                    "PixelDigitization/maps_PIX_200V_fl0_71e14.root",
                                    "PixelDigitization/maps_PIX_150V_fl0_48e14.root"};

    std::vector<double> m_fluenceLayer3D{5.0e15};
    std::vector<std::string> m_radSimFluenceMapList3D{"PixelDigitization/TCAD_IBL_3Dsensors_efields/phi_5e15_160V.root"};

    bool        m_cablingMapToFile{false};
    std::string m_cablingMapFileName{"PixelCabling/Pixels_Atlas_IdMapping_2016.dat"};

};

CLASS_DEF( PixelModuleData , 345932873 , 1 )

CONDCONT_DEF( PixelModuleData, 578988393 );

#endif
