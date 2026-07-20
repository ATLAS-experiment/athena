/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ARRAY_FLOAT_3_H
#define ARRAY_FLOAT_3_H

namespace xAOD {

  /// @brief A struct mimicking std::array<float ,3>
  /// this structure is a temporary solution for our dynamic variables.
  /// There is an issue with ROOT's handling of std::vector<std::array<T, N> >,
  /// followed in https://github.com/root-project/root/issues/12007, that prevents
  /// us from using std::vector< std::array<float, 3> > for dynamic variables.
  /// This structure bypass the issue.

  struct ArrayFloat3 {
    float* data() { return &values[0]; }
    const float* data() const { return &values[0]; }
    float& operator[](const std::size_t idx) { return values[idx]; }
    const float& operator[](const std::size_t idx) const { return values[idx]; }
    float values[3];
  };

}

#endif
