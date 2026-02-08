/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 *
 * @file ITrackAnalysisDefinition.h
 * @author mark sutton
 * @date 5 Feb 2026
 **/

#ifndef INDETTRACKPERFMON_ITRACKANALYSISDEFINITION_H
#define INDETTRACKPERFMON_ITRACKANALYSISDEFINITION_H

#include <string>
#include <vector>


class ITrackAnalysisDefinition {

public:

  virtual const std::vector<std::string>& configuredChains() const = 0;
  virtual const std::string& subFolder() const = 0;
  virtual const std::string& anaTag() const = 0;

  virtual bool useTrigger() const = 0;
  virtual bool useEFTrigger() const = 0;
  virtual bool useTruth() const = 0;
  virtual bool useOffline() const = 0;
  virtual bool doTrigNavigation() const = 0;

  virtual bool isTestTrigger() const = 0;
  virtual bool isTestEFTrigger() const = 0;
  virtual bool isTestTruth() const = 0;
  virtual bool isTestOffline() const = 0;
  virtual bool isReferenceTrigger() const = 0;
  virtual bool isReferenceEFTrigger() const = 0;
  virtual bool isReferenceTruth() const = 0;
  virtual bool isReferenceOffline() const = 0;

  virtual const std::string& testType() const = 0;
  virtual const std::string& referenceType() const = 0;
  virtual const std::string& testTag() const = 0;
  virtual const std::string& referenceTag() const = 0;
  virtual const std::string& matchingType() const = 0;
  virtual float truthProbCut() const = 0;

  virtual const std::vector<float>& etaBins() const = 0;
  virtual const std::vector<unsigned int>& minSilHits() const = 0;
  virtual const std::string& pileupSwitch() const = 0;
  virtual bool hasFullPileupTruth() const = 0;

};

#endif // > ! INDETTRACKPERFMON_ITRACKANALYSISDEFINITION_H
