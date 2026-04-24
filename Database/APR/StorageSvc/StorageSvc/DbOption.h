/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbOption class definitions
//--------------------------------------------------------------------
//
//  Package    : StorageSvc  (The POOL project)
//  @author      M.Frank
//====================================================================
#ifndef POOL_DBOPTION_H
#define POOL_DBOPTION_H 1

// Framework include files
#include "StorageSvc/DbColumn.h"
#include "GaudiKernel/StatusCode.h"

/*
 *  POOL namespace declaration
 */
namespace pool  {

  /** @class DbOption DbOption.h StorageSvc/DbOption.h
    *
    * Description:
    * Definition an option to be supplied to database objects.
    *
    * Note:
    * For any pointer argument, values are not copied. The values
    * must outlive the lifetime of the DbOption.
    *
    * @author  M.Frank
    * @version 1.0
    */
  class DbOption {
  public:

    union Value  {
      long long int     val_long;
      int               val_int;
      double            val_double;
      void*             val_pvoid;
      char*             val_pchar;
    };

    /// Buffer holding option value
    Value             m_value {};
    /// Option data type
    DbColumn::Type    m_type;
    /// Option name identifier
    std::string       m_name;
    /// Optional identifier
    std::string       m_opt;

  public:
    /// Initializing constructor 
    template <class T> DbOption(const std::string& nam, 
                                const std::string& opt, 
                                T value)
    : m_type(DbColumn::UNKNOWN), m_name(nam), m_opt(opt)
    { i_setValue(typeid(T), &value).ignore(); }

    /// Initializing constructor with type definition
    DbOption(const std::string& nam, const std::string& opt="")
    : m_type(DbColumn::UNKNOWN), m_name(nam), m_opt(opt)
    { }

    /// Access to column name
    const std::string& option() const   { return m_opt;     }
    /// Access to column name
    const std::string& name() const     { return m_name;    }
    /// Integer type identifier
    DbColumn::Type type() const         { return m_type;    }

    /// Set the option value
    template<class T> StatusCode setValue(T value)
    { return i_setValue(typeid(T), &value); }

    /// Read the option value
    template<class T> StatusCode getValue(T& value) const
    { return i_getValue(typeid(T), &value); }

    /// Set the option value
    StatusCode i_setValue(const std::type_info& typ, const void* value);  

    /// Read the option value
    StatusCode i_getValue(const std::type_info& typ, void* value) const;

    /// Access to OS independent type name
    std::string typeName() const { return DbColumn::typeName(m_type); }
  };
}       // End namespace pool
#endif  // POOL_DbOption_H
