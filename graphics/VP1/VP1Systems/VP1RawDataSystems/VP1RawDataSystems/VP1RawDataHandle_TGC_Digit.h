/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VP1RAWDATAHANDLE_TGC_DIGIT_H
#define VP1RAWDATAHANDLE_TGC_DIGIT_H

#include "VP1RawDataSystems/VP1RawDataHandleBase.h"

class TgcDigit;
namespace MuonGM { class TgcReadoutElement; }

class VP1RawDataHandle_TGC_Digit : public VP1RawDataHandleBase {
public:
  VP1RawDataHandle_TGC_Digit(VP1RawDataCollBase*, const TgcDigit*);
  QStringList clicked(bool verbose) const override;
protected:
  SoNode* buildShape() override;
  SoTransform* buildTransform() override;
private:
  const MuonGM::TgcReadoutElement* element() const;
  const TgcDigit* m_data;
};

#endif
