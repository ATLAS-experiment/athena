/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ClusterValidationAlg.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadHandle.h"
#include "xAODInDetMeasurement/PixelCluster.h"

#include <GaudiKernel/StatusCode.h>
#include <traccc/edm/silicon_cell_collection.hpp>

namespace {
// Ad-hoc comparators and streamers of xAOD cluster objects in the context of
// the Traccc to xAOD measurement conversion: only the relevant fields are
// compared and printed.

bool float_equal(float a, float b) {
  if (std::abs(a-b) < 1e-4f) return true;
  return false;
}

bool operator==(xAOD::PixelCluster const & lhs, xAOD::PixelCluster const & rhs) {
  if (// lhs.identifier() == rhs.identifier()
     float_equal(lhs.localPosition<2>()(0), rhs.localPosition<2>()(0))
    && float_equal(lhs.localPosition<2>()(1), rhs.localPosition<2>()(1))
    && float_equal(lhs.localCovariance<2>()(0, 0), rhs.localCovariance<2>()(0, 0))
    && float_equal(lhs.localCovariance<2>()(0, 1), rhs.localCovariance<2>()(0, 1))
    && float_equal(lhs.localCovariance<2>()(1, 0), rhs.localCovariance<2>()(1, 0))
    && float_equal(lhs.localCovariance<2>()(1, 1), rhs.localCovariance<2>()(1, 1))
    && float_equal(lhs.globalPosition()(0), rhs.globalPosition()(0))
    && float_equal(lhs.globalPosition()(1), rhs.globalPosition()(1))
    && float_equal(lhs.globalPosition()(2), rhs.globalPosition()(2))
    && lhs.identifierHash() == rhs.identifierHash()
    )
  {
    return true;
  }
  return false;
}

bool operator==(xAOD::StripCluster const & lhs, xAOD::StripCluster const & rhs) {
  if (// lhs.identifier() == rhs.identifier()
     float_equal(lhs.localPosition<1>()(0), rhs.localPosition<1>()(0))
    && float_equal(lhs.localCovariance<1>()(0), rhs.localCovariance<1>()(0))
    && lhs.identifierHash() == rhs.identifierHash()
    )
  {
    return true;
  }
  return false;
}

MsgStream & operator<<(MsgStream & ms, xAOD::PixelCluster const & cl) {
  ms << "id: " << cl.identifier()
      << ", idhash: " << cl.identifierHash()
      << ", locpos: [" << cl.localPosition<2>()(0) << ", " << cl.localPosition<2>()(1) << ']'
      << ", locvar: [" << '[' << cl.localCovariance<2>()(0, 0) << ", " << cl.localCovariance<2>()(0, 1) << ']'
          << ",[" << cl.localCovariance<2>()(1, 0) << ", " << cl.localCovariance<2>()(1, 1) << "]]"
      << ", glopos: [" << cl.globalPosition()(0) << ", " << cl.globalPosition()(1) << ", " << cl.globalPosition()(2) << ']'
      ;
  return ms;
}

MsgStream & operator<<(MsgStream & ms, xAOD::StripCluster const & cl) {
  ms << "id: " << cl.identifier()
      << ", idhash: " << cl.identifierHash()
      << ", locpos: " << cl.localPosition<1>()
      << ", locvar: " << cl.localCovariance<1>()
      ;
  return ms;
}

} // anonymous namespace

namespace ActsTrk {

StatusCode ClusterValidationAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing");

  ATH_CHECK(m_referencePixelsKey.initialize());
  ATH_CHECK(m_referenceStripsKey.initialize());
  ATH_CHECK(m_monitoredPixelsKey.initialize());
  ATH_CHECK(m_monitoredStripsKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode ClusterValidationAlg::execute(const EventContext& ctx) const
{
  auto ref_pixels = SG::makeHandle(m_referencePixelsKey, ctx);
  auto ref_strips = SG::makeHandle(m_referenceStripsKey, ctx);
  auto mon_pixels = SG::makeHandle(m_monitoredPixelsKey, ctx);
  auto mon_strips = SG::makeHandle(m_monitoredStripsKey, ctx);
  ATH_CHECK(ref_pixels.isValid());
  ATH_CHECK(ref_strips.isValid());
  ATH_CHECK(mon_pixels.isValid());
  ATH_CHECK(mon_strips.isValid());

  if (mon_pixels->size() != ref_pixels->size() || mon_strips->size() != ref_strips->size()) {
    std::ostringstream oss;
    oss << "different sizes:";
    if (mon_pixels->size() != ref_pixels->size()) {
      oss << " pixels (ref: " << ref_pixels->size() << ", mon: " << mon_pixels->size() << ")";
    }
    if (mon_strips->size() != ref_strips->size()) {
      oss << " strips (ref: " << ref_strips->size() << ", mon: " << mon_strips->size() << ")";
    }
    ATH_MSG_ERROR(oss.str());
    return StatusCode::FAILURE;
  }

  bool error = false;
  for (size_t i = 0; i < mon_pixels->size(); ++i) {
    xAOD::PixelCluster const & mon = *mon_pixels->at(i);
    xAOD::PixelCluster const & ref = *ref_pixels->at(i);
    if (mon != ref) {
      error = true;
      ATH_MSG_ERROR("pixel clusters differ at " << i
          << ": ref=[" << ref << ']'
          << ", mon=[" << mon << ']');
    }
  }
  for (size_t i = 0; i < mon_strips->size(); ++i) {
    xAOD::StripCluster const & mon = *mon_strips->at(i);
    xAOD::StripCluster const & ref = *ref_strips->at(i);
    if (mon != ref) {
      error = true;
      ATH_MSG_ERROR("strip clusters differ at " << i
          << ": ref=[" << ref << ']'
          << ", mon=[" << mon << ']');
    }
  }

  return error ? StatusCode::FAILURE : StatusCode::SUCCESS;
}

} // namespace ActsTrk