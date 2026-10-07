/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestReadJVec.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief For testing jagged vectors.
 */


#include "xAODTestReadJVec.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "AthContainers/JaggedVec.h"
#include "AthLinks/ElementLink.h"
#include <format>
#include <print>
#include <sstream>


namespace {


/** 
 * @brief Format an ElementLink to a string.
 */
template <class CONT>
std::string formEL (const ElementLink<CONT>& el)
{
  return std::format ("({}:{})",
                      el.dataID(),
                      static_cast<int>(el.index()) == -1 ? "inv" : std::to_string(el.index()));
}


/** 
 * @brief Format a vector element to a string.
 */
template <class T>
std::string form_vec_elt (const T& x)
{
  return std::format ("{}", x);
}
  


/** 
 * @brief Format a vector element to a string, specialized for std::string.
 */
std::string form_vec_elt (const std::string& x)
{
  return std::format ("'{}'", x);
}
  

/** 
 * @brief Format a vector element to a string, specialized for ElementLink.
 */
template <class CONT>
std::string form_vec_elt (const ElementLink<CONT>& x)
{
  return formEL (x);
}


/** 
 * @brief Format a jagged vector element as a string.
 */
template <class RANGE>
std::string formJVec (const RANGE& r) {
  std::ostringstream ss;
  std::print (ss, "[");
  std::string sep;
  for (const auto & elt : r) {
    std::print (ss, "{}", sep);
    sep = " ";
    std::print (ss, "{}", form_vec_elt (elt));
  }
  std::print (ss, "]");
  return ss.str();
}


}


namespace DMTest {


/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestReadJVec::initialize()
{
  ATH_CHECK( m_jvecContainerKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_jvecInfoKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_jvecDecorKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_jvecInfoDecorKey.initialize (SG::AllowEmpty) );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestReadJVec::execute (const EventContext& ctx) const
{
  if (!m_jvecContainerKey.empty()) {
    SG::ReadHandle<JVecContainer> jveccont (m_jvecContainerKey, ctx);
    ATH_MSG_INFO( m_jvecContainerKey.key() );
    for (const JVec* jvec : *jveccont) {
      ATH_CHECK( dumpJVec (*jvec) );
      ATH_CHECK( dumpDecor (ctx, *jvec) );
    }
  }

  if (!m_jvecInfoKey.empty()) {
    SG::ReadHandle<JVec> jvecinfo (m_jvecInfoKey, ctx);
    ATH_MSG_INFO( m_jvecInfoKey.key() );
    ATH_CHECK( dumpJVec (*jvecinfo) );
    ATH_CHECK( dumpInfoDecor (ctx, *jvecinfo) );
  }

  return StatusCode::SUCCESS;
}


/**
 * @brief Dump a JVec object.
 */
StatusCode xAODTestReadJVec::dumpJVec (const JVec& jvec) const
{
  ATH_MSG_INFO( "  ivec: {}  fvec: {}  svec: {}  lvec: {}",
                formJVec (jvec.ivec()),
                formJVec (jvec.fvec()),
                formJVec (jvec.svec()),
                formJVec (jvec.lvec()));
  return StatusCode::SUCCESS;
}


/**
 * @brief Dump decorations from a JVec object.
 */
StatusCode
xAODTestReadJVec::dumpDecor (const EventContext& ctx,
                             const JVec& jvec) const
{
  std::ostringstream ss;
  if (!m_jvecDecorKey.empty()) {
    SG::ReadDecorHandle<JVecContainer, SG::JaggedVecElt<double> > decor (m_jvecDecorKey, ctx);
    std::print (ss, "  decorJVec: {}", formJVec (decor (jvec)));
  }

  ATH_MSG_INFO( ss.str() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Dump decorations from a standalone JVec object.
 */
StatusCode
xAODTestReadJVec::dumpInfoDecor (const EventContext& ctx,
                                 const JVec& jvec) const
{
  std::ostringstream ss;
  if (!m_jvecInfoDecorKey.empty()) {
    SG::ReadDecorHandle<JVec, SG::JaggedVecElt<double> > decor (m_jvecInfoDecorKey, ctx);
    std::print (ss, "  decorJVec: {}", formJVec (decor (jvec)));
  }
  
  ATH_MSG_INFO( ss.str() );
  return StatusCode::SUCCESS;
}


} // namespace DMTest
