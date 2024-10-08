///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// IoSvc.cxx 
// Implementation file for class IoSvc
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// AthenaServices includes
#include "IoSvc.h"

// STL includes
#include <limits>

// FrameWork includes
#include "Gaudi/Property.h"


typedef IIoSvc::Fd Fd;
typedef IIoSvc::IoType IoType;

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
IoSvc::IoSvc( const std::string& name, 
		      ISvcLocator* pSvcLocator ) : 
  base_class( name, pSvcLocator ),
  m_fds(),
  m_last_fd(3) // 1==cout, 2==cerr, 3==clog
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty );

}

// Destructor
///////////////
IoSvc::~IoSvc()
{}

// Athena Service's Hooks
////////////////////////////
StatusCode IoSvc::initialize()
{
  ATH_MSG_INFO ("Initializing " << name() << "...");

  return StatusCode::SUCCESS;
}

StatusCode IoSvc::finalize()
{
  ATH_MSG_INFO ("Finalizing " << name() << "...");

  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////
// Const methods: 
///////////////////////////////////////////////////////////////////

/// test if a given file descriptor `fd` is known to us
bool 
IoSvc::has_fd(Fd fd) const
{
  return m_fds.find(fd) != m_fds.end();
}

/// retrieve the file descriptor associated with file `fname`
/// @returns -1 if no such `fname` is known
Fd 
IoSvc::fd(const std::string& fname) const
{
  for (FdMap_t::const_iterator 
         itr = m_fds.begin(),
         iend= m_fds.end();
       itr != iend;
       ++itr) {
    if (itr->second.fname == fname) {
      return itr->first;
    }
  }
  return -1;
}

/// retrieve the file `fname` associated with file descriptor `fd`
/// @returns empty string if no such `fd` is known
const std::string& 
IoSvc::fname(Fd fd) const
{
  FdMap_t::const_iterator itr = m_fds.find(fd);
  if (itr != m_fds.end()) {
    return itr->second.fname;
  }
  static const std::string s_empty = "";
  return s_empty;
}

/// retrieve the open mode associated with file descriptor `fd`
IoType 
IoSvc::mode(Fd fd) const
{
  FdMap_t::const_iterator itr = m_fds.find(fd);
  if (itr != m_fds.end()) {
    return itr->second.mode;
  }
  return IIoSvc::INVALID;
}

/// open file `fname` with open mode `mode`
/// @returns -1 if not successful
Fd 
IoSvc::open(const std::string& fname, IoType mode)
{
  // check this `fname` hasn't been already associated.
  Fd fd = this->fd(fname);
  if (fd == -1) {
    // FIXME: use a recycle bin of fds ?
    if ( m_last_fd == (std::numeric_limits<Fd>::max()-1)) {
      ATH_MSG_ERROR("too many file descriptors opened!");
      return -1;
    }
    fd = ++m_last_fd;
    FdInfos infos = {fname, mode};
    m_fds.insert(std::make_pair(fd, infos));
    return fd;
  }

  // check the previous open mode is the same
  if (mode == this->mode(fd)) {
    return fd;
  }

  // inconsistency...
  return -1;
}

/// close file `fd`
StatusCode 
IoSvc::close(Fd fd)
{
  FdMap_t::const_iterator itr = m_fds.find(fd);
  if (itr != m_fds.end()) {
    m_fds.erase(itr);
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

/////////////////////////////////////////////////////////////////// 
// Protected methods: 
/////////////////////////////////////////////////////////////////// 

/////////////////////////////////////////////////////////////////// 
// Const methods: 
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 


