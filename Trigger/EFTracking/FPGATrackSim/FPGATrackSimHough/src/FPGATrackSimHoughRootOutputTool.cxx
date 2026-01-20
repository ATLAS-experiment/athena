/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimHough/FPGATrackSimHoughRootOutputTool.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimFunctions.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"


/////////////////////////////////////////////////////////////////////////////
FPGATrackSimHoughRootOutputTool::FPGATrackSimHoughRootOutputTool(const std::string& algname, const std::string& name, const IInterface *ifc) :
  AthAlgTool(algname, name, ifc)
{
}



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimHoughRootOutputTool::initialize()
{
  ATH_CHECK(m_FPGATrackSimMapping.retrieve());
  ATH_CHECK(m_tHistSvc.retrieve());
  ATH_CHECK(m_EvtSel.retrieve());

  if(m_algorithm == "Normal") m_algo=ORAlgo::Normal;
  else if(m_algorithm == "Invert") m_algo=ORAlgo::InvertGrouping;

  ATH_CHECK(bookTree());
  return StatusCode::SUCCESS;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimHoughRootOutputTool::bookTree()
{
  std::string tree_str = "FPGATrackSimHoughRootOutput_reg" + m_region.value();
  m_tree = new TTree(tree_str.c_str(), tree_str.c_str());

  m_tree->Branch("x",&m_x);
  m_tree->Branch("y",&m_y);
  m_tree->Branch("z",&m_z);
  m_tree->Branch("volumeID",&m_volumeID);
  m_tree->Branch("custom_layerID",&m_custom_layerID);

  m_tree->Branch("layerID",&m_layerID);
  m_tree->Branch("etaID",&m_etaID);

  m_tree->Branch("associated_truth_d0",&m_associated_truth_d0);
  m_tree->Branch("associated_truth_z0",&m_associated_truth_z0);
  m_tree->Branch("associated_truth_pt",&m_associated_truth_pt);
  m_tree->Branch("associated_truth_q",&m_associated_truth_q);
  m_tree->Branch("associated_truth_eta",&m_associated_truth_eta);
  m_tree->Branch("associated_truth_phi",&m_associated_truth_phi);

  m_tree->Branch("gphi",&m_gphi);
  m_tree->Branch("zIdeal",&m_zIdeal);
  m_tree->Branch("gphiIdeal",&m_gphiIdeal);
  m_tree->Branch("phi",&m_phi);
  m_tree->Branch("invpt",&m_invpt);
  m_tree->Branch("tracknumber",&m_tracknumber);
  m_tree->Branch("roadnumber",&m_roadnumber);
  m_tree->Branch("barcode",&m_barcode);
  m_tree->Branch("barcodefrac",&m_barcodefrac);
  m_tree->Branch("eventindex",&m_eventindex);
  m_tree->Branch("isRealHit",&m_realHit);
  m_tree->Branch("isPixel",&m_isPixel);
  m_tree->Branch("isSP",&m_isSP);
  m_tree->Branch("layer",&m_layer);
  m_tree->Branch("isBarrel",&m_isBarrel);
  m_tree->Branch("etawidth",&m_etawidth);
  m_tree->Branch("phiwidth",&m_phiwidth);
  m_tree->Branch("etamodule",&m_etamodule);
  m_tree->Branch("phimodule",&m_phimodule);
  m_tree->Branch("ID",&m_ID);
  m_tree->Branch("diskLayer",&m_diskLayer);

  m_tree->Branch("candidate_barcodefrac",&m_candidate_barcodefrac);
  m_tree->Branch("candidate_barcode",&m_candidate_barcode);
  m_tree->Branch("candidate_eventindex",&m_candidate_eventindex);
  m_tree->Branch("treeindex",&m_treeindex);
  m_tree->Branch("subregion",&m_subregion);
  m_tree->Branch("fakelabel",&m_fakelabel);
  m_tree->Branch("passesOR",&m_passesOR);
  m_tree->Branch("roadChi2",&m_roadChi2);
  m_tree->Branch("roadChi2ndof",&m_roadChi2ndof);
  m_tree->Branch("roadNCoords",&m_roadNCoords);
  m_tree->Branch("nMissingHits",&m_nMissingHits);
  m_tree->Branch("NTracksORMinusRoads",&m_NTracksORMinusRoads);

  m_treeindex = 0;

  std::string truthtree_str = "FPGATrackSimTruthTree_reg" + m_region.value();
  m_truthtree = new TTree(truthtree_str.c_str(), truthtree_str.c_str());

  m_truthtree->Branch("truth_d0",&m_truth_d0);
  m_truthtree->Branch("truth_z0",&m_truth_z0);
  m_truthtree->Branch("truth_pt",&m_truth_pt);
  m_truthtree->Branch("truth_eta",&m_truth_eta);
  m_truthtree->Branch("truth_phi",&m_truth_phi);
  m_truthtree->Branch("truth_q",&m_truth_q);
  m_truthtree->Branch("truth_pdg",&m_truth_pdg);
  m_truthtree->Branch("truth_barcode",&m_truth_barcode);
  m_truthtree->Branch("truth_eventindex",&m_truth_eventindex);
  m_truthtree->Branch("truth_track_hit_x",&m_track_hit_x);
  m_truthtree->Branch("truth_track_hit_y",&m_track_hit_y);
  m_truthtree->Branch("truth_track_hit_z",&m_track_hit_z);
  m_truthtree->Branch("truth_track_hit_R",&m_track_hit_R);
  m_truthtree->Branch("truth_track_hit_phi",&m_track_hit_phi);
  m_truthtree->Branch("truth_track_hit_layer_disk",&m_track_hit_layer_disk);
  m_truthtree->Branch("has_strip_nonspacepoint",&m_has_strip_nonspacepoint);
  m_truthtree->Branch("truth_track_hit_isPixel", &m_track_hit_isPixel);
  m_truthtree->Branch("truth_track_hit_isStrip", &m_track_hit_isStrip);
  m_truthtree->Branch("truth_track_hit_isClustered", &m_track_hit_isClustered);
  m_truthtree->Branch("truth_track_hit_isSpacepoint", &m_track_hit_isSpacepoint);
  m_truthtree->Branch("truth_track_hit_barcode", &m_track_hit_barcode);
  m_truthtree->Branch("truth_track_hit_barcodefrac", &m_track_hit_barcodefrac);
  m_truthtree->Branch("truth_track_hit_zIdeal", &m_track_hit_zIdeal);
  m_truthtree->Branch("truth_track_hit_gphiIdeal", &m_track_hit_gphiIdeal);
  m_truthtree->Branch("truth_track_hit_fineID", &m_track_hit_fineID);
  m_truthtree->Branch("truth_track_hit_volumeID", &m_track_hit_volumeID);
  m_truthtree->Branch("truth_track_hit_isMapped", &m_track_hit_isMapped);

  std::string offltree_str = "FPGATrackSimOfflineTree_reg" + m_region.value();
  m_offlinetree = new TTree(offltree_str.c_str(), offltree_str.c_str());

  m_offlinetree->Branch("offline_d0",&m_offline_d0);
  m_offlinetree->Branch("offline_z0",&m_offline_z0);
  m_offlinetree->Branch("offline_pt",&m_offline_pt);
  m_offlinetree->Branch("offline_eta",&m_offline_eta);
  m_offlinetree->Branch("offline_phi",&m_offline_phi);
  m_offlinetree->Branch("offline_q",&m_offline_q);
  m_offlinetree->Branch("offline_barcode",&m_offline_barcode);
  m_offlinetree->Branch("offline_barcodefrac",&m_offline_barcodefrac);
  m_offlinetree->Branch("offline_n_holes",&m_offline_n_holes);
  m_offlinetree->Branch("offline_n_inertmaterial",&m_offline_n_inertmaterial);
  m_offlinetree->Branch("offline_n_measurement",&m_offline_n_measurement);
  m_offlinetree->Branch("offline_n_brempoint",&m_offline_n_brempoint);
  m_offlinetree->Branch("offline_n_scatterer",&m_offline_n_scatterer);
  m_offlinetree->Branch("offline_n_perigee",&m_offline_n_perigee);
  m_offlinetree->Branch("offline_n_outlier",&m_offline_n_outlier);
  m_offlinetree->Branch("offline_n_other",&m_offline_n_other);

  ATH_CHECK(m_tHistSvc->regTree(Form("/TRIGFPGATrackSimHOUGHOUTPUT/%s",m_tree->GetName()), m_tree));
  ATH_CHECK(m_tHistSvc->regTree(Form("/TRIGFPGATrackSimHOUGHOUTPUT/%s",m_truthtree->GetName()), m_truthtree));
  ATH_CHECK(m_tHistSvc->regTree(Form("/TRIGFPGATrackSimHOUGHOUTPUT/%s",m_offlinetree->GetName()), m_offlinetree));

  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimHoughRootOutputTool::fillTree(const std::vector<FPGATrackSimTrack> &track_cands, const std::vector<FPGATrackSimTruthTrack> &truthTracks, const std::vector<FPGATrackSimOfflineTrack> &offlineTracks, const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits_2nd, const bool writeOutNonSPStripHits, const bool roadsAreSecondStage)
{
  ATH_MSG_DEBUG("Running HoughOutputTool!!");

  m_tracknumber = 0;
  ResetVectors();

  if (roadsAreSecondStage) m_SUBREGIONMAP = m_FPGATrackSimMapping->SubRegionMap_2nd();
  else m_SUBREGIONMAP = m_FPGATrackSimMapping->SubRegionMap();


  std::vector<float> tmp_hits_x;
  std::vector<float> tmp_hits_y;
  std::vector<float> tmp_hits_z;
  std::vector<float> tmp_hits_R;
  std::vector<float> tmp_hits_phi;
  std::vector<int> tmp_hits_layer_disk;
  std::vector<bool> tmp_hits_mapped;
  std::vector<bool> tmp_hits_isPixel;
  std::vector<bool> tmp_hits_isStrip;
  std::vector<bool> tmp_hits_isClustered;
  std::vector<bool> tmp_hits_isSpacepoint;
  std::vector<float> tmp_hits_barcodefrac;
  std::vector<int> tmp_hits_barcode;
  std::vector<float> tmp_hits_zIdeal;
  std::vector<float> tmp_hits_gphiIdeal;
  std::vector<long> tmp_hits_fineID;
  std::vector<int> tmp_hits_volumeID;

  std::vector<float> tmp_hits_x_sorted;
  std::vector<float> tmp_hits_y_sorted;
  std::vector<float> tmp_hits_z_sorted;
  std::vector<float> tmp_hits_R_sorted;
  std::vector<float> tmp_hits_phi_sorted;
  std::vector<int> tmp_hits_layer_disk_sorted;
  std::vector<bool> tmp_hits_isPixel_sorted;
  std::vector<bool> tmp_hits_isStrip_sorted;
  std::vector<bool> tmp_hits_isClustered_sorted;
  std::vector<bool> tmp_hits_isSpacepoint_sorted;
  std::vector<float> tmp_hits_barcodefrac_sorted;
  std::vector<int> tmp_hits_barcode_sorted;
  std::vector<float> tmp_hits_zIdeal_sorted;
  std::vector<float> tmp_hits_gphiIdeal_sorted;
  std::vector<long> tmp_hits_fineID_sorted;
  std::vector<int> tmp_hits_volumeID_sorted;

  std::vector<bool> tmp_hits_mapped_sorted;
  bool has_strip_nonspacepoint;


  // fill the truth tree, simply once per event! to know which entry here to loop at for a given road or hit
  // combination below, use treeindex from below to find the entry here
  for (const auto & track : truthTracks) {
    if (!m_EvtSel->passCuts(track)) continue;
    if (track.getStatus() != 1) continue;


    tmp_hits_x.clear();
    tmp_hits_y.clear();
    tmp_hits_z.clear();
    tmp_hits_R.clear();
    tmp_hits_phi.clear();
    tmp_hits_layer_disk.clear();
    tmp_hits_mapped.clear();
    tmp_hits_isPixel.clear();
    tmp_hits_isStrip.clear();
    tmp_hits_isClustered.clear();
    tmp_hits_isSpacepoint.clear();
    tmp_hits_barcodefrac.clear();
    tmp_hits_barcode.clear();
    tmp_hits_zIdeal.clear();
    tmp_hits_gphiIdeal.clear();
    tmp_hits_fineID.clear();

    tmp_hits_x_sorted.clear();
    tmp_hits_y_sorted.clear();
    tmp_hits_z_sorted.clear();
    tmp_hits_R_sorted.clear();
    tmp_hits_phi_sorted.clear();
    tmp_hits_layer_disk_sorted.clear();
    tmp_hits_mapped_sorted.clear();
    tmp_hits_isPixel_sorted.clear();
    tmp_hits_isStrip_sorted.clear();
    tmp_hits_isClustered_sorted.clear();
    tmp_hits_isSpacepoint_sorted.clear();
    tmp_hits_barcodefrac_sorted.clear();
    tmp_hits_barcode_sorted.clear();
    tmp_hits_zIdeal_sorted.clear();
    tmp_hits_gphiIdeal_sorted.clear();
    tmp_hits_fineID_sorted.clear();
    tmp_hits_volumeID.clear();

    has_strip_nonspacepoint = false;

    double target_r;
    std::vector<float> idealized_coords;
    FPGATrackSimMultiTruth::Barcode tbarcode;
    FPGATrackSimMultiTruth::Weight tfrac;

    // Collect truth tracks' 2nd stage hits
    for (auto hit : hits_2nd) {
      // Only write out hits belonging to the truth track
      if (hit->getBarcode() == track.getBarcode()) {

        if (hit->isPixel() == 0 && hit->getHitType() != HitType::spacepoint) {has_strip_nonspacepoint = true;}

        FPGATrackSimMultiTruth truth = hit->getTruth();
        truth.assign_equal_normalization();

        const bool ok = truth.best(tbarcode, tfrac);
        if (tfrac < 1.0) { continue; }
        if( ok ) {
          tmp_hits_barcode.push_back((int)(tbarcode.second));
          tmp_hits_barcodefrac.push_back(tfrac);
        }
        else {
          tmp_hits_barcode.push_back(-1);
          tmp_hits_barcodefrac.push_back(-1);
        }

        tmp_hits_x.push_back(hit->getX());
        tmp_hits_y.push_back(hit->getY());
        tmp_hits_z.push_back(hit->getZ());
        tmp_hits_R.push_back(hit->getR());
        tmp_hits_phi.push_back(hit->getGPhi());
        tmp_hits_layer_disk.push_back(hit->getLayerDisk(true));
        tmp_hits_volumeID.push_back(getVolumeID(*hit));
        tmp_hits_mapped.push_back(hit->isMapped());
        tmp_hits_isPixel.push_back(hit->isPixel());
        tmp_hits_isStrip.push_back(hit->isStrip());
        tmp_hits_isClustered.push_back(hit->isClustered());

        tmp_hits_fineID.push_back(getFineID(*hit));

        target_r = m_SUBREGIONMAP->getAvgRadius(0, hit->getLayerDisk());
        idealized_coords = computeIdealCoords(*hit, hit->getGPhi(), track.getQOverPt(), target_r, true, TrackCorrType::None);
        tmp_hits_zIdeal.push_back(idealized_coords[0]);
        tmp_hits_gphiIdeal.push_back(idealized_coords[1]);

        if (hit->getHitType() == HitType::spacepoint) {
          tmp_hits_isSpacepoint.push_back(true);
        }
        else {
          tmp_hits_isSpacepoint.push_back(false);
        }
      }
    }


    m_has_strip_nonspacepoint.push_back(has_strip_nonspacepoint);

    if ( (! writeOutNonSPStripHits & has_strip_nonspacepoint)) {
      ATH_MSG_DEBUG("Truth Track is not written to HoughRootOutput file, because it contains a non-SP strip hit!");
      continue;
    }

    std::vector<int> sorting_index(tmp_hits_R.size(), 0);
    for (unsigned int i = 0 ; i != sorting_index.size() ; i++) {
        sorting_index[i] = i;
    }
    sort(sorting_index.begin(), sorting_index.end(),
        [&](const int& a, const int& b) {
            return (tmp_hits_R[a] < tmp_hits_R[b]);
        }
    );

    for (unsigned int i = 0 ; i != sorting_index.size() ; i++) {
      tmp_hits_x_sorted.push_back(tmp_hits_x[sorting_index[i]]);
      tmp_hits_y_sorted.push_back(tmp_hits_y[sorting_index[i]]);
      tmp_hits_z_sorted.push_back(tmp_hits_z[sorting_index[i]]);
      tmp_hits_R_sorted.push_back(tmp_hits_R[sorting_index[i]]);
      tmp_hits_phi_sorted.push_back(tmp_hits_phi[sorting_index[i]]);
      tmp_hits_layer_disk_sorted.push_back(tmp_hits_layer_disk[sorting_index[i]]);
      tmp_hits_isPixel_sorted.push_back(tmp_hits_isPixel[sorting_index[i]]);
      tmp_hits_isStrip_sorted.push_back(tmp_hits_isStrip[sorting_index[i]]);
      tmp_hits_isClustered_sorted.push_back(tmp_hits_isClustered[sorting_index[i]]);
      tmp_hits_isSpacepoint_sorted.push_back(tmp_hits_isSpacepoint[sorting_index[i]]);
      tmp_hits_barcode_sorted.push_back(tmp_hits_barcode[sorting_index[i]]);
      tmp_hits_barcodefrac_sorted.push_back(tmp_hits_barcodefrac[sorting_index[i]]);
      tmp_hits_zIdeal_sorted.push_back(tmp_hits_zIdeal[sorting_index[i]]);
      tmp_hits_gphiIdeal_sorted.push_back(tmp_hits_gphiIdeal[sorting_index[i]]);
      tmp_hits_fineID_sorted.push_back(tmp_hits_fineID[sorting_index[i]]);
      tmp_hits_volumeID_sorted.push_back(tmp_hits_volumeID[sorting_index[i]]);
      tmp_hits_mapped_sorted.push_back(tmp_hits_mapped[sorting_index[i]]);

    };

    m_track_hit_x.push_back(tmp_hits_x_sorted);
    m_track_hit_y.push_back(tmp_hits_y_sorted);
    m_track_hit_z.push_back(tmp_hits_z_sorted);
    m_track_hit_R.push_back(tmp_hits_R_sorted);
    m_track_hit_phi.push_back(tmp_hits_phi_sorted);
    m_track_hit_layer_disk.push_back(tmp_hits_layer_disk_sorted);
    m_track_hit_isPixel.push_back(tmp_hits_isPixel_sorted);
    m_track_hit_isStrip.push_back(tmp_hits_isStrip);
    m_track_hit_isClustered.push_back(tmp_hits_isClustered_sorted);
    m_track_hit_isSpacepoint.push_back(tmp_hits_isSpacepoint_sorted);
    m_track_hit_barcode.push_back(tmp_hits_barcode_sorted);
    m_track_hit_barcodefrac.push_back(tmp_hits_barcodefrac_sorted);
    m_track_hit_zIdeal.push_back(tmp_hits_zIdeal_sorted);
    m_track_hit_gphiIdeal.push_back(tmp_hits_gphiIdeal_sorted);
    m_track_hit_fineID.push_back(tmp_hits_fineID_sorted);
    m_track_hit_volumeID.push_back(tmp_hits_volumeID_sorted);
    m_track_hit_isMapped.push_back(tmp_hits_mapped_sorted);

    m_truth_d0.push_back(track.getD0());
    m_truth_z0.push_back(track.getZ0());
    m_truth_pt.push_back(track.getPt());
    m_truth_eta.push_back(track.getEta());
    m_truth_phi.push_back(track.getPhi());
    m_truth_barcode.push_back(track.getBarcode());
    m_truth_eventindex.push_back(track.getEventIndex());
    m_truth_q.push_back(track.getQ());
    m_truth_pdg.push_back(track.getPDGCode());
  }
  // Fill only if at least one truth track to save to avoid empty events
  if (m_truth_eventindex.size() != 0) {
    m_truthtree->Fill();
  }
  ResetVectors();


  // now do the same for offline tree, once per event
  for (const auto & track : offlineTracks) {
    if (!m_EvtSel->passCuts(track)) continue;

    m_offline_d0.push_back(track.getD0());
    m_offline_z0.push_back(track.getZ0());
    m_offline_pt.push_back(abs(1./track.getQOverPt()));
    m_offline_eta.push_back(track.getEta());
    m_offline_phi.push_back(track.getPhi());
    m_offline_barcode.push_back(track.getBarcode());
    m_offline_barcodefrac.push_back(track.getBarcodeFrac());

    m_offline_q.push_back(track.getQOverPt() > 0 ? 1 : -1);
    int nhole(0), nmeasurement(0), ninert(0), nbrem(0), nscatter(0), nperigee(0), noutlier(0), nother(0);
    for (const auto& hit : track.getOfflineHits()) {
      if (hit.getHitType() == OfflineHitType::Measurement) nmeasurement++;
      else if (hit.getHitType() == OfflineHitType::InertMaterial) ninert++;
      else if (hit.getHitType() == OfflineHitType::BremPoint) nbrem++;
      else if (hit.getHitType() == OfflineHitType::Scatterer) nscatter++;
      else if (hit.getHitType() == OfflineHitType::Perigee) nperigee++;
      else if (hit.getHitType() == OfflineHitType::Outlier) noutlier++;
      else if (hit.getHitType() == OfflineHitType::Hole) nhole++;
      else nother++;
    }
    m_offline_n_holes.push_back(nhole);
    m_offline_n_measurement.push_back(nmeasurement);
    m_offline_n_inertmaterial.push_back(ninert);
    m_offline_n_brempoint.push_back(nbrem);
    m_offline_n_scatterer.push_back(nscatter);
    m_offline_n_perigee.push_back(nperigee);
    m_offline_n_outlier.push_back(noutlier);
    m_offline_n_other.push_back(nother);
  }
  m_offlinetree->Fill();
  ResetVectors();

  // for calculating the truth for the entire combination, not just an individual hit
  std::vector<FPGATrackSimMultiTruth> mtv;

  double target_r;

  for (size_t iroad = 0; iroad < track_cands.size(); iroad++) {
    m_roadnumber = iroad;
    FPGATrackSimTrack road = track_cands[iroad];

    if (roadsAreSecondStage) {
      mtv.reserve( m_FPGATrackSimMapping->PlaneMap_2nd(road.getSubRegion())->getNLogiLayers());
    }
    else {
      mtv.reserve( m_FPGATrackSimMapping->PlaneMap_1st(road.getSubRegion())->getNLogiLayers());
    }

    m_roadChi2ndof.push_back(track_cands[iroad].getChi2ndof());
    m_roadChi2.push_back(track_cands[iroad].getChi2());
    m_roadNCoords.push_back(track_cands[iroad].getNCoords());

    m_nMissingHits.push_back(track_cands[iroad].getNMissing());

    m_passesOR.push_back(road.passedOR());

    m_phi = road.getHoughX();
    m_invpt = road.getHoughY();
    m_subregion = road.getRegion();
    std::vector<FPGATrackSimHit> hits = road.getFPGATrackSimHits();


    // Add the hits from each combination to the tree
    for (FPGATrackSimHit hit : hits) {

      m_realHit.push_back(hit.isReal());
      if (hit.getHitType() == HitType::spacepoint) {
        m_isSP.push_back(true);
      }
      else {
        m_isSP.push_back(false);
      }
      FPGATrackSimMultiTruth truth = hit.getTruth();

      truth.assign_equal_normalization();
      mtv.push_back( truth );

      FPGATrackSimMultiTruth::Barcode tbarcode;
      FPGATrackSimMultiTruth::Weight tfrac;
      const bool ok = truth.best(tbarcode,tfrac);
      if( ok ) {
        m_eventindex.push_back((int)(tbarcode.first));
        m_barcode.push_back((int)(tbarcode.second));
        m_barcodefrac.push_back(tfrac);
      }
      else {
        m_eventindex.push_back(-1);
        m_barcode.push_back(-1);
        m_barcodefrac.push_back(-1);
      }

      target_r = m_SUBREGIONMAP->getAvgRadius(0, hit.getLayer());
      std::shared_ptr<const FPGATrackSimHit> hit_ptr = std::make_shared<const FPGATrackSimHit>(hit);
      std::vector<float> idealized_coords = computeIdealCoords(hit, m_invpt, hit.getGPhi(), target_r, true, TrackCorrType::None);

      m_x.push_back(hit.getX());
      m_y.push_back(hit.getY());
      m_z.push_back(hit.getZ());
      m_volumeID.push_back(getVolumeID(*hit_ptr));
      m_custom_layerID.push_back(getFineID(*hit_ptr));
      m_layerID.push_back(hit.getLayerDisk(true));
      m_etaID.push_back(hit.getEtaModule(true));

      m_gphi.push_back(hit.getGPhi());
      m_zIdeal.push_back(idealized_coords[0]);
      m_gphiIdeal.push_back(idealized_coords[1]);
      m_isPixel.push_back(hit.isPixel() ? 1 : 0);
      m_layer.push_back(hit.getLayer());
      m_isBarrel.push_back(hit.isBarrel() ? 1 : 0);
      m_etawidth.push_back(hit.getEtaWidth());
      m_phiwidth.push_back(hit.getPhiWidth());
      m_etamodule.push_back(hit.getEtaModule(true));
      m_phimodule.push_back(hit.getPhiModule());
      m_ID.push_back(hit.getIdentifierHash());
      m_diskLayer.push_back(hit.getLayerDisk(true));
    }

    // done looping over hits, now we do the truth calculation for this track candidate
    // first compute the best geant match, the barcode with the largest number of contributing hits
    // frac is then the fraction of the total number of hits on the candidate attributed to the barcode.
    FPGATrackSimMultiTruth mt( std::accumulate(mtv.begin(),mtv.end(),FPGATrackSimMultiTruth(),FPGATrackSimMultiTruth::AddAccumulator()) );
    // retrieve the best barcode and frac and store it
    FPGATrackSimMultiTruth::Barcode tbarcode;
    FPGATrackSimMultiTruth::Weight tfrac;
    const bool ok = mt.best(tbarcode,tfrac);
    if( ok ) {
      m_candidate_eventindex = (int)(tbarcode.first);
      m_candidate_barcode = (int)(tbarcode.second);
      m_candidate_barcodefrac = tfrac;
      if ( tfrac <= 0.5 ) m_fakelabel = 1;
      else if ( tfrac == 1.0 ) m_fakelabel = 0;
    }
    else {
      m_candidate_eventindex = -1;
      m_candidate_barcode = -1;
      m_candidate_barcodefrac = 0;
      m_fakelabel = 2;
    }

    for (const auto & track : truthTracks) {
      if (!m_EvtSel->passCuts(track)) continue;
      if (track.getStatus() != 1) continue;

      m_associated_truth_d0 = track.getD0();
      m_associated_truth_z0 = track.getZ0();
      m_associated_truth_pt = track.getPt();
      m_associated_truth_q = track.getQ();
      m_associated_truth_eta = track.getEta();
      m_associated_truth_phi = track.getPhi();
      break;
    }

    m_tree->Fill();

    mtv.clear();
    ResetVectors();
    m_tracknumber++;
  }
  m_treeindex++;
  return StatusCode::SUCCESS;
}



StatusCode FPGATrackSimHoughRootOutputTool::fillTree(const std::vector<FPGATrackSimRoad> &roads, const std::vector<FPGATrackSimTruthTrack> &truthTracks, const std::vector<FPGATrackSimOfflineTrack> &offlineTracks, const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits_2nd, const bool writeOutNonSPStripHits, const float minChi2, const int maxOverlappingHits, const bool roadsAreSecondStage)
{
  ATH_MSG_DEBUG("Running HoughOutputTool!!");


  // Create a list of track candidates by taking all possible combinations of hits in road.
  std::vector<FPGATrackSimTrack> track_cands;

  for (size_t iroad = 0; iroad < roads.size(); iroad++) {
    m_roadnumber = iroad;
    const FPGATrackSimRoad& road = roads[iroad];
    std::vector<FPGATrackSimTrack> track_cand;

    auto pmap = m_FPGATrackSimMapping->PlaneMap_1st(road.getSubRegion());

    if (roadsAreSecondStage) {
      pmap = m_FPGATrackSimMapping->PlaneMap_2nd(road.getSubRegion());
    }

    std::vector<std::vector<int>> combs = ::getComboIndices(road.getNHits_layer());
    m_phi = road.getX();
    m_invpt = road.getY();

    // Build track candidate for OR tool
    int nMissing;
    bool missPixel;
    bool missStrip;
    layer_bitmask_t missing_mask;
    layer_bitmask_t norecovery_mask; // mask to prevent majority in planes with multiple hits
    getMissingInfo(road, nMissing, missPixel, missStrip, missing_mask, norecovery_mask, m_FPGATrackSimMapping, m_idealCoordFitType);
    // Create a template track with common parameters filled already for initializing below
    FPGATrackSimTrack temp;

    if (roadsAreSecondStage) {
      temp.setTrackStage(TrackStage::SECOND);
    }
    else {
      temp.setTrackStage(TrackStage::FIRST);
    }
    temp.setSecondSectorID(road.getSector());
    temp.setNLayers(pmap->getNLogiLayers());
    temp.setBankID(-1); // TODO
    temp.setPatternID(road.getPID());
    temp.setHitMap(missing_mask);
    temp.setNMissing(nMissing);
    temp.setHoughX(road.getX());
    temp.setHoughY(road.getY());
    temp.setQOverPt(road.getY());
    temp.setTrackCorrType(m_IdealCoordFitType);
    temp.setDoDeltaGPhis(true);
    temp.setPassedOR(1);

    makeTrackCandidates(road, temp, track_cand, m_FPGATrackSimMapping);
    for (auto const &tr : track_cand) {
      track_cands.push_back(tr);
    }
  }

  ATH_CHECK(runOverlapRemoval(track_cands, minChi2, maxOverlappingHits, m_algo, m_monTool));
  unsigned long passed = 0;
  for (auto const &cand : track_cands) {
    if (cand.passedOR()) {
      passed++;
    }
  }

  m_NTracksORMinusRoads = passed - roads.size();

  return fillTree(track_cands, truthTracks, offlineTracks, hits_2nd, writeOutNonSPStripHits, roadsAreSecondStage);
}

void FPGATrackSimHoughRootOutputTool::ResetVectors() {
  m_x.clear();
  m_y.clear();
  m_z.clear();
  m_volumeID.clear();
  m_custom_layerID.clear();

  m_layerID.clear();
  m_etaID.clear();

  m_barcode.clear();
  m_barcodefrac.clear();
  m_eventindex.clear();
  m_realHit.clear();
  m_isSP.clear();
  m_isPixel.clear();
  m_layer.clear();
  m_isBarrel.clear();
  m_etawidth.clear();
  m_phiwidth.clear();
  m_etamodule.clear();
  m_phimodule.clear();
  m_ID.clear();
  m_diskLayer.clear();
  m_passesOR.clear();
  m_roadChi2.clear();
  m_roadChi2ndof.clear();
  m_roadNCoords.clear();
  m_nMissingHits.clear();
  m_truth_d0.clear();
  m_truth_z0.clear();
  m_truth_pt.clear();
  m_truth_eta.clear();
  m_truth_phi.clear();
  m_truth_q.clear();
  m_truth_barcode.clear();
  m_truth_eventindex.clear();
  m_truth_pdg.clear();
  m_track_hit_volumeID.clear();
  m_offline_n_holes.clear();
  m_offline_n_measurement.clear();
  m_offline_n_inertmaterial.clear();
  m_offline_n_brempoint.clear();
  m_offline_n_scatterer.clear();
  m_offline_n_perigee.clear();
  m_offline_n_outlier.clear();
  m_offline_n_other.clear();
  m_offline_d0.clear();
  m_offline_z0.clear();
  m_offline_pt.clear();
  m_offline_eta.clear();
  m_offline_phi.clear();
  m_offline_q.clear();
  m_offline_barcode.clear();
  m_offline_barcodefrac.clear();
  m_track_hit_x.clear();
  m_track_hit_y.clear();
  m_track_hit_z.clear();
  m_track_hit_R.clear();
  m_track_hit_phi.clear();
  m_track_hit_layer_disk.clear();
  m_track_hit_isPixel.clear();
  m_track_hit_isStrip.clear();
  m_track_hit_isClustered.clear();
  m_track_hit_isSpacepoint.clear();
  m_track_hit_barcode.clear();
  m_track_hit_barcodefrac.clear();
  m_track_hit_zIdeal.clear();
  m_track_hit_gphiIdeal.clear();
  m_track_hit_fineID.clear();
  m_track_hit_isMapped.clear();

  m_gphi.clear();
  m_zIdeal.clear();
  m_gphiIdeal.clear();
  m_has_strip_nonspacepoint.clear();
}
