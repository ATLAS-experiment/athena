/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "VP1GeometrySystems/VisAttributes.h"

#include "VP1Base/VP1QtInventorUtils.h"
#include <Inventor/nodes/SoMaterial.h>
#include <iostream>
#include <map>
#include <QBuffer>

/////////////////////////////////////// Base class ///////////////////////////////////////

class VisAttributes::Imp {
public:
  // The material map is here:
  std::map< std::string, SoMaterial *> _map;

  QMap<QString,QByteArray> currentState() const;
  QMap<QString,QByteArray> initialState;
  
};

//____________________________________________________________________
void VisAttributes::init()
{
  m_d->initialState = m_d->currentState();
}

//____________________________________________________________________
QByteArray VisAttributes::getState(bool onlyChangedMaterials)
{
  //Figure out states to store
  QMap<QString,QByteArray> storedstates, statesnow = m_d->currentState();
  if (onlyChangedMaterials) {
    QMap<QString,QByteArray>::const_iterator it, itE(statesnow.constEnd());
    QMap<QString,QByteArray>::const_iterator itOrig, itOrigE(m_d->initialState.constEnd());
    for (it = statesnow.constBegin(); it!=itE; ++it) {
      itOrig = m_d->initialState.constFind(it.key());
      if (itOrig==itOrigE||it.value()!=itOrig.value())
	storedstates.insert(it.key(),it.value());
    }
  } else {
    storedstates = statesnow;
  }

  //Put map in bytearray and return:
  QByteArray byteArray;
  QBuffer buffer(&byteArray);
  buffer.open(QIODevice::WriteOnly);
  QDataStream out(&buffer);
  out << qint32(0);//version
  out << storedstates;
  buffer.close();
  return byteArray;

}

//____________________________________________________________________
void VisAttributes::applyState(QByteArray ba)
{
  //Get map out:
  QMap<QString,QByteArray> storedstates;
  QBuffer buffer(&ba);
  buffer.open(QIODevice::ReadOnly);
  QDataStream state(&buffer);
  qint32 version;
  state >> version;
  if (version!=0)
    return;//ignore silently
  state >> storedstates;
  buffer.close();

  std::map< std::string, SoMaterial *>::iterator itMat,itMatE(m_d->_map.end());

  //Apply states from map:
  QMap<QString,QByteArray>::const_iterator it, itE(storedstates.constEnd());
  for (it = storedstates.constBegin(); it!=itE; ++it) {
    itMat = m_d->_map.find(it.key().toStdString());
    if (itMat!=itMatE) {
      QByteArray b(it.value());
      VP1QtInventorUtils::deserialiseSoMaterial(b,itMat->second);
    }
  }

}

//____________________________________________________________________
QMap<QString,QByteArray> VisAttributes::Imp::currentState() const
{
  QMap<QString,QByteArray> outmap;
  std::map< std::string, SoMaterial *>::const_iterator it(_map.begin()), itE(_map.end());
  for (;it!=itE;++it)
    outmap.insert(QString(it->first.c_str()),VP1QtInventorUtils::serialiseSoMaterial(it->second));
  return outmap;
}

//____________________________________________________________________
VisAttributes::VisAttributes() : m_d(new Imp) {
}

//____________________________________________________________________
VisAttributes::~VisAttributes() {

  std::map<std::string, SoMaterial *>::iterator m,e=m_d->_map.end();
  for (m=m_d->_map.begin();m!=e;++m)
    (*m).second->unref();

  delete m_d;
}

SoMaterial *VisAttributes::get (const std::string & name) const {
  std::map <std::string, SoMaterial *>::const_iterator m = m_d->_map.find(name);
  if (m!=m_d->_map.end()) {
    return (*m).second;
  } else {
    return NULL;
  }
}

void VisAttributes::add(const std::string & name, SoMaterial *material) {
  if (m_d->_map.find(name)!=m_d->_map.end()) {
    std::cout<<"VisAttributes::add ERROR: Material " <<name<<" already added!"<<std::endl;
    return;
  }
  material->ref();
  m_d->_map[name]=material;
  if (material->transparency.getNum()!=1)
    std::cout<<"VisAttributes::add Warning: Found #transparency values different from 1 in material "<<name<<std::endl;
  // FIXME. Commented out, because Ric and I are not sure why this is an issue
  // We may want to add this back if this turns out to indicate a real problem
  // if (material->transparency[0]!=0.0)
  //   std::cout<<"VisAttributes::add Warning: Found transparency value different from 0 in material "<<name<<std::endl;
}

void VisAttributes::overrideTransparencies(float transpfact)
{
  std::map< std::string, SoMaterial *>::iterator it, itE = m_d->_map.end();
  for (it=m_d->_map.begin();it!=itE;++it)
    it->second->transparency.set1Value( 0, transpfact );
}

float VisAttributes::getValFromRGB(const int rgb)
{
    return rgb/255.0;
}

void VisAttributes::setColorFromRGB(SoMaterial* mat, const std::string& type, const int r, const int g, const int b)
{
    const float fr = getValFromRGB(r);
    const float fg = getValFromRGB(g);
    const float fb = getValFromRGB(b);
    if (type == "ambient")
        mat->ambientColor.setValue(fr, fg, fb);
    else if (type == "diffuse")
        mat->diffuseColor.setValue(fr, fg, fb);
    else if (type == "specular")
        mat->specularColor.setValue(fr, fg, fb);
    else 
        std::cout << "ERROR! Color type not supported ==> " << type << std::endl;
    
    // Debug Msg
    //std::cout << "Set color (" << r << "," << g << "," << b << ") to (" << fr << "," << fg << "," << fb << ")" << std::endl;
    return;
}


//////////////////////////////// Attributes for detectors ////////////////////////////////

