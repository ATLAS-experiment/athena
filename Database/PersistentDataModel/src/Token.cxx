/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PersistentDataModel/Token.h"

#include <cstdio>
#include <cstring>
#include <climits>
#include <atomic>
#include <format>
#include <charconv>
#include <string_view>

constexpr std::string_view LABEL_DB   = "[DB=";
constexpr std::string_view LABEL_CNT  = "[CNT=";
constexpr std::string_view LABEL_CLID = "[CLID=";
constexpr std::string_view LABEL_TECH = "[TECH=";
constexpr std::string_view LABEL_OID  = "[OID=";

static const int KEY_MASK = (~0u) << CHAR_BIT;
static std::atomic<int> s_numCount { 0 };

int Token::numInstances() { return s_numCount; }

/// Standard Constructor(1): Empty constructor
Token::Token() : m_refCount(1),
	m_technology(0),
	m_dbID(Guid::null()),
	m_classID(Guid::null()),
	m_oid(OID_t(~0x0LL, ~0x0LL)),
	m_type(0) {
   s_numCount.fetch_add(1, std::memory_order_relaxed);
}

/// Copy constructor
Token::Token(const Token& copy) : m_refCount(1),
	m_technology(copy.m_technology),
	m_dbID(copy.m_dbID),
	m_classID(copy.m_classID),
	m_oid(copy.m_oid),
	m_type(0) {
   copy.setData(this);
   s_numCount.fetch_add(1, std::memory_order_relaxed);
}

/// Copy constructor
Token::Token(const Token* source) : m_refCount(1),
	m_technology(0),
	m_dbID(Guid::null()),
	m_classID(Guid::null()),
	m_oid(OID_t(~0x0LL, ~0x0LL)),
	m_type(0) {
   if (source != 0) {
      source->setData(this);
   }
   s_numCount.fetch_add(1, std::memory_order_relaxed);
}

/// Move constructor.
Token::Token(Token&& source) noexcept
  : m_refCount (1),
    m_technology (source.m_technology),
    m_dbID (std::move (source.m_dbID)),
    m_cntID (std::move (source.m_cntID)),
    m_classID (std::move (source.m_classID)),
    m_oid (std::move (source.m_oid)),
    m_type (source.m_type),
    m_auxString (std::move (source.m_auxString))
{
   s_numCount.fetch_add(1, std::memory_order_relaxed);
}
    

Token::~Token() {
   s_numCount.fetch_sub(1, std::memory_order_relaxed);
}

/// Release token: Decrease reference count and eventually delete.
int Token::release() {
   int cnt = --m_refCount;
   if (0 >= cnt) {
      delete this;
   }
   return cnt;
}

/// Assignment operator
Token& Token::operator=(const Token& copy) {
  if (&copy != this) {
     copy.setData(this);
  }
  return *this;
}

/// Equality operator
bool Token::equal(const Token& copy) const {
   if (&copy != this) {
      if (m_oid.second == copy.m_oid.second) {
         if (m_classID == copy.m_classID) {
            if (m_dbID == copy.m_dbID) {
               if (m_cntID == copy.m_cntID) {
                  return true;
               }
            }
         }
      }
      return false;
   }
   return true;
}

/// Operator to allow ordering
bool Token::less(const Token& copy) const {
   if (&copy != this) {
      if (m_oid.second < copy.m_oid.second)
         return true;
      else if (m_oid.second > copy.m_oid.second)
         return false;
      if (!(m_classID == copy.m_classID)) {
         return (m_classID < copy.m_classID);
      }
      if (!(m_dbID == copy.m_dbID)) {
         return (m_dbID < copy.m_dbID);
      }
      int res = m_cntID.compare(copy.m_cntID);
      if (res != 0) {
         return (res < 0);
      }
   }
   return false;
}

