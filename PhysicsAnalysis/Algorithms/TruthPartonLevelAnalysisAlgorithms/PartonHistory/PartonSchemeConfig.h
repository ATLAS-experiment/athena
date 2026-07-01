/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef PARTONS_PARTONSCHEMECONFIG_H
#define PARTONS_PARTONSCHEMECONFIG_H

#include <string>
#include <vector>

namespace CP {

/// One call to FillGenericPartonHistory or FillGenericVectorPartonHistory.
struct GenericFillOp {
  std::vector<std::string>
      retrievalKeys;          ///< particle map keys to try (in order)
  std::string decorationKey;  ///< output decoration name (without prefix)
  int idx = 0;                ///< occurrence index in the particle map vector
  bool isVector = false;      ///< true → use FillGenericVectorPartonHistory
};

/// Maps to an existing Fill*PartonHistory method on CalcPartonHistory.
enum class SpecialFillType {
  Top,      ///< FillTopPartonHistory
  AntiTop,  ///< FillAntiTopPartonHistory
  Ttbar,    ///< FillTtbarPartonHistory
  Z,        ///< FillZPartonHistory(history, parent, dec, count, mode)
  Ztautau,  ///< FillZtautauPartonHistory(history, parent, dec, count, mode)
  W,        ///< FillWPartonHistory(history, parent, dec, count, mode)
  Higgs,    ///< FillHiggsPartonHistory(history, mode, dec)
  Gamma,    ///< FillGammaPartonHistory(history, parent, dec)
};

/// Parameterizes one special fill call.
struct SpecialFillOp {
  SpecialFillType type {};
  std::string parent = "";        ///< for Z/W/Gamma: parent string arg
  std::string mode = "resonant";  ///< for Z/W/H: mode string arg
  int count = 1;                  ///< for Z/W: nZs or nWs
  bool extended = false;  ///< for Z: whether to include tau decay products
};

/// Maps to an existing Initialize*Decorators method (non-parameterized ones).
enum class DecoratorGroup {
  Top,               ///< InitializeTopDecorators()
  AntiTop,           ///< InitializeAntiTopDecorators()
  FourTop,           ///< Initialize4TopDecorators()
  Ttbar,             ///< InitializeTtbarDecorators()
  Bottom,            ///< InitializeBottomDecorators()
  AntiBottom,        ///< InitializeAntiBottomDecorators()
  VectorBottom,      ///< InitializeVectorBottomDecorators()
  VectorAntiBottom,  ///< InitializeVectorAntiBottomDecorators()
  Charm,             ///< InitializeCharmDecorators()
  AntiCharm,         ///< InitializeAntiCharmDecorators()
  VectorCharm,       ///< InitializeVectorCharmDecorators()
  VectorAntiCharm,   ///< InitializeVectorAntiCharmDecorators()
  Photon,            ///< InitializePhotonDecorators()
  Higgs,             ///< InitializeHiggsDecorators()
};

/// Parameterized Z or W decorator initialisation (needs count/extended args).
struct DecoratorZW {
  enum Type { Z, W } type {Z};
  int count = 1;
  bool extended = false;  ///< only relevant for Z
};

/// Top-level configuration for a named parton history scheme.
struct PartonSchemeConfig {
  std::vector<std::string> truthCollections;  ///< truth containers to merge
  std::vector<DecoratorGroup>
      decoratorGroups;  ///< non-parameterized decorator groups
  std::vector<DecoratorZW>
      decoratorZWs;  ///< parameterized Z/W decorator groups
  std::vector<SpecialFillOp>
      specialFills;  ///< calls to dedicated Fill* methods
  std::vector<GenericFillOp>
      genericFills;  ///< calls to FillGenericPartonHistory
};

/// Returns the configuration for a given scheme name.
/// Throws std::runtime_error if the scheme is not registered.
const PartonSchemeConfig& getSchemeConfig(const std::string& schemeName);

}  // namespace CP

#endif  // PARTONS_PARTONSCHEMECONFIG_H