DetVisAttributes::DetVisAttributes() {

  {
    SoMaterial *material = new SoMaterial;
    material->ambientColor.setValue(0, .157811, .187004);
    material->diffuseColor.setValue(0, .631244, .748016);
    material->specularColor.setValue(.915152, .915152, .915152);
    material->shininess.setValue(0.642424);
    add("Pixel",material);
  }

  {
    SoMaterial *material = new SoMaterial;
    material->ambientColor.setValue(0, .157811, .187004);
    material->diffuseColor.setValue(0, .631244, .748016);
    material->specularColor.setValue(.915152, .915152, .915152);
    material->shininess.setValue(0.642424);
    add("ITkPixel",material);
  }

  {
    SoMaterial *material = new SoMaterial;
    material->ambientColor.setValue(0, .157811, .187004);
    material->diffuseColor.setValue(.40, .631244, .748016);
    material->specularColor.setValue(.915152, .915152, .915152);
    material->shininess.setValue(0.642424);
    add("Tile",material);
  }

  {
    SoMaterial *material = new SoMaterial;
    material->ambientColor.setValue(0, .157811, .187004);
    material->diffuseColor.setValue(1, .8, .7);
    material->specularColor.setValue(.915152, .915152, .915152);
    material->shininess.setValue(0.642424);
    add("BeamPipe",material);
  }

  {
    SoMaterial *material = new SoMaterial;
    material->ambientColor.setValue(0, .6, .6);
    material->diffuseColor.setValue(1, .8, .7);
    material->specularColor.setValue(.91515, .915152, .915152);
    material->shininess.setValue(0.642424);
    add("CavernInfra",material);
  }

  {
    SoMaterial *material = new SoMaterial;
    material->ambientColor.setValue(0.2, 0.2, 0.2);
    material->diffuseColor.setValue(0, 0.6667, 1.0);
    material->specularColor.setValue(0,0,0);
    //    material->shininess.setValue(0.642424);
//     material->ambientColor.setValue(0, .157811, .187004);
//     material->diffuseColor.setValue(1, 0, 0);
//     material->specularColor.setValue(.915152, .915152, .915152);
//     material->shininess.setValue(0.642424);

    add("Muon",material);

    {
       SoMaterial *material = new SoMaterial;
       material->ambientColor.setValue(0, .157811, .187004);
       material->diffuseColor.setValue(.98, .8, .21);
       material->specularColor.setValue(.915152, .915152, .915152);
       material->shininess.setValue(0.2);
       add("CSC",material);
    }

    {
      SoMaterial *material = new SoMaterial;
      material->ambientColor.setValue(0, .157811, .187004);
      material->diffuseColor.setValue(0, .9, .5);
      material->specularColor.setValue(.915152, .915152, .915152);
      material->shininess.setValue(0.2);
      add("EndcapMdt",material);
    }

    {
      SoMaterial *material = new SoMaterial;
      material->ambientColor.setValue(0,0,0);
      material->diffuseColor.setValue(0.41,0,0.26);
      material->specularColor.setValue(0,0,0);
      material->shininess.setValue(0.2);
      add("TGC",material);
    }

    {
      SoMaterial *material = new SoMaterial;

    material->ambientColor.setValue(0.2, 0.2, 0.2);
    material->diffuseColor.setValue(0, 0.6667, 1.0);
    material->specularColor.setValue(0,0,0);
//       material->ambientColor.setValue(0, .157811, .187004);
//       material->diffuseColor.setValue(1, .2, .7);
//       material->specularColor.setValue(.915152, .915152, .915152);
    material->shininess.setValue(0.2);


      add("BarrelInner",material);
      add("BarrelMiddle",material);
      add("BarrelOuter",material);
    }

    {
      SoMaterial *material = new SoMaterial;
      material->ambientColor.setValue(0, .157811, .187004);
      material->diffuseColor.setValue(1, .5, .5);
      material->specularColor.setValue(.915152, .915152, .915152);
      material->shininess.setValue(0.2);
      add("BarrelToroid",material);
      add("EndcapToroid",material);
    }

    {
      SoMaterial *material = new SoMaterial;
      material->ambientColor.setValue(0, .157811, .187004);
      material->diffuseColor.setValue(.5, .5, 1.0);
      material->specularColor.setValue(.915152, .915152, .915152);
      material->shininess.setValue(0.2);
      add("Feet",material);
    }
  }

  {
    SoMaterial *material = new SoMaterial;
    material->ambientColor.setValue(.37, .69, 1.00);
    material->diffuseColor.setValue(.21, .64, 1.00);
    material->specularColor.setValue(1, 1, 1);
    material->shininess.setValue(1.0);
    add("SCT",material);
  }

  {
    SoMaterial *material = new SoMaterial;
    material->ambientColor.setValue(.37, .69, 1.00);
    material->diffuseColor.setValue(.21, .64, 1.00);
    material->specularColor.setValue(1, 1, 1);
    material->shininess.setValue(1.0);
    add("ITkStrip",material);
  }

  {
    SoMaterial *material = new SoMaterial;
    add("LAr",material);
    add("LArBarrel",material);
    add("LArEndcapPos",material);
    add("LArEndcapNeg",material);
  }

  {
    SoMaterial *material = new SoMaterial;
    add("TRT",material);
  }

  {
    SoMaterial *material = new SoMaterial;
    add("InDetServMat",material);
    material->diffuseColor.setValue(0.4,0.31,0);
    material->shininess.setValue(1.0);
  }

  {
    SoMaterial *material = new SoMaterial();
    add("LucidSideA",material);
    add("LucidSideC",material);
    material->diffuseColor.setValue(0.6,0.11,0.3);
    material->shininess.setValue(1.0);
  }

  {
    SoMaterial *material = new SoMaterial;
    material->ambientColor.setValue(0, .157811, .187004);
    material->diffuseColor.setValue(0., 0., 0.56862745);
    material->specularColor.setValue(.915152, .915152, .915152);
    material->shininess.setValue(0.2);
    add("Zdc",material);
  }


  //   {
  //     SoMaterial *material = new SoMaterial();
  //     add("LcdXXX",material);
  //     add("LcdYYY",material);
  //     material->diffuseColor.setValue(0.6,0.11,0.3);
  //     material->shininess.setValue(1.0);
  //   }

  init();
}

//////////////////////////////// Attributes for materials ////////////////////////////////

