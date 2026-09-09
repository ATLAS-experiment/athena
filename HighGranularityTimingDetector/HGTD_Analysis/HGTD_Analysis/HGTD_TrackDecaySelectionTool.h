/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
*/

#ifndef HGTD_TRACKDECAYSELECTIONTOOL_H
#define HGTD_TRACKDECAYSELECTIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "HGTD_Analysis/IHGTD_TrackSelectionTool.h"
#include "xAODTracking/TrackParticle.h"

class HGTD_TrackDecaySelectionTool
    : public extends<AthAlgTool, IHGTD_TrackSelectionTool> {

public:
  HGTD_TrackDecaySelectionTool(const std::string&, const std::string&,
                               const IInterface*);

  virtual ~HGTD_TrackDecaySelectionTool() = default;

  //////////////////////////////////////////////////////////////////////////////
  // for AthAlgTool interface
  virtual StatusCode initialize() override final;

  // for IHGTD_TrackSelectionTool interface

  virtual bool trackPassesSelection(
      const xAOD::TrackParticle* track_particle) const override final;
  //////////////////////////////////////////////////////////////////////////////

private:
  Gaudi::Property<float> m_min_radius_prod{
      this, "MinRadiusProduction", 0,
      "Minimum accepted radius of the production vertex"};
  Gaudi::Property<float> m_max_radius_prod{
      this, "MazRadiusProduction", 10,
      "Maximum accepted radius of the production vertex"};
  Gaudi::Property<float> m_min_z_prod{
      this, "MinZProduction", 0,
      "Minimum accepted z coordinate of the production vertex"};
  Gaudi::Property<float> m_max_z_prod{
      this, "MaxZProduction", 100,
      "Maximum accepted z coordinate of the production vertex"};

  Gaudi::Property<float> m_min_radius_dec{
      this, "MinRadiusDecay", 0, "Minimum accepted radius of the decay vertex"};
  Gaudi::Property<float> m_max_radius_dec{
      this, "MazRadiusDecay", 999999999,
      "Maximum accepted radius of the decay vertex"};
  Gaudi::Property<float> m_min_z_dec{
      this, "MinZDecay", 3479,
      "Minimum accepted z coordinate of the decay vertex"};
  Gaudi::Property<float> m_max_z_dec{
      this, "MaxZDecay", 999999999,
      "Maximum accepted z coordinate of the decay vertex"};
};

#endif // HGTD_TRACKDECAYSELECTIONTOOL_H