const std::string Token::toString() const {
   return std::format(
      "[DB={}][CNT={}][CLID={}][TECH={:08X}][OID={:016X}-{:016X}]{}",
      m_dbID.to_fixed_string(),
      m_cntID,
      m_classID.to_fixed_string(),
      m_technology,
      static_cast<uint64_t>(m_oid.first),
      static_cast<uint64_t>(m_oid.second),
      m_auxString
   );
}

Token& Token::fromString(const std::string_view src)    {
   m_auxString.clear();
   size_t pos = 0;
   while (pos < src.size()) {
      size_t start = src.find('[', pos);
      if (start == std::string_view::npos) break;
      size_t eq = src.find('=', start);
      size_t end = src.find(']', start);
      if (eq != std::string_view::npos && end != std::string_view::npos) {
         std::string_view label = src.substr(start, eq - start + 1);
         if (label == LABEL_DB) {
               m_dbID.fromString(src.substr(eq + 1, end - eq - 1));
         } else if (label == LABEL_CNT) {
               m_cntID = std::string(src.substr(eq + 1, end - eq - 1));
         } else if (label == LABEL_CLID) {
               m_classID.fromString(src.substr(eq + 1, end - eq - 1));
         } else if (label == LABEL_TECH) {
               std::string_view num_str = src.substr(eq + 1, end - eq - 1);
               int tech = 0;
               std::from_chars(num_str.data(), num_str.data() + num_str.size(), tech, 16);
               m_technology = tech;
         } else if (label == LABEL_OID) {
               std::string_view oid_str = src.substr(eq + 1, end - eq - 1);
               size_t dash = oid_str.find('-');
               if (dash != std::string_view::npos) {
                  std::string_view first_str = oid_str.substr(0, dash);
                  std::string_view second_str = oid_str.substr(dash + 1);

                  // Check if this is legacy format (8 digits) vs modern format (16 digits)
                  // Legacy format: 5 + 8 + 1 + 8 + 1 = 23 characters total for [OID=XXXXXXXX-XXXXXXXX]
                  // Modern format: 5 + 16 + 1 + 16 + 1 = 39 characters total for [OID=XXXXXXXXXXXXXXXX-XXXXXXXXXXXXXXXX]
                  bool is_legacy = (end - start + 1) == 23; // Total bracket length check

                  uint64_t first = 0;
                  uint64_t second = 0;
                  std::from_chars(first_str.data(), first_str.data() + first_str.size(), first, 16);
                  std::from_chars(second_str.data(), second_str.data() + second_str.size(), second, 16);

                  if (is_legacy) {
                     // Handle legacy format: extend 32-bit ~0x0 to 64-bit ~0x0LL
                     if (static_cast<uint32_t>(first) == ~0x0U) first = ~0x0ULL;
                     if (static_cast<uint32_t>(second) == ~0x0U) second = ~0x0ULL;
                  }

                  m_oid.first = static_cast<long long int>(first);
                  m_oid.second = static_cast<long long int>(second);
               }
         } else {
               m_auxString += src.substr(start, end - start + 1);
         }
         pos = end + 1;
      } else {
         break;
      }
   }
   return *this;
}

/// Retrieve the string representation of the token.
const std::string Token::key() const {
   return std::format(
      "[DB={}][CNT={}][CLID={}][TECH={:08X}]",
      m_dbID.to_fixed_string(),
      m_cntID,
      m_classID.to_fixed_string(),
      m_technology & KEY_MASK
   );
}

const Token& Token::set(Token* pToken) const {
   pToken->m_technology = m_technology;
   pToken->m_dbID = m_dbID;
   pToken->m_cntID = m_cntID;
   pToken->m_classID = m_classID;
   pToken->m_oid.first = m_oid.first;
   return *this;
}

const Token& Token::setData(Token* pToken) const {
   this->set(pToken);
   pToken->m_oid.second = m_oid.second;
   pToken->m_type = m_type;
   pToken->m_auxString = m_auxString;
   return *this;
}
