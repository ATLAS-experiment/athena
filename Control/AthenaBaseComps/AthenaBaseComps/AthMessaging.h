///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// AthMessaging.h 
// Header file for class AthMessaging
// Author: S.Binet<binet@cern.ch>, Frank Winklmeier
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENABASECOMPS_ATHMESSAGING_H
#define ATHENABASECOMPS_ATHMESSAGING_H 1

// STL includes
#include <iosfwd>
#include <string>
#include <atomic>

// framework includes
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "Gaudi/Property.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "CxxUtils/checker_macros.h"

#include <boost/thread/tss.hpp>


/** @class AthMessaging AthMessaging.h AthenaBaseComps/AthMessaging.h
 *
 *  @brief Class to provide easy @c MsgStream access and capabilities.
 *
 *  All the expensive operations (e.g. retrieval of MessageSvc) are
 *  done lazily, i.e. only when a message is printed to keep the
 *  construction of this class as fast as possible.
 *
 *  One usually inherits from this class and uses it like so:
 *  @code
 *   class MyClass : public AthMessaging, ... {
 *    public:
 *     MyClass() : AthMessaging("MyName") {}
 *     void print() {
 *       ATH_MSG_INFO("Hello World");
 *     }
 *   };
 *  @endcode
 *
 *  The above will retrieve the pointer to the MessageSvc when needed.
 *  If you have access to it already, it is preferred to pass it explicitly:
 *  @code
 *     MyClass() : AthMessaging(msgSvc, "MyName") {}
 *  @endcode
 */
class AthMessaging
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /** Constructor
   *  @param msgSvc Pointer to the MessageSvc
   *  @param name Name of the message stream
   */
  AthMessaging (IMessageSvc* msgSvc, const std::string& name);

  /** Constructor with auto-retrieval of the MessageSvc
   *  @param name Name of the message stream
   */
  AthMessaging (const std::string& name);

  /// Destructor: 
  virtual ~AthMessaging(); 

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /** @brief Test the output level
   *  @param lvl The message level to test against
   *  @return boolean Indicating if messages at given level will be printed
   *  @retval true Messages at level "lvl" will be printed
   */
  bool 
  msgLvl (const MSG::Level lvl) const;

  /** The standard message stream.
   *  Returns a reference to the default message stream
   *  May not be invoked before sysInitialize() has been invoked.
   */
  MsgStream& msg() const;

  /** The standard message stream.
   *  Returns a reference to the default message stream
   *  May not be invoked before sysInitialize() has been invoked.
   */
  MsgStream& 
  msg (const MSG::Level lvl) const;

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /** Change the current logging level.
   *  Use this rather than msg().setLevel() for proper operation with MT.
   */
  void setLevel (MSG::Level lvl);

  /////////////////////////////////////////////////////////////////// 
  // Private methods: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Default constructor: 
  AthMessaging(); //> not implemented
  AthMessaging( const AthMessaging& rhs ); //> not implemented
  AthMessaging& operator=( const AthMessaging& rhs ); //> not implemented

  /// Initialize our message level and MessageSvc
  void initMessaging() const;

  ///////////////////////////////////////////////////////////////////
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Message source name
  std::string m_nm;

  /// MsgStream instance (a std::cout like with print-out levels)
  mutable boost::thread_specific_ptr<MsgStream> m_msg_tls;

  /// MessageSvc pointer
  mutable std::atomic<IMessageSvc*> m_imsg{ nullptr };

  /// Current logging level.
  mutable std::atomic<MSG::Level> m_lvl{ MSG::NIL };

  /// Messaging initialized (initMessaging)
  mutable std::atomic_flag m_initialized ATLAS_THREAD_SAFE = ATOMIC_FLAG_INIT;

}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

inline
bool
AthMessaging::msgLvl (const MSG::Level lvl) const
{
  if (m_lvl <= lvl) {
    msg() << lvl;
    return true;
  } else {
    return false;
  }
}

inline
MsgStream&
AthMessaging::msg() const 
{
  MsgStream* ms = m_msg_tls.get();
  if (!ms) {
    if (!m_initialized.test_and_set()) initMessaging();
    ms = new MsgStream(m_imsg,m_nm);
    m_msg_tls.reset( ms );
  }

  ms->setLevel (m_lvl);
  return *ms;
}

inline
MsgStream&
AthMessaging::msg (const MSG::Level lvl) const 
{ return msg() << lvl; }


#endif //> !ATHENABASECOMPS_ATHMESSAGING_H
