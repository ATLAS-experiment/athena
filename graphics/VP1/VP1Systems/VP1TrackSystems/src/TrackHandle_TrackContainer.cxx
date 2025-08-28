/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class TrackHandle_TrackContainer         //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: May 2008                                 //
//                                                            //
////////////////////////////////////////////////////////////////

#include "VP1TrackSystems/TrackHandle_TrackContainer.h"

#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "ActsEvent/TrackParameters.h"
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "VP1Base/VP1Msg.h"
#include "VP1TrackSystems/AscObj_TrackState.h"
#include "VP1TrackSystems/TrackCollHandle_TrackContainer.h"
#include "VP1TrackSystems/VP1TrackSanity.h"
#include "AthenaKernel/Units.h"
#include "ActsGeometry/ActsDetectorElement.h"
#include <typeinfo>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoNode.h>

//____________________________________________________________________
TrackHandle_TrackContainer::TrackHandle_TrackContainer(
    TrackCollHandleBase* ch, ActsTrk::TrackContainer::ConstTrackProxy tp,
    const ActsTrk::TrackContainer& container)
    : TrackHandleBase(ch), m_track(tp), m_container(container) {}

//____________________________________________________________________
TrackHandle_TrackContainer::~TrackHandle_TrackContainer() {}

//____________________________________________________________________
QStringList TrackHandle_TrackContainer::clicked() const {
  QStringList l;
  l << "TrackProxy:";
  l << TrackHandleBase::baseInfo();
  return l;
}

//____________________________________________________________________
Amg::Vector3D TrackHandle_TrackContainer::momentum() const {
  const Acts::BoundTrackParameters trackparams(
      m_track.referenceSurface().getSharedPtr(), m_track.parameters(),
      std::nullopt, Acts::ParticleHypothesis::pion());
  return trackparams.momentum()*1000.;
}

const std::vector<Amg::Vector3D>*
TrackHandle_TrackContainer::provide_pathInfoPoints() {
  ensureInitTrackStateCache();  
  auto trackContainerCollHandle =
      dynamic_cast<const TrackCollHandle_TrackContainer*>(collHandle());
  auto points = new std::vector<Amg::Vector3D>;

  auto ctx = trackContainerCollHandle->common()->geometryContext().context();
  for (auto trackstate : m_trackStates) {
    if (trackstate.hasSmoothed() && trackstate.hasReferenceSurface()) {
      const Acts::BoundTrackParameters params(
          trackstate.referenceSurface().getSharedPtr(), trackstate.smoothed(),
          trackstate.smoothedCovariance(), Acts::ParticleHypothesis::pion());
      points->push_back(params.position(ctx));
    }
  }

  return points;
}

//____________________________________________________________________
void TrackHandle_TrackContainer::visibleStateChanged() {
  TrackHandleBase::visibleStateChanged();
}

//____________________________________________________________________
void TrackHandle_TrackContainer::currentMaterialChanged() {}

