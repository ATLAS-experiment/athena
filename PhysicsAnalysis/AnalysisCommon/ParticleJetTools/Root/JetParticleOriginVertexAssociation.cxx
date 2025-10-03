/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// author: cpollard@cern.ch

#include "ParticleJetTools/JetParticleOriginVertexAssociation.h"
#include "AsgMessaging/Check.h"

using namespace std;
using namespace xAOD;

StatusCode JetParticleOriginVertexAssociation::initialize() {
  ATH_CHECK(JetParticleAssociation::initialize());

  CHECK(m_TrackContainerKey.initialize());
  
  
  m_dec_d0       = m_TrackContainerKey.key() + "." + m_prefix.value() +  m_dec_d0.key();
  m_dec_z0       = m_TrackContainerKey.key() + "." + m_prefix.value() +  m_dec_z0.key();
  m_dec_d0_sigma = m_TrackContainerKey.key() + "." + m_prefix.value() +  m_dec_d0_sigma.key();
  m_dec_z0_sigma = m_TrackContainerKey.key() + "." + m_prefix.value() +  m_dec_z0_sigma.key();
  m_dec_DupTrk_link = m_TrackContainerKey.key() + "." + m_prefix.value() +  m_dec_DupTrk_link.key();
  m_dec_track_pos = m_TrackContainerKey.key() + "." + m_prefix.value() +  m_dec_track_pos.key();
  m_dec_track_mom = m_TrackContainerKey.key() + "." + m_prefix.value() +  m_dec_track_mom.key();
  m_dec_invalid = m_TrackContainerKey.key() + "." + m_prefix.value() +  m_dec_invalid.key();
  m_dec_trk_origin_vtx_idx = m_TrackContainerKey.key() + "." + m_prefix.value() +  m_dec_trk_origin_vtx_idx.key();

  m_dzCut_bool = (m_dzCut > 4);

  
  //if (m_dzCut_bool){
  CHECK( m_dec_d0.initialize(m_dzCut_bool) );
  CHECK( m_dec_z0.initialize(m_dzCut_bool) );
  CHECK( m_dec_d0_sigma.initialize(m_dzCut_bool) );
  CHECK( m_dec_z0_sigma.initialize(m_dzCut_bool) );
  CHECK( m_dec_DupTrk_link.initialize(m_dzCut_bool) );   
  CHECK( m_dec_track_pos.initialize(m_dzCut_bool) ); 
  CHECK( m_dec_track_mom.initialize(m_dzCut_bool) ); 
  CHECK( m_dec_invalid.initialize(m_dzCut_bool) ); 
  CHECK( m_dec_trk_origin_vtx_idx.initialize(m_dzCut_bool) );  
  CHECK( m_outDuplicatedTrackContainerKey.initialize(m_dzCut_bool) );
  CHECK( m_readDuplicatedTrackContainerKey.initialize(!m_dzCut_bool) );
  //}

  return StatusCode::SUCCESS;
}

JetParticleOriginVertexAssociation::JetParticleOriginVertexAssociation(const string& name)
    : JetParticleAssociation(name), m_dzCut_bool(false) {

        declareProperty("coneSizeFitPar1", m_coneSizeFitPar1=0);
        declareProperty("coneSizeFitPar2", m_coneSizeFitPar2=0);
        declareProperty("coneSizeFitPar3", m_coneSizeFitPar3=0);
        declareProperty("dzCut", m_dzCut=10);
        declareProperty("useMinZ0Vertex", m_useMinZ0Vertex=false);

        return;
    }

struct MatchInfo {
  unsigned int jetIdx;
  // Float variables
  float d0 = -99;
  float z0SinTheta = -99;
  float d0Uncertainty = -99;
  float z0SinThetaUncertainty = -99;

  // Int variables
  int TrkOriginVtx_idx = -99;

  // Char variables
  char invalidIp = -99; 

