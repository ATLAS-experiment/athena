/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughDataNtuple.h"
#include "MuonTesterTree/EventInfoBranch.h"
#include "TruthUtils/MagicNumbers.h"
#include <unordered_set>

struct HitTruthMatching{
    HitTruthMatching(const xAOD::TruthParticle* truthPart_):
      truthPart{truthPart_}{
        for (const std::string hitColl : {"truthMdtHits", "truthRpcHits", "truthTgcHits", "truthCscHits"}) {
            const SG::AuxElement::ConstAccessor<std::vector<long long unsigned>> acc{hitColl};
            if (!acc.isAvailable(*truthPart)) continue;
            for (const long long unsigned id : acc(*truthPart)){
                 assocHits.emplace(id);
            }
        }
    }
   const xAOD::TruthParticle* truthPart{nullptr};
   std::unordered_set<Identifier> assocHits{};
};

MuonHoughDataNtuple::MuonHoughDataNtuple(const std::string& name, ISvcLocator* pSvcLocator) :
  AthHistogramAlgorithm(name, pSvcLocator) {}

MuonHoughDataNtuple::~MuonHoughDataNtuple() {}

StatusCode MuonHoughDataNtuple::initialize()
{
  ATH_CHECK(m_houghDataPerSectorVecKey.initialize());        
  ATH_CHECK(m_truthMuonKey.initialize());
  ATH_CHECK(m_truthSegmentsKey.initialize());
  m_truth_tree.addBranch(std::make_unique<MuonVal::EventInfoBranch>(m_truth_tree, 0));
  m_eta_hit_tree.addBranch(std::make_unique<MuonVal::EventInfoBranch>(m_eta_hit_tree, 0));
  m_phi_hit_tree.addBranch(std::make_unique<MuonVal::EventInfoBranch>(m_phi_hit_tree, 0));
   
  
  ATH_CHECK(m_truth_tree.init(this));

  ATH_CHECK(m_eta_hit_tree.init(this));
  ATH_CHECK(m_phi_hit_tree.init(this));
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_MSG_DEBUG("MuonHoughDataNtuple succesfully initialised");

  return StatusCode::SUCCESS;
}

StatusCode MuonHoughDataNtuple::finalize()
{
  ATH_CHECK(m_truth_tree.write());
  ATH_CHECK(m_eta_hit_tree.write());
  ATH_CHECK(m_phi_hit_tree.write());
  return StatusCode::SUCCESS;
}

