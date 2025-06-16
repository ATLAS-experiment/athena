/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "EFTrackingFPGAUtility/MasqueradeCoordinates.h"
#include "EFTrackingFPGAUtility/FPGADataFormatUtilities.h"

MasqueradeCoordinates::MasqueradeCoordinates(
  const std::string& name,
  ISvcLocator* pSvcLocator
) : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode MasqueradeCoordinates::initialize() {
  ATH_CHECK(m_cylindricalDataStreamKey.initialize());
  ATH_CHECK(m_cartesianDataStreamKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode MasqueradeCoordinates::execute(const EventContext& ctx) const {
  SG::ReadHandle<std::vector<unsigned long>> cylindricalDataStream(
    m_cylindricalDataStreamKey,
    ctx
  );

  SG::WriteHandle<std::vector<unsigned long>> cartesianDataStream(
    m_cartesianDataStreamKey,
    ctx
  );

  ATH_CHECK(cartesianDataStream.record(std::make_unique<std::vector<unsigned long>>()));
  cartesianDataStream->reserve(cylindricalDataStream->size());

  enum State {
    LOOKING_FOR_HDR,
    READING_EVT_HDR_W2,
    READING_EVT_HDR_W3,
    READING_RD_HDR_W2,
    READING_GTRACK_HDR_W2,
    READING_GTRACK_HDR_W3,
    READING_EVT_FTR_W2,
    READING_EVT_FTR_W3,
    READING_GHITZ_W1,
    READING_GHITZ_W2,
    READING_LAST_GHITZ_W2,
    DONE,
  } state = State::LOOKING_FOR_HDR;

  for (const unsigned long& word : *cylindricalDataStream) {
    const unsigned long flag = word >> 56;

    if (state == State::LOOKING_FOR_HDR) {
      cartesianDataStream->push_back(word);

      if (flag == FPGADataFormatUtilities::EVT_HDR_FLAG) {
        state = State::READING_EVT_HDR_W2;
      }
      else if (flag == FPGADataFormatUtilities::RD_HDR_FLAG) {
        state = State::READING_RD_HDR_W2;
      }
      else if (flag == FPGADataFormatUtilities::GTRACK_HDR_FLAG) {
        state = State::READING_GTRACK_HDR_W2;
      }
      else if (flag == FPGADataFormatUtilities::EVT_FTR_FLAG) {
        state = State::READING_EVT_FTR_W2;
      }
    }
    else if (state == State::READING_EVT_HDR_W2) {
      cartesianDataStream->push_back(word);
      state = State::READING_EVT_HDR_W3;
    }
    else if (state == State::READING_EVT_HDR_W3) {
      cartesianDataStream->push_back(word);
      state = State::READING_GHITZ_W1;
    }
    else if (state == State::READING_RD_HDR_W2) {
      cartesianDataStream->push_back(word);
      state = State::READING_GHITZ_W1;
    }
    else if (state == State::READING_GTRACK_HDR_W2) {
      cartesianDataStream->push_back(word);
      state = State::READING_GTRACK_HDR_W3;
    }
    else if (state == State::READING_GTRACK_HDR_W3) {
      cartesianDataStream->push_back(word);
      state = State::READING_GHITZ_W1;
    }
    else if (state == State::READING_EVT_FTR_W2) {
      cartesianDataStream->push_back(word);
      state = State::READING_EVT_FTR_W3;
    }
    else if (state == State::READING_EVT_FTR_W3) {
      cartesianDataStream->push_back(word);
      state = State::DONE;
    }
    else if (state == State::READING_GHITZ_W1) {
      const FPGADataFormatUtilities::GHITZ_w1 hit = FPGADataFormatUtilities::get_bitfields_GHITZ_w1(word);
    
      const float radius = hit.rad / FPGADataFormatUtilities::GHITZ_W1_RAD_mf;
      const float phi = hit.phi / FPGADataFormatUtilities::GHITZ_W1_PHI_mf;

      // Move to fixed point representation. RAD and PHI multiplicative factors 
      // are used as this is where we will be hiding these values.
      const long x = (radius * std::cos(phi)) * FPGADataFormatUtilities::GHITZ_W1_RAD_mf;
      const long y = (radius * std::sin(phi)) * FPGADataFormatUtilities::GHITZ_W1_PHI_mf;
 
      // rad is unsigned but x is not. reinterpret_cast to avoid any 
      // conversions.
      FPGADataFormatUtilities::GHITZ_w1 masqueradedHit = hit;
      masqueradedHit.rad = *reinterpret_cast<std::add_pointer<const decltype(hit.rad)>::type>(&x);
      masqueradedHit.phi = *reinterpret_cast<std::add_pointer<const decltype(hit.phi)>::type>(&y);

      cartesianDataStream->push_back(get_dataformat_GHITZ_w1(masqueradedHit));

      if (hit.last == 0) {
        state = State::READING_GHITZ_W2;
      }
      else {
        state = State::READING_LAST_GHITZ_W2;
      }
    }
    else if (state == State::READING_GHITZ_W2) {
      cartesianDataStream->push_back(word);
      state = State::READING_GHITZ_W1;
    }
    else if (state == State::READING_LAST_GHITZ_W2) {
      cartesianDataStream->push_back(word);
      state = State::LOOKING_FOR_HDR;
    }
    else if (state == State::DONE) {
      ATH_MSG_ERROR("Found something in test vector after event footer.");
     
      return StatusCode::FAILURE;
    }
  } 
  
  return StatusCode::SUCCESS;
}