void TrackHandle_TrackContainer::fillObjectBrowser(
    QList<QTreeWidgetItem*>& listOfItems) {
  TrackHandleBase::fillObjectBrowser(listOfItems);  // Obligatory!


  // Fill sub-data.
  QList<AssociatedObjectHandleBase*> list = getAllAscObjHandles();
  unsigned int listSize = list.size();

  ensureInitTrackStateCache();

  unsigned int trackStateNum=0;
  for (const auto trackState : m_trackStates) {
    QString surfaceText = QString(" ");
    surfaceText += trackState.hasReferenceSurface()?QString::number(trackState.referenceSurface().type()):"No Surface";

    // VP1Msg::messageVerbose("TrackState #"+QString::number(trackStateNum) + surfaceText);

    bool visible=false;
    QTreeWidgetItem* TSOSitem = new QTreeWidgetItem(browserTreeItem());
    
    AscObj_TrackState* asc=nullptr;
    if (trackStateNum<listSize) asc = dynamic_cast<AscObj_TrackState*>(list.at(trackStateNum));

    if (asc) {
      asc->setBrowserTreeItem(TSOSitem);
      visible=asc->visible();
    }
    
    if (!visible) {
      TSOSitem->setFlags(Qt::ItemFlag());// not selectable, not enabled
      QFont itemFont = TSOSitem->font(0);
      itemFont.setStrikeOut(true);
      TSOSitem->setFont(0, itemFont);
      TSOSitem->setFont(1, itemFont);
    } 
    std::ostringstream s;

    s << " | Flags: "<< trackState.typeFlags();
    QString text = QString::number( trackStateNum )+QString(s.str().c_str());
    TSOSitem->setText(0, QString("Track State "+text  ) );

    if (trackState.hasReferenceSurface()) {
      auto& surface = trackState.referenceSurface();
      QString surfaceText = QString(surface.toString(common()->geometryContext().context()).c_str());
      QTreeWidgetItem* surfItem = new QTreeWidgetItem(TSOSitem);
      surfItem->setExpanded(true);
      surfItem->setFlags(Qt::ItemIsEnabled);
      surfItem->setText(0, QString(surface.name().c_str()));
      surfItem->setText(1, surfaceText );
    }

    // TSOSitem->setText(1, 

    QString trackStateText;      
    // bool first=true;

    // const Trk::MeasurementBase* meas = (*it)->measurementOnTrack();
    if (trackState.hasCalibrated() ){
      QString measName = QString(TrkObjToString::typeName(measurementType(trackState)).c_str());
       trackStateText+=measName;
      
        QTreeWidgetItem* measItem = new QTreeWidgetItem(TSOSitem);
        measItem->setExpanded(true); // want it opened so subparams seen easily
      measItem->setFlags(Qt::ItemIsEnabled);
      measItem->setText(0, measName);
      measItem->setText(1, measurementText(trackState) ); //TODO expand this with more info
      
      // QStringList list = TrkObjToString::fullInfo( *((*it)->measurementOnTrack ()) );
      // for (int i = 0; i < (list.size()-1); ){
      //   QTreeWidgetItem* subparamItem = new QTreeWidgetItem(measItem);
      //   subparamItem->setText(0, list.at(i++) );
      //   subparamItem->setText(1, list.at(i++) );   
      //   subparamItem->setFlags(Qt::ItemIsEnabled); 
      // }
    }
    //   first=false;
    // }
    // if ( (*it)->trackParameters () ) {
    //   if (!first) trackStateText.append(" + ");
    //   trackStateText.append("Parameters");
      
    //   QTreeWidgetItem* paramItem = new QTreeWidgetItem(TSOSitem);
    //   paramItem->setExpanded(true); // want it opened so subparams seen easily
    //   paramItem->setFlags(Qt::ItemIsEnabled); 
    //   paramItem->setText(0, TrkObjToString::name(      *((*it)->trackParameters ()) ) );
    //   // paramItem->setText(1, TrkObjToString::shortInfo( *((*it)->trackParameters ()) ) );
      
    //   QStringList list = TrkObjToString::fullInfo( *((*it)->trackParameters ()) );
    //   for (int i = 0; i < (list.size()-1); ){
    //     QTreeWidgetItem* subparamItem = new QTreeWidgetItem(paramItem);
    //     subparamItem->setText(0, list.at(i++) );
    //     subparamItem->setText(1, list.at(i++) ); 
    //     subparamItem->setFlags(Qt::ItemIsEnabled);   
    //   }
    //   first=false;
    // }
    // if ( (*it)->materialEffectsOnTrack () ){
    //   if (!first) trackStateText.append(" + ");
    //   trackStateText.append("MaterialEffectsOnTrack"); 
    //   QTreeWidgetItem* meItem = new QTreeWidgetItem(TSOSitem);
    //   meItem->setExpanded(true); // want it opened so subparams seen easily
    //   meItem->setFlags(Qt::ItemIsEnabled);
      
    //   meItem->setText(0, TrkObjToString::name(      *((*it)->materialEffectsOnTrack ()) ) );
    //   meItem->setText(1, TrkObjToString::shortInfo( *((*it)->materialEffectsOnTrack ()) ) );
      
    //   first=false;
    // }
    // if ( (*it)->fitQualityOnSurface () ){ 
    //   if (!first) trackStateText.append(" + ");
    //   trackStateText.append("FitQuality");
    //   QTreeWidgetItem* fqItem = new QTreeWidgetItem(TSOSitem);
    //   fqItem->setExpanded(true); // want it opened so subparams seen easily
    //   fqItem->setFlags(Qt::ItemIsEnabled);
    //   fqItem->setText(0,       QString("FitQuality") );
    //   fqItem->setText(1, TrkObjToString::shortInfo( (*it)->fitQualityOnSurface () ) );
      
    //   first=false;
    // }
    // FIXME - add information about chamber for Muon systems?
    
    // if ( (*it)->type(Trk::TrackStateOnSurface::Outlier) ){ 
    //   if (!first) trackStateText.append(" + ");
    //   trackStateText.append("Outlier"); 
    // }
    
    // if ( (*it)->type(Trk::TrackStateOnSurface::InertMaterial) ){ 
    //   if (!first) trackStateText.append(" + ");
    //   trackStateText.append("InertMaterial"); 
    // }
    
    // if ( (*it)->type(Trk::TrackStateOnSurface::BremPoint) ){ 
    //   if (!first) trackStateText.append(" + ");
    //   trackStateText.append("BremPoint"); 
    // }
    
    // if ( (*it)->type(Trk::TrackStateOnSurface::Perigee) ){ 
    //   if (!first) trackStateText.append(" + ");
    //   trackStateText.append("Perigee"); 
    // }
    
    // if ( (*it)->type(Trk::TrackStateOnSurface::Hole) ){ 
    //   if (!first) trackStateText.append(" + ");
    //   trackStateText.append("Hole"); 
    // }
    TSOSitem->setText(1, trackStateText );
    trackStateNum++;
  }
  // listOfItems << browserTreeItem();

}

