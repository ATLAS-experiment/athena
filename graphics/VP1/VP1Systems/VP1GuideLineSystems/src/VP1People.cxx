/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


//////////////////////////////////////////////////////////////////////    
//                                                                  //
//  Implementation of class VP1People                               //
//                                                                  //
//  Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch> //
//  Initial version: November 2021                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

#include "VP1GuideLineSystems/VP1People.h"

#include "VP1HEPVis/SbPolyhedron.h"
#include "VP1HEPVis/nodes/SoPolyhedron.h"

#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoMaterialBinding.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoRotationXYZ.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/VRMLnodes/SoVRMLMaterial.h>

#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoOutput.h>

#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include <algorithm>
#include <vector>

#ifdef BUILDVP1LIGHT
  #include "CLHEP/Units/SystemOfUnits.h"
  #define SYSTEM_OF_UNITS CLHEP
#else
  #include "GaudiKernel/SystemOfUnits.h"
  #define SYSTEM_OF_UNITS Gaudi::Units
#endif

//____________________________________________________________________
class VP1People::Imp {
public:
  Imp(VP1People *,
      SoMaterial * mat,
      SoSeparator * attachsep);
  VP1People * theclass;
  SoMaterial * material;
  SoSeparator * attachSep;

  SoSeparator* loadModel(const std::string& fpath);
  double getScaleFactor(SoNode* node, double desiredHeight);

  bool shown;
  SbColor4f colourAndTransp;
  double zpos;
  double vertpos;

  SoSeparator * sep;
  SoTranslation * trans1;
  SoTranslation * trans2;
  SoTranslation * trans3;

  void updateFields();
  void ensureInit3DObjects();
  void updateColour();

};

//____________________________________________________________________
VP1People::VP1People(SoMaterial * mat,SoSeparator * attachsep,
		       IVP1System * sys,QObject * parent)
  : QObject(parent), VP1HelperClassBase(sys,"VP1People"), m_d(new Imp(this,mat,attachsep))
{
}

//____________________________________________________________________
VP1People::~VP1People()
{
  setShown(false);
  if (m_d->sep)
    m_d->sep->unref();
  if (m_d->trans1)
    m_d->trans1->unref();
  if (m_d->trans2)
    m_d->trans2->unref();
  m_d->material->unref();
  m_d->attachSep->unref();
  delete m_d;
}

//____________________________________________________________________
VP1People::Imp::Imp(VP1People *tc,SoMaterial * mat,SoSeparator * as)
  : theclass(tc), material(mat), attachSep(as), shown(false),
    zpos(0),vertpos(0), sep(0), trans1(0), trans2(0), trans3(0)
{
  material->ref();
  attachSep->ref();
}



//____________________________________________________________________
SoSeparator* VP1People::Imp::loadModel(const std::string& fpath) 
{
    SoDB::init();
    SoInput myInput;
    if (!myInput.openFile(fpath.c_str()))
        exit (1);
    SoSeparator *model = SoDB::readAll(&myInput);
    if (model == NULL)
        exit (1);

    unsigned int nChildren = model->getNumChildren();
    std::cout << "nChildren: " << nChildren << std::endl;
    for (unsigned int i=0; i < nChildren; ++i) {
        const SoNode* node = model->getChild(i);
        std::cout << "node " << i << ": " << node << std::endl;
        if (node->getTypeId() == SoVRMLMaterial::getClassTypeId()) {
            std::cout << "Found a SoVRMLMaterial node!\n";
            const SoNode * matvrmlnode = static_cast<const SoVRMLMaterial *>(node);  // safe downward cast, knows the type
            std::cout << "Material node: " << matvrmlnode << std::endl;
        }
    }

    // WIP
    /*
    SoSearchAction sa;
    sa.setType(SoVRMLMaterial::getClassTypeId());
    sa.setInterest(SoSearchAction::ALL);
    sa.setSearchingAll(TRUE);
    sa.apply(model);
    SoPathList & pl = sa.getPaths();
    SbDict namedict;
    double plLen = pl.getLength();
    std::cout << "plLength: " << plLen << std::endl;

    for (int i = 0; i < plLen; i++) {
        
        SoFullPath * p = (SoFullPath*) pl[i];
        
        if (p->getTail()->isOfType(SoVRMLMaterial::getClassTypeId())) {

            std::cout << "Found a SoVRMLMaterial node in Search!\n";
            SoVRMLMaterial* vmat = (SoVRMLMaterial*) p->getTail();
           
            if (vmat->filename.getValue().getLength()) {
                SbName name = tex->filename.getValue().getString();
                unsigned long key = (unsigned long) ((void*) name.getString());
                void * tmp;
                if (!namedict.find(key, tmp)) {
                    // new texture. just insert into list
                    (void) namedict.enter(key, tex);
                }
                else if (tmp != (void*) tex) { // replace with node found in dict
                    SoGroup * parent = (SoGroup*) p->getNodeFromTail(1);
                    int idx = p->getIndexFromTail(0);
                    parent->replaceChild(idx, (SoNode*) tmp);
                }
            }
        }
    }
    
    sa.reset();
    
    // output fixed scene to stdout
    SoOutput out;
    SoWriteAction wa(&out);
    wa.apply(model);
    */

    return model;
}




