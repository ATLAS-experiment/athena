/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FASTCALOSIMMODEL_H
#define G4FASTSIMULATION_FASTCALOSIMMODEL_H

#include "Gaudi/Property.h"
#include "GaudiKernel/ToolHandle.h"
#include "G4VFastSimulationModel.hh"
#include "G4AtlasInterfaces/IPunchThroughSimWrapper.h"
#include "G4AtlasInterfaces/IFastCaloSimParametrizationTool.h"

class CaloCellContainerSD;
class G4FieldTrack;
class G4Region;
class G4SafetyHelper;

class FastCaloSimModel: public G4VFastSimulationModel
{
 public:

  FastCaloSimModel(const std::string& name,
                   G4Region* region,
                   const Gaudi::Property<std::string>& CaloCellContainerSDName,
                   const PublicToolHandle<IFastCaloSimParametrizationTool>& FastCaloSimParametrizationTool,
                   const PublicToolHandle<IPunchThroughSimWrapper>& PunchThroughSimWrapper,
                   const Gaudi::Property<bool>& doPunchThrough);
  ~FastCaloSimModel() override = default;

  G4bool IsApplicable(const G4ParticleDefinition&) override final;
  void DoIt(const G4FastTrack&, G4FastStep&) override final;

  /// Check the model's particle, energy, and geometry requirements.
  G4bool ModelTrigger(const G4FastTrack &) override final;

  /// Retrieve the sensitive detector that owns the calorimeter cells.
  CaloCellContainerSD* getCaloCellContainerSD();

  /// Check that the particle crosses the ID-Calo boundary outwards.
  G4bool passedIDCaloBoundary(const G4FastTrack& fastTrack);

 private:
  Gaudi::Property<std::string> m_CaloCellContainerSDName;
  PublicToolHandle<IFastCaloSimParametrizationTool> m_FastCaloSimParametrizationTool;
  PublicToolHandle<IPunchThroughSimWrapper> m_PunchThroughSimWrapper;
  Gaudi::Property<bool> m_doPunchThrough;
};

#endif  // G4FASTSIMULATION_FASTCALOSIMMODEL_H