SoNode* TrackHandle_TrackContainer::zoomToTSOS(unsigned int index) {
  ensureInitTrackStateCache();
  // now find matching AscObj_TSOS
  auto trackState = m_trackStates.at(index);
  QList<AssociatedObjectHandleBase*> list = getAllAscObjHandles();
    VP1Msg::messageVerbose(
        "TrackHandle_TrackContainer::zoomToTSOS: checking ASC " +
        QString::number(index) + " of " + QString::number(list.size()));

  if ( (int)index < list.size()){
    AscObj_TrackState* asc = dynamic_cast<AscObj_TrackState*>(list.at(index));
    if (asc && asc->trackState().index()==trackState.index()) {
      VP1Msg::messageVerbose(
          "TrackHandle_TrackContainer::zoomToTSOS: this ASC matches " +
          QString::number(index));
      return asc->shapeDetailed();
    }
  }
  
  return nullptr;
}

QString TrackHandle_TrackContainer::shortInfo() const {
  Amg::Vector3D mom = momentum();
  mom /= CLHEP::GeV;

  // format info string
  QString l;
  l += "|Pt|=" + VP1Msg::str(mom.perp()) + " [GeV], ";
  l += "|P|=" + VP1Msg::str(mom.mag()) + " [GeV], ";
  return l;
}

void TrackHandle_TrackContainer::updateObjectBrowser() {
  VP1Msg::messageVerbose("TrackHandle_TrackContainer::updateObjectBrowser");

  if (!browserTreeItem()) {
    VP1Msg::messageVerbose(
        "TrackHandle_TrackContainer::updateObjectBrowser: No m_objBrowseTree!");
    return;
  }

  if (!visible()) {
    browserTreeItem()->setFlags(Qt::ItemFlag());  // not selectable, not enabled
  } else {
    browserTreeItem()->setFlags(Qt::ItemIsSelectable |
                                Qt::ItemIsEnabled);  //  selectable,  enabled
  }
  QFont font = browserTreeItem()->font(0);
  font.setStrikeOut(!visible());
  browserTreeItem()->setFont(0, font);
  browserTreeItem()->setFont(1, font);
  // FIXME! Only do if necessary i.e. if something affecting this TSOS has
  // changed.

  QList<AssociatedObjectHandleBase*> list = getAllAscObjHandles();

  if (list.empty()) {
    VP1Msg::message(
        "No ASC objects associated with this track - no track components "
        "visible yet?");
    return;
  }

  unsigned int numOfTS = 0;

  for (auto trackstate : m_trackStates) {
    // We assume that there is an ASC for every trackstate
    AscObj_TrackState* asc = dynamic_cast<AscObj_TrackState*>(list.at(numOfTS));
    if (!asc) {
      VP1Msg::message(
          "Could not cast to AscObj_TrackState, or could not find matching Asc "
          "in list of size " +
          QString::number(list.size()));
      numOfTS++;
      continue;
    }

    if (asc->trackState().index() != trackstate.index()) {
      VP1Msg::message(
          "WARNING! TrackHandle_TrackContainer::updateObjectBrowser: " +
          QString::number(numOfTS) +
          ": ASC index mismatch with trackstate from loop having " +
          QString::number(trackstate.index()) + " and ASC TS having " +
          QString::number(asc->trackState().index()));
      numOfTS++;
      continue;
    }

    if (!asc->browserTreeItem()) {
      // not set yet - so need to do this now. Can we just use the index?
      asc->setBrowserTreeItem(browserTreeItem()->child(numOfTS));
    }

    if (!asc->browserTreeItem()) {
      VP1Msg::message("Could not find matching browserTreeItem");
      numOfTS++;
      continue;
    }

    if (!asc->visible()) {
      asc->browserTreeItem()->setFlags(
          Qt::ItemFlag());  // not selectable, not enabled
    } else {
      asc->browserTreeItem()->setFlags(
          Qt::ItemIsSelectable | Qt::ItemIsEnabled);  //  selectable,  enabled
    }
    QFont itemFont = asc->browserTreeItem()->font(0);
    itemFont.setStrikeOut(!asc->visible());
    asc->browserTreeItem()->setFont(0, itemFont);
    asc->browserTreeItem()->setFont(1, itemFont);
    numOfTS++;
  }
}

