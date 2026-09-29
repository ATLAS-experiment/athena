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

/// Decorates a 0/1 flag from key existence: 1 if any of retrievalKeys is
/// present in the particle map, 0 otherwise. No kinematic values are set.
struct IsOnShellFillOp {
  std::vector<std::string>
      retrievalKeys;          ///< particle map keys to check (any match)
  std::string decorationKey;  ///< output decoration name (without prefix)
};

/// Maps to an existing Fill*PartonHistory method on CalcPartonHistory.
enum class SpecialFillType {
  Top,          ///< FillTopPartonHistory
  TopFCNC,      ///< FillTopPartonHistory(true)
  AntiTop,      ///< FillAntiTopPartonHistory
  AntiTopFCNC,  ///< FillAntiTopPartonHistory(true)
  Ttbar,        ///< FillTtbarPartonHistory
  TtbarFCNC,    ///< pass
  Z,            ///< FillZPartonHistory
  Ztautau,      ///< FillZtautauPartonHistory
  W,            ///< FillWPartonHistory
  Higgs,        ///< FillHiggsPartonHistory
  Gamma,        ///< FillGammaPartonHistory
};

/// Parameterizes one special fill call.
struct SpecialFillOp {
  SpecialFillType type {};
  std::string parent = "";        ///< for Z/W: parent string arg
  std::string mode = "resonant";  ///< for Z/W/H: mode string arg
  int count = 1;                  ///< for Z/W: nZs or nWs
};

/// Maps to an existing Initialize*Decorators method (non-parameterized ones).
enum class DecoratorGroup {
  Top,               ///< InitializeTopDecorators()
  TopFCNC,           ///< InitializeTopDecorators(true)
  AntiTop,           ///< InitializeAntiTopDecorators()
  AntiTopFCNC,       ///< InitializeAntiTopDecorators(true)
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
  std::vector<IsOnShellFillOp>
      isOnShellFills;  ///< existence-flag decorations, filled after genericFills
};

/// Returns the configuration for a given scheme name.
/// Throws std::runtime_error if the scheme is not registered.
const PartonSchemeConfig& getSchemeConfig(const std::string& schemeName);

}  // namespace CP

#endif  // PARTONS_PARTONSCHEMECONFIG_H
