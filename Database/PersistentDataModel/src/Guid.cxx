/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PersistentDataModel/Guid.h"

#include <iostream>

#include "uuid/uuid.h"

//{ 0x0,0x0,0x0,{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}};
static constexpr Guid clid_null("00000000-0000-0000-0000-000000000000");

const Guid& Guid::null() noexcept {
   return clid_null;
}

const Guid::GuidGenMethod Guid::m_guidGenMethod = Guid::initGuidGenMethod();

Guid::GuidGenMethod Guid::initGuidGenMethod() {
   char* envv = getenv("POOL_GUID_TIME");
   if (envv != 0 && *envv) return GuidGenByTime;
   envv = getenv("POOL_GUID_RANDOM");
   if (envv != 0 && *envv) return GuidGenRandom;
   return GuidGenDefault;
}

/// Create a new Guid
void Guid::create(Guid& guid, GuidGenMethod method) {
   uuid_t me_;
   if (method == GuidGenDefault) method = m_guidGenMethod;
   switch(method) {
    case GuidGenRandom:
      ::uuid_generate(me_);
      break;
    case GuidGenByTime:
      ::uuid_generate_time(me_);
      break;
    default:
      ::uuid_generate(me_);
      break;
   }
   unsigned int *d1 = (unsigned int*)me_;
   unsigned short *d2 = (unsigned short*)(me_ + 4);
   unsigned short *d3 = (unsigned short*)(me_ + 6);
   guid.m_data1 = *d1;
   guid.m_data2 = *d2;
   guid.m_data3 = *d3;
   for (unsigned int i = 0; i < 8; i++) {
      guid.m_data4[i] = me_[i + 8];
   }
}

bool Guid::isGuid(std::string_view sv) noexcept{
    // The GUID must be exactly 36 characters long
    if (sv.size() != 36) {
        return false;
    }

    // Check for hyphens at specific positions (0-based indices: 8, 13, 18, 23)
    if (sv[8] != '-' || sv[13] != '-' || sv[18] != '-' || sv[23] != '-') {
        return false;
    }

    // Validate that all other characters are hexadecimal digits (0-9, a-f, A-F)
    for (size_t i = 0; i < 36; ++i) {
        // Skip hyphen positions
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            continue;
        }

        char c = sv[i];
        // Check if it's a valid hex digit
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return false;
        }
    }
    return true;
}

bool Guid::operator==(std::string_view str) const {
   return str.size() == 36 && *this == Guid(str);
}


std::ostream& operator<<(std::ostream& os, const Guid& rhs) {
  std::array<char, 36> buff;
  rhs.toString(buff);
  os.write(buff.data(), 36);
  return os; 
}

