/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VP1RAWDATAHANDLE_RPC_DIGIT_H
#define VP1RAWDATAHANDLE_RPC_DIGIT_H

#include "VP1RawDataSystems/VP1RawDataHandleBase.h"
class RpcDigit;
namespace MuonGM { class RpcReadoutElement; }

class VP1RawDataHandle_RPC_Digit : public VP1RawDataHandleBase {
public:
  VP1RawDataHandle_RPC_Digit(VP1RawDataCollBase*, const RpcDigit*);
  bool measuresPhi() const;
  bool oppositeSide() const;
  double time() const;
  double timeOverThreshold() const;
  QStringList clicked(bool verbose) const override;
protected:
  SoNode* buildShape() override;
  SoTransform* buildTransform() override;
private:
  const MuonGM::RpcReadoutElement* element() const;
  const RpcDigit* m_data;
};

#endif
