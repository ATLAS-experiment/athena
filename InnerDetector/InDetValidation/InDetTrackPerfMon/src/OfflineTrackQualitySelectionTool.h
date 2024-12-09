/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_IDTPM_OFFLINETRACKQUALITYSELECTIONTOOL_H
#define INDETTRACKPERFMON_IDTPM_OFFLINETRACKQUALITYSELECTIONTOOL_H

// Local includes
#include "ITrackSelectionTool.h"

// Framework includes
#include "AsgTools/AsgTool.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"

// STL includes
#include <string>

namespace IDTPM {

/**
 * @class OfflineTrackQualitySelectionTool
 * @brief Uses InDetTrackSelection tool and working points defined there for tracks quality selection
 * In future it is possible that the selection cuts & logic will be moved here
 **/

class OfflineTrackQualitySelectionTool :
    public virtual IDTPM::ITrackSelectionTool, public asg::AsgTool {

public:

  ASG_TOOL_CLASS( OfflineTrackQualitySelectionTool, ITrackSelectionTool );

  OfflineTrackQualitySelectionTool( const std::string& name );

  virtual StatusCode initialize() override;

  virtual StatusCode selectTracks(
      TrackAnalysisCollections& trkAnaColls ) override;

  bool accept( const xAOD::TrackParticle* track );

private:

  ToolHandle< InDet::IInDetTrackSelectionTool > m_offlineTool { this, "offlineTool", "", "Instance name of track selection tool" };

  FloatProperty   m_minAbsEta   { this, "minAbsEta", -9999., "Lower cut on |eta| for truth particles" };
  FloatProperty   m_minAbsPhi   { this, "minAbsPhi", -9999., "Lower cut on |phi| for truth particles" };
  FloatProperty   m_maxAbsPhi   { this, "maxAbsPhi", -9999., "Higher cut on |phi| for truth particles" };
  FloatProperty   m_minAbsD0    { this, "minAbsD0", -9999., "Lower cut on |d0| for truth particles" };
  FloatProperty   m_maxAbsD0    { this, "maxAbsD0", -9999., "Higher cut on |d0| for truth particles" };
  FloatProperty   m_minAbsZ0    { this, "minAbsZ0", -9999., "Lower cut on |z0| for truth particles" };
  FloatProperty   m_maxAbsZ0    { this, "maxAbsZ0", -9999., "Higher cut on |z0| for truth particles" };
  FloatProperty   m_minAbsQoPT  { this, "minAbsQoPT", -9999., "Lower cut on |q/pt| for truth particles" };
  FloatProperty   m_maxAbsQoPT  { this, "maxAbsQoPT", -9999., "Higher cut on |q/pt| for truth particles" };
  FloatProperty   m_maxPt       { this, "maxPt", -9999., "Higher cut on phi for truth particles" };
  FloatProperty   m_minEta      { this, "minEta", -9999., "Lower cut on eta for truth particles" };
  FloatProperty   m_maxEta      { this, "maxEta", -9999., "Higher cut on eta for truth particles" };
  FloatProperty   m_minPhi      { this, "minPhi", -9999., "Lower cut on phi for truth particles" };
  FloatProperty   m_maxPhi      { this, "maxPhi", -9999., "Higher cut on phi for truth particles" };
  FloatProperty   m_minD0       { this, "minD0", -9999., "Lower cut on d0 for truth particles" };
  FloatProperty   m_minZ0       { this, "minZ0", -9999., "Lower cut on z0 for truth particles" };
  FloatProperty   m_minQoPT     { this, "minQoPT", -9999., "Lower cut on q/pt for truth particles" };
  FloatProperty   m_maxQoPT     { this, "maxQoPT", -9999., "Higher cut on q/pt for truth particles" };

};

} // namespace IDTPM

#endif // INDETTRACKPERFMON_IDTPM_OFFLINETRACKQUALITYSELECTIONTOOL_H
