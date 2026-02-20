/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STOREGATE_READCONDHANDLEKEY_H
#define STOREGATE_READCONDHANDLEKEY_H

#include "StoreGate/CondHandleKey.h"
#include <concepts>
#include <string>


namespace SG {

  template <class T>
  class ReadCondHandle;

  template <class T>
  class ReadCondHandleKey
    : public CondHandleKey<T>
  {      
  public:
    
    friend class ReadCondHandle<T>;
        
    explicit ReadCondHandleKey (const std::string& key, const std::string& dbKey="") :
      CondHandleKey<T>(key, dbKey, Gaudi::DataHandle::Reader)
    {}    


  /**
   * @brief auto-declaring Property Constructor.
   * @param name name of the Property
   * @param key The StoreGate key for the object
   * @param doc documentation string
   *
   * will associate the named Property with this RHK via declareProperty
   *
   * The provided key may actually start with the name of the store,
   * separated by a "+":  "MyStore+Obj".  If no "+" is present
   * the store named by @c storeName is used.   
   */
  template <std::derived_from<IProperty> OWNER>
  inline ReadCondHandleKey( OWNER* owner,
                            std::string name,
                            const std::string& key={},
                            std::string doc="") :
    ReadCondHandleKey<T>( key ) {
    auto p = owner->declareProperty(std::move(name), *this, std::move(doc));
    p->template setOwnerType<OWNER>();
  }


  /**
   * @brief Change the key of the object to which we're referring.
   * @param sgkey The StoreGate key for the object.
   *
   * The provided key may actually start with the name of the store,
   * separated by a "+":  "MyStore+Obj".  If no "+" is present,
   * the store is not changed.
   */
    inline ReadCondHandleKey& operator= (const std::string& sgkey) {
      VarHandleKey::operator= (sgkey);
      return *this;
    }

};


} // namespace SG

#endif // not STOREGATE_READCONDHANDLEKEY_H
