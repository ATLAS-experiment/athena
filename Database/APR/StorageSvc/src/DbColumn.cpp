/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "StorageSvc/pool.h"
#include "StorageSvc/DbColumn.h"
// C++ header files

#include <cstdio>
#include <array>
#include <string_view>
#include <charconv>

using namespace std;

/// Initializing constructor for composite columns
DbColumn::DbColumn( const string& nam,
                    const string& clazz,
                    int   object_type,
                    int   object_offset)
: m_typeName(clazz),
  m_colName(nam), 
  m_offset(object_offset),
  m_nElem(1),
  m_opts(0),
  m_size(0),
  m_type(object_type)
{
}

/// Initializing constructor for fixed size arrays
DbColumn::DbColumn( const string& nam, 
                    int object_type,
                    int object_offset,
                    int object_size,
                    int number_of_elements,
                    int column_opts)
: m_colName(nam), 
  m_offset(object_offset),
  m_nElem(number_of_elements),
  m_opts(column_opts),
  m_size(object_size),
  m_type(object_type)
{
  m_typeName = typeName(m_type);
}

/// Create persistable description
const string DbColumn::toString() const   {
  char txt[4096];
  ::sprintf(txt,"{NAME=%s}{CLASS=%s}{TYP=%d}{OPT=%d}{OFF=%d}{SIZ=%d}{CNT=%d}",
      m_colName.c_str(),
      m_typeName.c_str(),
      m_type,
      m_opts,
      m_offset,
      m_size,
      m_nElem);
  return txt;
}


StatusCode DbColumn::fromString( const string& string_rep)  {
  using namespace std::literals;
  static constexpr std::array keys{
    "{NAME="sv, 
    "{CLASS="sv,
    "{TYP="sv,
    "{OPT="sv, 
    "{OFF="sv, 
    "{SIZ="sv,
    "{CNT="sv
  };
  static constexpr std::array intMembers{
    &DbColumn::m_type,
    &DbColumn::m_opts,
    &DbColumn::m_offset,
    &DbColumn::m_size,
    &DbColumn::m_nElem
  };
  m_colName = typeName(UNKNOWN);
  std::string_view tmp = string_rep;
  std::size_t nread = 0;
  for (std::size_t i = 0; i < keys.size(); ++i) {
    const auto pos = tmp.find(keys[i]);
    if (pos == std::string_view::npos) {
      continue;
    }
    tmp.remove_prefix(pos + keys[i].size());
    const std::size_t end = tmp.find('}');
    if (end == std::string_view::npos) {
      continue;
    }
    const auto value = tmp.substr(0, end);
    switch(i)   {
      case 0:
        m_colName = value;
        ++nread;
        break;
      case 1:
        m_typeName = value;
        ++nread;
        break;
      default:{
        int& target = this->*intMembers[i - 2];
        const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(),
          target);
        if (ec == std::errc{} && ptr == value.data() + value.size()) {
          ++nread;
        }
      }
      break;
    }
    tmp.remove_prefix(end + 1);
  }
  
  return nread == keys.size() ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

#define _NAME_(x) #x

// Access the type name by typeID
const string DbColumn::typeName(int typ)    {
  switch(typ)   {
  case UCHAR:
    return _NAME_(unsigned char);
  case USHORT:
    return _NAME_(unsigned short);
  case UINT:
    return _NAME_(unsigned int);
  case ULONG:
    return _NAME_(unsigned long);
  case CHAR:
    return _NAME_(char);
  case SHORT:
    return _NAME_(short);
  case INT:
    return _NAME_(int);
  case LONG:
    return _NAME_(long);
  case BOOL:
    return _NAME_(bool);
  case FLOAT:
    return _NAME_(float);
  case DOUBLE:
    return _NAME_(double);
  case LONGLONG:
    return _NAME_(long long);
  case ULONGLONG:
    return _NAME_(unsigned long long);
  case STRING:
  case LONG_STRING:
    return "string"; //_NAME_(string);
  case NTCHAR:
  case LONG_NTCHAR:
    return _NAME_(char*);
  case TOKEN:
    return "Token";
  case BLOB:
    return "Blob";
  case ANY:
    return "Any";
  case OBJECT_REF:
    return "OBJECT_REF";
  case CONTAINED_REF:
    return "CONTAINED_REF";
  case UNKNOWN:
  default:
    return "UNKNOWN";
  }
}
