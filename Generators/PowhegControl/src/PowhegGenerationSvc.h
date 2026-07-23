// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#ifndef POWHEGCONTROL_POWHEGGENERATIONSVC_H
#define POWHEGCONTROL_POWHEGGENERATIONSVC_H

#include "AthenaBaseComps/AthService.h"
#include "Gaudi/Property.h"

#include <string>

class PowhegGenerationSvc final : public AthService {
 public:
  PowhegGenerationSvc(const std::string& name, ISvcLocator* svcLoc);

  StatusCode initialize() override;

 private:
  StringProperty m_runnerModule{
      this, "RunnerModule", "PowhegControl.PowhegRunner",
      "Python module that executes a serialized Powheg run plan"};
  StringProperty m_configuration{
      this, "Configuration", "", "Serialized Powheg run plan"};
  StringProperty m_outputLHE{
      this, "OutputLHE", "", "Expected LHE output file"};
  StringProperty m_workingDirectory{
      this, "WorkingDirectory", ".", "Directory in which Powheg is run"};
  StringProperty m_pythonExecutable{
      this, "PythonExecutable", "python3", "Python interpreter for the runner"};
  BooleanProperty m_validateOutput{
      this, "ValidateOutput", true,
      "Require a non-empty LHE output after the runner exits"};
};

#endif

