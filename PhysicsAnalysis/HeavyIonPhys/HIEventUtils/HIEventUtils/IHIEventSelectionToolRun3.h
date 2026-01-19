/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HIEVENTUTILS_IHIEVENTSELECTIONTOOLRUN3_H__
#define HIEVENTUTILS_IHIEVENTSELECTIONTOOLRUN3_H__

#include "AsgTools/IAsgTool.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"

namespace HI {
enum class IonDataType : uint8_t {
  PbPb2023 = 0,
  PbPb2024_Shadowing,
  PbPb2024_NoShadowing,
  OO2025,
  NeNe2025,
  PbPb2025,
  PbPb2026
};

std::string toString(IonDataType);

enum class PileupVariation : uint8_t { Nominal = 0, Up, Down };

std::string toString(PileupVariation);

class IHIEventSelectionToolRun3 : public virtual asg::IAsgTool {

  ASG_TOOL_INTERFACE(HI::IHIEventSelectionToolRun3)

 public:
  virtual ~IHIEventSelectionToolRun3() = default;

  /// @brief Checks basic event flags
  /// @param eventInfo
  /// @return true if no error
  virtual bool noDetectorError(const xAOD::EventInfo* eventInfo) const = 0;

  /// @brief true if this is pileup event
  virtual bool puZDCvsFCal(
      IonDataType dataType, float fcalEt, float zdcE,
      PileupVariation variation = PileupVariation::Nominal) const = 0;

  /// @brief true if this is pileup event
  virtual bool puNtrkvsFCal(
      IonDataType dataType, float fcalEt, int ntrk,
      PileupVariation variation = PileupVariation::Nominal) const = 0;

  /// @brief true if this is pileup event
  /// Code sample to obtain presampler energies
  /// Float_t PreSamplerAmp_A = 0;
  /// Float_t PreSamplerAmp_C = 0;
  /// xAOD::ZdcModuleContainer * zdcModules = 0;
  /// CHECK( evtStore()->retrieve(zdcModules, "ZdcModules") );
  /// for (const auto ZdcModule : *zdcModules) {
  ///     if (ZdcModule->zdcType()!=0) continue;
  ///     if (ZdcModule->zdcSide()>0) PreSamplerAmp_C+=accPreSamplerAmpC(*ZdcModule); 
  ///     if (ZdcModule->zdcSide()<0) PreSamplerAmp_A+=accPreSamplerAmpA(*ZdcModule); 
  /// }
  virtual bool puZDCPSvsFCal(
      IonDataType dataType, float fcalEt, float presamplerA, float presamplerC,
      PileupVariation variation = PileupVariation::Nominal) const = 0;

  /// @brief true if this is pileup event
  virtual bool puOOVertexCuts(IonDataType dataType,
                              const xAOD::VertexContainer* vertices) const = 0;

  /// @brief translates info in EV into HI data type
  virtual IonDataType toDataType(const xAOD::EventInfo* eventInfo) const = 0;
};

}  // namespace HI

#endif