  // Vector<float> variables
  std::vector<float> trackDisplacement;
  std::vector<float> trackMomentum;
  /*
  // std::variant allows multiple types
  using VarType = std::variant<float, int, char, std::vector<float>>;
  std::map<std::string, VarType> values;
  */
};
// function to pair the z0 vector and vertex links together to sort them in ascending order of abs(z0)
std::vector<std::pair<float, ElementLink<xAOD::VertexContainer>>>
combined_sorted(const std::vector<float>& z0, const std::vector<ElementLink<xAOD::VertexContainer>>& vertexLink){
  std::vector<std::pair<float,ElementLink<xAOD::VertexContainer>>> combined;
  for (size_t i = 0; i < z0.size(); i++) {
    combined.emplace_back(z0[i], vertexLink[i]);
  }

  std::sort(combined.begin(), combined.end(),
            [](auto &a, auto &b) { return std::abs(a.first) < std::abs(b.first); });

  return combined;
}

const std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >*
JetParticleOriginVertexAssociation::match(const xAOD::JetContainer& jets, const xAOD::IParticleContainer& parts) const {
    std::vector<std::string> perVertexVarNames = {
      "d0",
      "z0SinTheta",
      "d0Uncertainty",
      "z0SinThetaUncertainty"
      "trackDisplacement",
      "trackMomentum",
      "invalidIp"
      "TrkOriginVtx_idx"
    };
    //Get the vertex associated to each track by reading the decoration
    const SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::VertexContainer>>> trkOrigin("btagIp_ByVertex_TrkOriginVtx");
    const SG::AuxElement::ConstAccessor<std::vector<float>> z0SinTheta("btagIp_ByVertex_z0SinTheta");
    //Create the 2d output vector 
    std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >* matchedparts =
        new vector<std::vector<ElementLink<xAOD::IParticleContainer> > >(jets.size());
    
    // tupple storing the jet index and z0 value of the jets matched to the track
    std::vector<MatchInfo> matchjet_vtx;
    if (m_dzCut_bool){
        auto outDuplicatedTrackContainerHandle = SG::makeHandle(m_outDuplicatedTrackContainerKey);
        auto DuplicatedTrks = std::make_unique<xAOD::TrackParticleContainer>();
        auto DuplicatedTrksAux = std::make_unique<xAOD::TrackParticleAuxContainer>();
        DuplicatedTrks->setStore (DuplicatedTrksAux.get()); //< Connect the two
        // SG::WriteHandle to write the container
        // Creatde the decoration handles 
        SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> decor_d0(m_dec_d0);
        SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> decor_z0(m_dec_z0);
        SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> decor_d0_sigma(m_dec_d0_sigma);
        SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> decor_z0_sigma(m_dec_z0_sigma);
        SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float> > decor_track_pos(m_dec_track_pos);
        SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float> > decor_track_mom(m_dec_track_mom);
        SG::WriteDecorHandle<xAOD::TrackParticleContainer, int> decor_TrkOriginVtx_idx(m_dec_trk_origin_vtx_idx);
        SG::WriteDecorHandle<xAOD::TrackParticleContainer, char > decor_invalid(m_dec_invalid);
        SG::WriteDecorHandle<xAOD::IParticleContainer, std::vector<ElementLink<xAOD::IParticleContainer>> > decor_DupTrk_link(m_dec_DupTrk_link);

        //loop through the tracks
        for (const IParticle* part: parts) {   
          // Retrieve the vector of vertex ElementLinks associated with the track and the corresponding z0 vector
          const std::vector<ElementLink<xAOD::VertexContainer>>& vertexLink_vec = trkOrigin(*part);
          const std::vector<float>& z0SinTheta_vec = z0SinTheta(*part); // sort this 
          // check if the vectors are not empty
          if(z0SinTheta_vec.empty()) continue;
          // Combine the 2 vectors and sort them according to abs(z0)
          //const std::vector<std::pair<float,ElementLink<xAOD::VertexContainer>>>& z0SinTheta_vertexLink_vec = combined_sorted(z0SinTheta_vec, vertexLink_vec);
          // clear the vector
          matchjet_vtx.clear();
          // loop through vertices
          for (unsigned int iVtx=0; iVtx < z0SinTheta_vec.size(); iVtx++){
            // retrieve vertex link
            const ElementLink<xAOD::VertexContainer>& vertexLink = vertexLink_vec[iVtx];
            // check if link is valid
            if (!vertexLink.isValid()) {
              ATH_MSG_WARNING("Track decoration 'btagIp_TrkOriginVertex' is missing for this track");
              continue;
            }
            // if the dz(track, vertex) doesnt pass the cut then continue to next vertex
            if (std::abs(z0SinTheta_vec[iVtx]) > m_dzCut) continue;
            //Get vertex associated with the track
            const Vertex* vtx_to_trk = *vertexLink; 
            int matchjetidx = -1;
            double drmin = -1.0;
            // Loop through jets
            for (unsigned int iJet = 0; iJet < jets.size(); iJet++) {
              //get jet
              const Jet* jet = jets[iJet];
              // if origin of jet is not the same as the vertex associated to the track then continue to next jet
              if (jet->getAssociatedObject<xAOD::Vertex>("OriginVertex") != vtx_to_trk) continue;
              
              // do dR matching between jet and track
              double match_dr = coneSize(jet->pt());
              double dr = jet->p4().DeltaR(part->p4());
              if (dr > match_dr) continue;
              if (drmin < 0 || dr < drmin) {
                  drmin = dr;
                  matchjetidx = iJet;
              }
            }
            // Save info of the IP variables to a vector
            if (matchjetidx < 0){continue;}
            
            SG::AuxElement::Accessor<std::vector<float>> acc_d0("btagIp_ByVertex_d0");
            SG::AuxElement::Accessor<std::vector<float>> acc_z0("btagIp_ByVertex_z0SinTheta");
            SG::AuxElement::Accessor<std::vector<float>> acc_d0_sigma("btagIp_ByVertex_d0Uncertainty");
            SG::AuxElement::Accessor<std::vector<float>> acc_z0_sigma("btagIp_ByVertex_z0SinThetaUncertainty");
            SG::AuxElement::Accessor<std::vector<std::vector<float>>> acc_track_pos("btagIp_ByVertex_trackDisplacement");
            SG::AuxElement::Accessor<std::vector<std::vector<float>>> acc_track_mom("btagIp_ByVertex_trackMomentum");
            SG::AuxElement::Accessor<std::vector<char>> acc_invalid("btagIp_ByVertex_invalidIp");
            SG::AuxElement::Accessor<std::vector<int>> acc_TrkOriginVtx_idx("btagIp_ByVertex_TrkOriginVtx_idx");

            MatchInfo info;
            info.jetIdx = matchjetidx;
            info.d0 = acc_d0(*part).at(iVtx);
            info.z0SinTheta = acc_z0(*part).at(iVtx);
            info.d0Uncertainty = acc_d0_sigma(*part).at(iVtx);
            info.z0SinThetaUncertainty = acc_z0_sigma(*part).at(iVtx);
            info.trackDisplacement = acc_track_pos(*part).at(iVtx);
            info.trackMomentum = acc_track_mom(*part).at(iVtx);
            info.invalidIp = acc_invalid(*part).at(iVtx);
            info.TrkOriginVtx_idx = acc_TrkOriginVtx_idx(*part).at(iVtx);
            // loop over all per-vertex variables
            /*
            for (const auto& varName : perVertexVarNames) {
              SG::AuxElement::ConstAccessor<std::vector<float>> acc("btagIp_ByVertex_" + varName);
              const std::vector<float>& vec = acc(*part);
              if (iVtx < vec.size()) {
                info.values[varName] = vec.at(iVtx);
              }
            }
            */
            matchjet_vtx.push_back(std::move(info));          
            
          }
          //sort vector by abs(z0)
          std::sort(matchjet_vtx.begin(), matchjet_vtx.end(),
          [](const MatchInfo& a, const MatchInfo& b) {
            return std::abs(a.z0SinTheta) < std::abs(b.z0SinTheta);
          });
          // if there is more than one jet associated with track, then deep copy track
          const xAOD::TrackParticle* trk = dynamic_cast<const xAOD::TrackParticle*>(part);
          if (matchjet_vtx.size() > 0){
            unsigned int count_trks=1; // counter 
            for (const auto& match: matchjet_vtx){
              // for the first jet associated with the track save it to the original track collection
              if (count_trks == 1){ // add track IP value to original trk collection for the first jet that is associated with

                decor_d0(*trk) = match.d0;
                decor_z0(*trk) = match.z0SinTheta;
                decor_d0_sigma(*trk) = match.d0Uncertainty;
                decor_z0_sigma(*trk) = match.z0SinThetaUncertainty;
                decor_track_pos(*trk) = match.trackDisplacement;
                decor_track_mom(*trk) = match.trackMomentum;
                decor_invalid(*trk) = match.invalidIp;
                decor_TrkOriginVtx_idx(*trk) = match.TrkOriginVtx_idx;
                
                ElementLink<xAOD::IParticleContainer> EL; 
                EL.toContainedElement(parts, part);
                (*matchedparts)[match.jetIdx].push_back(EL);
              }else{ // if track is associated with other jets, then duplicate it and add it to the new track collection
                xAOD::TrackParticle* DuplicatedTrk = new xAOD::TrackParticle();
                DuplicatedTrks->push_back(DuplicatedTrk); // track acquires the DuplicatedTrk auxstore
                *DuplicatedTrk = *trk; // copies auxdata from one auxstore to the other
               
                SG::AuxElement::Accessor<float> acc_d0("btagIp_ByVertex1_d0");
                SG::AuxElement::Accessor<float> acc_z0("btagIp_ByVertex1_z0SinTheta");
                SG::AuxElement::Accessor<float> acc_d0_sigma("btagIp_ByVertex1_d0Uncertainty");
                SG::AuxElement::Accessor<float> acc_z0_sigma("btagIp_ByVertex1_z0SinThetaUncertainty");
                SG::AuxElement::Accessor<std::vector<float>> acc_track_pos("btagIp_ByVertex1_trackDisplacement");
                SG::AuxElement::Accessor<std::vector<float>> acc_track_mom("btagIp_ByVertex1_trackMomentum");
                SG::AuxElement::Accessor<char> acc_invalid("btagIp_ByVertex1_invalidIp");
                SG::AuxElement::Accessor<int> acc_TrkOriginVtx_idx("btagIp_ByVertex1_TrkOriginVtx_idx");
                
                acc_d0(*DuplicatedTrk) = match.d0;
                acc_z0(*DuplicatedTrk) = match.z0SinTheta;
                acc_d0_sigma(*DuplicatedTrk) = match.d0Uncertainty;
                acc_z0_sigma(*DuplicatedTrk) = match.z0SinThetaUncertainty;
                acc_track_pos(*DuplicatedTrk) = match.trackDisplacement;
                acc_track_mom(*DuplicatedTrk) = match.trackMomentum;
                acc_invalid(*DuplicatedTrk) = match.invalidIp;
                acc_TrkOriginVtx_idx(*DuplicatedTrk) = match.TrkOriginVtx_idx;
                
                // Match track to the jet 
                ElementLink<xAOD::IParticleContainer> EL; 
                EL.toContainedElement(*DuplicatedTrks, DuplicatedTrk);
                decor_DupTrk_link(*part).push_back(EL);
                (*matchedparts)[match.jetIdx].push_back(EL);
              }
              count_trks++;
            }
          }
        }

        if (!outDuplicatedTrackContainerHandle.record(std::move(DuplicatedTrks), std::move(DuplicatedTrksAux))) {
          ATH_MSG_ERROR("Unable to write new DuplicateTrks to event store: " 
                        << m_outDuplicatedTrackContainerKey.key());
          return nullptr;
        }
        
        //ATH_CHECK( evtStore()->record(std::move(DuplicatedTrks), "DuplicatedTrks") );
        //ATH_CHECK( evtStore()->record(std::move(DuplicatedTrksAux), "DuplicatedTrksAux") );
        //StatusCode sc = evtStore()->record(DuplicatedTrks.release(), "DuplicatedTrks");
        //sc = evtStore()->record(DuplicatedTrksAux.release(), "DuplicatedTrksAux");
    } else{
      SG::ReadHandle< xAOD::TrackParticleContainer > ExistingDuplicatedTrksHandle = SG::makeHandle< xAOD::TrackParticleContainer >(m_readDuplicatedTrackContainerKey);
    
      const xAOD::TrackParticleContainer* ExistingDuplicatedTrks = ExistingDuplicatedTrksHandle.get();
      const SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::IParticleContainer>>> DupTrk_link("btagIp_ByVertex1_DupTrk_link");

      //loop through the tracks
      for (const IParticle* part: parts) {   
        // Retrieve the vector of vertex ElementLinks associated with the track and the corresponding z0 vector
        const std::vector<ElementLink<xAOD::VertexContainer>>& vertexLink_vec = trkOrigin(*part);
        const std::vector<float>& z0SinTheta_vec = z0SinTheta(*part); // sort this 
        // check if the vectors are not empty
        if(z0SinTheta_vec.empty()) continue;
        // Combine the 2 vectors and sort them according to abs(z0)
        //const std::vector<std::pair<float,ElementLink<xAOD::VertexContainer>>>& z0SinTheta_vertexLink_vec = combined_sorted(z0SinTheta_vec, vertexLink_vec);
        // clear the vector
        matchjet_vtx.clear();
        // loop through vertices
        for (unsigned int iVtx=0; iVtx < z0SinTheta_vec.size(); iVtx++){
          // retrieve vertex link
          const ElementLink<xAOD::VertexContainer>& vertexLink = vertexLink_vec[iVtx];
          // check if link is valid
          if (!vertexLink.isValid()) {
            ATH_MSG_WARNING("Track decoration 'btagIp_TrkOriginVertex' is missing for this track");
            continue;
          }
          // if the dz(track, vertex) doesnt pass the cut then continue to next vertex
          if (std::abs(z0SinTheta_vec[iVtx]) > m_dzCut) continue;
          //Get vertex associated with the track
          const Vertex* vtx_to_trk = *vertexLink; 
          int matchjetidx = -1;
          double drmin = -1.0;
          // Loop through jets
          for (unsigned int iJet = 0; iJet < jets.size(); iJet++) {
            //get jet
            const Jet* jet = jets[iJet];
            // if origin of jet is not the same as the vertex associated to the track then continue to next jet
            if (jet->getAssociatedObject<xAOD::Vertex>("OriginVertex") != vtx_to_trk) continue;
            
            // do dR matching between jet and track
            double match_dr = coneSize(jet->pt());
            double dr = jet->p4().DeltaR(part->p4());
            if (dr > match_dr) continue;
            if (drmin < 0 || dr < drmin) {
                drmin = dr;
                matchjetidx = iJet;
            }
          }
          // Only need the z0 now
          if (matchjetidx >=0){
            SG::AuxElement::Accessor<std::vector<float>> acc_z0("btagIp_ByVertex_z0SinTheta");
            MatchInfo info;
            info.jetIdx = matchjetidx;
            info.z0SinTheta = acc_z0(*part).at(iVtx); 
            matchjet_vtx.push_back(std::move(info));          
          }
        }
        //sort vector by abs(z0)
        std::sort(matchjet_vtx.begin(), matchjet_vtx.end(),
        [](const MatchInfo& a, const MatchInfo& b) {
          return std::abs(a.z0SinTheta) < std::abs(b.z0SinTheta);
        });
        // Get the element links from the different Track containers
        if (matchjet_vtx.size() > 0){
          unsigned int count_trks=1; // counter 
          for (const auto& match: matchjet_vtx){              
            // for the first jet the track link comes from the original track container      
            if (count_trks ==1){
              ElementLink<xAOD::IParticleContainer> EL; 
              EL.toContainedElement(parts, part);
              (*matchedparts)[match.jetIdx].push_back(EL);
            // for the following jets associated with the same track, the track links come from the Duplicated Tracks container
            } else {
              const std::vector<ElementLink<xAOD::IParticleContainer>>& DupTrk_link_vec = DupTrk_link(*part);
              for (const auto& link: DupTrk_link_vec){
                if (!link.isValid()) continue;
                //get vertex
                const xAOD::TrackParticle* DupTrk = dynamic_cast<const xAOD::TrackParticle*>(*link);
                SG::AuxElement::Accessor<float> acc_z0("btagIp_ByVertex1_z0SinTheta");
                if (match.z0SinTheta != acc_z0(*DupTrk)) continue;
          
                ElementLink<xAOD::IParticleContainer> EL; 
                EL.toContainedElement(*ExistingDuplicatedTrks, DupTrk);
                (*matchedparts)[match.jetIdx].push_back(EL);

              }
            }
            count_trks++;
          }  
        }
      }
    }

    return matchedparts;   
}