StatusCode MuonHoughDataNtuple::execute() 
{
  // retrieve containers
  const EventContext & context = Gaudi::Hive::currentContext();
  SG::ReadHandle<Muon::HoughDataPerSectorVec> houghSecVec{m_houghDataPerSectorVecKey, context};

  SG::ReadHandle<xAOD::MuonSegmentContainer> truthSegContainer(m_truthSegmentsKey, context);

  if(!houghSecVec.isValid()){
    ATH_MSG_FATAL("Failed to retrieve Hough data per sector vector");
    return StatusCode::FAILURE;
  }
  
  std::vector<HitTruthMatching> truthMuons{};

  SG::ReadHandle<xAOD::TruthParticleContainer> truthMuonContainer(m_truthMuonKey, context);

  if(!truthMuonContainer.isValid()){
    ATH_MSG_FATAL("Failed to retrieve truth muon container");
    return StatusCode::FAILURE;
  }
  for (const xAOD::TruthParticle* truth_mu : *truthMuonContainer) {
     truthMuons.emplace_back(truth_mu);
  }

  if(!truthSegContainer.isValid()){
    ATH_MSG_FATAL("Failed to retrieve truth segment container");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Houghidipuff retrieved all input collections! Start puffing...");

  // filling variables for eta/phi hit with maximum entries for hough 
  for(unsigned int sec_i=0; sec_i<houghSecVec->vec.size(); ++sec_i){ // looping sector
    // Eta Hit 
    // starting from all layers
    for (const std::vector<std::shared_ptr<MuonHough::MuonLayerHough::Maximum>>& maxvec : 
        houghSecVec->vec[sec_i].maxVec){
      // then to all Maxima in the same layer
      for(const std::shared_ptr<MuonHough::MuonLayerHough::Maximum>& max : maxvec){ // looping maximum collection
        m_maxHit_sector = sec_i;
        m_maxHit_z0 = max->pos;
        m_maxHit_theta = max->theta;
        m_maxHit_region = max->refregion;
        bool firstHit = true; // to store station information
        // now record all hits in the maximum
        int hit_index = 0;
        for (const std::shared_ptr<MuonHough::Hit>& hit : max->hits){
          m_hit_x.push_back(hit->x);
          m_hit_ymin.push_back(hit->ymin);
          m_hit_ymax.push_back(hit->ymax);
          m_hit_w.push_back(hit->w);
          // hit debug info
          const MuonHough::HitDebugInfo* hdi = hit->debugInfo();
          m_hit_tech.push_back(hdi->type);
          // more information from prd
          if(hit->prd){
            const Trk::PrepRawData* prd = hit->prd;
            const Identifier measId = prd->identify();
            // station info
            if(firstHit){
              m_maxHit_stationIndex = m_idHelperSvc->stationIndex(measId);
              m_maxHit_stationEta = m_idHelperSvc->stationEta(measId);
              m_maxHit_stationPhi = m_idHelperSvc->stationPhi(measId);
              firstHit = false;
            }
            // local position 
            m_hit_local.push_back(prd->localPosition());
            // global position -- which is the real one??? 
            const Muon::MdtPrepData* DC{dynamic_cast<const Muon::MdtPrepData*>(prd)};
            if (DC) m_hit_global.push_back(DC->globalPosition());
            const Muon::MuonCluster* CL{dynamic_cast<const Muon::MuonCluster*>(prd)};
            if (CL) m_hit_global.push_back(CL->globalPosition());
            // information from identifiers
            if (m_idHelperSvc->isMdt(measId)) {
                m_hit_mdtId.push_back(measId);
                m_hit_mdtIndex.push_back(hit_index);
            } 
            else if (m_idHelperSvc->isCsc(measId)) {
                m_hit_cscId.push_back(measId);
                m_hit_cscIndex.push_back(hit_index);
            }
            else if (m_idHelperSvc->isTgc(measId)) {
                m_hit_tgcId.push_back(measId);
                m_hit_tgcIndex.push_back(hit_index);
            }
            else if (m_idHelperSvc->isRpc(measId)) {
                m_hit_rpcId.push_back(measId);
                m_hit_rpcIndex.push_back(hit_index);
            }
            else if (m_idHelperSvc->isMM(measId)) {
                m_hit_mmId.push_back(measId);
                m_hit_mmIndex.push_back(hit_index);
            }
            else if (m_idHelperSvc->issTgc(measId)) {
                m_hit_stgcId.push_back(measId);
                m_hit_stgcIndex.push_back(hit_index);
            }
            hit_index++;
            // truth matching to muons
            bool matched = std::find_if(truthMuons.begin(), truthMuons.end(), 
                                         [prd](const HitTruthMatching& hit){
                                            return hit.assocHits.count(prd->identify());
                                        }) != truthMuons.end();
            
            m_hit_truthMatched.push_back(matched);
          }
        }
        ATH_CHECK(m_eta_hit_tree.fill(context));
      }
    }
    // PhiHit vec
    // starting from all layers
    for (const std::vector<std::shared_ptr<MuonHough::MuonPhiLayerHough::Maximum>>& phimaxvec : 
        houghSecVec->vec[sec_i].phiMaxVec){
      // then to all Maxima in the same layer
      for(const std::shared_ptr<MuonHough::MuonPhiLayerHough::Maximum>& max : phimaxvec){ // looping maximum collection
        m_maxPhiHit_sector = sec_i;
        m_maxPhiHit_z0 = max->pos;
        bool firstHit = true; // to store station information
        // now record all hits in the maximum
        int phiHit_index = 0;
        for (const std::shared_ptr<MuonHough::PhiHit>& phi_hit : max->hits){
          m_phiHit_x.push_back(phi_hit->r);
          m_phiHit_ymin.push_back(phi_hit->phimin);
          m_phiHit_ymax.push_back(phi_hit->phimax);
          m_phiHit_w.push_back(phi_hit->w);
          // phi hit debug info
          const MuonHough::HitDebugInfo* hdi = phi_hit->debugInfo();
          m_phiHit_tech.push_back(hdi->type);
          // information from prd
          if(phi_hit->prd){
            const Trk::PrepRawData* prd = phi_hit->prd;
            const Identifier measId = prd->identify();
            // station info
            if(firstHit){
              m_maxPhiHit_stationIndex = m_idHelperSvc->stationIndex(prd->identify());
              m_maxPhiHit_stationEta = m_idHelperSvc->stationEta(prd->identify());
              m_maxPhiHit_stationPhi = m_idHelperSvc->stationPhi(prd->identify());
              firstHit = false;
            }
            // local position 
            m_phiHit_local.push_back(prd->localPosition());
            // global position -- which is the real one??? 
            const Muon::MdtPrepData* DC{dynamic_cast<const Muon::MdtPrepData*>(prd)};
            if (DC) m_phiHit_global.push_back(DC->globalPosition());
            const Muon::MuonCluster* CL{dynamic_cast<const Muon::MuonCluster*>(prd)};
            if (CL) m_phiHit_global.push_back(CL->globalPosition());
            // information from identifiers
            if (m_idHelperSvc->isMdt(measId)) {
                m_phiHit_mdtId.push_back(measId);
                m_phiHit_mdtIndex.push_back(phiHit_index);
            } 
            else if (m_idHelperSvc->isCsc(measId)) {
                m_phiHit_cscId.push_back(measId);
                m_phiHit_cscIndex.push_back(phiHit_index);
            }
            else if (m_idHelperSvc->isTgc(measId)) {
                m_phiHit_tgcId.push_back(measId);
                m_phiHit_tgcIndex.push_back(phiHit_index);
            }
            else if (m_idHelperSvc->isRpc(measId)) {
                m_phiHit_rpcId.push_back(measId);
                m_phiHit_rpcIndex.push_back(phiHit_index);
            }
            else if (m_idHelperSvc->isMM(measId)) {
                m_phiHit_mmId.push_back(measId);
                m_phiHit_mmIndex.push_back(phiHit_index);
            }
            else if (m_idHelperSvc->issTgc(measId)) {
                m_phiHit_stgcId.push_back(measId);
                m_phiHit_stgcIndex.push_back(phiHit_index);
            }
            phiHit_index++;

            // truth matching to muons
            bool matched = std::find_if(truthMuons.begin(), truthMuons.end(), 
                                         [prd](const HitTruthMatching& hit){
                                            return hit.assocHits.count(prd->identify());
                                        }) != truthMuons.end();
            
            m_phiHit_truthMatched.push_back(matched);
          }
        }
        ATH_CHECK(m_phi_hit_tree.fill(context));
      }
    }
  }
  
  // filling truth values 
  for(const xAOD::TruthParticle* truthMu: *truthMuonContainer){
    m_truth_pdgId = truthMu->pdgId();
    m_truth_barcode = HepMC::barcode(truthMu); // FIXME barcode-based

    m_truth_pt = truthMu->pt();
    m_truth_eta = truthMu->eta();
    m_truth_phi = truthMu->phi();
    // access the truth segment from truth particle
    static const SG::AuxElement::ConstAccessor<ElementLink<xAOD::TruthParticleContainer>> truthMuLink("truthParticleLink");
    // filling truth segment values
    for(const xAOD::MuonSegment* truthSeg: *truthSegContainer){
      if(!truthMuLink.isAvailable(*truthSeg)) { continue; } // if segment isn't linked to truth muon, skip
      ElementLink<xAOD::TruthParticleContainer> truthLink = truthMuLink(*truthSeg);
      const xAOD::TruthParticle* truthParticle = *truthLink;
      if(truthParticle!=truthMu) { continue; } // i have no idea if this works
      m_truth_seg_pos.push_back(truthSeg->x(), truthSeg->y(), truthSeg->z());
      m_truth_seg_p.push_back(truthSeg->px(), truthSeg->py(), truthSeg->pz());

      m_truth_seg_nPrecisionHits.push_back(truthSeg->nPrecisionHits());
      m_truth_seg_sector.push_back(truthSeg->sector());

      int nTriggerHits = truthSeg->nPhiLayers() + truthSeg->nTrigEtaLayers();
      m_truth_seg_nTriggerHits.push_back(nTriggerHits);
    }
    ATH_CHECK(m_truth_tree.fill(context));
  }


  return StatusCode::SUCCESS;
}
