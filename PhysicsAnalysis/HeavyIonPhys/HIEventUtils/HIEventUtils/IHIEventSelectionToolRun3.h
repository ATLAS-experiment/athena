/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HIEVENTUTILS_IHIEVENTSELECTIONTOOLRUN3_H__
#define HIEVENTUTILS_IHIEVENTSELECTIONTOOLRUN3_H__

#include "AsgTools/IAsgTool.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODHIEvent/HIEventShapeContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

namespace HI {
enum class IonDataType : uint8_t {
  PbPb2015 = 0,
  PbPb2018,
  PbPb2023,
  PbPb2024_Shadowing,
  PbPb2024_NoShadowing,
  OO2025,
  NeNe2025,
  PbPb2025,
  PbPb2026
};

std::string toString(IonDataType);

enum class PileupVariation : uint8_t { Nominal = 0, Tight, Loose };

std::string toString(PileupVariation);

constexpr unsigned int bit(int n) {
  return 1 << n;
}

// never change bits assignment, feel free to add
enum class SelectionMask : unsigned int {
  NoEventError = bit(0),
  NoPUFCalVsNTrackLoose = bit(1),
  NoPUFCalVsNTrackNominal = bit(2),
  NoPUFCalVsNTrackTight = bit(3),
  NoPUFCalVsNTrackAny =
      NoPUFCalVsNTrackLoose | NoPUFCalVsNTrackNominal | NoPUFCalVsNTrackTight,
  NoPUFCalVsZDCLoose = bit(4),
  NoPUFCalVsZDCNominal = bit(5),
  NoPUFCalVsZDCTight = bit(6),
  NoPUFCalVsZDCAny =
      NoPUFCalVsZDCLoose | NoPUFCalVsZDCNominal | NoPUFCalVsZDCTight,
  NoPUOOSingleVertexNominal = bit(7),
  NoPUZDCPresampler = bit(8), // at the moment there is only one cut (shall we have Nominal Loose & Tight)

  // default cuts for PB
  PBDefault = NoEventError | NoPUFCalVsZDCLoose |
              NoPUZDCPresampler,  // | NoPUFCalVsNTrackLoose , this needs to be added again when we have cut values
  // default cuts for OO
  OODefault = NoEventError | NoPUOOSingleVertexNominal | NoPUFCalVsNTrackLoose |
              NoPUFCalVsZDCLoose

};

std::string toString(SelectionMask);

class IHIEventSelectionToolRun3 : public virtual asg::IAsgTool {

  ASG_TOOL_INTERFACE(HI::IHIEventSelectionToolRun3)

 public:
  virtual ~IHIEventSelectionToolRun3() = default;

  /// @brief Checks basic event flags
  /// @param eventInfo
  /// @return true if no error
  virtual bool noDetectorError(const xAOD::EventInfo* eventInfo) const = 0;

  /// @brief true if this is NOT pileup event
  /// It computes necessary quantities and invokes method defined next to
  /// perform actual selection
  virtual bool noPUZDCvsFCal(HI::IonDataType when,
                             const xAOD::HIEventShapeContainer* es,
                             const xAOD::ZdcModuleContainer* zdcModules,
                             HI::PileupVariation variation) const = 0;

  virtual float fcalEt(HI::IonDataType when,
                       const xAOD::HIEventShapeContainer* es) const = 0;

  virtual float zdcE(HI::IonDataType when,
                     const xAOD::ZdcModuleContainer* zdcModules) const = 0;

  /// @brief true if this is NOT pileup event
  virtual bool noPUZDCvsFCal(
      IonDataType dataType, float fcalEt, float zdcE,
      PileupVariation variation = PileupVariation::Nominal) const = 0;

  /// @brief true if this is NOT pileup event
  /// The fool performs track selection
  virtual bool noPUFCalVsNtracks(
      IonDataType dataType, const xAOD::HIEventShapeContainer* es,
      const xAOD::TrackParticleContainer* tracks,
      const xAOD::VertexContainer* vertices,
      PileupVariation variation = PileupVariation::Nominal) const = 0;

  virtual int nTrk(IonDataType dataType,
                   const xAOD::TrackParticleContainer* tracks,
                   const xAOD::VertexContainer* vertices) const = 0;

  virtual bool noPUFCalVsNtracks(
      IonDataType dataType, float fcalEt, int ntrk,
      PileupVariation variation = PileupVariation::Nominal) const = 0;

  /// @brief true if this is NOT pileup event
  virtual bool noPUZDCPresampler(HI::IonDataType when,
                                 const xAOD::ZdcModuleContainer* zdcModules,
                                 HI::PileupVariation variation) const = 0;
  virtual bool noPUZDCPresampler(
      IonDataType dataType, float presamplerA, float presamplerC,
      PileupVariation variation = PileupVariation::Nominal) const = 0;

  /// @brief  obtain presampler amplitudes
  /// @param zdcModules
  /// @return A & C side sums
  virtual std::pair<float, float> ZDCPresamplerAmps(
      const xAOD::ZdcModuleContainer* zdcModules) const = 0;

  /// @brief true if this is NOT pileup event
  virtual bool noPUOOVertexCuts(
      IonDataType dataType, const xAOD::VertexContainer* vertices) const = 0;

  /// @brief translates info in EV into HI data type
  virtual IonDataType toDataType(const xAOD::EventInfo* eventInfo) const = 0;

  /// @brief provides default set of cuts for given period
  virtual unsigned int defaultMaskForPeriod(IonDataType period) const = 0;
};

}  // namespace HI

#endif
