/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestReadPLinks.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Dec, 2023
 * @brief Test reading packed links.
 */

#include "xAODTestReadPLinks.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "AthContainers/PackedLink.h"
#include <format>
#include <print>
#include <sstream>


namespace {


/**
 * @brief Format an ElementLink to a string.
 */
std::string formEL (const DMTest::xAODTestReadPLinks::EL& el)
{
  return std::format ("({}:{})",
                      el.dataID(),
                      static_cast<int>(el.index()) == -1 ? "inv" : std::to_string(el.index()));
}


/**
 * @brief Format a DataLink to a string.
 */
std::string formDL (const DMTest::xAODTestReadPLinks::DL& dl)
{
  return std::format ("({})", dl.dataID());
}


} // anonymous namespace


namespace DMTest {


/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestReadPLinks::initialize()
{
  ATH_CHECK( m_plinksContainerKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_plinksInfoKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_plinksDecorLinkKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_plinksDecorVLinksKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_plinksInfoDecorLinkKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_plinksInfoDecorVLinksKey.initialize (SG::AllowEmpty) );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestReadPLinks::execute (const EventContext& ctx) const
{
  static const SG::ConstAccessor< SG::PackedLink<CVec> > acc_plink ( "plink" );

  if (!m_plinksContainerKey.empty()) {
    SG::ReadHandle<PLinksContainer> plinkscont (m_plinksContainerKey, ctx);
    ATH_MSG_INFO( m_plinksContainerKey.key() );

    std::ostringstream ss;
    std::print (ss, "  plink_DLinks: ");
    for (const DL& dl : acc_plink.getDataLinkSpan (*plinkscont)) {
      std::print (ss, "{} ", formDL (dl));
    }
    ATH_MSG_INFO( ss.str() );

    for (const PLinks* plinks : *plinkscont) {
      ATH_CHECK( dumpPLinks (*plinks) );
      ATH_CHECK( dumpDecor (ctx, *plinks) );
    }
  }

  if (!m_plinksInfoKey.empty()) {
    SG::ReadHandle<PLinks> plinksinfo (m_plinksInfoKey, ctx);
    ATH_MSG_INFO( m_plinksInfoKey.key() );

    std::ostringstream ss;
    std::print (ss, "  plinkInfo_DLinks: ");
    for (const DL& dl : acc_plink.getDataLinkSpan (*plinksinfo->container())) {
      std::print (ss, "{} ", formDL (dl));
    }
    ATH_MSG_INFO( ss.str() );

    ATH_CHECK( dumpPLinks (*plinksinfo) );
    ATH_CHECK( dumpInfoDecor (ctx, *plinksinfo) );
  }

  return StatusCode::SUCCESS;
}


/**
 * @brief Dump a PLinks object.
 */
StatusCode xAODTestReadPLinks::dumpPLinks (const PLinks& plinks) const
{
  std::ostringstream ss;
  std::print (ss, "  link: {}", formEL (plinks.plink()));
  std::print (ss, "  links:");
  for (EL el : plinks.vlinks()) {
    std::print (ss, " {}", formEL (el));
  }
  ATH_MSG_INFO( ss.str() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Dump decorations from a PLinks object.
 */
StatusCode
xAODTestReadPLinks::dumpDecor (const EventContext& ctx,
                               const PLinks& plinks) const
{
  std::ostringstream ss;
  if (!m_plinksDecorLinkKey.empty()) {
    SG::ReadDecorHandle<PLinksContainer, SG::PackedLink<CVec> > decor (m_plinksDecorLinkKey, ctx);
    std::print (ss, "  decorLink: {}", formEL (decor (plinks)));
  }

  if (!m_plinksDecorVLinksKey.empty()) {
    SG::ReadDecorHandle<PLinksContainer, std::vector<SG::PackedLink<CVec> > > decor (m_plinksDecorVLinksKey, ctx);
    std::print (ss, " decorVLinks:");
    for (ElementLink<CVec> el : decor (plinks)) {
      std::print (ss, " {}", formEL (el));
    }
  }
  ATH_MSG_INFO( ss.str() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Dump decorations from a standalone PLinks object.
 */
StatusCode
xAODTestReadPLinks::dumpInfoDecor (const EventContext& ctx,
                                   const PLinks& plinks) const
{
  std::ostringstream ss;
  if (!m_plinksInfoDecorLinkKey.empty()) {
    SG::ReadDecorHandle<PLinks, SG::PackedLink<CVec> > decor (m_plinksInfoDecorLinkKey, ctx);
    std::print (ss, "  decorLink: {}", formEL (decor (plinks)));
  }

  if (!m_plinksInfoDecorVLinksKey.empty()) {
    SG::ReadDecorHandle<PLinks, std::vector<SG::PackedLink<CVec> > > decor (m_plinksInfoDecorVLinksKey, ctx);
    std::print (ss, " decorVLinks:");;
    for (ElementLink<CVec> el : decor (plinks)) {
      std::print (ss, " {}", formEL (el));
    }
  }
  ATH_MSG_INFO( ss.str() );
  return StatusCode::SUCCESS;
}


} // namespace DMTest
