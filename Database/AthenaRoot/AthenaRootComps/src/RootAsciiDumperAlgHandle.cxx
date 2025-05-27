///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// RootAsciiDumperAlgHandle.cxx 
// Implementation file for class RootAsciiDumperAlgHandle
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// AthenaRootComps includes
#include "RootAsciiDumperAlgHandle.h"

// STL includes
#include <sstream>
#include <stdio.h>
// to get the printing format specifiers (e.g. PRId64)
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

// linux i/o includes
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// FrameWork includes
#include "Gaudi/Property.h"
#include "AthContainers/ConstAccessor.h"

// SGTools
#include "SGTools/BuiltinsClids.h"  // to put ints,... in evtstore
#include "SGTools/StlVectorClids.h" // to put std::vectors... in evtstore

namespace Athena {

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Athena Algorithm's Hooks
////////////////////////////
StatusCode RootAsciiDumperAlgHandle::initialize()
{
  ATH_MSG_INFO ("Initializing " << name() << "...");
  ATH_CHECK( m_eiKey.initialize() );
  ATH_CHECK( m_el_jetcone_dr.initialize() );
  ATH_CHECK( m_el_n.initialize() );
  ATH_CHECK( m_el_eta.initialize() );
  ATH_CHECK( m_evtnbr.initialize() );
  ATH_CHECK( m_runnbr.initialize() );

  ATH_MSG_INFO("dumping data into file ["
               << m_ofname << "]...");
  if (m_ofname.empty()) {
    ATH_MSG_ERROR("cannot dump data into an empty file name!");
    return StatusCode::FAILURE;
  }
  m_ofd = open(m_ofname.value().c_str(), 
               O_WRONLY | O_CREAT | O_TRUNC,
               S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

  if (m_ofd < 0) {
    ATH_MSG_ERROR("problem opening file [" << m_ofname << "] with "
                  "write permissions.");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode RootAsciiDumperAlgHandle::finalize()
{
  ATH_MSG_INFO ("Finalizing " << name() << "...");

  if (m_ofd > 0) {
    fflush(NULL);
    if (close(m_ofd) < 0) {
      ATH_MSG_WARNING("problem while closing [" << m_ofname << "]");
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode RootAsciiDumperAlgHandle::execute()
{  
  ATH_MSG_DEBUG ("Executing " << name() << "...");

  const EventContext& ctx = Gaudi::Hive::currentContext();

  uint64_t nevts = m_nentries;
  m_nentries += 1;

  SG::ReadHandle<uint32_t> runnbr (m_runnbr, ctx);
  SG::ReadHandle<uint32_t> evtnbr (m_evtnbr, ctx);

  SG::ReadHandle<int32_t> el_n (m_el_n, ctx);

  SG::ReadHandle<xAOD::EventInfo> ei (m_eiKey, ctx);
  static const SG::ConstAccessor<std::string> tupleName ("tupleName");
  static const SG::ConstAccessor<std::string> collectionName ("collectionName");
  std::string collName = collectionName(*ei);
  std::string::size_type pos = collName.rfind ("/");
  if (pos != std::string::npos) {
    collName.erase (0, pos+1);
  }

  {
    char* buf = 0;
    int buf_sz = asprintf
      (&buf,
       "%03" PRId64 ".%s = %s\n"
       "%03" PRId64 ".%s = %s\n"
       "%03" PRId64 ".%s = %u\n"
       "%03" PRId64 ".%s = %u\n"
       "%03" PRId64 ".%s = %i\n",
       nevts,
       "collectionName",
       collName.c_str(),
       nevts,
       "tupleName",
       tupleName(*ei).c_str(),
       nevts,
       "RunNumber",
       *runnbr,
       nevts,
       "EventNumber",
       *evtnbr,
       nevts,
       "el_n",
       *el_n);
    write(m_ofd, buf, buf_sz);
    free(buf);
  }

  if (*el_n > 0) {
    SG::ReadHandle<std::vector<float> > el_eta (m_el_eta, ctx);
    SG::ReadHandle<std::vector<std::vector<float> > > el_jetcone_dr (m_el_jetcone_dr, ctx);
    
    {
      std::stringstream bufv;
      for (int32_t ii = 0; ii < *el_n; ++ii) {
        bufv << (*el_eta)[ii];
        if (ii != (*el_n)-1) {
          bufv << ", ";
        }
      }
      char* buf = 0;
      int buf_sz = asprintf
        (&buf,
         "%03" PRId64 ".%s = [%s]\n",
         nevts,
         "el_eta",
         bufv.str().c_str());
      write(m_ofd, buf, buf_sz);
      free(buf);
    }


    {
      std::stringstream bufv;
      for (int32_t ii = 0; ii < *el_n; ++ii) {
        bufv << "[";
        for (std::size_t jj = 0, jjmax = (*el_jetcone_dr)[ii].size();
             jj < jjmax;
             ++jj) {
          bufv << (*el_jetcone_dr)[ii][jj];
          if (jj != jjmax-1) {
            bufv << ", ";
          }
        }
        bufv << "]";
        if (ii != (*el_n)-1) {
          bufv << ", ";
        }
      }
      char* buf = 0;
      int buf_sz = asprintf
        (&buf,
         "%03" PRId64 ".%s = [%s]\n",
         nevts,
         "el_jetcone_dr",
         bufv.str().c_str());
      write(m_ofd, buf, buf_sz);
      free(buf);
    }
  }

  return StatusCode::SUCCESS;
}

} //> end namespace Athena
