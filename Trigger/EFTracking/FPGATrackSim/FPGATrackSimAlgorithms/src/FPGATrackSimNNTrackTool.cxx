/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file FPGATrackSimNNTrackTool.cxx
 * @author Elliott Cheu
 * @date April 28, 2021
 * @brief Does NN tracking
 *
 * Uses lwtnn to calculate an NN score for a set of hits from a track. This is
 * then stored in an FPGATrackSimTrack object
 */

#include "FPGATrackSimAlgorithms/FPGATrackSimNNTrackTool.h"
#include "FPGATrackSimMaps/FPGATrackSimNNMap.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimFunctions.h"
#include "FPGATrackSimObjects/FPGATrackSimMultiTruth.h"

/////////////////////////////////////////////////////////////////////////////
FPGATrackSimNNTrackTool::FPGATrackSimNNTrackTool(const std::string &algname, const std::string &name, const IInterface *ifc) : FPGATrackSimTrackingToolBase(algname, name, ifc), OnnxRuntimeBase() {}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimNNTrackTool::initialize() {
  ATH_CHECK(m_FPGATrackSimMapping.retrieve());
  ATH_CHECK(m_tHistSvc.retrieve());
  if (m_useSpacePoints) ATH_CHECK(m_spRoadFilterTool.retrieve(EnableTool{m_spRoadFilterTool}));

  if (m_FPGATrackSimMapping->getFakeNNMapString() != "") {
    m_fakeNN.initialize(m_FPGATrackSimMapping->getFakeNNMapString());
  }
  else {
    ATH_MSG_ERROR("Path to NN-based fake track removal ONNX file is empty! If you want to run this pipeline, you need to provide an input file.");
    return StatusCode::FAILURE;
  }

  if (m_FPGATrackSimMapping->getParamNNMapString() != "") {
    m_paramNN.initialize(m_FPGATrackSimMapping->getParamNNMapString());
  }
  else {
    ATH_MSG_INFO("Path to NN-based track parameter estimation ONNX file is empty! Estimation is not run...");
    m_useParamNN = false;
  }

  return StatusCode::SUCCESS;
}

void FPGATrackSimNNTrackTool::setTrackParameters(FPGATrackSimTrack& track, std::vector<float> inputTensorValues) {

  ATH_MSG_DEBUG("Running NN-based track parameter estimation!");
  std::vector<float> paramNNoutput = m_paramNN.runONNXInference(inputTensorValues);

  ATH_MSG_DEBUG("Estimated Track Parameters"); 
  for (unsigned int i = 0; i < paramNNoutput.size(); i++) {
    ATH_MSG_DEBUG(paramNNoutput[i]);
  }

  track.setQOverPt(paramNNoutput[0]);
  track.setEta(paramNNoutput[1]);
  track.setPhi(paramNNoutput[2]);
  track.setD0(paramNNoutput[3]);
  track.setZ0(paramNNoutput[4]);
}

