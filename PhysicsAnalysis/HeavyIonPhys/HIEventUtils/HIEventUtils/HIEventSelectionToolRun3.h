/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HIEVENTUTILS_HIEVENTSELECTIONTOOLRUN3_H
#define HIEVENTUTILS_HIEVENTSELECTIONTOOLRUN3_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "HIEventUtils/IHIEventSelectionToolRun3.h"

namespace HI {

class HIEventSelectionToolRun3 : public virtual HI::IHIEventSelectionToolRun3,
                                 public virtual asg::AsgTool {
  ASG_TOOL_CLASS(HIEventSelectionToolRun3, IHIEventSelectionToolRun3)

 public:
  HIEventSelectionToolRun3(const std::string& name);

  virtual ~HIEventSelectionToolRun3() = default;
  virtual StatusCode initialize() override;
  virtual bool noDetectorError(const xAOD::EventInfo* eventInfo) const override;

  virtual bool puZDCvsFCal(HI::IonDataType when,
                           const xAOD::HIEventShapeContainer* es,
                           const xAOD::ZdcModuleContainer* zdcModules,
                           HI::PileupVariation variation) const override;

  virtual bool puZDCvsFCal(
      IonDataType dataType, float fcalEt, float zdcE,
      PileupVariation variation = PileupVariation::Nominal) const override;

  virtual bool puNtrkvsFCal(
      IonDataType dataType, float fcalEt, int ntrk,
      PileupVariation variation = PileupVariation::Nominal) const override;

  virtual bool puZDCPSvsFCal(
      IonDataType dataType, float fcalEt, float presamplerA, float presamplerC,
      PileupVariation variation = PileupVariation::Nominal) const override;

  virtual bool puOOVertexCuts(
      IonDataType dataType,
      const xAOD::VertexContainer* vertices) const override;

  virtual IonDataType toDataType(
      const xAOD::EventInfo* eventInfo) const override;

  virtual unsigned int defaultMaskForPeriod(IonDataType period) const override;


 private:
  float zdcCutValue(IonDataType, float fcalEt, PileupVariation) const;
  float ntrkCutValue(IonDataType, float fcalEt, PileupVariation) const;
  IonDataType runNumberToDataType(uint32_t run) const;
};

}  // namespace HI
#endif
