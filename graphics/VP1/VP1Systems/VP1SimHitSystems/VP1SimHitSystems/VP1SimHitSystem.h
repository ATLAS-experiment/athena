/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VP1SIMHITSYSTEM_H
#define VP1SIMHITSYSTEM_H

#include "VP1Base/IVP13DSystemSimple.h"

// fwd 
class CaloDetDescrElement;
class SoVertexProperty;

class VP1SimHitSystem: public IVP13DSystemSimple
{
  Q_OBJECT

 public:
  VP1SimHitSystem();
  ~VP1SimHitSystem();

  QWidget* buildController();

  void systemcreate(StoreGateSvc* detstore);
  void buildEventSceneGraph(StoreGateSvc* sg, SoSeparator *root);

  protected Q_SLOTS:
    void checkboxChanged();

 private:
  class Clockwork;
  Clockwork* m_clockwork;

  void buildHitTree(const QString& detector);
  void handleDetDescrElementHit(const CaloDetDescrElement *hitElement, SoVertexProperty* hitVtxProperty, unsigned int &hitCount);

  /**
   * @brief Helper function to get the global position of a SiHit item
   * @param[in] collName The name of the specific SiHit collection
   * @param[in] sg A pointer to the StoreGate
   * @param[in,out] hitVtxProperty To store the hit's positions in X,Y,Z
   * @param[in,out] hitCount A counter, which gets incremented according to the number of hits stored in the SiHit collection
   */
  void fillHitPositionsFromSiHitCollection(const std::string& collName, const StoreGateSvc* sg, SoVertexProperty* hitVtxProperty, unsigned int& hitCount);

};

#endif
