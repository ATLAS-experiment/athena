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

#include "InDetTrackPerfMon/ITrackAnalysisDefinition.h"

class ITrackAnalysisDefinitionSvc :
  virtual public asg::IAsgService, virtual public ITrackAnalysisDefinition {

public:

  /// Creates the InterfaceID and interfaceID() method
  DeclareInterfaceID( ITrackAnalysisDefinitionSvc, 1, 0 );

  virtual const ITrackAnalysisDefinition* get() const = 0;

  virtual std::string plotsFullDir( std::string chain="" ) const = 0;

  /// histogram properties
  virtual bool plotTrackParameters() const = 0;
  virtual bool plotTrackParametersErrors() const = 0;
  virtual bool plotTrackMultiplicities() const = 0;
  virtual bool plotEfficiencies() const = 0;
  virtual bool plotTechnicalEfficiencies() const = 0;
  virtual bool plotResolutions() const = 0;
  virtual bool plotFakeRates() const = 0;
  virtual bool unlinkedAsFakes() const = 0;
  virtual bool plotDuplicateRates() const = 0;
  virtual bool plotHitsOnTracks() const = 0;
  virtual bool plotHitsOnTracksExpert() const = 0;
  virtual bool plotHitsOnTracksReference() const = 0;
  virtual bool plotHitsOnMatchedTracks() const = 0;
  virtual bool plotHitsOnFakeTracks() const = 0;
  virtual bool plotVertexParameters() const = 0;
  virtual bool useSelectedVertexTracks() const = 0;
  virtual bool plotOfflineElectrons() const = 0;
  virtual bool plotTracksInJets() const = 0;
  virtual unsigned int resolutionMethod() const = 0;
  virtual bool isITk() const = 0;
  
};

#endif // > ! INDETTRACKPERFMON_ITRACKANALYSISDEFINITIONSVC_H
