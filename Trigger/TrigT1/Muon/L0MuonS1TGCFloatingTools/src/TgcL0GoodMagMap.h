/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MUONS1TGCFLOATINGTOOLS_TGCL0GOODMAGMAP_H
#define L0MUONS1TGCFLOATINGTOOLS_TGCL0GOODMAGMAP_H

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace L0Muon {

/** @brief Immutable sparse GoodMag map loaded from ASCII. */
class TgcL0GoodMagMap {
 public:
  static std::unique_ptr<TgcL0GoodMagMap> loadAscii(
      const std::string& calibrationPath, std::string& error);

  /** @brief Return false for a masked or out-of-range calibration bin. */
  bool isGood(int etaBin, int phiFoldBin) const;

  const std::string& version() const { return m_version; }
  unsigned int etaBins() const { return m_etaBins; }
  unsigned int phiBinsPerFold() const { return m_phiBinsPerFold; }
  std::size_t poorBinCount() const { return m_poorBinCount; }

 private:
  TgcL0GoodMagMap() = default;

  std::string m_version{};
  unsigned int m_etaBins{0U};
  unsigned int m_phiBinsPerFold{0U};
  std::vector<bool> m_poorBins{};
  std::size_t m_poorBinCount{0U};
};

}  // namespace L0Muon

#endif