//
double VP1People::Imp::getScaleFactor(SoNode* node, double desiredHeight)
{

  // Get the global BoundingBox of the imported model.
  // NOTE: 
  // SoGet-BoundingBoxAction is typically called on a path, 
  // which enables you to obtain a bounding box for a specific object in world coordinates. 
  // This action returns an SbBox3f, 
  // which specifies a 3D box aligned with the x-, y-, and z-axes in world coordinate space. 
  SbViewportRegion dummyViewport; // we need one just for creating the action
  SoGetBoundingBoxAction bboxAction(dummyViewport);
  bboxAction.apply(node);
  SbXfBox3f xfbox = bboxAction.getXfBoundingBox();
  SbBox3f box = xfbox.project();
  float minx, miny, minz, maxx, maxy, maxz;
  box.getBounds(minx, miny, minz, maxx, maxy, maxz);
  float cx,cy,cz;
  box.getCenter().getValue(cx,cy,cz);

  // get the height of the model (it's along Y axis)
  float heightModel = (maxy - miny); 
  // set the desired final height for the model
  float heightDesired = desiredHeight;
  // get the required scale scale factor
  double scaleFactor = heightDesired / heightModel;
 
  /*
  // DEBUG MSG
  std::cout << miny << ", " << maxy 
            << ", heightModel: " << heightModel 
            << ", heightDesired: " << heightDesired 
            << ", scaleFactor: " << scaleFactor 
            << std::endl;
  */
  return scaleFactor;
}


//____________________________________________________________________
void VP1People::Imp::ensureInit3DObjects()
{
  if (sep)
    return;
  theclass->messageVerbose("Building 3D objects");
  sep = new SoSeparator; sep->ref();
  sep->addChild(material);


  // Create people:
  // at first we need to copy the 3D file in the Qt resources file to the local filesystem
  // for the SoQt load method to be able to load it
  // Then, we load the model into the Coin geometry tree
  QTemporaryDir tempDir;
  QFile w1 = QFile(":/vp1/data/models/worker1.wrl");
  QFile w2 = QFile(":/vp1/data/models/worker2.wrl");
  SoSeparator* worker1 = nullptr;
  SoSeparator* worker2 = nullptr;
  if (tempDir.isValid()) {
      const QString tempFile1 = tempDir.path() + "/worker1.wrl";
      if ( w1.copy(tempFile1) ) {
          worker1 = loadModel( tempFile1.toStdString() ); 
      } else {
          std::cout << "ERROR!! QFile copy of Worker1 was NOT successful!\n";
      }
      const QString tempFile2 = tempDir.path() + "/worker2.wrl";
      if ( w2.copy(tempFile2) ) {
          worker2 = loadModel( tempFile2.toStdString() ); 
      } else {
          std::cout << "ERROR!! QFile copy of Worker2 was NOT successful!\n";
      }
  } else {
      std::cout << "ERROR!! TempDir is NOT valid!\n";
  }
  if (worker1 == nullptr) {
      std::cout << "\n\nERROR! The 3D model Worker1 could  not be loaded!\n\n";
      exit(1);
  }
  if (worker2 == nullptr) {
      std::cout << "\n\nERROR! The 3D model Worker2 could  not be loaded!\n\n";
      exit(1);
  }
 
  // TEST CODE with a test shape
  //SoCube* box = new SoCube(); // test shape
  //worker1->materialBinding=SoMaterialBinding::OVERALL;
  //SoMaterialBinding::OVERALL;
  //
  // get the required scale factor for the model
  double scaleFactor1 = getScaleFactor(worker1, 1.80*SYSTEM_OF_UNITS::m);
  double scaleFactor2 = getScaleFactor(worker2, 1.80*SYSTEM_OF_UNITS::m);
  
  // set a scale transformation to apply the required scale factor:
  SoScale* myScale1 = new SoScale();
  myScale1->scaleFactor.setValue(scaleFactor1, scaleFactor1, scaleFactor1);
  SoScale* myScale2 = new SoScale();
  myScale2->scaleFactor.setValue(scaleFactor2, scaleFactor2, scaleFactor2);

  // Translation for people models
  trans1 = new SoTranslation;
  trans1->ref();
  trans2 = new SoTranslation;
  trans2->ref();
  trans3 = new SoTranslation;
  trans3->ref();
  
  // rotate the model around the Y axis
  SoRotationXYZ * rotY = new SoRotationXYZ();
  rotY->axis=SoRotationXYZ::Y;
  rotY->angle = 45*SYSTEM_OF_UNITS::deg;
 
  SoVRMLMaterial* matVRML = new SoVRMLMaterial();
  matVRML->diffuseColor.setValue(0., 1., 1.);

  // create a separator for the scaled/rotated model,
  // then
  // create a separator for the translated model
  SoSeparator* sepScaledWorker1 = new SoSeparator();
  sepScaledWorker1->addChild(matVRML);
  sepScaledWorker1->addChild(myScale1);
  sepScaledWorker1->addChild(rotY);
  sepScaledWorker1->addChild(worker1);
  //sepScaledWorker1->addChild(box); // test shape
  SoSeparator* sepWorker1 = new SoSeparator();
  sepWorker1->addChild(trans1);
  sepWorker1->addChild(sepScaledWorker1);

  SoSeparator* sepScaledWorker2 = new SoSeparator();
  sepScaledWorker2->addChild(myScale2);
  sepScaledWorker2->addChild(rotY);
  sepScaledWorker2->addChild(worker2);
  SoSeparator* sepWorker2 = new SoSeparator();
  sepWorker2->addChild(trans2);
  sepWorker2->addChild(sepScaledWorker2);

  SoTranslation* xt = new SoTranslation();
  xt->translation.setValue(2*SYSTEM_OF_UNITS::m, 0., 1*SYSTEM_OF_UNITS::m);
  SoSeparator* sepWorker3 = new SoSeparator();
  sepWorker3->addChild(trans3);
  sepWorker3->addChild(sepScaledWorker1);
  sepWorker3->addChild(xt);
  sepWorker3->addChild(sepScaledWorker2);
  
  // add the scaled model to the scenegraph
  sep->addChild(sepWorker1);
  sep->addChild(sepWorker2);
  sep->addChild(sepWorker3);
}

