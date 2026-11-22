/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_DEMOSCHEMA_H
#define ATHEXREMOTEEXEC_DEMOSCHEMA_H

/**
 * @file DemoSchema.h
 * @brief The demonstration implementing payload conversions between proto and
 * plain C++.
 */

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace AthRemoteExec::Example::DemoSchema {

/// @name Schema names as they appear in Payload::schema
/// @{
extern const char* const ints;     ///< athexremoteexec.demo.v1.Ints
extern const char* const doubles;  ///< athexremoteexec.demo.v1.Doubles
/// @}

/// One entry of an Ints message: a StoreGate key and its value.
struct NamedInt {
  std::string name;
  int64_t value = 0;
};

/// @return false on failure, with @c error set.
bool encodeInts(const std::vector<NamedInt>& values, std::string& bytes,
                std::string& error);
bool decodeInts(std::string_view bytes, std::vector<NamedInt>& values,
                std::string& error);

bool encodeDoubles(const std::string& name, const std::vector<double>& values,
                   std::string& bytes, std::string& error);
bool decodeDoubles(std::string_view bytes, std::string& name,
                   std::vector<double>& values, std::string& error);

}  // namespace AthRemoteExec::Example::DemoSchema

#endif  // ATHEXREMOTEEXEC_DEMOSCHEMA_H