StatusCode FPGATrackSimNNTrackTool::getTracks(std::vector<std::shared_ptr<const FPGATrackSimRoad>> &roads, std::vector<FPGATrackSimTrack> &tracks) {

  ATH_CHECK(setRoadSectors(roads));
  int n_track = 0;

  // Loop over roads
  for (auto const &iroad : roads) {

    double y = iroad->getY();

    // Error checking
    int sector = iroad->getSector();
    if (sector < 0) {
      ATH_MSG_DEBUG("Bad sector " << sector);
      return StatusCode::SUCCESS;
    }

    // Get info on layers with missing hits
    int nMissing = 0;
    layer_bitmask_t missing_mask = 0;

    // Just used to get number of layers considered
    const FPGATrackSimPlaneMap *planeMap = nullptr;
    if (!m_do2ndStage) planeMap = m_FPGATrackSimMapping->PlaneMap_1st(0);
    else planeMap = m_FPGATrackSimMapping->PlaneMap_2nd(0);

    // Create a template track with common parameters filled already for
    // initializing below
    FPGATrackSimTrack temp;
    temp.setTrackStage(TrackStage::FIRST);
    temp.setNLayers(planeMap->getNLogiLayers());
    temp.setBankID(-1);
    temp.setPatternID(iroad->getPID());
    temp.setFirstSectorID(iroad->getSector());
    temp.setHitMap(missing_mask);
    temp.setNMissing(nMissing);
    temp.setQOverPt(y);

    temp.setSubRegion(iroad->getSubRegion());
    temp.setHoughX(iroad->getX());
    temp.setHoughY(iroad->getY());
    temp.setHoughXBin(iroad->getXBin());
    temp.setHoughYBin(iroad->getYBin());

    ////////////////////////////////////////////////////////////////////////
    // Get a list of indices for all possible combinations given a certain
    // number of layers
    std::vector<std::vector<int>> combs =
        getComboIndices(iroad->getNHits_layer());

    // Loop over possible combinations for this road
    for (size_t icomb = 0; icomb < combs.size(); icomb++) {

      // list of indices for this particular combination
      std::vector<int> const &hit_indices = combs[icomb];
      std::vector<std::shared_ptr<const FPGATrackSimHit>> hit_list;

      // Loop over all layers
      for (unsigned layer = 0; layer < planeMap->getNLogiLayers(); layer++) {

        // Check to see if this is a valid hit
        if (hit_indices[layer] >= 0) {

          std::shared_ptr<const FPGATrackSimHit> hit = iroad->getHits(layer)[hit_indices[layer]];
          // Add this hit to the road
          if (hit->isReal()){
            hit_list.push_back(hit);
          }
        }
      }

      // TODO: for now ignore roads with non-real hits, because the network needs all hits as input
      if (hit_list.size() < 9) continue;

      // Sort the list by radial distance
      std::sort(hit_list.begin(), hit_list.end(),
                [](std::shared_ptr<const FPGATrackSimHit> &hit1, std::shared_ptr<const FPGATrackSimHit> &hit2) {
                  double rho1 = std::hypot(hit1->getX(), hit1->getY());
                  double rho2 = std::hypot(hit2->getX(), hit2->getY());
                  return rho1 < rho2;
                });

      std::vector<float> inputTensorValues;


      int index = 1;
      bool flipZ = false;
      double rotateAngle = 0;
      bool gotSecondSP = false;
      float tmp_xf;
      float tmp_yf;
      float tmp_zf;

      // Loop over all hits
      for (const auto &hit : hit_list) {

        // Need to rotate hits
        float x0 = hit->getX();
        float y0 = hit->getY();
        float z0 = hit->getZ();
        if (index == 1) {
          if (z0 < 0)
            flipZ = true;
          rotateAngle = std::atan(x0 / y0);
          if (y0 < 0)
            rotateAngle += M_PI;
        }

        float xf = x0 * std::cos(rotateAngle) - y0 * std::sin(rotateAngle);
        float yf = x0 * std::sin(rotateAngle) + y0 * std::cos(rotateAngle);
        float zf = z0;

        if (flipZ) zf = z0 * -1;

        // Get average of values for strip hit pairs
        // TODO: this needs to be fixed in the future, for this to work for other cases
        if (hit->isStrip()) {
          if (!gotSecondSP) {
            tmp_xf = xf;
            tmp_yf = yf;
            tmp_zf = zf;
            gotSecondSP = true;
          }
          else {

            float xf_scaled = (xf + tmp_xf) / (2.*getXScale());
            float yf_scaled = (yf + tmp_yf) / (2.*getYScale());
            float zf_scaled = (zf + tmp_zf) / (2.*getZScale());

            // Get average of two hits for strip hits 
            inputTensorValues.push_back(xf_scaled);
            inputTensorValues.push_back(yf_scaled);
            inputTensorValues.push_back(zf_scaled);
            index++;
            gotSecondSP = false;
          }
        }
        else {
          float xf_scaled = (xf) / (getXScale());
          float yf_scaled = (yf) / (getYScale());
          float zf_scaled = (zf) / (getZScale());
          inputTensorValues.push_back(xf_scaled);
          inputTensorValues.push_back(yf_scaled);
          inputTensorValues.push_back(zf_scaled);
          index++;
        }
      }
      
      std::vector<float> NNoutput = m_fakeNN.runONNXInference(inputTensorValues);

      ATH_MSG_DEBUG("NN InputTensorValues:");
      ATH_MSG_DEBUG(inputTensorValues);
      ATH_MSG_DEBUG("NN output:" << NNoutput);
      
      float nn_val = NNoutput[0];

      // Save all hits on this road if it passes our "good" criteria
      if (nn_val > m_NNCut) {

        // This is a track. Fill class and add to track list
        n_track++;
        FPGATrackSimTrack track_cand;
        track_cand.setTrackID(n_track);
        track_cand.setNLayers(planeMap->getNLogiLayers());
        for (const auto &ihit : hit_list) {
          unsigned int layer = ihit->getLayer();
          track_cand.setFPGATrackSimHit(layer, *ihit);
        }
        // Nominal chi2ndof cut is 40 and we want to use NN>0.0075 (or
        // NN<(1-0.0075) Nominal chi2ndof cut is 40 and we want to use NN>0.001
        // (or NN<(1-0.1)
        // the 5 comes from the 5 dof we nominally get from a chi2
        double scale = m_chi2_scalefactor *
                       (track_cand.getNCoords() - track_cand.getNMissing() - 5);
        double chi2 = scale * (1 - nn_val);
        track_cand.setOrigChi2(chi2);
        track_cand.setChi2(chi2);
        tracks.push_back(track_cand);
      }


      if (m_useParamNN) {
        for (auto& track : tracks) {
          setTrackParameters(track, inputTensorValues);
        }
      }
    }  // loop over combinations

  }  // loop over roads

  // Add truth info
  for (FPGATrackSimTrack &t : tracks) {
    compute_truth(t);  // match the track to a geant particle using the
                       // channel-level geant info in the hit data.
  }

  return StatusCode::SUCCESS;
}

