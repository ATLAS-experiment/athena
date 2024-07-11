/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODMuonRDO/versions/NSWTPRDOAuxContainer_v1.h"

namespace {
   static const std::string preFixStr {"NSWTP_"};
}
#define TPAUX_VARIABLE(VAR) \
   do { \
      static const std::string varName =preFixStr+#VAR; \
      static const auxid_t auxid = getAuxID(varName, VAR); \
      regAuxVar(auxid, varName, VAR); \
    } while (false);
namespace xAOD {
    NSWTPRDOAuxContainer_v1::NSWTPRDOAuxContainer_v1()
    : AuxContainerBase() { 
      TPAUX_VARIABLE(ROD_L1ID);
      TPAUX_VARIABLE(sectID);
      TPAUX_VARIABLE(EC);
      TPAUX_VARIABLE(BCID);
      TPAUX_VARIABLE(L1ID);
      TPAUX_VARIABLE(window_open_bcid);
      TPAUX_VARIABLE(l1a_request_bcid);
      TPAUX_VARIABLE(window_close_bcid);
      TPAUX_VARIABLE(config_window_open_bcid_offset);
      TPAUX_VARIABLE(config_l1a_request_bcid_offset);
      TPAUX_VARIABLE(config_window_close_bcid_offset);
   

      TPAUX_VARIABLE(pad_coincidence_wedge);
      TPAUX_VARIABLE(pad_candidateNumber);
      TPAUX_VARIABLE(pad_phiID);
      TPAUX_VARIABLE(pad_bandID);
      TPAUX_VARIABLE(pad_BCID);
      TPAUX_VARIABLE(pad_idleFlag);
    
      TPAUX_VARIABLE(merge_LUT_choiceSelection);
      TPAUX_VARIABLE(merge_nsw_segmentSelector);
      TPAUX_VARIABLE(merge_valid_segmentSelector);
      TPAUX_VARIABLE(merge_candidateNumber);
      TPAUX_VARIABLE(merge_segments)
      TPAUX_VARIABLE(merge_BCID_sectorID);
    // Register variables with auxid
    auxid_t auxid_NSWTP_mm_segments = getAuxID("NSWTP_mm_segments", NSWTP_mm_segments);
    regAuxVar(auxid_NSWTP_mm_segments, "NSWTP_mm_segments", NSWTP_mm_segments);

    auxid_t auxid_NSWTP_mm_BCID = getAuxID("NSWTP_mm_BCID", NSWTP_mm_BCID);
    regAuxVar(auxid_NSWTP_mm_BCID, "NSWTP_mm_BCID", NSWTP_mm_BCID);

    auxid_t auxid_NSWTP_strip_bands_bandID = getAuxID("NSWTP_strip_bands_bandID", NSWTP_strip_bands_bandID);
    regAuxVar(auxid_NSWTP_strip_bands_bandID, "NSWTP_strip_bands_bandID", NSWTP_strip_bands_bandID);

    auxid_t auxid_NSWTP_strip_bands_phiID = getAuxID("NSWTP_strip_bands_phiID", NSWTP_strip_bands_phiID);
    regAuxVar(auxid_NSWTP_strip_bands_phiID, "NSWTP_strip_bands_phiID", NSWTP_strip_bands_phiID);

    auxid_t auxid_NSWTP_strip_bands_BCID = getAuxID("NSWTP_strip_bands_BCID", NSWTP_strip_bands_BCID);
    regAuxVar(auxid_NSWTP_strip_bands_BCID, "NSWTP_strip_bands_BCID", NSWTP_strip_bands_BCID);

    auxid_t auxid_NSWTP_strip_bands_HLbit = getAuxID("NSWTP_strip_bands_HLbit", NSWTP_strip_bands_HLbit);
    regAuxVar(auxid_NSWTP_strip_bands_HLbit, "NSWTP_strip_bands_HLbit", NSWTP_strip_bands_HLbit);

    auxid_t auxid_NSWTP_strip_bands_layer = getAuxID("NSWTP_strip_bands_layer", NSWTP_strip_bands_layer);
    regAuxVar(auxid_NSWTP_strip_bands_layer, "NSWTP_strip_bands_layer", NSWTP_strip_bands_layer);

    auxid_t auxid_NSWTP_strip_bands_charge = getAuxID("NSWTP_strip_bands_charge", NSWTP_strip_bands_charge);
    regAuxVar(auxid_NSWTP_strip_bands_charge, "NSWTP_strip_bands_charge", NSWTP_strip_bands_charge);

    auxid_t auxid_NSWTP_strip_BBbit = getAuxID("NSWTP_strip_BBbit", NSWTP_strip_BBbit);
    regAuxVar(auxid_NSWTP_strip_BBbit, "NSWTP_strip_BBbit", NSWTP_strip_BBbit);

    auxid_t auxid_NSWTP_strip_centroids_bandID = getAuxID("NSWTP_strip_centroids_bandID", NSWTP_strip_centroids_bandID);
    regAuxVar(auxid_NSWTP_strip_centroids_bandID, "NSWTP_strip_centroids_bandID", NSWTP_strip_centroids_bandID);

    auxid_t auxid_NSWTP_strip_centroids_phiID = getAuxID("NSWTP_strip_centroids_phiID", NSWTP_strip_centroids_phiID);
    regAuxVar(auxid_NSWTP_strip_centroids_phiID, "NSWTP_strip_centroids_phiID", NSWTP_strip_centroids_phiID);

    auxid_t auxid_NSWTP_strip_centroids_layer = getAuxID("NSWTP_strip_centroids_layer", NSWTP_strip_centroids_layer);
    regAuxVar(auxid_NSWTP_strip_centroids_layer, "NSWTP_strip_centroids_layer", NSWTP_strip_centroids_layer);

    auxid_t auxid_NSWTP_strip_centroids_offset = getAuxID("NSWTP_strip_centroids_offset", NSWTP_strip_centroids_offset);
    regAuxVar(auxid_NSWTP_strip_centroids_offset, "NSWTP_strip_centroids_offset", NSWTP_strip_centroids_offset);

    auxid_t auxid_NSWTP_strip_centroids_loc = getAuxID("NSWTP_strip_centroids_loc", NSWTP_strip_centroids_loc);
    regAuxVar(auxid_NSWTP_strip_centroids_loc, "NSWTP_strip_centroids_loc", NSWTP_strip_centroids_loc);

    auxid_t auxid_NSWTP_strip_segments_bandID = getAuxID("NSWTP_strip_segments_bandID", NSWTP_strip_segments_bandID);
    regAuxVar(auxid_NSWTP_strip_segments_bandID, "NSWTP_strip_segments_bandID", NSWTP_strip_segments_bandID);

    auxid_t auxid_NSWTP_strip_segments_phiID = getAuxID("NSWTP_strip_segments_phiID", NSWTP_strip_segments_phiID);
    regAuxVar(auxid_NSWTP_strip_segments_phiID, "NSWTP_strip_segments_phiID", NSWTP_strip_segments_phiID);

    auxid_t auxid_NSWTP_strip_segments_rA = getAuxID("NSWTP_strip_segments_rA", NSWTP_strip_segments_rA);
    regAuxVar(auxid_NSWTP_strip_segments_rA, "NSWTP_strip_segments_rA", NSWTP_strip_segments_rA);

    auxid_t auxid_NSWTP_strip_segments_rB = getAuxID("NSWTP_strip_segments_rB", NSWTP_strip_segments_rB);
    regAuxVar(auxid_NSWTP_strip_segments_rB, "NSWTP_strip_segments_rB", NSWTP_strip_segments_rB);

    auxid_t auxid_NSWTP_strip_segment = getAuxID("NSWTP_strip_segment", NSWTP_strip_segment);
    regAuxVar(auxid_NSWTP_strip_segment, "NSWTP_strip_segment", NSWTP_strip_segment);

    auxid_t auxid_NSWTP_strip_segments_BCID = getAuxID("NSWTP_strip_segments_BCID", NSWTP_strip_segments_BCID);
    regAuxVar(auxid_NSWTP_strip_segments_BCID, "NSWTP_strip_segments_BCID", NSWTP_strip_segments_BCID);

    }
}
#undef TPAUX_VARIABLE
