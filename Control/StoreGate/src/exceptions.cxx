/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file StoreGate/src/exceptions.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2016
 * @brief Exceptions that can be thrown from StoreGate.
 */


#include "StoreGate/exceptions.h"
#include <format>


namespace SG {


/**
 * @brief Constructor.
 */
ExcNullHandleKey::ExcNullHandleKey()
  : std::runtime_error ("SG::ExcNullHandleKey: Attempt to dereference a Read/Write/UpdateHandle with a null key.")
{
}


/**
 * @brief Throw a SG::ExcNullHandleKey exception.
 */
[[noreturn]]
void throwExcNullHandleKey()
{
  throw ExcNullHandleKey();
}


//****************************************************************************


/**
 * @brief Constructor.
 * @param key The supplied key string.
 */
ExcBadHandleKey::ExcBadHandleKey (const std::string& key)
  : std::runtime_error ("SG::ExcBadHandleKey: Bad key format for VarHandleKey: `" + key + "'")
{
}


//****************************************************************************


/**
 * @brief Constructor.
 * @param name Name of the called method.
 */
ExcForbiddenMethod::ExcForbiddenMethod (const std::string& name)
  : std::runtime_error ("SG::ExcForbiddenMethod: Forbidden method called: `" + name + "'")
{
}


//****************************************************************************


/// Helper: format exception error string.
std::string excHandleInitError_format (CLID clid,
                                       const std::string& sgkey,
                                       const std::string& storename)
                                       
{
  return std::format ("SG::ExcHandleInitError: "
                      "Error initializing VarHandle from VarHandleKey: {}+{}[{}]",
                      storename, sgkey, clid);
}


/**
 * @brief Constructor.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 */
ExcHandleInitError::ExcHandleInitError (CLID clid,
                                        const std::string& sgkey,
                                        const std::string& storename)
  : std::runtime_error (excHandleInitError_format (clid, sgkey, storename))
{
}


//****************************************************************************


/// Helper: format exception error string.
std::string excUninitKey_format (CLID clid,
                                 const std::string& sgkey,
                                 const std::string& storename,
                                 const std::string& holdername,
                                 const std::string& htype)
                                       
{
  std::string s =
    std::format ("SG::ExcUninitKey: "
                 "Error initializing {} from uninitialized {}Key: {}+{}[{}]; "
                 "keys should be initialized in your initialize().",
                 htype, htype, storename, sgkey, clid);
  if ( holdername.size() )
    s += std::format (" Key held by {}.", holdername);
  return s;
}


/**
 * @brief Constructor.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 * @param holdername IDataHandleHolder holding the key.
 * @param htype Handle type.
 */
ExcUninitKey::ExcUninitKey (CLID clid,
                            const std::string& sgkey,
                            const std::string& storename,
                            const std::string& holdername /*= ""*/,
                            const std::string& htype /*= "VarHandle"*/)
  : std::runtime_error (excUninitKey_format (clid, sgkey, storename, holdername, htype))
{
}


//****************************************************************************


/// Helper: format exception error string.
std::string excConstObject_format (CLID clid,
                                   const std::string& sgkey,
                                   const std::string& storename)
  
{
  return std::format ("SG::ExcConstObject: "
                      "Tried to retrieve non-const pointer to const object via VarHandleKey: {}+{}[{}]",
                      storename, sgkey, clid);
}


/**
 * @brief Constructor.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 */
ExcConstObject::ExcConstObject (CLID clid,
                                const std::string& sgkey,
                                const std::string& storename)
  : std::runtime_error (excConstObject_format (clid, sgkey, storename))
{
}


//****************************************************************************


/// Helper: format exception error string.
std::string excNullWriteHandle_format (CLID clid,
                                       const std::string& sgkey,
                                       const std::string& storename)
  
{
  return std::format ("SG::ExcNullWriteHandle: "
                      "Attempt to dereference write handle before record: {}+{}[{}]",
                      storename, sgkey, clid);
}


/**
 * @brief Constructor.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 */
ExcNullWriteHandle::ExcNullWriteHandle (CLID clid,
                                        const std::string& sgkey,
                                        const std::string& storename)
  : std::runtime_error (excNullWriteHandle_format (clid, sgkey, storename))
{
}


/**
 * @brief Throw a SG::ExcNullWriteHandle exception.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 */
void throwExcNullWriteHandle (CLID clid,
                              const std::string& sgkey,
                              const std::string& storename)
{
  throw ExcNullWriteHandle (clid, sgkey, storename);
}


//****************************************************************************


/// Helper: format exception error string.
std::string excNullReadHandle_format (CLID clid,
                                      const std::string& sgkey,
                                      const std::string& storename)
  
{
  return std::format ("SG::ExcNullReadHandle: "
                      "Dereference of read handle failed: {}+{}[{}]",
                      storename, sgkey, clid);
}


/**
 * @brief Constructor.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 */
ExcNullReadHandle::ExcNullReadHandle (CLID clid,
                                      const std::string& sgkey,
                                      const std::string& storename)
  : std::runtime_error (excNullReadHandle_format (clid, sgkey, storename))
{
}


/**
 * @brief Throw a SG::ExcNullReadHandle exception.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 */
void throwExcNullReadHandle (CLID clid,
                             const std::string& sgkey,
                             const std::string& storename)
{
  throw ExcNullReadHandle (clid, sgkey, storename);
}


//****************************************************************************


/// Helper: format exception error string.
std::string excNullUpdateHandle_format (CLID clid,
                                        const std::string& sgkey,
                                        const std::string& storename)
  
{
  return std::format ("SG::ExcNullUpdateHandle: "
                      "Dereference of update handle failed: {}+{}[{}]",
                      storename, sgkey, clid);
}


/**
 * @brief Constructor.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 */
ExcNullUpdateHandle::ExcNullUpdateHandle (CLID clid,
                                          const std::string& sgkey,
                                          const std::string& storename)
  : std::runtime_error (excNullUpdateHandle_format (clid, sgkey, storename))
{
}


/**
 * @brief Throw a SG::ExcNullUpdateHandle exception.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 */
void throwExcNullUpdateHandle (CLID clid,
                               const std::string& sgkey,
                               const std::string& storename)
{
  throw ExcNullUpdateHandle (clid, sgkey, storename);
}


//****************************************************************************


/// Helper: format exception error string.
std::string excNonConstHandleKey_format (CLID clid,
                                         const std::string& sgkey,
                                         const std::string& storename)
  
{
  return std::format ("SG::ExcNonConstHandleKey: "
                      "Attempt to get non-const VarHandleKey from non-owning VarHandle: {}+{}[{}]",
                      storename, sgkey, clid);
}


/**
 * @brief Constructor.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 */
ExcNonConstHandleKey::ExcNonConstHandleKey (CLID clid,
                                            const std::string& sgkey,
                                            const std::string& storename)
  : std::runtime_error (excNonConstHandleKey_format (clid, sgkey, storename))
{
}


/**
 * @brief Throw a SG::ExcNonConstHandleKey exception.
 * @param clid CLID from the key.
 * @param sgkey StoreGate key from the key.
 * @param storename Store name from the key.
 */
void throwExcNonConstHandleKey (CLID clid,
                                const std::string& sgkey,
                                const std::string& storename)
{
  throw ExcNonConstHandleKey (clid, sgkey, storename);
}


//****************************************************************************


/**
 * @brief Constructor.
 */
ExcInvalidIterator::ExcInvalidIterator()
  : std::runtime_error ("SG::ExcInvalidIterator: Attempt to dereference invalid SG::Iterator/SG::ConstIterator")
{
}


//****************************************************************************


/**
 * @brief Constructor.
 */
ExcBadInitializedReadHandleKey::ExcBadInitializedReadHandleKey()
  : std::runtime_error ("Initialization of InitializedReadHandleKey failed.")
{
}


//****************************************************************************


/// Helper: format exception error string.
std::string excBadContext_format (const EventContext& ctx,
                                  const std::string& key)
  
{
  return std::format ("SG::ExcBadContext: "
                      "Bad EventContext extension while building ReadCondHandle. "
                      "The EventContext extension is not {} for key {}.",
                      (ctx.hasExtension() ? "of type Atlas::ExtendedEventContext" : "set"),
                      key);
}


/**
 * @brief Constructor.
 * @param ctx The bad EventContext.
 * @param key The key of the handle being built.
 */
ExcBadContext::ExcBadContext (const EventContext& ctx, const std::string& key)
  : std::runtime_error (excBadContext_format (ctx, key))
{
}


//****************************************************************************


/// Helper: format exception error string.
std::string excNoCondCont_format (const std::string& key,
                                  const std::string& why)
  
{
  return std::format ("SG::ExcNoCondCont: "
                      "Can't retrieve CondCont from ReadCondHandle for key {}. {}",
                      key, why);
}


/**
 * @brief Constructor.
 * @param key The key being looked up.
 * @param why Further description.
 */
ExcNoCondCont::ExcNoCondCont (const std::string& key, const std::string& why)
  : std::runtime_error (excNoCondCont_format (key, why))
{
}


//****************************************************************************


/**
 * @brief Constructor.
 */
ExcBadReadCondHandleInit::ExcBadReadCondHandleInit()
  : std::runtime_error ("SG::ExcBadReadCondHandleInit: ReadCondHandle didn't initialize in getRange().")
{
}


//****************************************************************************


/**
 * @brief Constructor.
 */
ExcNoRange::ExcNoRange()
  : std::runtime_error ("SG::ExcBadReadCondHandleInit: Range not set in ReadCondHandle::getRange().")
{
}


//****************************************************************************


/// Helper: format exception error string.
std::string excBadDecorElement_format (Gaudi::DataHandle::Mode mode,
                                       CLID clid,
                                       const std::string& decorKey)
{
  std::string modestr = "???";
  if (mode == Gaudi::DataHandle::Writer)
    modestr = "Write";
  else if (mode == Gaudi::DataHandle::Reader)
    modestr = "Read";
  return std::format ("SG::ExcBadDecorElement: "
                      "{}DecorHandle {}[{}]"
                      " given an element not in the requested container.",
                      modestr, decorKey, clid);
}


/**
 * @brief Constructor.
 * @param mode Reader or Writer, depending on the handle type.
 * @param clid CLID from the handle.
 * @param decorKey Decoration key in CONTAINER.DECOR format.
 */
ExcBadDecorElement::ExcBadDecorElement (Gaudi::DataHandle::Mode mode,
                                        CLID clid,
                                        const std::string& decorKey)
  : std::runtime_error (excBadDecorElement_format (mode, clid, decorKey))
{
}


/**
 * @brief Throw a SG::ExcBadDecorElement exception.
 */
[[noreturn]]
void throwExcBadDecorElement (Gaudi::DataHandle::Mode mode,
                              CLID clid,
                              const std::string& decorKey)
{
  throw SG::ExcBadDecorElement (mode, clid, decorKey);
}


} // namespace SG
