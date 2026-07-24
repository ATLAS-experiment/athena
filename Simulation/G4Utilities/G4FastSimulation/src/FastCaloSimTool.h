/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FASTCALOSIMTOOL_H
#define G4FASTSIMULATION_FASTCALOSIMTOOL_H

#include "G4AtlasTools/FastSimulationBase.h"
#include "G4AtlasInterfaces/IPunchThroughSimWrapper.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "G4AtlasInterfaces/IFastCaloSimParametrizationTool.h"
#include "G4Cache.hh"

class G4VFastSimulationModel;
class FastCaloSimModel;

class FastCaloSimTool : public FastSimulationBase {
 public:
  FastCaloSimTool(const std::string& type, const std::string& name,
                  const IInterface* parent);
  ~FastCaloSimTool() override = default;

  StatusCode initialize() override final;

  /// Prepare the thread-local model for an Athena event.
  StatusCode BeginOfAthenaEvent(HitCollectionMap&) override final;
  /// Finish the thread-local model's Athena event.
  StatusCode EndOfAthenaEvent(HitCollectionMap&) override final;

 protected:
  G4VFastSimulationModel* makeFastSimModel() override final;

 private:
  ServiceHandle<IAthRNGSvc> m_rndmGenSvc{this, "RandomSvc", "AthRNGSvc", ""};
  Gaudi::Property<std::string> m_randomEngineName{
      this, "RandomStream", "FastCaloSimRnd", "Random-number stream"};
  Gaudi::Property<std::string> m_CaloCellContainerSDName{this, "CaloCellContainerSDName", "", "Name of the associated CaloCellContainerSD"};
  PublicToolHandle<IFastCaloSimParametrizationTool> m_FastCaloSimParametrizationTool{this, "FastCaloSimParametrizationTool", "FastCaloSimParametrizationTool", "Thread-shared FastCaloSimParametrizationTool"};
  PublicToolHandle<IPunchThroughSimWrapper> m_PunchThroughSimWrapper{this, "PunchThroughSimWrapper", "PunchThroughSimWrapper", ""};
  Gaudi::Property<bool> m_doPunchThrough{this, "doPunchThrough", true, "Run punch-through simulation at the Calo-MS boundary"};

  G4Cache<FastCaloSimModel*> m_fastSimModel;
};

#endif  // G4FASTSIMULATION_FASTCALOSIMTOOL_H
