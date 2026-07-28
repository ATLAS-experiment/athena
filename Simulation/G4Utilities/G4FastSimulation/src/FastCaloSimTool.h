/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FASTCALOSIMTOOL_H
#define G4FASTSIMULATION_FASTCALOSIMTOOL_H

#include "G4AtlasTools/FastSimulationBase.h"
#include "G4AtlasInterfaces/IPunchThroughSimWrapper.h"
#include "G4AtlasInterfaces/IFastCaloSimParametrizationTool.h"

class G4VFastSimulationModel;

class FastCaloSimTool : public FastSimulationBase {
 public:
  FastCaloSimTool(const std::string& type, const std::string& name,
                  const IInterface* parent);
  ~FastCaloSimTool() override = default;

  StatusCode initialize() override final;

 protected:
  G4VFastSimulationModel* makeFastSimModel() override final;

 private:
  Gaudi::Property<std::string> m_CaloCellContainerSDName{this, "CaloCellContainerSDName", "", "Name of the associated CaloCellContainerSD"};
  PublicToolHandle<IFastCaloSimParametrizationTool> m_FastCaloSimParametrizationTool{this, "FastCaloSimParametrizationTool", "FastCaloSimParametrizationTool", "Thread-shared FastCaloSimParametrizationTool"};
  PublicToolHandle<IPunchThroughSimWrapper> m_PunchThroughSimWrapper{this, "PunchThroughSimWrapper", "PunchThroughSimWrapper", ""};
  Gaudi::Property<bool> m_doPunchThrough{this, "doPunchThrough", true, "Run punch-through simulation at the Calo-MS boundary"};
};

#endif  // G4FASTSIMULATION_FASTCALOSIMTOOL_H
