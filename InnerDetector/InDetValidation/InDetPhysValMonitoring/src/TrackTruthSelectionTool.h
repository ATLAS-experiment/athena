/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETPHYSVALMONITORING_TRACKTRUTHSELECTORTOOL_H
#define INDETPHYSVALMONITORING_TRACKTRUTHSELECTORTOOL_H 1

#include "AthenaBaseComps/AthAlgTool.h"
#include "PATCore/IAsgSelectionTool.h"
#include "xAODTruth/TruthParticle.h"
#include "AsgTools/AsgTool.h"

#include <atomic>
#include <mutex>

class TrackTruthSelectionTool:
  public virtual ::IAsgSelectionTool,
  public asg::AsgTool  {
  ASG_TOOL_CLASS1(TrackTruthSelectionTool, IAsgSelectionTool);
public:
  TrackTruthSelectionTool(const std::string& name);
  // TrackTruthSelectionTool(const std::string& type,const std::string& name,const IInterface* parent);
  virtual
  ~TrackTruthSelectionTool();

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual const asg::AcceptInfo& getAcceptInfo( ) const override;
  virtual asg::AcceptData accept(const xAOD::IParticle* p) const override;
  virtual asg::AcceptData accept(const xAOD::TruthParticle* p) const;
private:
  asg::AcceptInfo m_accept{"TrackTruthSelection"};
  std::vector<std::pair<std::string, std::string> > m_cuts;
  mutable std::atomic<ULong64_t> m_numTruthProcessed{0}; // !< a counter of the number of tracks proccessed
  mutable std::atomic<ULong64_t> m_numTruthPassed{0}; // !< a counter of the number of tracks that passed all cuts
  mutable std::vector<ULong64_t> m_numTruthPassedCuts ATLAS_THREAD_SAFE; // !< tracks the number of tracks that passed each cut family. Guarded by m_mutex
  mutable std::mutex m_mutex; // !< To guard m_numTruthPassedCuts

  // Cut values;
  FloatProperty m_maxEta{this, "maxEta", 2.5};
  FloatProperty m_maxPt{this, "maxPt", -1.};
  FloatProperty m_minPt{this, "minPt", 400.};
  BooleanProperty m_requireOnlyPrimary{this, "requireOnlyPrimary", true};
  BooleanProperty m_requireCharged{this, "requireCharged", true};
  BooleanProperty m_requireStable{this, "requireStable", true};
  // max decay radius for secondaries [mm];
  // set to within (Run2) pixel by default; set to <0 for no cut
  FloatProperty m_maxProdVertRadius{this, "maxProdVertRadius", 110.};
  IntegerProperty m_pdgId{this, "pdgId", -1};
};

#endif // > !INDETPHYSVALMONITORING_TRACKTRUTHSELECTORTOOL_H