MatVisAttributes::MatVisAttributes() {
  {
    //NB: Must be here!!
    SoMaterial *m = new SoMaterial;
    add("DEFAULT",m);
  }



  /*
   * ETHER MATERIAL
   * This transparent material was used to hide the "fakeVolume" added by AGDD,
   * but there are other "standard" volumes, which use "Ether" material, for example in CSC chambers.
   * So we do not use this workaround anymore...
  {
	//
    // Ether: ---> it's a special "totally transparent" material, made to hide the "fakeVol" volume used in GeoModel to cope with the G4GeoAssembly objects in G4
	// more details:
	// - https://its.cern.ch/jira/browse/ATLASVPONE-166
	// - https://svnweb.cern.ch/trac/atlasoff/browser/DetectorDescription/AGDD/AGDDControl/trunk/src/AGDD2GeoModelBuilder.cxx?rev=648789#L502
	//
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue(0.2, 0.2, 0.2);
    m->diffuseColor.setValue(0.58, 0.47, 0.81);
    m->specularColor.setValue(0.56, 0.55, 0.56);
    m->transparency.setValue(1.0); // --> TOTALLY TRANSPARENT!!!  (set it to < 1.0 (e.g. 0.5) if you want to see the "fakeVol" cylinder in GeoModel)
    add("Ether",m);
  }
  */
 

  {
    // C02:
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue(0.2, 0.2, 0.2);
    m->diffuseColor.setValue(0.58, 0.47, 0.81);
    m->specularColor.setValue(0.56, 0.55, 0.56);
    add("CO2",m);
    add("ArCO2",m);
    add("trt::CO2",m);
  }

  {
    // Silicon
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue(0.98, 0.82, 0.02);
    m->diffuseColor.setValue(0.16, 0.26, 0.36);
    m->specularColor.setValue(0.56, 0.55, 0.56);
    m->shininess.setValue(0.13);
    add("Silicon",m);
    add("std::Silicon",m);
  }

  {
    // Kapton
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue(0.2, 0.127931, 0.177402);
    m->diffuseColor.setValue( 0.57665, 0.155139, 0.180815);
    m->specularColor.setValue(0.441667, 0.441667, 0.441667);
    m->shininess.setValue(0.67);
    add("Kapton",m);
    add("std::Kapton",m);
    add("KaptonC",m);  //used by LAr.
    add("FCalCableHarness",m);  //used by LAr.
  }

  {
    // Aluminium
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.70, 0.72, 0.72);
    m->diffuseColor.setValue (0.56, 0.57, 0.57);
    m->specularColor.setValue(0.71, 0.48, 0.46);
    m->shininess.setValue(0.23);
    add("Aluminium",m);
    add("std::Aluminium",m);
  }

  {
    // Iron
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.1, 0.1, 0.1);
    m->specularColor.setValue(0.92, 0.92, 0.89);
    m->shininess.setValue(0.60);
    add("Iron",m);
    add("std::Iron",m);
  }

  {
    // Tungsten
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.14, 0.14, 0.14);
    m->specularColor.setValue(0.84, 0.94, 1.00);
    m->shininess.setValue(0.20);
    add("Tungsten",m);
    add("Wolfram",m);
    add("FCal23Absorber",m);
  }
  {
    // Lead
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.05, .05, 0.7);
    m->diffuseColor.setValue (0.39, 0.36, 0.47);
    m->specularColor.setValue(0.33, 0.33, 0.35);
    m->shininess.setValue(0.30);
    add("Lead",m);
    add("Thinabs",m);
    add("Thickabs",m);
  }

  {
    // G10
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.308764, 0.314286, 0.142384);
    m->specularColor.setValue(0.268276, 0.315312, 0.308455);
    m->shininess.setValue(0.617021);
    add("G10",m);

  }

  {
    // Muon Scintillator
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.381944, 0.748016, 0);
    m->specularColor.setValue(0.963636, 0.963636, 0.963636);
    m->shininess.setValue(0.981818);
    add("Scintillator",m);
  }

  {
    // Tile Glue
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 102, 161, 191); // a pale azure
    setColorFromRGB(m, "ambient", 0, 40, 47); // a dark blue
    setColorFromRGB(m, "specular", 234,234, 234); // a light grey
    m->shininess.setValue(0.64);
    add( "Glue",m);
  }
  
  {
    // Tile Scintillator
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 97, 191, 0); // a bright green 
    setColorFromRGB(m, "ambient", 51, 51, 51); // a dark grey
    setColorFromRGB(m, "specular", 246, 246, 246); // almost white
    m->shininess.setValue(0.98);
    add("tile::Scintillator",m);
  }
  {
    // Epoxy
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0, 0.748016, 0.176015);
    m->specularColor.setValue(0.981818, 0.981818, 0.981818);
    m->shininess.setValue(0.721212);
    add("Epoxy",m);
  }

  {
    // Stainless steel
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.424238, 0.424238, 0.424238);
    m->specularColor.setValue(0.168, 0.168, 0.168);
    m->shininess.setValue(0.153696);
    add("Stainless",m);
  }
  {
    // Liquid Argon
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.05, .05, .06);
    m->diffuseColor.setValue (0.39, .36, .48);
    m->specularColor.setValue(0.33, .33, .35);
    m->emissiveColor.setValue(0.45, .60, .60);
    m->shininess.setValue(0.3);
    add("LiquidArgon",m);
    add("std::LiquidArgon",m);
  }
  {
    // Copper
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.0, 0.0,  0.0);
    m->diffuseColor.setValue (1.0, .43, .36);
    m->specularColor.setValue(1.0, 1.0, 1.0);
    m->shininess.setValue(0.4);
    add("Copper",m);
    add("FCal1Absorber",m);
  }

  {
    // Cables
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (1.0, 0, 0);
    add("Cables",m);
  }

  {
    // MBoards
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0, 1, 0);
    add("MBoards",m);
  }

  {
    // Carbon
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.2, 0.2, 0.2);
    m->ambientColor.setValue (0.07, 0.07, 0.07);
    m->specularColor.setValue (0.8, 0.8, 0.8);
    m->emissiveColor.setValue (0.028, 0.028, 0.028);
    add("Carbon",m);
  }

  {
    //Titanium
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.62, 0.62, 0.62);
    m->specularColor.setValue (0.294, 0.294, 0.294);
    m->shininess.setValue(.20);
    add("Titanium",m);
  }

  {
    //ECServices
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.7, 0.7, 0.7);
    m->specularColor.setValue (0.5, 0.5, 0.5);
    add("ECServices",m);
  }

  {
    //ECCables
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.1, 0.1, 0.1);
    add("ECCables",m);
  }

  {
    //Services
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.765, 0.718, 0.541);
    m->specularColor.setValue (0.5, 0.5, 0.5);
    add("Services",m);
  }

  {
    //MatOmega
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.22, 0.22, 0.22);
    add("MatOmega",m);
    add("pix::Omega_BL",m);
    add("pix::Omega_L1",m);
    add("pix::Omega_L2",m);
  }

  {
    //MatConn
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0, 0, 0);
    m->specularColor.setValue (0.8, 0.8, 0.8);
    add("MatConn",m);
    add("pix::Connector_BL",m);
    add("pix::Connector_L1",m);
    add("pix::Connector_L2",m);
  }

  {
    //MatPigtail
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.95, 0.58, 0.13);
    m->specularColor.setValue (0.8, 0.75, 0.7);
    m->shininess.setValue(.30);
    add("MatPigtail",m);
  }

  {
    // MatAlTube
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.70, 0.72, 0.72);
    m->diffuseColor.setValue (0.56, 0.57, 0.57);
    m->specularColor.setValue(0.71, 0.48, 0.46);
    m->shininess.setValue(0.23);
    add("MatAlTube",m);
    add("pix::AlTube_BL",m);
    add("pix::AlTube_L1",m);
    add("pix::AlTube_L2",m);
    add("MatAlTubeFix",m);
  }

  {
    //MatT0 (Cable)
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.06, 0.06, 0.06);
    m->specularColor.setValue (0.8, 0.8, 0.8);
    add("MatT0",m);
    add("Cable",m);
  }

  {
    //MatCap1
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0, 0, 0.2);
    add("MatCap1",m);
  }

  {
    //MatCap2
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0, 0, 0.27);
    add("MatCap2",m);
  }

  {
    //MatTMT
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.42, 0.42, 0.42);
    add("MatTMT",m);
    add("pix::AsTMT_BL",m);
    add("pix::AsTMT_L1",m);
    add("pix::AsTMT_L2",m);
  }

  {
    //MatGlue
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (1, 1, 0.78);
    add("MatGlue",m);
    add("pix::GlueOmegaStave_BL0",m);
    add("pix::GlueOmegaStave_L10",m);
    add("pix::GlueOmegaStave_L20",m);
    add("OmegaGlue_IBL",m);
  }

  {
    //Glue
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (1, 1, 0.75);
    add("pix::GlueOmegaStave_BL1",m);
    add("pix::GlueOmegaStave_L11",m);
    add("pix::GlueOmegaStave_L21",m);
  }

  {
    //MatPP11
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.08, 0.03, 0.03);
    add("MatPP11",m);
  }

  {
    //MatPP12
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.325, 0.292, 0.257);
    add("MatPP12",m);
  }

  {
    //MatPP13
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.42, 0.38, 0.30);
    m->emissiveColor.setValue (0.028, 0.028, 0.028);
    add("MatPP13",m);
  }

  {
    //MatPP14
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.3, 0.3, 0.3);
    m->emissiveColor.setValue (0.028, 0.028, 0.028);
    add("MatPP14",m);
  }

  {
    //MatPP15
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.2, 0.2, 0.23);
    m->emissiveColor.setValue (0.028, 0.028, 0.028);
    add("MatPP15",m);
  }

  {
    //MatPP16
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.4, 0.4, 0.4);
    m->emissiveColor.setValue (0.028, 0.028, 0.028);
    add("MatPP16",m);
  }

  {
    //MatPP17
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.17, 0.19, 0.16);
    m->emissiveColor.setValue (0.028, 0.028, 0.028);
    add("MatPP17",m);
  }

  {
    //Disk
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.5, 0.5, 0.5);
    m->specularColor.setValue (0.5, 0.5, 0.5);
    m->emissiveColor.setValue (0.028, 0.028, 0.028);
    add("Disk",m);
  }

  {
    //Hybrid
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.15, 0.33, 0);
    m->specularColor.setValue (0.39, 0.39, 0.39);
    m->emissiveColor.setValue (0.028, 0.028, 0.028);
    m->shininess.setValue(.60);
    add("Hybrid",m);
    add("pix::Hybrid",m);
  }

  {
    //Chip
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.8, 0.51, 0.105);
    m->specularColor.setValue (0.39, 0.39, 0.39);
    m->emissiveColor.setValue (0.028, 0.028, 0.028);
    m->shininess.setValue(.60);
    add("Chip",m);
    add("pix::Chip",m);
  }


  {
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.42, 0.42, 0.42);
    add("pix:AsTMT_BL",m);
  }

  {
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.95, 0.52, 0.12);
    m->specularColor.setValue (0.8, 0.75, 0.7);
    m->shininess.setValue(.30);
    add("pix::PigtailCyl",m);
    add("pix::PigtailFlat",m);
  }

  {
    // Rings and Service Support
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.50, 0.49, 0.12);
    m->diffuseColor.setValue (0.665, 0.618, 0.441);
    m->specularColor.setValue(0.51, 0.48, 0.16);
    m->shininess.setValue(0.37);
    //Rings
    add("pix::AsRingCen_BL",m);
    add("pix::AsRingInt_BL",m);
    add("pix::AsRingOut_BL",m);
    add("pix::AsRing_L1",m);
    add("pix::AsRingOut_L1",m);
    add("pix::AsRing_L2",m);
    add("pix::AsRingOut_L2",m);
    //ServiceSupport
    add("pix::ServiceSupport_BL",m);
    add("pix::ServiceSupport_L1",m);
    add("pix::ServiceSupport_L2",m);
  }
  {
    // Halfshell
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.21, 0.23, 0.23);
    m->diffuseColor.setValue (0.1, 0.11, 0.11);
    m->specularColor.setValue(0.31, 0.28, 0.06);
    m->shininess.setValue(0.43);
    add("pix::Halfshell_BL",m);
    add("pix::Halfshell_L1",m);
    add("pix::Halfshell_L2",m);
  }

  {
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.40, 0.42, 0.42);
    m->diffuseColor.setValue (0.36, 0.37, 0.37);
    m->specularColor.setValue(0.51, 0.28, 0.26);
    m->shininess.setValue(0.38);
    add("Prepreg",m);
  }

  {
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.10, 0.12, 0.12);
    m->diffuseColor.setValue (0.09, 0.10, 0.17);
    add("pix::CablesAxial_BL",m);
    add("pix::CoolingAxial_BL",m);
    add("pix::CabCoolAxial_L1",m);
    add("pix::CabCoolAxial_L2",m);
    add("pix::CabCoolRadial_L1",m);
    add("pix::OuterCable_BL",m);
    add("pix::OuterCabCool_L1",m);
    add("pix::OuterCabCool_L2",m);
  }
  {
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.12, 0.14, 0.14);
    m->diffuseColor.setValue (0.10, 0.11, 0.19);
    add("pix::CablesRadial_BL",m);
    add("pix::CoolingRadial_BL",m);
    add("pix::CabCoolRadial_L2",m);
    add("pix::OuterCooling_BL",m);
  }

  {
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.20, 0.25, 0.25);
    m->diffuseColor.setValue (0.22, 0.22, 0.22);
    add("pix::Ulink_BL_A",m);
    add("pix::AsUlink_BL_C",m);
    add("pix::AsUlink_L1",m);
    add("pix::AsUlink_L2",m);
  }

  {
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.40, 0.42, 0.42);
    m->diffuseColor.setValue (0.36, 0.37, 0.37);
    m->specularColor.setValue(0.51, 0.28, 0.26);
    m->shininess.setValue(0.38);
    add("pix::SSR_BL_A",m);
    add("pix::SSR_BL_C",m);
    add("pix::SSR_L1_A",m);
    add("pix::SSR_L1_C",m);
    add("pix::SSR_L2",m);
  }

  {
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.35, 0.37, 0.37);
    m->diffuseColor.setValue (0.31, 0.32, 0.32);
    m->specularColor.setValue(0.51, 0.28, 0.26);
    m->shininess.setValue(0.38);
    add("pix::InnerSkin_BL",m);
    add("pix::InnerSkin_L1",m);
    add("pix::InnerSkin_L2",m);
  }

  {
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.55, 0.57, 0.57);
    m->diffuseColor.setValue (0.51, 0.52, 0.52);
    m->specularColor.setValue(0.51, 0.28, 0.26);
    m->shininess.setValue(0.33);
    add("pix::Fingers1",m);
    add("pix::Fingers2",m);
    add("pix::Fingers3",m);
    add("pix::Fingers4",m);
  }

  {
    //Glass
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.8, 0.9, 1.0);
    m->specularColor.setValue (0.39, 0.39, 0.39);
    m->emissiveColor.setValue (0.028, 0.028, 0.028);
    m->shininess.setValue(.60);
    add("Glass",m);
  }

  {
    SoMaterial *material = new SoMaterial;
    material->diffuseColor.setValue(0.9,0.7,0.5);
    material->ambientColor.setValue(0.57,0.57,0.57);
    material->specularColor.setValue(0.27,0.27,0.27);
    material->shininess.setValue(.80);
    add("AlNitride",material);
    add("Dogleg",material);
    add("BrlBaseBoard",material);
    add("BrlHybrid",material);
    add("BrlBracket",material);
    add("PigTail",material);
  }

  {
    SoMaterial *material = new SoMaterial;
    material->diffuseColor.setValue(0.40,0.60,0.40);
    material->ambientColor.setValue(0.57,0.57,0.57);
    material->specularColor.setValue(0.27,0.27,0.27);
    material->shininess.setValue(.80);
    add("PowerTape",material);
  }

  {
    SoMaterial *material = new SoMaterial;
    material->diffuseColor.setValue(0.57,0.82,0.9);
    material->ambientColor.setValue(0.57,0.57,0.57);
    material->specularColor.setValue(0.27,0.27,0.27);
    material->shininess.setValue(.80);
    add("CoolingBlock",material);
    add("sct::CoolBlockSecHi",material);
    add("sct::CoolBlockSecLo",material);
    add("sct::DiscCoolingInn",material);
    add("sct::DiscCoolingOut",material);
  }

  { /* Large structures in sct */
    SoMaterial *material = new SoMaterial;
    //material->diffuseColor.setValue(0.7,0.6,0.5);
    material->diffuseColor.setValue(0.8,0.7,0.6);
    material->ambientColor.setValue(0.57,0.57,0.57);
    material->specularColor.setValue(0.27,0.27,0.27);
    material->shininess.setValue(.80);
    add("sct::DiscCoolingMid",material);
    add("sct::DiscPowerTapeMid",material);
    add("sct::DiscPowerTapeInn",material);
    add("sct::DiscPowerTapeOut",material);
    add("sct::EMI",material);
    add("sct::EMIJoint",material);
    add("sct::Flange0",material);
    add("sct::Flange1",material);
    add("sct::Flange2",material);
    add("sct::Flange3",material);
    add("sct::FwdFrontSupport",material);
    add("sct::FwdITE",material);
    add("sct::FwdLMT",material);
    add("sct::FwdLMTCooling",material);
    add("sct::FwdOTE",material);
    add("sct::FwdRearSupport",material);
    add("sct::FwdRail",material);
    add("sct::FwdSupport",material);
    add("sct::FwdSpineMid",material);
    add("sct::FwdSpineOut",material);
    add("sct::FwdSpineInn",material);
    add("sct::Harness",material);
    add("sct::OptoHarnessO",material);
    add("sct::OptoHarnessOM",material);
    add("sct::OptoHarnessOMI",material);
    add("sct::Spider",material);
    add("sct::SupportCyl0",material);
    add("sct::SupportCyl1",material);
    add("sct::SupportCyl2",material);
    add("sct::SupportCyl3",material);
    add("sct::TSCylinder",material);
  }

  { /* Details in sct (A) */
    SoMaterial *material = new SoMaterial;
    material->diffuseColor.setValue(0.6,0.525,0.45);
    material->ambientColor.setValue(0.57,0.57,0.57);
    material->specularColor.setValue(0.27,0.27,0.27);
    material->shininess.setValue(.80);
    add("sct::FwdHybrid",material);
    add("sct::CoolBlockMainHi",material);
    add("sct::CoolBlockMainLo",material);
  }

  { /* Details in sct (B) */
    SoMaterial *material = new SoMaterial;
    material->diffuseColor.setValue(0.4,0.35,0.30);
    material->ambientColor.setValue(0.57,0.57,0.57);
    material->specularColor.setValue(0.27,0.27,0.27);
    material->shininess.setValue(.80);
    add("sct::CFiberInterLink",material);
    add("sct::Clamp0",material);
    add("sct::Clamp1",material);
    add("sct::Clamp2",material);
    add("sct::Clamp3",material);
    add("sct::CoolingEnd0",material);
    add("sct::CoolingEnd1",material);
    add("sct::CoolingEnd2",material);
    add("sct::CoolingEnd3",material);
    add("sct::CoolingPipe",material);
    add("sct::DiscFixation",material);
    add("sct::DiscSupport0",material);
    add("sct::DiscSupport1",material);
    add("sct::ModuleConnector",material);
    add("sct::DiscSupport2",material);
    add("sct::DiscSupport3",material);
    add("sct::DiscSupport4",material);
    add("sct::DiscSupport5",material);
    add("sct::DiscSupport6",material);
    add("sct::DiscSupport7",material);
    add("sct::DiscSupport8",material);
    add("sct::FSIBL",material);
    add("sct::FSIBH",material);
    add("sct::FSIFL",material);
    add("sct::FSIFH",material);
    add("sct::FwdCoolingPipe",material);
    add("sct::FwdFlangeFrontInn",material);
    add("sct::FwdFlangeFrontOut",material);
    add("sct::FwdFlangeRearInn",material);
    add("sct::FwdFlangeRearOut",material);
    add("sct::FwdNPipe",material);
    add("sct::FwdShuntShield",material);
    add("sct::PPConnector",material);
    add("sct::PPCooling",material);
    add("sct::PPF0c",material);
    add("sct::PPF0e",material);
    add("sct::PPF0o",material);
    add("sct::TSBulkhead",material);
    add("sct::TSEndPanel",material);
  }
  {
    SoMaterial * m = new SoMaterial;
    m->diffuseColor.setValue(SbColor(0.33333,0.33333,0.49804));
    m->shininess.setValue(0);
    add("sct::FwdFibres",m);
  }

  { 
    // NSW - sTGC
    SoMaterial * m = new SoMaterial;
    m->ambientColor.setValue(0.2, 0.2, 0.2);
    setColorFromRGB(m, "diffuse", 4, 142, 167); // #048EA7 - Blue Munsell (greenish azure)
    m->specularColor.setValue(0,0,0);
    m->shininess.setValue(0.2);
    add("Honeycomb",m);
    add("muo::Honeycomb",m); // there's a typo in the Muon material, should be "muon::", not "muo::" ...

  }
  {
    // NSW - MicroMegas (MM)
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 212, 194, 126); // #D4C27E - Ecru
    m->specularColor.setValue (0.5, 0.5, 0.5);
    m->shininess.setValue(0.2);
    add("PCB",m);
    add("sct::PCB",m);
  }
  {
    // NSW - Shield Steel
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.424238, 0.424238, 0.424238);
    m->specularColor.setValue(0.168, 0.168, 0.168);
    m->shininess.setValue(0.153696);
    add("ShieldSteel",m);
    add("shield::ShieldSteel",m);
  }
 
  /* WIP
  {
    // NSW - Special material for the Run3 Detector Pape
    // The aim is to colorize the JD shield plate 
    // in contrasting color, for the NSW images 
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 114, 83, 143); // 72538F - Royal Purple
    //m->diffuseColor.setValue (0.424238, 0.424238, 0.424238);
    m->specularColor.setValue(0.168, 0.168, 0.168);
    m->shininess.setValue(0.153696);
    add("shield::ShieldSteel",m);
  }
  */

  // Adding ITk colors
  // PP0 material
  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 255, 128, 0);
    m->shininess.setValue(0.67);
    // inner pixel system
    add( "PP0BMaterial",m);
    add( "PP0CMaterial",m);
    add( "PP0DMaterial",m);
    add( "PP0SMaterial",m);
    add( "PP0QMaterial",m);
    add( "SvcBrlPP0_30_Hor2_L2_Sec0_Material",m);
    add( "SvcBrlPP0_40_Hor3_L3_Sec0_Material",m);
    add( "SvcBrlPP0_50_Hor4_L4_Sec0_Material",m);
    add( "OutPixIncSec2PP0Material",m);
    add( "OutPixIncSec3PP0Material",m);
    add( "OutPixIncSec4PP0Material",m);
  }

  // Services and Cooling
  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 0, 70, 0);
    m->shininess.setValue(0.67);
    // inner pixel system
    add( "Type1ServiceBMaterial",m);
    add( "Type1ServiceCMaterial",m);
    add( "Type1ServiceDMaterial",m);
    add( "Type1ServiceSMaterial",m);
    add( "Type1ServiceQMaterial",m);
    add( "Type1CoolingBMaterial",m);
    add( "Type1CoolingEMaterial",m);

    // PP1
    add( "matPixType2D",m);
    add( "matPixType2F",m);
    add( "matPixType2H",m);
    add( "matPixType2I",m);
    add( "matPixType2L",m);
    add( "matPixReadout1",m);
    add( "matPixReadout2",m);
    add( "matPixType2G",m);
    add( "matPixType2E",m);
    add( "PP1_T2_Power_lowr",m);
    add( "PP1_T2_Power",m);
    add( "PP1_T2_Power_midr",m);
    add( "PP1_T1_Inner_Cone",m);
    add( "PP1_T1_Outer_Cone",m);
    add( "matPP1Type1PixOuter",m);
    add( "matPP1OuterConnectors",m);
    add( "PP1_T1_Inner",m);
    add( "PP1_T1_Outer",m);
    add( "matPP1Type1PixInner",m);
    add( "PP1_T1_powerconnector_Al",m);
    add( "matPP1InnerConnectors",m);
    add( "PP1_T2_Power_highr",m);
    add( "PP1_T2_cooling_quadrant",m);
    add( "pixSvc_PP1_T2_R347_R420_CoolingInner",m);
    add( "MatB_PP1",m);
    add( "MatEC_PP1",m);

  }

  // other PP1
  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 76, 76, 204);
    m->shininess.setValue(0.67);
    // inner pixel system
    add( "HeatExchanger",m);
    add( "Pix_PP1OuterServices",m);
    add( "PP1_T1_Outer_Cyl",m);
    add( "PP1_T1_Inner_Cyl",m);
    add( "matPixCoolingSum",m);
    add( "AlAnticorodal",m);
    add( "matHeatExchanger",m);
    add( "PP1_T1_cooling_Steel",m);
    add( "matPixCoolingInner",m);
    add( "matPixCoolingOuter",m);

  }

  // ITk Strip detector
  {
    // Aluminium
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue (0.70, 0.72, 0.72);
    m->diffuseColor.setValue (0.56, 0.57, 0.57);
    m->specularColor.setValue(0.71, 0.48, 0.46);
    m->shininess.setValue(0.23);
    add("AlMetal",m);
  }

  {
    // Silicon
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 46, 87, 11);
    m->shininess.setValue(0.67);
    add( "SiMetal", m);
  }

  {
    // Titanium
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.62, 0.62, 0.62);
    m->specularColor.setValue (0.294, 0.294, 0.294);
    m->shininess.setValue(.20);
    add("TiMetal",m);
    add("Ti6Al4V",m);
  }

  {
    // Copper
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.36, 0.43, 1.0);
    m->specularColor.setValue (1.0, 1.0, 1.0);
    m->shininess.setValue(.40);
    add("CuMetal",m);
  }

  // a lot of flanges and mounting structures
  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 150, 0, 0);
    m->shininess.setValue(0.67);
    m->transparency.setValue(0.5);
    add( "CFRP",m);
    add( "matSVatBulkhead",m);
    add( "Honeycomb2pcf",m);
    add( "Honeycomb3pcf",m);
    add( "Honeycomb10pcf",m);
    add( "CFRP174",m);
    add( "CFRP179",m);
    add( "matStiffDiscAve",m);
    add( "SS304",m);
    add( "K13C2U",m);
    add( "k9Allcomp",m);
    add( "Peek",m);
    add( "K13D2U",m);
    add( "CFoam",m);
  }

  // polymoderator
  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 204, 178, 153);
    m->shininess.setValue(0.67);
    add( "SWX-201HD1Z",m);
    add( "BoratedPolyethylene",m);
  }

  {
    // Tungsten
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (0.14, 0.14, 0.14);
    m->specularColor.setValue(0.84, 0.94, 1.00);
    m->shininess.setValue(0.20);
    add( "matDCDC_PCB",m);
    add( "matDCDC_Box",m);
  }

  {
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (1, 1, 0.78);
    add( "SE4445",m);
    add( "GraphiteLoadedEpoxy",m);
  }

  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 0, 70, 0);
    m->shininess.setValue(0.67);
    add( "matEOS",m);
    add( "Torlon",m);
    add( "TorlonForCloseoutFar",m);
    add( "TorlonForCloseoutNear",m);
    add( "T300CF",m);
  }

  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 186, 122, 29);
    m->shininess.setValue(0.67);
    add( "matB_HybridPCB",m);
    add( "matEC_HybridPCB",m);
    add( "matEC_HybridR0H0",m);
    add( "matEC_HybridR0H1",m);
    add( "matEC_HybridR1H0",m);
    add( "matEC_HybridR1H1",m);
    add( "matEC_HybridR2H0",m);
    add( "matEC_HybridR3H0",m);
    add( "matEC_HybridR3H1",m);
    add( "matEC_HybridR3H2",m);
    add( "matEC_HybridR3H3",m);
    add( "matEC_HybridR4H0",m);
    add( "matEC_HybridR4H1",m);
    add( "matEC_HybridR5H0",m);
    add( "matEC_HybridR5H1",m);
    add( "matPetalBusKapton",m);
  }

  {
    // C02:
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue(0.2, 0.2, 0.2);
    m->diffuseColor.setValue(0.58, 0.47, 0.81);
    m->specularColor.setValue(0.56, 0.55, 0.56);
    add("CO2Liquid",m);
  }

