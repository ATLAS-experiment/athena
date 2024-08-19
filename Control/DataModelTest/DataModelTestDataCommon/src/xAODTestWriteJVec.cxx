/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestWriteJVec.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief For testing jagged vectors.
 */


#include "xAODTestWriteJVec.h"
#include "DataModelTestDataCommon/JVecAuxContainer.h"
#include "DataModelTestDataCommon/JVecAuxInfo.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include <numeric>


namespace {


/// Helper for filling a range.
template <class CONT, class T>
void xiota (CONT& v, T x)
{
  std::iota (v.begin(), v.end(), x);
}


} // anonymous namespace


namespace DMTest {


/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestWriteJVec::initialize()
{
  ATH_CHECK( m_cvecKey.initialize() );
  ATH_CHECK( m_jvecContainerKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_jvecInfoKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_jvecDecorKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_jvecInfoDecorKey.initialize (SG::AllowEmpty) );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestWriteJVec::execute (const EventContext& ctx) const
{
  SG::ReadHandle<CVec> cvec (m_cvecKey, ctx);

  if (!m_jvecContainerKey.empty()) {
    auto jveccont = std::make_unique<JVecContainer>();
    auto jvecauxcont = std::make_unique<JVecAuxContainer>();
    jveccont->setStore (jvecauxcont.get());

    for (size_t i = 0; i < 5; i++) {
      jveccont->push_back (std::make_unique<JVec>());
      ATH_CHECK( fillJVec (m_cvecKey.key(), *cvec,
                           i + ctx.evt(),
                           *jveccont->back()) );
    }

    SG::WriteHandle<JVecContainer> jvecContH (m_jvecContainerKey, ctx);
    ATH_CHECK( jvecContH.record (std::move (jveccont),
                                 std::move (jvecauxcont)) );
  }

  if (!m_jvecInfoKey.empty()) {
    auto jvecinfo = std::make_unique<JVec>();
    auto jvecauxinfo = std::make_unique<JVecAuxInfo>();
    jvecinfo->setStore (jvecauxinfo.get());
    ATH_CHECK( fillJVec (m_cvecKey.key(), *cvec,
                         ctx.evt() + 1001, *jvecinfo) );

    SG::WriteHandle<JVec> jvecInfoH (m_jvecInfoKey, ctx);
    ATH_CHECK( jvecInfoH.record (std::move (jvecinfo),
                                 std::move (jvecauxinfo)) );
  }

  ATH_CHECK( decorJVec (ctx) );

  return StatusCode::SUCCESS;
}


StatusCode xAODTestWriteJVec::fillJVec (const std::string& key1,
                                        const CVec& cvec1,
                                        size_t ndx, JVec& jvec) const
{
  std::vector<int> vi (ndx%5);
  xiota (vi, ndx+100);
  jvec.setIVec (vi);

  std::vector<float> vf ((ndx+1)%5);
  xiota (vf, ndx+200.5f);
  jvec.setFVec (vf);

  std::vector<std::string> vs ((ndx+2)%5);
  for (size_t i = 0; i < vs.size(); i++) {
    vs[i] = "str" + std::to_string(ndx+i+450);
  }
  jvec.setSVec (vs);

  std::vector<ElementLink<CVec> > vl ((ndx+3)%5);
  for (size_t i = 0; i < vl.size(); i++) {
    if (!cvec1.empty()) {
      vl[i].resetWithKeyAndIndex (key1, (ndx+i)%cvec1.size());
    }
  }
  jvec.setLVec (vl);

  return StatusCode::SUCCESS;
}


StatusCode xAODTestWriteJVec::decorJVec (const EventContext& ctx) const
{
  if (!m_jvecDecorKey.empty()) {
    SG::WriteDecorHandle<JVecContainer, SG::JaggedVecElt<double> > decor (m_jvecDecorKey, ctx);
    size_t ndx = ctx.evt();
    for (const JVec* jvec : *decor) {
      std::vector<double> vd ((ndx+2)%5);
      xiota (vd, ndx+300.5f);
      decor (*jvec) = vd;
      ++ndx;
    }
  }

  if (!m_jvecInfoDecorKey.empty()) {
    SG::WriteDecorHandle<JVec, SG::JaggedVecElt<double> > decor (m_jvecInfoDecorKey, ctx);
    size_t ndx = ctx.evt() + 4;
    std::vector<double> vd ((ndx+2)%5);
    xiota (vd, ndx+400.5f);
    decor (*decor) = vd;
  }

  return StatusCode::SUCCESS;
}
  
                                            
} // namespace DMTest
