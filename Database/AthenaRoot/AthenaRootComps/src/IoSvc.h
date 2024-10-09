///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// IoSvc.h 
// Header file for class IoSvc
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENASERVICES_IOSVC_H
#define ATHENASERVICES_IOSVC_H 1

// STL includes
#include <string>
#include <unordered_map>

// FrameWork includes
#include "AthenaBaseComps/AthService.h"

// AthenaKernel
#include "AthenaRootKernel/IIoSvc.h"

// Forward declaration
class ISvcLocator;
template <class TYPE> class SvcFactory;



class IoSvc
  : public extends<::AthService, ::IIoSvc>
{
  friend class SvcFactory<IoSvc>;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 
  typedef IIoSvc::Fd Fd;

  /// Constructor with parameters:
  IoSvc( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~IoSvc(); 

  // Assignment operator: 
  //IoSvc &operator=(const IoSvc &alg); 

  /// Gaudi Service Implementation
  //@{
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  //@}

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /// test if a given file descriptor `fd` is known to us
  virtual bool has_fd(Fd fd) const override;

  /// retrieve the file descriptor associated with file `fname`
  /// @returns -1 if no such `fname` is known
  virtual Fd fd(const std::string& fname) const override;

  /// retrieve the file `fname` associated with file descriptor `fd`
  /// @returns empty string if no such `fd` is known
  virtual const std::string& fname(Fd fd) const override;

  /// retrieve the open mode associated with file descriptor `fd`
  virtual IoType mode(Fd fd) const override;

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /// open file `fname` with open mode `mode`
  /// @returns -1 if not successful
  virtual Fd open(const std::string& fname, IoType mode) override;

  /// close file `fd`
  virtual StatusCode close(Fd fd) override;

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Default constructor: 
  IoSvc();

  struct FdInfos {
    std::string fname;
    IIoSvc::IoType mode;
  };

  typedef std::unordered_map<Fd, FdInfos> FdMap_t;

  /// map of fd->fdinfos
  FdMap_t m_fds;

  /// last created Fd
  Fd m_last_fd;
}; 

#endif //> !ATHENASERVICES_IOSVC_H