//////////////
  init();
}

//////////////////////////////// Attributes for volumes ////////////////////////////////

VolVisAttributes::VolVisAttributes() {

  // WARM CYLINDER
  {
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue(1, .4, .4);
    m->diffuseColor.setValue(1, .4, .4);
    m->specularColor.setValue(0.441667, 0.441667, 0.441667);
    m->shininess.setValue(0.67);
    add( "LAr::Barrel::Cryostat::Cylinder::#4",m);
    add( "LAr::Barrel::Cryostat::Cylinder::#5",m);
    add( "LAr::Barrel::Cryostat::Cylinder::#6",m);
    add( "LAr::Barrel::Cryostat::Cylinder::#7",m);
    add( "LAr::Barrel::Cryostat::Cylinder::#8",m);
    add( "LAr::Barrel::Cryostat::Cylinder::#9",m);
    add( "LAr::Barrel::Cryostat::InnerWall",m);
    add( "LAr::Barrel::Cryostat::InnerEndWall",m);
    add( "LAr::Barrel::Cryostat::Leg",m);
  }

  {
    // WARM CYLINDER
    SoMaterial *m = new SoMaterial;
    m->ambientColor.setValue(.4, .4, 1);
    m->diffuseColor.setValue(.4, .4, 1);
    m->specularColor.setValue(0.441667, 0.441667, 0.441667);
    m->shininess.setValue(0.67);
    add( "LAr::Barrel::Cryostat::Cylinder::#0",m);
    add( "LAr::Barrel::Cryostat::Cylinder::#1",m);
    add( "LAr::Barrel::Cryostat::Cylinder::#2",m);
    add( "LAr::Barrel::Cryostat::Cylinder::#3",m);
    add( "LAr::Barrel::Cryostat::Ear",m);
    add( "LAr::Barrel::Cryostat::OuterWall",m);
  }

  {
    SoMaterial *m = new SoMaterial;
    m->diffuseColor.setValue (1, 1, 0.5);
    m->ambientColor.setValue (0.54, 0.54, 0.27);
    m->emissiveColor.setValue (0.133, 0.133, 0.067);
    add("bcmModLog",m);
    add("bcmWallLog",m);
  }

  
  
  {
    // Cutout - EMEC
    SoMaterial *m = new SoMaterial;
    //setColorFromRGB(m, "diffuse", 255, 190, 11); // ffbe0b - Mango (lighter)
    setColorFromRGB(m, "diffuse",245, 180, 0); // F5B400 - Selective Yellow (darker)
    m->shininess.setValue(0.67);
    add( "LAr::EMEC::Mother",m);
  }
  {
    // Cutout - HEC
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 47, 25, 95); // 2f195f - Russian Violet (darker)
    //setColorFromRGB(m, "diffuse",56, 30, 113); // 381E71 - Persian Indigo (lighter)
    m->shininess.setValue(0.67);
    add( "LAr::HEC::LiquidArgon",m);
  }
  {
    // Cutout - FCAL
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 246, 247, 64); // f6f740 - Maximum Yellow
    m->shininess.setValue(0.67);
    add( "LAr::FCAL::LiquidArgonC",m);
    add( "LAr::FCAL::LiquidArgonA",m);
  }
 
  {
    // Cutout - TRT
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 114, 83, 143); // 72538F - Royal Purple
    m->shininess.setValue(0.67);
    add( "TRTEndcapWheelAB",m);
    add( "TRTBarrel",m);
  }
  
  {
    // Cutout - SCT
    SoMaterial *m = new SoMaterial;
    //setColorFromRGB(m, "diffuse", 80, 255, 177); // 50FFB1 - Medium Spring Green
    setColorFromRGB(m, "diffuse", 170, 255, 255); // AAFFFF - Celeste
    m->shininess.setValue(0.67);
    add( "SCT_Barrel",m);
    add( "SCT_ForwardA",m);
    add( "SCT_ForwardC",m);
  }

  // Detailed Muon View for Run3 Detector Paper
  // RPC layers
  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 61, 52, 139); // 3D348B - Dark Slate Blue
    m->shininess.setValue(0.67);
    //add( "RPC_AL_extsuppanel",m); // WIP
    add( "Rpclayer",m);
  }

  // Adding ITk colors
  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 46, 87, 11);
    m->shininess.setValue(0.67);
    // innermost pixel barrel layer
    add( "InnerBarrelSingleMod_Sensor",m);
    add( "InnerBarrelSingleMod_DeadVolume",m);
    // next-to-innermost pixel barrel layer
    add( "InnerBarrelQuadMod_Sensor",m);
    // innermost pixel endcap layer
    add( "InnerRingSingleMod_Sensor",m);
    // next-to-innermost pixel endcap layer
    add( "InnerEndcapQuadMod_Sensor",m);
    // outer pixel barrel layers
    add( "OuterBarrelQuadMod_Sensor",m);
    add( "InclinedQuadMod_Sensor",m);
    // outer pixel endcap layers
    add( "OuterEndcapQuadMod_Sensor",m);
  }

  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 140, 143, 143);
    m->shininess.setValue(0.67);
    // innermost pixel barrel layer
    add( "InnerBarrelSingleMod_Chip",m);
    add( "InnerBarrelSingleMod_Bonding",m);
    // innermost pixel endcap layer
    add( "InnerRingSingleMod_Chip",m);
    add( "InnerRingSingleMod_Bonding",m);
    // next-to-innermost pixel layers
    add( "InnerQuadMod_Chip",m);
    add( "InnerQuadMod_Bonding",m);
    // outer pixel layers
    add( "OuterQuadMod_Chip",m);
    add( "OuterQuadMod_Bonding",m);
  }

  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 186, 122, 29);
    m->shininess.setValue(0.67);
    // innermost pixel barrel layer
    add( "InnerBarrelSingleMod_Pigtail",m);
    // next-to-innermost pixel layers
    add( "InnerQuadMod_QuadPigtail",m);
    add( "InnerQuadMod_QuadFlex",m);
    // outer pixel layers
    add( "OuterQuadMod_QuadFlex",m);
  }

  // Services and cooling
  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 0, 70, 0);
    m->shininess.setValue(0.67);
    // inner pixel system
    add( "InnerPixelBarrel_T0",m);
    add( "InnerPixEndcap_L0T0Back_ring",m);
    add( "InnerPixEndcap_L1T0Front_ring",m);
    add( "InnerPixEndcap_L1T0Back_ring",m);
    add( "InnerPixEndcap_L2T0Front_ring",m);
    add( "InnerPixEndcap_L2T0Back_ring",m);
    // outer pixel system
    add( "L2HalfRingEndCapBusTapeRing",m);
    add( "L3HalfRingEndCapBusTapeRing",m);
    add( "L4endcapBusTape",m);

    //
    unsigned int sectors = 9;
    std::vector<unsigned int> layers = {2, 3, 4};
    for (auto& layer : layers) {
      for (unsigned int sector = 0; sector<sectors; sector++) {
        std::string volumeName = "Pixel__ModuleSvcM" + std::to_string(sectors-sector) + "_L" + std::to_string(layer) + "_S" + std::to_string(sector);
        add( volumeName,m);
      }
    }
    add( "PixelSvcBrlT1_L2",m);
    add( "PixelSvcBrlT1_L3",m);
    add( "PixelSvcBrlT1_L4in",m);
    add( "PixelSvcBrlT1_L4out",m);
    for (unsigned int sector = 0; sector<sectors; sector++) {
      std::string volumeName = "PixelSvcBrlT1_Radial" + std::to_string(sectors-sector);
      add( volumeName,m);
    }

    add( "svcEcT1_r199_z103",m);
    add( "svcEcT1_r199_z116",m);
    add( "svcEcT1_r199_z127",m);
    add( "svcEcT1_r199_z141",m);
    add( "svcEcT1_r199_z1366",m);
    add( "svcEcT1_r198_z172",m);
    add( "svcEcT1_r198_z190",m);
    add( "svcEcT1_r198_z210",m);
    add( "svcEcT1_r198_z232",m);
    add( "svcEcT1_r198_z257",m);
    add( "svcEcT1_r198_z149",m);

    add( "svcEcT1_r259_z151",m);
    add( "svcEcT1_r259_z176",m);
    add( "svcEcT1_r259_z203",m);
    add( "svcEcT1_r259_z234",m);
    add( "svcEcT1_r259_z270",m);
    add( "svcEcT1_r259_z311",m);
    add( "svcEcT1_r259_z359",m);
    add( "svcEcT1_r259_z149",m);

    add( "svcEcT1_r319_z131",m);
    add( "svcEcT1_r319_z150",m);
    add( "svcEcT1_r319_z170",m);
    add( "svcEcT1_r319_z192",m);
    add( "svcEcT1_r319_z218",m);
    add( "svcEcT1_r319_z246",m);
    add( "svcEcT1_r319_z746",m);
    add( "svcEcT1_r318_z317",m);
    add( "svcEcT1_r318_z149",m);

    add( "SvcEcT0TwdBS_r156_199_z.5",m);
    add( "SvcEcT0TwdBS_r156_198_z.5",m);
    add( "SvcEcT0TwdBS_r199_199_z.5",m);
    add( "SvcEcT0TwdBS_r216_259_z.5",m);
    add( "SvcEcT0TwdBS_r276_319_z.5",m);
    add( "SvcEcT0TwdBS_r276_318_z.5",m);
    add( "SvcEcT0TwdBS_r318_319_z.5",m);

    add( "SvcEc_r196.7_197.1_z78.6",m);
    add( "SvcEc_r196.7_197.1_z97.6",m);
    add( "SvcEc_r196.7_197.1_z121.2",m);
    add( "SvcEc_r196.7_197.1_z150.3",m);
    add( "SvcEc_r196.7_197.1_z186.7",m);
    add( "SvcEc_r196.7_197.1_z23.3",m);
    add( "SvcEc_r337.8_338.2_z1939.2",m);


    add( "SvcEc_r264.4_264.8_z58.1",m);
    add( "SvcEc_r264.4_264.8_z66.6",m);
    add( "SvcEc_r264.4_264.8_z76.4",m);
    add( "SvcEc_r264.4_264.8_z87.5",m);
    add( "SvcEc_r264.4_264.8_z100.3",m);
    add( "SvcEc_r264.4_264.8_z115",m);
    add( "SvcEc_r264.4_264.8_z131.9",m);
    add( "SvcEc_r264.4_264.8_z30.9",m);
    add( "SvcEc_r338.8_339.2_z1937.2",m);

    add( "SvcEc_r327.3_327.7_z52.7",m);
    add( "SvcEc_r327.3_327.7_z58.8",m);
    add( "SvcEc_r327.3_327.7_z65.6",m);
    add( "SvcEc_r327.3_327.7_z73.1",m);
    add( "SvcEc_r327.3_327.7_z81.7",m);
    add( "SvcEc_r327.3_327.7_z91.1",m);
    add( "SvcEc_r327.3_327.7_z101.6",m);
    add( "SvcEc_r327.3_327.7_z113.4",m);
    add( "SvcEc_r327.3_327.7_z34.7",m);
    add( "SvcEc_r339.8_340.2_z1935.2",m);

    add( "SvcEc_r197.1_202.8_z.4",m);
    add( "SvcEc_r203.2_263.4_z.4",m);
    add( "SvcEc_r264.8_270.8_z.4",m);
    add( "SvcEc_r271.2_326.3_z.4",m);
    add( "SvcEc_r326.7_327.3_z.4",m);
    add( "SvcEc_r327.7_333.8_z.4",m);
    add( "SvcEc_r334.2_337.3_z.4",m);
    add( "SvcEc_r337.7_337.8_z.4",m);
    add( "SvcEc_r338.2_338.3_z.4",m);
    add( "SvcEc_r338.7_338.8_z.4",m);
    add( "SvcEc_r339.2_339.3_z.4",m);
    add( "SvcEc_r339.7_339.8_z.4",m);

    // ITk Strip
    add ("SV_Barrel01",m);
    add ("SV_Barrel12",m);
    add ("SV_Barrel23",m);
    add ("SV_Barrel3Out",m);
    add ("SV_BarrelPastEndcap",m);
    add ("SV_Endcap01",m);
    add ("SV_Endcap12",m);
    add ("SV_Endcap23",m);
    add ("SV_Endcap34",m);
    add ("SV_Endcap45",m);
    add ("SV_Endcap5Out",m);

  }

  // Cooling
  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 170, 255, 255);
    m->shininess.setValue(0.67);
    // inner pixel system
    add( "InnerPixelBarrelCoolingL0",m);
    add( "InnerPixelBarrelCoolingL1",m);
    // outer pixel system
    add( "OuterQuadMod_Cell",m);
    add( "LongeronCoolingTube",m);
    add( "L2HalfRingCoolingPipe",m);
    add( "L3HalfRingCoolingPipe",m);
    add( "L4endcapcoolingpipe",m);
  }

  // Support structures
  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 150, 0, 0);
    m->shininess.setValue(0.67);
    m->transparency.setValue(0.5);
    // inner pixel system
    add( "IPTvol",m);
    add( "ISTvol",m);
    add( "InnerPixBarrelSupport_Stave",m);
    add( "InnerPixBarrelSupport_Stave1",m);
    add( "InnerPixBarrelSupport_Global",m);
    add( "InnerPixEndcap_CoupledRingSupport",m);
    add( "InnerPixEndcap_IntermediateRingSupport",m);
    add( "InnerPixEndcap_L1RingSupport",m);
    add( "QuarterShell",m);
    // outer pixel system
    add( "InclL2HalfShell",m);
    add( "InclL2Support",m);
    add( "InclL3HalfShell",m);
    add( "InclL3Support",m);
    add( "InclL4HalfShell",m);
    add( "InclL4Support",m);
    add( "LongeronCornerBase",m);
    add( "LongeronCornerEnd",m);
    add( "LongeronTrussWall",m);
    add( "LongeronCapBase",m);
    add( "LongeronTopCap",m);
    add( "L2HalfRingCarbonFoamInner",m);
    add( "L2HalfRingCarbonFoamOuter",m);
    add( "L2HalfRingFaceSheet",m);
    add( "L3HalfRingCarbonFoamInner",m);
    add( "L3HalfRingCarbonFoamOuter",m);
    add( "L3HalfRingFaceSheet",m);
    add( "L4endcapinnerCarbonFoam",m);
    add( "L4endcapouterCarbonFoam",m);
    add( "L4endcapFaceSheet",m);
    add( "L2HalfShell",m);
    add( "L3HalfShell",m);
    add( "L4HalfShell",m);
  }

  // Other support structures
  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 204, 178, 153);
    m->shininess.setValue(0.67);

    add( "L2HalfRingEndCapFixingLug",m);
    add( "L3HalfRingEndCapFixingLug",m);
    add( "HalfRingEndCapFixingLug",m);

    add( "FrontSupportFacing",m);
    add( "FrontSupportCore",m);
    add( "RearSupport",m);


  }

  // volumes that don't need colors as they are made of air
  {
    SoMaterial *m = new SoMaterial;
    setColorFromRGB(m, "diffuse", 204, 178, 153);
    m->transparency.setValue(1.0);

    add( "StaveCchannelAirEOS",m);
    add( "StaveCchannelAirLong",m);
    add( "StaveCchannelAirLongEOS",m);
    add( "PetalCloseoutShortSpace",m);
    add( "PetalCloseoutLongSpace",m);

  }
  
  init();
}
