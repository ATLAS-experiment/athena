/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELCHARGECALIBCONDDATA_H
#define PIXELCHARGECALIBCONDDATA_H

#include <AthenaKernel/CLASS_DEF.h>
#include <PixelReadoutDefinitions/PixelReadoutDefinitions.h>
#include "PixelConditionsData/ChargeCalibParameters.h" //Thresholds, LegacyFitParameters etc

#include <map>
#include <vector>
#include <array>

namespace PixelChargeCalib{
  struct ChargeCalibrationBundle;
}

class PixelModuleData;
class PixelID;

class PixelChargeCalibCondData
{
  public:
    PixelChargeCalibCondData() = default;
    PixelChargeCalibCondData(std::size_t max_module_hash);

    static constexpr size_t IBLCalibrationSize{16};
    using IBLCalibration = std::array<float, IBLCalibrationSize>;
    using IBLModule = std::vector<IBLCalibration>;

    enum class CalibrationStrategy
    {
      RUN1PIX,
      RUN3PIX,
      LUTFEI4,
      RD53
    };

    void 
    setAllFromBundle(unsigned int moduleHash, const PixelChargeCalib::ChargeCalibrationBundle& b);
    
    void 
    setAllFromConfigData(unsigned int moduleHash, const PixelModuleData * configData, const std::pair<int, int> &becLayer, unsigned int numFE);
    
    void 
    setThresholds(InDetDD::PixelDiodeType type, unsigned int moduleHash, const std::vector<PixelChargeCalib::Thresholds> & thresholds);
    //
    void 
    setAnalogThreshold(InDetDD::PixelDiodeType type, unsigned int moduleHash, std::vector<int> &&value);
    //
    void 
    setLegacyFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, const std::vector<PixelChargeCalib::LegacyFitParameters> &parameters);
    //
    void 
    setLinearFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, const std::vector<PixelChargeCalib::LinearFitParameters> &parameters);
    //
    void 
    setTotResolutions(unsigned int moduleHash, const std::vector<PixelChargeCalib::Resolutions> &value);
    //
    PixelChargeCalib::Thresholds 
    getThresholds(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    //
    PixelChargeCalib::LegacyFitParameters 
    getLegacyFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    //
    PixelChargeCalib::LinearFitParameters 
    getLinearFitParameters(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    //
    float 
    getQ2TotF(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    //
    float 
    getQ2TotG(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE) const;
    //
    float 
    getTotRes(unsigned int moduleHash, unsigned int FE, float Q) const;
    //
    float 
    getToT(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE, float Q) const;
    //
    float 
    getCharge(InDetDD::PixelDiodeType type, unsigned int moduleHash, unsigned int FE, float ToT) const;

    // Avoids looking for Calibration Strategy multiple times
    float
    getCharge(InDetDD::PixelDiodeType type,
	      const CalibrationStrategy calibStrategy,
	      unsigned int moduleHash, unsigned int FE, float ToT) const;
    //
    // new IBL calibration
    void 
    setCalibrationStrategy(unsigned int moduleHash, CalibrationStrategy strategy);
    //
    void 
    setTot2Charges(unsigned int moduleHash, IBLModule charges);
    //
    const IBLCalibration &
    getTot2Charges(unsigned int moduleHash, unsigned int FE) const;
    //
    CalibrationStrategy 
    getCalibrationStrategy(unsigned int moduleHash) const;
    //
    float 
    getChargeLUTFEI4(unsigned int moduleHash, unsigned int FE, unsigned int ToT) const;
    //
    float 
    getToTLUTFEI4(unsigned int moduleHash, unsigned int FE, float Q) const;
    
    inline constexpr int 
    getFEI4OverflowToT() const {
      return 16;
    }

  private:
    std::size_t m_sizeOfHashVector = 0;
    constexpr static std::size_t s_NPixelDiodes = enum2uint(InDetDD::PixelDiodeType::N_DIODETYPES);
    
    std::array<std::vector<std::vector<PixelChargeCalib::Thresholds>>, s_NPixelDiodes> m_thresholds;
    std::array<std::vector<std::vector<PixelChargeCalib::LegacyFitParameters>>, s_NPixelDiodes> m_legacyFit;
    std::array<std::vector<std::vector<PixelChargeCalib::LinearFitParameters>>, s_NPixelDiodes> m_linFit;
    std::vector<std::vector<PixelChargeCalib::Resolutions>> m_totRes;

    // new IBL calibration
    std::map<int, CalibrationStrategy> m_calibrationStrategy;
    std::map<int, IBLModule> m_tot2Charges;

};

CLASS_DEF( PixelChargeCalibCondData , 345532779 , 1 )

#include "AthenaKernel/CondCont.h"
CONDCONT_DEF( PixelChargeCalibCondData, 578786399 );

#endif
