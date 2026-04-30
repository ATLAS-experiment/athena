/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HIEVENTUTILS_HIEVENTSELECTIONTOOLRUN3_H
#define HIEVENTUTILS_HIEVENTSELECTIONTOOLRUN3_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "HIEventUtils/IHIEventSelectionToolRun3.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "TH1D.h"

namespace HI {

class HIEventSelectionToolRun3 : public virtual HI::IHIEventSelectionToolRun3,
                                 public virtual asg::AsgTool {
  ASG_TOOL_CLASS(HIEventSelectionToolRun3, IHIEventSelectionToolRun3)

 public:
  HIEventSelectionToolRun3(const std::string& name);

  virtual ~HIEventSelectionToolRun3() = default;
  virtual StatusCode initialize() override;
  virtual bool noDetectorError(const xAOD::EventInfo* eventInfo) const override;

  virtual bool noPUZDCvsFCal(HI::IonDataType period,
                             const xAOD::HIEventShapeContainer* es,
                             const xAOD::ZdcModuleContainer* zdcModules,
                             HI::PileupVariation variation) const override;

  virtual float fcalEt(HI::IonDataType period,
                       const xAOD::HIEventShapeContainer* es) const override;

  virtual float zdcE(HI::IonDataType period,
                     const xAOD::ZdcModuleContainer* zdcModules) const override;

  virtual bool noPUZDCvsFCal(
      IonDataType dataType, float fcalEt, float zdcE,
      PileupVariation variation = PileupVariation::Nominal) const override;

  virtual bool noPUFCalVsNtracks(
      IonDataType dataType, const xAOD::HIEventShapeContainer* es,
      const xAOD::TrackParticleContainer* tracks,
      const xAOD::VertexContainer* vertices,
      PileupVariation variation = PileupVariation::Nominal) const override;

  virtual int nTrk(HI::IonDataType dataType,
                   const xAOD::TrackParticleContainer* tracks,
                   const xAOD::VertexContainer* vertices) const override;

  virtual bool noPUFCalVsNtracks(
      IonDataType dataType, float fcalEt, int ntrk,
      PileupVariation variation = PileupVariation::Nominal) const override;

  virtual bool noPUZDCPresampler(HI::IonDataType period,
                                 const xAOD::ZdcModuleContainer* zdcModules,
                                 HI::PileupVariation variation) const override;

  virtual bool noPUZDCPresampler(
      IonDataType dataType, float presamplerA, float presamplerC,
      PileupVariation variation = PileupVariation::Nominal) const override;

  virtual std::pair<float, float> ZDCPresamplerAmps(
      const xAOD::ZdcModuleContainer* zdcModules) const override;

  virtual bool noPUOOVertexCuts(
      IonDataType dataType,
      const xAOD::VertexContainer* vertices) const override;

  virtual IonDataType toDataType(
      const xAOD::EventInfo* eventInfo) const override;

  virtual unsigned int defaultMaskForPeriod(IonDataType period) const override;

 private:
  ToolHandle<InDet::IInDetTrackSelectionTool> m_trackSelectionTool{
      this, "TrackSelectionTool", "", ""};

  std::unique_ptr<TH1D> m_ZDCEt_UpperCut_5p5Sigma_OO;
  std::unique_ptr<TH1D> m_ZDCEt_UpperCut_4p0Sigma_NeNe;
  std::unique_ptr<TH1D> m_ZDCEt_UpperCut_5Sigma_PbPb2023;

  float zdcCutValue(IonDataType, float fcalEt, PileupVariation) const;

  float ntrkCutValue(IonDataType, float fcalEt, PileupVariation) const;
  IonDataType runNumberToDataType(uint32_t run) const;
};

}  // namespace HI
#endif