bool TrackHandle_TrackContainer::containsDetElement(
    const QString& /**id*/) const {
  return false;
}

void TrackHandle_TrackContainer::ensureInitTrackStateCache()
  {
    // Unfortunately actsTracks are stored in reverse order, so we need to do some
  // gymnastics (There is certainly a more elegant way to do this, but since
  // this will all be changed soon I don't think it matters)
  if (!m_trackStates.empty())
    return;
  m_trackStates.reserve(m_track.nTrackStates());
  for (auto trackstate : m_track.trackStatesReversed()) {
    m_trackStates.push_back(trackstate);
  }

  std::reverse(m_trackStates.begin(), m_trackStates.end());
}


void TrackHandle_TrackContainer::ensureInitTSOSs(
    std::vector<AssociatedObjectHandleBase*>*& ascobjs) {
  if (ascobjs)
    return;
  ensureInitTrackStateCache();

  ascobjs = new std::vector<AssociatedObjectHandleBase*>;
  unsigned int index =0;

  for (const auto trackState : m_trackStates) {
    addTrackState(trackState, ascobjs, index++);
  }
}

void TrackHandle_TrackContainer::addTrackState(
    const typename ActsTrk::TrackContainer::ConstTrackStateProxy& state,
    std::vector<AssociatedObjectHandleBase*>* ascobjs, unsigned int index) {

  AscObj_TrackState* ao = new AscObj_TrackState(this, index, state);

  registerAssocObject(ao);

  ascobjs->push_back(ao);
  if (ao->parts() & shownTSOSParts()){
    ao->setVisible(true);
  }
}

TrkObjToString::MeasurementType TrackHandle_TrackContainer::measurementType(
    const ActsTrk::TrackContainer::ConstTrackStateProxy& state) const {
  TrkObjToString::MeasurementType type = TrkObjToString::Unknown;
  if (state.hasReferenceSurface()) {
      const auto *actsElement = dynamic_cast<const ActsDetectorElement *>(
          state.referenceSurface().associatedDetectorElement());
      if (actsElement && common()->muonIdHelperSvc().get()) {
        auto& idhelper = common()->muonIdHelperSvc()->mdtIdHelper(); // This is a lazy way to get an AtlasID helper. Not ideal if muon geometry is off.
        if (idhelper.is_mdt(actsElement->identify())) {
          type = TrkObjToString::MDT;
        } else if (idhelper.is_tgc(actsElement->identify())) {
          type = TrkObjToString::TGC;
        } else if (idhelper.is_rpc(actsElement->identify())) {
          type = TrkObjToString::RPC;
        } else if (idhelper.is_csc(actsElement->identify())) {
          type = TrkObjToString::CSC;
        } else if (idhelper.is_stgc(actsElement->identify())) {
          type = TrkObjToString::sTGC;
        } else if (idhelper.is_mm(actsElement->identify())) {
          type = TrkObjToString::MM;
        }
      }
    return type;
  }
  return TrkObjToString::Unknown;
}

QString TrackHandle_TrackContainer::measurementText(
    const ActsTrk::TrackContainer::ConstTrackStateProxy& state) const {
  QString text("Unknown Measurement");
  if (state.hasReferenceSurface()) {
    const auto* actsElement = dynamic_cast<const ActsDetectorElement*>(
        state.referenceSurface().associatedDetectorElement());
    if (actsElement) {
      auto& helperSvc = common()->muonIdHelperSvc();
      if (helperSvc->isMuon(actsElement->identify()))
        text = QString(helperSvc->toString(actsElement->identify()).c_str());
    }
  }
  return text;
}
