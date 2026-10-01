/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#ifndef ACCESSORHELPERS_H
#define ACCESSORHELPERS_H

#include <AthContainers/Accessor.h>
#include <Math/Vector4D.h>
#include <TMath.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace CP {

/**
 * @brief Struct to manage accessors to objects.
 *
 * PartonAccessor provides methods to initialize, retrieve, and apply accessors
 * for various properties like mass, pt, eta, phi, and PDG ID. It manages both
 * float and integer accessors.
 */
struct PartonAccessor {
  std::map<std::string, std::unique_ptr<SG::Accessor<float>>> floatAccessors;
  std::map<std::string, std::unique_ptr<SG::Accessor<int>>> intAccessors;

  /**
   * @brief Initialize a float accessor by name.
   *
   * @param name The name of the float accessor to initialize.
   */
  void initializeFloatAccessor(const std::string& name) {
    floatAccessors[name] = std::make_unique<SG::Accessor<float>>(name);
  }

  /**
   * @brief Initialize multiple float accessors by a list of names.
   *
   * @param names A vector of accessor names.
   */
  void initializeFloatAccessor(const std::vector<std::string>& names) {
    for (const auto& name : names) {
      initializeFloatAccessor(name);
    }
  }

  /**
   * @brief Initialize an integer accessor by name.
   *
   * @param name The name of the integer accessor to initialize.
   */
  void initializeIntAccessor(const std::string& name) {
    intAccessors[name] = std::make_unique<SG::Accessor<int>>(name);
  }

  /**
   * @brief Initialize multiple integer accessors by a list of names.
   *
   * @param names A vector of accessor names.
   */
  void initializeIntAccessor(const std::vector<std::string>& names) {
    for (const auto& name : names) {
      initializeIntAccessor(name);
    }
  }

  /**
   * @brief Initialize a PtEtaPhiMVector using floats
   *
   * @param prefix A prefix for the name of the PtEtaPhiMvector.
   */
  void initializePtEtaPhiMAccessor(const std::string& prefix) {
    initializeFloatAccessor(prefix + "_pt");
    initializeFloatAccessor(prefix + "_eta");
    initializeFloatAccessor(prefix + "_phi");
    initializeFloatAccessor(prefix + "_m");
  }

  /**
   * @brief Template to get an Float via the name of an accessor.
   *
   * @param name The name of the Float accessor.
   * @param object The object to access.
   */
  template <typename T>
  float getFloat(const std::string& name, T* object) {
    return (*floatAccessors.at(name))(*object);
  }

  /**
   * @brief Template to get an Int via the name of an accessor.
   *
   * @param name The name of the Int accessor.
   * @param object The object to access.
   */
  template <typename T>
  int getInt(const std::string& name, T* object) {
    return (*intAccessors.at(name))(*object);
  }

  /**
   * @brief Template to get a PtEtaPhiMVector via the name of an accessor.
   *
   * @param prefix The prefix of the accessor holding the different components.
   * @param p The PtEtaPhiVector to fill with pT, eta, phi and m values.
   * @param object The object to access.
   */
  template <typename T>
  void getPtEtaPhiMVector(const std::string& prefix,
                          ROOT::Math::PtEtaPhiMVector& p, T* object) {
    p.SetCoordinates(
        getFloat(prefix + "_pt", object), getFloat(prefix + "_eta", object),
        getFloat(prefix + "_phi", object), getFloat(prefix + "_m", object));
  }
};

}  // namespace CP

#endif  // ACCESSORHELPERS_H
