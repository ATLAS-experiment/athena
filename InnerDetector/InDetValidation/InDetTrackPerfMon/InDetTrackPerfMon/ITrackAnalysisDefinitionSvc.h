/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_ITRACKANALYSISDEFINITIONSVC_H
#define INDETTRACKPERFMON_ITRACKANALYSISDEFINITIONSVC_H

/**
 * @file ITrackAnalysisDefinitionSvc.h
 * Service interface to hold (and propagate) the basic properties
 * of each defined TrackAnalysis and of their hisotgrams
 * @author marco aparo
 * @date 19 June 2023
**/

/// Athena include(s).
#include <AsgServices/IAsgService.h>

/// STL include(s)
#include <string>
#include <utility>
#include <vector>


class ITrackAnalysisDefinitionSvc :
    virtual public asg::IAsgService {

public:

  /// Creates the InterfaceID and interfaceID() method
  DeclareInterfaceID( ITrackAnalysisDefinitionSvc, 1, 0 );

  virtual const std::vector< std::string >& configuredChains() const = 0;
  virtual const std::string& subFolder() const = 0;
  virtual const std::string& anaTag() const = 0;

  virtual bool useTrigger() const = 0;
  virtual bool useTruth() const = 0;
  virtual bool useOffline() const = 0;

  virtual bool isTestTrigger() const = 0;
  virtual bool isTestTruth() const = 0;
  virtual bool isTestOffline() const = 0;
  virtual bool isReferenceTrigger() const = 0;
  virtual bool isReferenceTruth() const = 0;
  virtual bool isReferenceOffline() const = 0;

  virtual const std::string& testType() const = 0;
  virtual const std::string& referenceType() const = 0;
  virtual const std::string& testTag() const = 0;
  virtual const std::string& referenceTag() const = 0;
  virtual const std::string& matchingType() const = 0;

  /// histogram properties
  virtual bool doTrackParameters() const = 0;
  virtual bool doEfficiencies() const = 0;
  virtual bool doOfflineElectrons() const = 0;
  
};

#endif // > ! INDETTRACKPERFMON_ITRACKANALYSISDEFINITIONSVC_H
