/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STOREGATE_READMETAHANDLEKEY_H
#define STOREGATE_READMETAHANDLEKEY_H

#include "StoreGate/MetaHandleKey.h"
#include <concepts>
#include <string>

namespace SG {

  template <class T> class ReadMetaHandle;
  
  template <class T> class ReadMetaHandleKey
    : public MetaHandleKey<T>
    {      
    public:
      friend class ReadMetaHandle<T>;
        
      explicit ReadMetaHandleKey (const std::string& key
			 , const std::string& dbKey="")
	: MetaHandleKey<T>(key, dbKey, Gaudi::DataHandle::Reader)
	{}   

    template <std::derived_from<IProperty> OWNER>
	inline ReadMetaHandleKey( OWNER* owner
				  , std::string name
                  , const std::string& key={}
				  , std::string doc="") 
	: ReadMetaHandleKey<T>( key ) {
	auto p = owner->declareProperty(std::move(name), *this, std::move(doc));
	p->template setOwnerType<OWNER>();
      }

    inline ReadMetaHandleKey& operator= (const std::string& sgkey) {
      VarHandleKey::operator= (sgkey);
      return *this;
    }

    };

} // namespace SG

#endif // not STOREGATE_READMETAHANDLEKEY_H
