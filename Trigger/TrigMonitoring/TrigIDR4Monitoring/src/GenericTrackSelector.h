/** emacs: this is -*- c++ -*-
 **
 **   @file    GenericTrackSelector.h 
 **
 **   @author  sutt 
 **   @date    Fri 13 Mar 2026 22:58:12 GMT
 **
 **   $Id: GenericTrackSelector.h, v0.0   Fri 13 Mar 2026 22:58:12 GMT sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#ifndef IDTPM2_GENERICTRACKSELECTOR_H
#define IDTPM2_GENERICTRACKSELECTOR_H

// Framework includes
#include "AsgTools/AsgTool.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"


#include "ITrackSelector.h"

#include <string>

#include "ConfiguredValueUtils.h"


namespace IDTPM2 {


// @class GenericTrackSelector

class GenericTrackSelector :
    public virtual IDTPM2::ITrackSelector, public asg::AsgTool {

public:

  ASG_TOOL_CLASS( GenericTrackSelector, ITrackSelector );

  GenericTrackSelector( const std::string& name );

  virtual StatusCode initialize() override;

  virtual bool selectTrack( const TrackView& t ) const;

  virtual bool highestPTonly() const { return m_highestPTOnly; }

private:

  Gaudi::Property<ConfiguredValue<float>>   m_minPt       { this, "minPt", {}, "Low cut on phi for generic tracks"  };
  Gaudi::Property<ConfiguredValue<float>>   m_maxPt       { this, "maxPt", {}, "High cut on phi for generic tracks" };

  Gaudi::Property<ConfiguredValue<float>>   m_minET       { this, "minET",     {}, "ET cut"      };
  Gaudi::Property<ConfiguredValue<float>>   m_minETovpt   { this, "minETovpt", {}, "ET / pt cut" };

  Gaudi::Property<ConfiguredValue<float>>   m_minAbsEta   { this, "minAbsEta", {}, "Low cut on |eta| for generic tracks"  };
  Gaudi::Property<ConfiguredValue<float>>   m_minEta      { this, "minEta",    {}, "Lower cut on eta for generic tracks"  };
  Gaudi::Property<ConfiguredValue<float>>   m_maxEta      { this, "maxEta",    {}, "Higher cut on eta for generic tracks" };

  Gaudi::Property<ConfiguredValue<float>>   m_minD0       { this, "minD0",    {}, "Lower cut on d0 for generic tracks"  };
  Gaudi::Property<ConfiguredValue<float>>   m_maxD0       { this, "maxD0",    {}, "Higher cut on d0 for generic tracks" };
  Gaudi::Property<ConfiguredValue<float>>   m_minZ0       { this, "minZ0",    {}, "Lower cut on z0 for generic tracks"  };
  Gaudi::Property<ConfiguredValue<float>>   m_maxZ0       { this, "maxZ0",    {}, "Higher cut on z0 for generic tracks" };
  Gaudi::Property<ConfiguredValue<float>>   m_minAbsD0    { this, "minAbsD0", {}, "Lower cut on |d0| for generic tracks"  };
  Gaudi::Property<ConfiguredValue<float>>   m_maxAbsD0    { this, "maxAbsD0", {}, "Higher cut on |d0| for generic tracks" };
  Gaudi::Property<ConfiguredValue<float>>   m_minAbsZ0    { this, "minAbsZ0", {}, "Lower cut on |z0| for generic tracks"  };
  Gaudi::Property<ConfiguredValue<float>>   m_maxAbsZ0    { this, "maxAbsZ0", {}, "Higher cut on |z0| for generic tracks" };

  
  Gaudi::Property<ConfiguredValue<float>>   m_minPhi      { this, "minPhi", {}, "Lower cut on phi for generic tracks"  };
  Gaudi::Property<ConfiguredValue<float>>   m_maxPhi      { this, "maxPhi", {}, "Higher cut on phi for generic tracks" };
  /// Abs phi ???? why would anyone ever want abs(phi) ?? 
  //  Gaudi::Property<ConfiguredValue<float>>   m_minAbsPhi   { this, "minAbsPhi", {}., "Lower cut on |phi| for generic tracks" };
  //  Gaudi::Property<ConfiguredValue<float>>   m_maxAbsPhi   { this, "maxAbsPhi", {}., "Higher cut on |phi| for generic tracks" };

  Gaudi::Property<ConfiguredValue<float>>   m_minQoPT     { this, "minQoPT", {}, "Lower cut on q/pt for generic tracks"  };
  Gaudi::Property<ConfiguredValue<float>>   m_maxQoPT     { this, "maxQoPT", {}, "Higher cut on q/pt for generic tracks" };

  Gaudi::Property<ConfiguredValue<float>>   m_minAbsQoPT  { this, "minAbsQoPT", {}, "Lower cut on |q/pt| for generic tracks"  };
  Gaudi::Property<ConfiguredValue<float>>   m_maxAbsQoPT  { this, "maxAbsQoPT", {}, "Higher cut on |q/pt| for generic tracks" };

  Gaudi::Property<bool> m_highestPTOnly  { this, "HighestPTOnly",  false, "use only the highest PT track" };

  bool m_initialized = false;
};

} // namespace IDTPM2

#endif // IDTPM2_GENERICTRACKSELECTOR_H
