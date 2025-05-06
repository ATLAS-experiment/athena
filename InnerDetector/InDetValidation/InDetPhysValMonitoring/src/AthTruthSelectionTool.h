/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETPHYSVALMONITORING_ATHTRUTHSELECTIONTOOL_H
#define INDETPHYSVALMONITORING_ATHTRUTHSELECTIONTOOL_H

/**
 * @file AthTruthSelectionTool.h
 * header file for truth selection in this package
 * @author shaun roe
 * @date 10 October 2016
 **/

// STL includes
#include <string>
#include "TrkTruthTrackInterfaces/IAthSelectionTool.h"
#include "xAODTruth/TruthParticle.h" // typedef, can't fwd declare
#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetPhysValMonitoring/CutFlow.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkExInterfaces/IExtrapolator.h"


/// class to apply selection to xAOD::TruthParticles,required by validation
class AthTruthSelectionTool: virtual public IAthSelectionTool, public AthAlgTool {
public:
  AthTruthSelectionTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~AthTruthSelectionTool () {
    /*nop*/
  };
  StatusCode initialize() final;
  StatusCode finalize() final;

  virtual IAthSelectionTool::CutResult
  accept(const xAOD::IParticle* particle) const final;

  virtual IAthSelectionTool::CutResult
  testAllCuts(const xAOD::IParticle * p, std::vector<unsigned int> &counter) const final;

  unsigned int nCuts() const final {
    return m_cutList.size();
  }

  std::vector<std::string> names() const final;

private:
  CutList<xAOD::TruthParticle> m_cutList;
  // Cut values;
  FloatProperty m_maxEta{this, "maxEta", 2.5};
  FloatProperty m_maxPt{this, "maxPt", -1.};
  FloatProperty m_minPt{this, "minPt", 400.};
  BooleanProperty m_requireOnlyPrimary{this, "requireOnlyPrimary", true};
  BooleanProperty m_requireCharged{this, "requireCharged", true};
  IntegerProperty m_selectedCharge{this, "selectedCharge", 0};
  BooleanProperty m_requireStable{this, "requireStable", true};
  IntegerProperty m_requireSiHit{this, "requireSiHit", 0};
  // max decay radius for secondaries [mm];
  // set to within (Run2) pixel by default
  FloatProperty m_maxProdVertRadius{this, "maxProdVertRadius", 110.};
  IntegerProperty m_pdgId{this, "pdgId", -1};
  IntegerProperty m_vetoPdgId{this, "vetoPdgId", -1};
  BooleanProperty m_grandparent{this, "hasNoGrandparent", false};
  BooleanProperty m_poselectronfromgamma{this, "poselectronfromgamma", false};
  std::vector<unsigned int> m_counters{};
  IntegerArrayProperty m_ancestors{this, "ancestorList", {}};
  
  /* \defgroup Selection on extrapolated particle to cylinder or disk surface
   * @{
   */
  FloatProperty m_radiusCylinder{this, "radiusCylinder", -1.,
    "Select truth particle based on extrapolated position on cylinder placed at this radius. Enabled if greater than 0."};
  FloatProperty m_minZCylinder{this, "minZCylinder", 0.,
    "Minimum |Z| on cylinder for accepting extrapolated truth particle to surface."};
  FloatProperty m_maxZCylinder{this, "maxZCylinder", 0.,
    "Maximum |Z| on cylinder for accepting extrapolated truth particle to surface."};
  FloatProperty m_zDisc{this, "zDisc", -1.,
    "Select truth particle based on extrapolated position on disks placed at +/- z positions. Enabled if greater than 0."};
  FloatProperty m_minRadiusDisc{this, "minRadiusDisc", 0.,
    "Minimum radius on disk for accepting extrapolated truth particle to surface."};
  FloatProperty m_maxRadiusDisc{this, "maxRadiusDisc", 0.,
    "Maximum radius on disk for accepting extrapolated truth particle to surface."};

  //cache surfaces
  std::unique_ptr<Trk::CylinderSurface> m_cylinder;
  std::unique_ptr<Trk::DiscSurface>     m_disc1;
  std::unique_ptr<Trk::DiscSurface>     m_disc2;


  /* @} */

  ///Too handle for truth-track extrapolation
  PublicToolHandle<Trk::IExtrapolator> m_extrapolator
     {this,"Extrapolator","Trk::Extrapolator/AtlasExtrapolator",""};

};


#endif
