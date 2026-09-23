/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCPatchPanelOut_hh
#define TGCPatchPanelOut_hh

#include "TrigT1TGC/TGCHitPattern.h"
#include "MuonDigitContainer/TgcDigit.h"
#include <memory>
#include <array>

namespace LVL1TGCTrigger {

constexpr int NumberOfConnectorPerPPOut = 2;

class TGCPatchPanel;

class TGCPatchPanelOut {
 public:
  TGCPatchPanelOut() = default;
  ~TGCPatchPanelOut() = default;
  void swap(TGCPatchPanelOut& other) noexcept;
  TGCPatchPanelOut(const TGCPatchPanelOut& right);
  TGCPatchPanelOut& operator=(const TGCPatchPanelOut& right);
  
  TGCPatchPanelOut(TGCPatchPanelOut&&) noexcept = default;
  TGCPatchPanelOut& operator=(TGCPatchPanelOut&&) noexcept = default;

  const TGCHitPattern* getHitPattern(int connector) const;
  TGCHitPattern* getHitPattern(int connector);
  void setHitPattern(int connector, int nCh);

  const TGCPatchPanel* getOrigin() const;
  void setOrigin(const TGCPatchPanel* pp);

  int getBid() const;
  void setBid(const int bidIn);
  
  void print() const;
  void deleteHitPattern(int i);

 private:
  int m_bid{TgcDigit::BC_UNDEFINED};        ///< bunch ID number
  const TGCPatchPanel* m_origin{nullptr};   ///< pointer to Patch Panel generate this PatchPanelOut
  std::array<std::unique_ptr<TGCHitPattern>, NumberOfConnectorPerPPOut> m_signalPattern{};
};


inline
const TGCHitPattern* TGCPatchPanelOut::getHitPattern(int connector) const
{
  return m_signalPattern[connector].get();
}

inline
TGCHitPattern* TGCPatchPanelOut::getHitPattern(int connector)
{
  return m_signalPattern[connector].get();
}

inline
const TGCPatchPanel* TGCPatchPanelOut::getOrigin() const
{
  return m_origin;
}

inline
void TGCPatchPanelOut::setBid(const int bidIn) 
{
  m_bid = bidIn; 
}

inline
void TGCPatchPanelOut::setHitPattern(int connector, int nCh)
{
  m_signalPattern[connector] = std::make_unique<TGCHitPattern>(nCh);
}

inline
int TGCPatchPanelOut::getBid() const
{
  return m_bid;
}

inline
void TGCPatchPanelOut::setOrigin(const TGCPatchPanel* pp)
{
  m_origin = pp;
}


} //end of namespace bracket

#endif // TGCPatchPanelOut_hh





