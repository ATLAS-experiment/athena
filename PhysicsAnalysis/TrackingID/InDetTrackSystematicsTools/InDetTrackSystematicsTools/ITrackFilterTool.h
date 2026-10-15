// -*- c++ -*-
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKSYSTEMATICSTOOLS_ITRACKFILTERTOOL_H
#define INDETTRACKSYSTEMATICSTOOLS_ITRACKFILTERTOOL_H

#include "AsgTools/IAsgTool.h"
#include "PATInterfaces/ISystematicsTool.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODJet/JetContainer.h"
#include <string>

namespace InDet {

  class InDetTrackTruthOriginTool;

  /// @class ITrackFilterTool
  /// This class selects tracks based on their truth origin
  /// and tracks in the core of a jet
  /// @author Henry van der Smagt (henry.van.der.smagt@cern.ch)
    /// This tool is a mix of the InDetTrackTruthFilterTool and the JetTrackFilterTool
  /// @author Remi Zaidan (remi.zaidan@cern.ch)
  /// @author Felix Clark (michael.ryan.clark@cern.ch)

  class ITrackFilterTool
    : public virtual asg::IAsgTool
    , public virtual CP::ISystematicsTool
  {

  public:
    ASG_TOOL_INTERFACE( InDet::ITrackFilterTool )

    virtual StatusCode initialize() = 0;
    virtual void prepare() = 0; // not sure if/why this function is necessary - felix

    // standard accept method to determine if a track should be kept or not
    virtual bool accept(const xAOD::TrackParticle* track) const = 0;

    /// Reentrant accept: applies @p syst without mutating shared state.
    /// Requires applySystematicVariation(@p syst) during initialize().
    virtual bool accept(const xAOD::TrackParticle* track,
                        const CP::SystematicSet& syst) const = 0;

    virtual bool accept( const xAOD::TrackParticle*, const xAOD::Jet* ) const = 0;
    virtual bool accept( const xAOD::TrackParticle*, const xAOD::JetContainer* ) const = 0;

    /// Reentrant accept: applies @p syst without mutating shared state.
    /// Requires applySystematicVariation(@p syst) during initialize().
    virtual bool accept(const xAOD::TrackParticle*,
                        const xAOD::JetContainer*,
                        const CP::SystematicSet& syst) const = 0;

    // implement the CP::ISystematicTool interface

    /// returns: whether the tool is affected by the systematic
    virtual bool isAffectedBySystematic( const CP::SystematicVariation& ) const = 0;
    /// returns: list of systematics this tool can be affected by
    virtual CP::SystematicSet affectingSystematics() const = 0;
    /// returns: list of recommended systematics to use with this tool
    virtual CP::SystematicSet recommendedSystematics() const = 0;
    /// configure the tool to apply a given list of systematic variations
    virtual StatusCode applySystematicVariation( const CP::SystematicSet& ) = 0;
    
  }; // class ITrackFilterTool

} // namespace InDet

#endif