// Borrowed same code from TrackFitter - probably a nicer way to inherit instead
void FPGATrackSimNNTrackTool::compute_truth(FPGATrackSimTrack &t) const {
  std::vector<FPGATrackSimMultiTruth> mtv;

  const FPGATrackSimPlaneMap* planeMap = nullptr;
  if (!m_do2ndStage) planeMap = m_FPGATrackSimMapping->PlaneMap_1st(0);
  else planeMap = m_FPGATrackSimMapping->PlaneMap_2nd(0);

  for (unsigned layer = 0; layer < planeMap->getNLogiLayers(); layer++) {
    if (t.getHitMap() & (1 << planeMap->getCoordOffset(layer)))
      continue;  // no hit in this plane
    // Sanity check that we have enough hits.
    if (layer < t.getFPGATrackSimHits().size())
      mtv.push_back(t.getFPGATrackSimHits().at(layer).getTruth());

    // adjust weight for hits without (and also with) a truth match, so that
    // each is counted with the same weight.
    mtv.back().assign_equal_normalization();
  }

  FPGATrackSimMultiTruth mt(std::accumulate(mtv.begin(), mtv.end(), FPGATrackSimMultiTruth(),
                                   FPGATrackSimMultiTruth::AddAccumulator()));
  // frac is then the fraction of the total number of hits on the track
  // attributed to the barcode.

  FPGATrackSimMultiTruth::Barcode tbarcode;
  FPGATrackSimMultiTruth::Weight tfrac;
  const bool ok = mt.best(tbarcode, tfrac);
  if (ok) {
    t.setEventIndex(tbarcode.first);
    t.setBarcode(tbarcode.second);
    t.setBarcodeFrac(tfrac);
  }
}