//____________________________________________________________________
void VP1People::Imp::updateFields()
{
  ensureInit3DObjects();
  theclass->messageVerbose("Updating fields");
  trans1->translation.setValue(0, vertpos, zpos); // x,y,z
  trans2->translation.setValue(4*SYSTEM_OF_UNITS::m, vertpos, zpos); // x,y,z
  trans3->translation.setValue(8*SYSTEM_OF_UNITS::m, vertpos, zpos-10*SYSTEM_OF_UNITS::m); // x,y,z
}

//____________________________________________________________________
void VP1People::Imp::updateColour()
{
  theclass->messageVerbose("Updating packed colour");
  if (!sep||sep->getNumChildren()<1)
    return;
  SoNode * n = sep->getChild(0);
  if (!n||n->getTypeId()!=SoVRMLMaterial::getClassTypeId())
    return;

  SoVRMLMaterial* matVRML = static_cast<SoVRMLMaterial*>(n);
  matVRML->diffuseColor.setValue( colourAndTransp.getValue() );
}

//____________________________________________________________________
void VP1People::setShown(bool b)
{
  messageVerbose("Signal received: setShown("+str(b)+")");
  if (m_d->shown==b)
    return;
  m_d->shown=b;
  if (m_d->shown) {
    m_d->updateFields();
    if (m_d->attachSep->findChild(m_d->sep)<0)
      m_d->attachSep->addChild(m_d->sep);
  } else {
    if (m_d->sep&&m_d->attachSep->findChild(m_d->sep)>=0)
      m_d->attachSep->removeChild(m_d->sep);
  }
}

//____________________________________________________________________
void VP1People::setColourAndTransp(const SbColor4f&ct)
{
  messageVerbose("Signal received in setColourAndTransp slot.");
  if (m_d->colourAndTransp==ct)
    return;
  m_d->colourAndTransp=ct;
  if (m_d->shown)
    m_d->updateColour();
}

//____________________________________________________________________
void VP1People::setZPos(const double&p)
{
  messageVerbose("Signal received: setZPos("+str(p)+")");
  if (m_d->zpos==p)
    return;
  m_d->zpos=p;
  if (m_d->shown)
    m_d->updateFields();
}

//____________________________________________________________________
void VP1People::setVerticalPosition(const double&p)
{
  messageVerbose("Signal received: setVerticalPosition("+str(p)+")");
  if (m_d->vertpos==p)
    return;
  m_d->vertpos=p;
  if (m_d->shown)
    m_d->updateFields();
}
