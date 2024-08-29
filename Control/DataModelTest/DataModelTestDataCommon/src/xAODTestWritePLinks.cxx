/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestWritePLinks.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2023
 * @brief Test writing packed links.
 */


#include "xAODTestWritePLinks.h"
#include "DataModelTestDataCommon/PLinksAuxContainer.h"
#include "DataModelTestDataCommon/PLinksAuxInfo.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadHandle.h"


namespace DMTest {


/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestWritePLinks::initialize()
{
  ATH_CHECK( m_cvecKey.initialize() );
  ATH_CHECK( m_ctrigKey.initialize() );
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
StatusCode xAODTestWritePLinks::execute (const EventContext& ctx) const
{
  SG::ReadHandle<CVec> cvec (m_cvecKey, ctx);
  SG::ReadHandle<CVec> ctrig (m_ctrigKey, ctx);

  if (!m_plinksContainerKey.empty()) {
    auto plinkscont = std::make_unique<PLinksContainer>();
    auto plinksauxcont = std::make_unique<PLinksAuxContainer>();
    plinkscont->setStore (plinksauxcont.get());
    for (size_t i = 0; i < 5; i++) {
      plinkscont->push_back (std::make_unique<PLinks>());
      ATH_CHECK( fillPLinks (m_cvecKey.key(), *cvec,
                             m_ctrigKey.key(), *ctrig,
                             i + ctx.evt(),
                             *plinkscont->back()) );
    }

    SG::WriteHandle<PLinksContainer> plinksContH (m_plinksContainerKey, ctx);
    ATH_CHECK( plinksContH.record (std::move (plinkscont),
                                   std::move (plinksauxcont)) );
  }

  if (!m_plinksInfoKey.empty()) {
    auto plinksinfo = std::make_unique<PLinks>();
    auto plinksauxinfo = std::make_unique<PLinksAuxInfo>();
    plinksinfo->setStore (plinksauxinfo.get());
    ATH_CHECK( fillPLinks (m_cvecKey.key(), *cvec,
                           m_ctrigKey.key(), *ctrig,
                           ctx.evt() + 3,
                           *plinksinfo) );

    SG::WriteHandle<PLinks> plinksInfoH (m_plinksInfoKey, ctx);
    ATH_CHECK( plinksInfoH.record (std::move (plinksinfo),
                                   std::move (plinksauxinfo)) );
  }

  ATH_CHECK( decorPLinks (ctx,
                          m_cvecKey.key(), *cvec,
                          m_ctrigKey.key(), *ctrig) );

  return StatusCode::SUCCESS;
}


auto xAODTestWritePLinks::makeLink (const std::string& key1,
                                    const CVec& cvec1,
                                    const std::string& key2,
                                    const CVec& cvec2,
                                    size_t ndx) const -> EL
{
  if ((ndx%3) == 0 && !cvec1.empty()) {
    return EL (key1, (ndx % cvec1.size()));
  }
  else if ((ndx%3) == 1 && !cvec2.empty()) {
    return EL (key2, (ndx % cvec2.size()));
  }
  // else leave it null
  return EL();
}


StatusCode xAODTestWritePLinks::fillPLinks (const std::string& key1,
                                            const CVec& cvec1,
                                            const std::string& key2,
                                            const CVec& cvec2,
                                            size_t ndx,
                                            PLinks& plinks) const
{
  plinks.setPLink (makeLink (key1, cvec1, key2, cvec2, ndx));
  std::vector<EL> v;
  for (size_t i = 0; i < ndx%5; ++i) {
    v.emplace_back (makeLink (key1, cvec1, key2, cvec2, ndx+i+1));
  }
  plinks.setVLinks (v);
  return StatusCode::SUCCESS;
}


StatusCode xAODTestWritePLinks::decorPLinks (const EventContext& ctx,
                                             const std::string& key1,
                                             const CVec& cvec1,
                                             const std::string& key2,
                                             const CVec& cvec2) const
{
  if (!m_plinksDecorLinkKey.empty()) {
    SG::WriteDecorHandle<PLinksContainer, SG::PackedLink<CVec> > decor (m_plinksDecorLinkKey, ctx);
    size_t ndx = ctx.evt();
    for (const PLinks* plinks : *decor) {
      decor (*plinks) = makeLink (key1, cvec1, key2, cvec2, ndx);
      ++ndx;
    }
  }

  if (!m_plinksDecorVLinksKey.empty()) {
    SG::WriteDecorHandle<PLinksContainer, std::vector<SG::PackedLink<CVec> > > decor (m_plinksDecorVLinksKey, ctx);
    size_t ndx = ctx.evt() + 3;
    for (const PLinks* plinks : *decor) {
      std::vector<EL> v;
      for (size_t i = 0; i < ndx%5; ++i) {
        v.emplace_back (makeLink (key1, cvec1, key2, cvec2, ndx+i+1));
      }
      decor (*plinks) = v;
      ++ndx;
    }
  }

  if (!m_plinksInfoDecorLinkKey.empty()) {
    SG::WriteDecorHandle<PLinks, SG::PackedLink<CVec> > decor (m_plinksInfoDecorLinkKey, ctx);
    size_t ndx = ctx.evt() + 4;
    decor (*decor) = makeLink (key1, cvec1, key2, cvec2, ndx);
  }

  if (!m_plinksInfoDecorVLinksKey.empty()) {
    SG::WriteDecorHandle<PLinks, std::vector<SG::PackedLink<CVec> > > decor (m_plinksInfoDecorVLinksKey, ctx);
    size_t ndx = ctx.evt() + 7;
    std::vector<EL> v;
    for (size_t i = 0; i < ndx%5; ++i) {
      v.emplace_back (makeLink (key1, cvec1, key2, cvec2, ndx+i+1));
    }
    decor (*decor) = v;
  }

  return StatusCode::SUCCESS;
}


} // namespace DMTest
