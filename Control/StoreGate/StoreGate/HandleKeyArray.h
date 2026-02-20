/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STOREGATE_HANDLEKEYARRAY_H
#define STOREGATE_HANDLEKEYARRAY_H 1

#include "StoreGate/VarHandleKeyArray.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include <vector>
#include <string>

namespace SG {

  /**
   * @class SG::HandleKeyArray<T>
   * @brief class to hold an array of HandleKeys
   *
   * since it inherits from std::vector, all vector operations are 
   * permitted.
   *
   * initialization can be done in four ways.
   * 1: with an std::vector<HandleKey> as a parameter
   *    SG::ReadHandleKeyArray<foo>  m_foo ( std::vector<ReadHandleKey>(...) );
   *    SG::WriteHandleKeyArray<foo> m_foo ( std::vector<WriteCondHandleKey>(...) );
   * 2: with an initializer list of HandleKeys
   *    SG::ReadHandleKeyArray<foo>  m_foo { ReadHandleKey<foo>(...), ReadHandleKey<foo> (...) };
   *    SG::WriteHandleKeyArray<foo> m_foo { WriteCondHandleKey<foo> (...), WriteCondHandleKey<foo> (...) };
   * 3: with an initializer list of std::strings, that will be used to
   *    internally create HandleKeys with those initializers
   *    SG::ReadHandleKeyArray<foo>  m_foo { "key1", "key2", "key3" };
   *    SG::WriteHandleKeyArray<foo> m_foo { "key1", "key2", "key3" };
   * 4: For decorations only: with a container handle and an initializer list
   *    of std::strings.
   *    The strings will be used to internally create HandleKeys, all with
   *    the same container key.
   *    SG::ReadHandleKey<foo> m_fooCont ("foo");
   *    SG::ReadDecorHandleKeyArray<foo>  m_foo { m_fooCont, {"key1", "key2", "key3"} };
   *    SG::WriteDecorHandleKeyArray<foo> m_foo { m_fooCont, {"key1", "key2", "key3"} };
   */

  template <class T_Handle, class T_HandleKey, Gaudi::DataHandle::Mode MODE>
  class HandleKeyArray : public VarHandleKeyArrayCommon< T_HandleKey > {
  public:
    /**
     * @brief default Constructor from a HandleKeyArray
     */
    HandleKeyArray(){}

    /**
     * @brief Constructor from a HandleKeyArray that takes a vector
     * of ReadHandleKeys
     * @param v vector of HandleKey
     */
    HandleKeyArray( const std::vector<T_HandleKey>& v ) :
      VarHandleKeyArrayCommon<T_HandleKey> ( v ) {}

    /**
     * @brief Constructor from a HandleKeyArray that takes an 
     * initializer list of HandleKeys
     * @param l initializer list of HandleKey
     */
    HandleKeyArray( std::initializer_list<T_HandleKey> l ):
      VarHandleKeyArrayCommon<T_HandleKey> {l} {}

    /**
     * @brief Constructor from a HandleKeyArray that takes an 
     * initializer list of std::strings.
     * @param l initializer list of std::strings used to create the
     *          HandleKeys
     */
    HandleKeyArray( std::initializer_list<std::string> key_names):
      VarHandleKeyArrayCommon<T_HandleKey> {key_names} {}

    /**
     * @brief base Constructor that takes an associated container and an
     * initializer list of std::strings.
     * @param contKey VarHandleKey of the associated container
     * @param l initializer list of std::strings used to create the
     *          VarHandleKeys
     *
     * All decorations will be read from the container referenced
     * by @contKey.
     */
    template <class T = T_HandleKey>
    requires T::isDecorHandleKey
    HandleKeyArray( VarHandleKey& contKey,
                    std::initializer_list<std::string> key_names ):
      VarHandleKeyArrayCommon<T_HandleKey> {contKey, key_names} {}

    /**
     * @brief auto-declaring Property Constructor from a HandleKeyArray 
     * that takes an initializer list of std::strings, and associates the WHKA
     * with the specified Property name
     * @param name name of Property
     * @param l initializer list of std::strings used to create the
     *          HandleKeys
     * @param doc documentation string
     */
    template <std::derived_from<IProperty> OWNER>
    inline HandleKeyArray( OWNER* owner,
                                std::string name,
                                std::initializer_list<std::string> l,
                                std::string doc="") :
      VarHandleKeyArrayCommon<T_HandleKey> {l} {
      auto p = owner->declareProperty(std::move(name), *this, std::move(doc));
      p->template setOwnerType<OWNER>();
    }


    /**
     * @brief auto-declaring Property Constructor from a HandleKeyArray
     * that takes an initializer list of std::strings, and associates the WHKA
     * with the specified Property name
     * @param name name of Property
     * @param contKey VarHandleKey of the associated container
     * @param l initializer list of std::strings used to create the
     *          HandleKeys
     * @param doc documentation string
     *
     * All decorations will be read from the container referenced
     * by @contKey.
     */
    template <std::derived_from<IProperty> OWNER, class T = T_HandleKey>
    requires T::isDecorHandleKey
    inline HandleKeyArray( OWNER* owner,
                           std::string name,
                           VarHandleKey& contKey,
                           std::initializer_list<std::string> l,
                           std::string doc="") :
      VarHandleKeyArrayCommon<T_HandleKey> {contKey, l} {
      auto p = owner->declareProperty(std::move(name), *this, std::move(doc));
      p->template setOwnerType<OWNER>();
    }


    /**
     * @brief return the type (Read/Write/Update) of handle
     */
    virtual Gaudi::DataHandle::Mode mode() const override { return MODE; }

    /**
     * @brief create a vector of Handles from the HandleKeys
     * in the array
     */
    std::vector< T_Handle > makeHandles() const {
      const EventContext& ctx = Gaudi::Hive::currentContext();
      std::vector< T_Handle > hndl;
      for (const T_HandleKey& k : *this) {
        hndl.emplace_back ( k, ctx );
      }
      return hndl;
    }

    /**
     * @brief create a vector of Handles from the HandleKeys
     * in the array, with explicit EventContext.
     */
    std::vector< T_Handle > makeHandles (const EventContext& ctx) const
    {
      std::vector< T_Handle > hndl;
      for (const T_HandleKey& k : *this) {
        hndl.emplace_back ( k, ctx);
      }
      return hndl;
    }

  };

} // namespace SG

#endif
