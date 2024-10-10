/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

/**
 * @file 
 *
 * Extrapolation for HepMC particles.
 *
 * @author Andrei Gaponenko, 2005, 2009
 */

#ifndef TRUTHTOTRACK_H
#define TRUTHTOTRACK_H

#include "GaudiKernel/ToolHandle.h"
#include "TrkToolInterfaces/ITruthToTrack.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkParameters/TrackParameters.h"

namespace Trk {

  
  class TruthToTrack : virtual public ITruthToTrack,
		       public ::AthAlgTool
  {
  public:
    // FIXME: interfaceID() should not be here, but clients that use TruthToTrack directly
    // (not via ITruthToTrack) will break without this:
    // static const InterfaceID& interfaceID() { return ITruthToTrack::interfaceID(); }

    TruthToTrack(const std::string& type, const std::string& name, const IInterface* parent);

    virtual StatusCode initialize();

    /** Implements interface method. */
    using ITruthToTrack::makeProdVertexParameters;
    virtual const Trk::TrackParameters* makeProdVertexParameters(HepMC::ConstGenParticlePtr part) const;
    virtual const Trk::TrackParameters* makeProdVertexParameters(const xAOD::TruthParticle* part) const;

    /** Implements interface method.
     * You can configure an extrapolator through the job options.
     */
    using ITruthToTrack::makePerigeeParameters;
    virtual const Trk::TrackParameters* makePerigeeParameters(HepMC::ConstGenParticlePtr part) const;
    virtual const Trk::TrackParameters* makePerigeeParameters(const xAOD::TruthParticle* part) const;

  private:
    ToolHandle<Trk::IExtrapolator> m_extrapolator;
  };
}

#endif/*TRUTHTOTRACK_H*/
