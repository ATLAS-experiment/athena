/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestReadCLinks.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2019
 * @brief Read and dump CLinks/CLinksAOD objects.
 */


#include "xAODTestReadCLinks.h"
#include "StoreGate/ReadHandle.h"
#include <format>
#include <print>
#include <sstream>


namespace {


typedef ElementLink<DMTest::CVec> EL;


/** 
 * @brief Format an ElementLink to a string.
 */
std::string formEL (const EL& el)
{
  return std::format ("({}:{})",
                      el.dataID(),
                      static_cast<int>(el.index()) == -1 ? "inv" : std::to_string(el.index()));
}


} // anonymous namespace


namespace DMTest {


/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestReadCLinks::initialize()
{
  ATH_CHECK( m_clinksKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_clinksContainerKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_clinksAODKey.initialize (SG::AllowEmpty) );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestReadCLinks::execute (const EventContext& ctx) const
{
  if (!m_clinksKey.empty()) {
    SG::ReadHandle<CLinks> clinks (m_clinksKey, ctx);
    ATH_MSG_INFO( m_clinksKey.key() );
    ATH_CHECK( dumpCLinks (*clinks) );
  }

  if (!m_clinksContainerKey.empty()) {
    SG::ReadHandle<CLinksContainer> clinkscont (m_clinksContainerKey, ctx);
    ATH_MSG_INFO( m_clinksContainerKey.key() );
    for (const CLinks* clinks : *clinkscont) {
      ATH_CHECK( dumpCLinks (*clinks) );
    }
  }

  if (!m_clinksAODKey.empty()) {
    SG::ReadHandle<CLinksAOD> clinksaod (m_clinksAODKey, ctx);
    ATH_MSG_INFO( m_clinksAODKey.key() );

    std::ostringstream ss1;
    std::print (ss1, "  ");
    for (const EL& el : clinksaod->vel()) {
      std::print (ss1, "{} ", formEL (el));
    }
    ATH_MSG_INFO (ss1.str());

    std::ostringstream ss2;
    std::print (ss2, "  ");
    for (const EL el : clinksaod->elv()) {
      std::print (ss2, "{} ", formEL (el));
    }
    ATH_MSG_INFO (ss2.str());
  }

  return StatusCode::SUCCESS;
}


/**
 * @brief Dump a CLinks object.
 */
StatusCode xAODTestReadCLinks::dumpCLinks (const CLinks& clinks) const
{
  std::ostringstream ss;
  std::print (ss, "  link: {} links: ", formEL (clinks.link()));
  for (const EL& el : clinks.links()) {
    std::print (ss, "{} ", formEL (el));
  }
  ATH_MSG_INFO( ss.str() );
  return StatusCode::SUCCESS;
}


} // namespace DMTest
