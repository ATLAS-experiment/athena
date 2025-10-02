/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef InDetSecVtxTruthMatchTool_h
#define InDetSecVtxTruthMatchTool_h

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"

// EDM include(s):
#include "xAODTracking/VertexContainerFwd.h"
#include "xAODTruth/TruthParticleFwd.h"
#include "xAODTruth/TruthVertexFwd.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "InDetSecVtxTruthMatchTool/IInDetSecVtxTruthMatchTool.h"
#include "InDetTrackSystematicsTools/IInDetTrackTruthOriginTool.h"


// standard includes
#include<vector>
#include <bitset>

namespace InDetSecVtxTruthMatchUtils {

  //Namespace for useful analysis things on the truth matching decorations applied to the VertexContainer

  //How the matching info is stored; link to the truth vertex, a float with its contribution to the relative track weight, and a float with its contribution to the track sumpt2 of the truth vertex
  typedef std::tuple<ElementLink<xAOD::TruthVertexContainer>, float, float> VertexTruthMatchInfo;

  //type codes for vertex matching on all vertices
  enum VertexMatchType {
    Matched=0, // > threshold (default 50%) from one truth interaction
    Merged,    // not matched
    Split,     // highest weight truth interaction contributes to >1 vtx (vtx with highest fraction of sumpT2 remains matched/merged)
    Fake,      // highest contribution is fake (if pile-up MC info not present those tracks end up as "fakes")
    Other
  };

  //type codes for optional SM origin matching
  //used to determine the content of "other" type vertices above for in-depth studies
  //should only be used where extended truth info is available -- e.g when LLP1 has been run with saveFullTruth=True
  //NOTE: types are NOT exclusive -- vertex can be both from B and D decay, signal and hadronic interation, etc
enum VertexMatchOriginType {
  // Must match InDet::TrkOrigin::OriginType values!
  FakeOrigin = 0,
  Pileup,
  KshortDecay,
  StrangeMesonDecay,
  LambdaDecay,
  StrangeBaryonDecay,
  TauDecay,
  GammaConversion,
  OtherDecay,
  HadronicInteraction,
  OtherSecondary,
  BHadronDecay,
  DHadronDecay,
  Fragmentation,
  OtherOrigin,
  Signal
};

  //type codes for truth vertices
  //NOTE: types are subsets of subsequent types
  enum TruthVertexMatchType {
    Reconstructable=0, // fiducial cuts, >= 2 charged daughters
    Accepted,          // >= 2 reco tracks
    Seeded,            // tracks pass cuts
    Reconstructed,     // matched to reco vtx
    ReconstructedSplit // matched to multiple vtx
  };

  inline bool isMatched(int matchInfo) {
    if (matchInfo & (0x1 << Matched)) return true;
    return false;
  }
  inline bool isMerged(int matchInfo) {
    if (matchInfo & (0x1 << Merged)) return true;
    return false;
  }
  inline bool isSplit(int matchInfo) {
    if (matchInfo & (0x1 << Split)) return true;
    return false;
  }
  inline bool isFake(int matchInfo) {
    if (matchInfo & (0x1 << Fake)) return true;
    return false;
  }
  inline bool isOther(int matchInfo) {
    if (matchInfo & (0x1 << Other)) return true;
    return false;
  }

  inline bool isReconstructable(int matchInfo) {
    if (matchInfo & (0x1 << Reconstructable)) return true;
    return false;
  }
  inline bool isAccepted(int matchInfo) {
    if (matchInfo & (0x1 << Accepted)) return true;
    return false;
  }
  inline bool isSeeded(int matchInfo) {
    if (matchInfo & (0x1 << Seeded)) return true;
    return false;
  }
  inline bool isReconstructed(int matchInfo) {
    if (matchInfo & (0x1 << Reconstructed)) return true;
    return false;
  }
  inline bool isReconstructedSplit(int matchInfo) {
    if (matchInfo & (0x1 << ReconstructedSplit)) return true;
    return false;
  }

  inline bool isOriginType(int matchInfo, VertexMatchOriginType type) {
    if (type < 0 || type > Signal) { // Signal is the last valid value
      return false;
    }
    return matchInfo & (0x1 << type);
  }
}

/** Class for vertex truth matching.
 * Match reconstructed vertices to truth level decay vertices
 * Categorize reconstructed vertices depending on their composition.
 */
class InDetSecVtxTruthMatchTool : public virtual IInDetSecVtxTruthMatchTool,
                                     public asg::AsgTool {

  ASG_TOOL_CLASS( InDetSecVtxTruthMatchTool, IInDetSecVtxTruthMatchTool )

 public:

  InDetSecVtxTruthMatchTool( const std::string & name );

  virtual StatusCode initialize() override final;

  // take collection of vertices, match them, and decorate with matching info
  virtual StatusCode matchVertices( std::vector<const xAOD::Vertex*> recoVerticesToMatch, std::vector<const xAOD::TruthVertex*> truthVerticesToMatch, const xAOD::TrackParticleContainer* trackParticles ) override;

 private:

  Gaudi::Property<float> m_trkMatchProb{this, "trackMatchProb", 0.5, "Required MC match probability to consider track a good match" };
  Gaudi::Property<float> m_vxMatchWeight{this, "vertexMatchWeight", 0.5, "Relative weight threshold to consider vertex matched"};
  Gaudi::Property<float> m_trkPtCut{this, "trackPtCut", 1000., "pt cut to apply on tracks"};
  Gaudi::Property<std::string> m_selectedTrackFlag{this, "selectedTrackFlag", "is_selected", "Aux decoration on tracks for seeding efficiencies"};
  Gaudi::Property<bool> m_doMuSA{this, "doMuSA", false, "Combination flag for special MuSA logic" };
  Gaudi::Property<bool> m_doSMOrigin{this, "doSMOrigin", false, "Enable decoration of SM origin types"};

  ToolHandle<InDet::IInDetTrackTruthOriginTool> m_trackTruthOriginTool{this, "TrackTruthOriginTool", "InDet::InDetTrackTruthOriginTool/TrackTruthOriginTool"};

  //private methods to check if particles are good to use
  //returns barcode of LLP production truth vertex
  int checkProduction( const xAOD::TruthParticle& truthPart, std::vector<const xAOD::TruthVertex*> truthVerticesToMatch ) const;
  void countReconstructibleDescendentParticles(const xAOD::TruthVertex& signalTruthVertex,
                                               std::vector<const xAOD::TruthParticle*>& set, int counter) const;
  std::vector<int> checkParticle( const xAOD::TruthParticle& part, const xAOD::TrackParticleContainer* tkCont ) const;
  bool isFrom(const xAOD::TruthParticle& truth, int flav) const;
  int checkSMProduction( const xAOD::TruthParticle& truthPart) const;
};

#endif
