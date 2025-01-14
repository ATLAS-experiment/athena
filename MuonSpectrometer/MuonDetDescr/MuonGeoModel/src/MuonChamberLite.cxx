/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
//
#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonGeoModel/Csc.h"
#include "MuonGeoModel/Cutout.h"
#include "MuonGeoModel/Ded.h"
#include "MuonGeoModel/Mdt.h"
#include "MuonGeoModel/MuonChamberLite.h"
#include "MuonGeoModel/Position.h"
#include "MuonGeoModel/Rpc.h"
#include "MuonGeoModel/Spacer.h"
#include "MuonGeoModel/SpacerBeam.h"
#include "MuonGeoModel/Station.h"
#include "MuonGeoModel/Tgc.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MuonStation.h"
//
#include "MuonGeoModel/CbmComponent.h"
#include "MuonGeoModel/CscComponent.h"
#include "MuonGeoModel/LbiComponent.h"
#include "MuonGeoModel/MdtComponent.h"
#include "MuonGeoModel/RpcComponent.h"
#include "MuonGeoModel/StandardComponent.h"
#include "MuonGeoModel/SupComponent.h"
#include "MuonGeoModel/TgcComponent.h"
#include "MuonReadoutGeometry/CscReadoutElement.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"

// just to check subtype, cutout:
#include "MuonGeoModel/CSC_Technology.h"
#include "MuonGeoModel/LBI_Technology.h"
#include "MuonGeoModel/MDT_Technology.h"
#include "MuonGeoModel/MYSQL.h"
#include "MuonGeoModel/RPC_Technology.h"
#include "MuonGeoModel/TGC_Technology.h"
//
#include "MuonIdHelpers/CscIdHelper.h"
#include "MuonIdHelpers/MdtIdHelper.h"
#include "MuonIdHelpers/RpcIdHelper.h"
#include "MuonIdHelpers/TgcIdHelper.h"
//
#include "GaudiKernel/SystemOfUnits.h"
#include "GeoModelKernel/GeoDefinitions.h"


#include <fstream>
#include <format>
#include <iomanip>
#include <vector>
#include <stdexcept>
#include <set> 

#define RPCON true
#define useAssemblies false
namespace {
    // const maps holding the y/z translation for BIS RPCs (since they cannot be parsed by amdb)
    const std::map<std::string, float> rpcYTrans = {
        std::make_pair<std::string, float>("RPC26", -9.1),  // big RPC7
        std::make_pair<std::string, float>("RPC27", -9.1),  // small RPC7
        std::make_pair<std::string, float>("RPC28", -27.7), // big RPC8
        std::make_pair<std::string, float>("RPC29", -8.8),  // small RPC8
    };
    const std::map<std::string, float> rpcZTrans = {
        std::make_pair<std::string, float>("RPC26", 3.22), // big RPC7
        std::make_pair<std::string, float>("RPC27", 3.06), // small RPC7
        std::make_pair<std::string, float>("RPC28", 3.11), // big RPC8
        std::make_pair<std::string, float>("RPC29", 3.11), // small RPC8
    };
} // namespace



namespace MuonGM {

    // cutouts for BMS at eta=+-1 and phi=4 (RPC/DED/MDT) ok //16 tubes shorter + entire RPC and DED (of dbz1) narrower

  MuonChamberLite::MuonChamberLite(const MYSQL& mysql, Station *s,
                   std::map<std::string, GeoFullPhysVol*> * mapFPV,
                   std::map<std::string, GeoAlignableTransform *> *mapAXF) : 
    DetectorElement(s->GetName()),
    AthMessaging{"MuonchamberLite"},
    m_mapFPV(mapFPV),
    m_mapAXF(mapAXF)
  {
        width = s->GetWidth1();
        longWidth = s->GetWidth2();
        thickness = s->GetThickness(mysql);
        length = s->GetLength();
        m_station = s;

        // CSL envelope is too small for its components - enlarge it slightly
        std::string stname(m_station->GetName(), 0, 3);
        if (stname == "CSL")
            longWidth *= 1.015;

       
    }

    GeoVPhysVol *MuonChamberLite::addReadoutLayers(
                                    const MYSQL& mysql,
                                    MuonDetectorManager *manager, int zi, int fi, bool is_mirrored, bool &isAssembly) {


        
        ATH_MSG_VERBOSE( " Building a MuonChamberLite for m_station " << m_station->GetName() << " at zi, fi " << zi << " " << fi + 1 << " is_mirrored " << is_mirrored
                        << " is assembly = " << isAssembly );
        
        std::string stname(m_station->GetName(), 0, 3);

        double halfpitch = m_station->mdtHalfPitch(mysql);
        const std::string stName = m_station->GetName();

        const MdtIdHelper *mdt_id = manager->mdtIdHelper();
        int stationType = mdt_id->stationNameIndex(stName.substr(0, 3));
        
        double extratop = m_station->GetExtraTopThickness();
        double extrabottom = m_station->GetExtraBottomThickness();
        double totthick = thickness + extratop + extrabottom;


        double amdbOrigine_along_length = m_station->getAmdbOrigine_along_length();
        double amdbOrigine_along_thickness = m_station->getAmdbOrigine_along_thickness(mysql);


        // This will allow the MDT tube structure to be mirrored w.r.t. the chamber at z>0
        // and to correctly place any other component in the m_station
        if (zi < 0 && !is_mirrored && stName[0] == 'B') {
            if (m_station->hasMdts()) {
                amdbOrigine_along_length += halfpitch;
            }
        }
        ATH_MSG_VERBOSE( "amdb origine: in the length direction = " 
                            << amdbOrigine_along_length << " in the thickness direction = " << amdbOrigine_along_thickness);
        
        if (isAssembly) {
            ATH_MSG_DEBUG("Station  " << stName << " at zi, fi " << zi << " " << fi + 1 << " will be described as  Assembly" );
        
        }

        // for BOG in layout Q we will have to shorten CHV, CMI as these
        //   are not shortened in AMDB
        

        // if this is a BOG, we want to make cutouts in the MOTHER VOLUME
        if (stName.compare(0, 3, "BOG") == 0 && (manager->IncludeCutoutsBogFlag() || manager->IncludeCutoutsFlag())) {

            ATH_MSG_VERBOSE( "amdb org: length= " << amdbOrigine_along_length << " thickness= " << amdbOrigine_along_thickness );

            std::string statType = stName.substr(0, 3);
            if (m_station->GetNrOfCutouts() > 0) {
                ATH_MSG_DEBUG( "Station  " << stName << " at zi, fi " << zi << " " << fi + 1 << " has components with cutouts " );
                isAssembly = true;

                // look for FIRST component with cutouts and loop over all of the cutouts:
                bool foundCutouts = false;
                for (int j = 0; j < m_station->GetNrOfComponents(); j++) {
                    StandardComponent *c = (StandardComponent *)m_station->GetComponent(j);

                    if (!foundCutouts) {
                        for (int ii = 0; ii < m_station->GetNrOfCutouts(); ii++) {
                            Cutout *cut = m_station->GetCutout(ii);
                            // if this is a BOG in layout Q, set the CP param:
                            //   (both cuts have same length so ok to reset it)
                            // also do here some tweaking to prevent undershoot
                            //  of the cutouts wrt mother volume:
                            if (std::abs(cut->dx - 600.7) < 0.1) {
                                cut->dx = cut->dx + 10. * Gaudi::Units::mm;
                                cut->widthXs = cut->widthXs + 20. * Gaudi::Units::mm;
                                cut->widthXl = cut->widthXl + 20. * Gaudi::Units::mm;
                            }
                            if (std::abs(cut->dx + 600.7) < 0.1) {
                                cut->dx = cut->dx - 10. * Gaudi::Units::mm;
                                cut->widthXs = cut->widthXs + 20. * Gaudi::Units::mm;
                                cut->widthXl = cut->widthXl + 20. * Gaudi::Units::mm;
                            }
                            if (std::abs(cut->lengthY - 180.2) < 0.001) {
                                cut->lengthY = cut->lengthY + (0.010) * Gaudi::Units::mm;
                            }
                            if (std::abs(cut->dy - 1019.8) < 0.001) {
                                cut->dy = 1216.4185 - cut->lengthY;
                            }
                            // create the cutout with the full thickness of the STATION
                            cut->setThickness(totthick * 1.01); // extra to be sure
                            if ((cut->subtype == mysql.allocPosFindSubtype(std::string(statType), fi, zi)) && (cut->icut == mysql.allocPosFindCutout(std::string(statType), fi, zi)) &&
                                (cut->ijob == c->index)) {

                                foundCutouts = true;
                            }
                        } // Loop over cutouts
                    }     // If no cutouts
                }         // Loop over components
            }
        } // end of special loop just for cutouts



        double ypos{0.}, zpos{0.}, xpos{0.}, irad{0.};
        std::array<int, 2 > ndbz{0, 0};

        // Compute how many RPC modules there are in the m_station
        int nDoubletR{0}, nRpc{0}, nTgc{0}, nCsc{0}, nMdt{0};
        double previous_depth = 0.;

        ATH_MSG_VERBOSE( " Station Name = " << stName << " fi/zi " << fi << "/" << zi << " defining the n. of DoubletR to " );
        

        for (int j = 0; j < m_station->GetNrOfComponents(); j++) {
            StandardComponent *d = (StandardComponent *)m_station->GetComponent(j);
            std::string_view cn = std::string_view(d->name).substr(0, 3);
            if (cn == "RPC") {
                nRpc++;
                if (nRpc == 1)
                    nDoubletR++;
                double depth = -thickness / 2. + d->posz + d->GetThickness(mysql) / 2.;
                // BI RPC Chambers have one one doubletR
                if (!(stname.compare(0, 2, "BI") == 0) && nDoubletR == 1 && nRpc > 1 && depth * previous_depth < 0)
                    nDoubletR++;

                previous_depth = depth;
            }
            else if (cn == "CSC") {
                nCsc++;
            }
            else if (cn == "TGC") {
                nTgc++;
            }
            else if (cn == "MDT") {
                nMdt++;
            }
        }
        ATH_MSG_DEBUG( "nDoubletR: " << nDoubletR<<" nMdt/Rpc/Tgc/Csc " << nMdt << "/" << nRpc << "/" << nTgc << "/" << nCsc );
        
        // Get location and dimensions of long beams and pass them to cross beams
        // in order to make holes
        int numLB = -1;
        double LBheight{0.}, LBwidth{0.};
        std::array<double, 2> LBpos{-1, -1};
        for (int i = 0; i < m_station->GetNrOfComponents(); i++) {
            StandardComponent *c = (StandardComponent *)m_station->GetComponent(i);
            std::string_view cname = std::string_view(c->name).substr(0, 2);
            if (cname == "LB") {
                const LBI *lb = dynamic_cast<const LBI *>(mysql.GetTechnology(c->name));
                numLB++;
                LBpos[numLB] = c->posy + c->dy / 2.;
                LBheight = lb->height;
                LBwidth = c->dy;
            }
            if (numLB > 0)
                break; // only 2 LBs per chamber
        }

        for (int i = 0; i < m_station->GetNrOfComponents(); i++) {
            StandardComponent *c = (StandardComponent *)m_station->GetComponent(i);
            std::string_view cname = std::string_view(c->name).substr(0, 3);
            if (cname == "CRO" || cname == "CMI" || cname == "CHV") {
                CbmComponent *ccbm = (CbmComponent *)c;
                ccbm->lb_height = LBheight;
                ccbm->lb_width = LBwidth;
                ccbm->hole_pos1 = LBpos[0];
                ccbm->hole_pos2 = LBpos[1];
            }
        }

        // Look for the subtype of the CMI in the chamber to let LB know ...
        std::string CMIcomponentNumber = "";
        for (int j = 0; j < m_station->GetNrOfComponents(); j++) {
            StandardComponent *d = (StandardComponent *)m_station->GetComponent(j);
            std::string_view cn = std::string_view(d->name).substr(0, 3);
            if (cn == "CMI") {
                CMIcomponentNumber = (d->name).substr(3, 2);
                break;
            }
        }

        for (int j = 0; j < m_station->GetNrOfComponents(); j++) {
            StandardComponent *d = (StandardComponent *)m_station->GetComponent(j);
            std::string_view cn = std::string_view(d->name).substr(0, 2);
            if (cn == "LB") {
                LbiComponent *lbic = (LbiComponent *)d;
                if (lbic) {
                    lbic->associated_CMIsubtype = CMIcomponentNumber;
                } else
                    ATH_MSG_ERROR( "MuonChamberLite :: cannot associate a CMI subtype to the LB component " );
            }
        }

        // Build the MuonStation(readout-geometry) corresponding to this MuonChamberLite(raw-geometry)
        MuonStation *mstat;
        if (stName.compare(0, 1, "B") == 0) {
            mstat = new MuonStation(stName.substr(0, 3), width, totthick, length, longWidth, totthick, length, zi, fi + 1,
                                    (zi < 0 && !is_mirrored)); //!< fi here goes from 0 to 7; in amdb from 1 to 8;
        } else {
            mstat = new MuonStation(stName.substr(0, 3), width, length, totthick, longWidth, length, totthick, zi, fi + 1,
                                    (zi < 0 && !is_mirrored)); //!< fi here goes from 0 to 7; in amdb from 1 to 8;
        }
       
        manager->addMuonStation(std::unique_ptr<MuonStation>(mstat));
        ATH_MSG_DEBUG( " Building a MuonStation for this MuonChamberLite " << m_station->GetName() 
                        << " at zi, fi " << zi << " " << fi + 1 << " is_mirrored " << is_mirrored);
        
        GeoFullPhysVol *ptrd=(*m_mapFPV)[std::string(stName)+"_Station"+"_"+std::to_string(zi)+"_"+std::to_string(fi)];
        mstat->setPhysVol(ptrd);
        // here the big loop over the components !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        for (int i = 0; i < m_station->GetNrOfComponents(); i++) {
            StandardComponent *c = (StandardComponent *)m_station->GetComponent(i);
 	     ATH_MSG_VERBOSE( " Component index " << c->index << " in loop for " << stName << " " << stationType << " at zi, fi " << zi << " " << fi + 1 << "  cName "
                              << c->name << " thickness " << c->GetThickness(mysql) << " length " << c->dy << " w, lw " << c->dx1 << " " << c->dx2 );
            ATH_MSG_VERBOSE( " Component local (amdb) coords " << c->posx << " " << c->posy << " " << c->posz );
           
            ypos = -thickness / 2. + c->posz + c->GetThickness(mysql) / 2.;
            zpos = 0.;
            xpos = 0.;

            ypos = -thickness / 2. + (c->posz + amdbOrigine_along_thickness) + c->GetThickness(mysql) / 2.;
            zpos = -length / 2. + amdbOrigine_along_length + c->posy + c->dy / 2.;
            xpos = c->posx;

            const std::string &techname = c->name;
            std::string_view type = std::string_view(techname).substr(0, 3);



            GeoIntrusivePtr<GeoFullPhysVol> lvm{}, lvr{}, lvt{}, lvc{};



            // Are there cutouts?
            std::string statType = stName.substr(0, 3);
            double cthickness = c->GetThickness(mysql);
            int ncutouts = 0;
            std::vector<Cutout *> vcutdef;
            std::vector<std::unique_ptr<Cutout>> vcutdef_todel;
            for (int ii = 0; ii < m_station->GetNrOfCutouts(); ii++) {
               Cutout *cut = m_station->GetCutout(ii);
                cut->setThickness(cthickness * 1.01); // extra thickness to be sure

                if ((cut->subtype == mysql.allocPosFindSubtype(std::string(statType), fi, zi)) && (cut->icut == mysql.allocPosFindCutout(std::string(statType), fi, zi)) && (cut->ijob == c->index)) {

                    double tempdx = cut->dx;
                    double tempdy = cut->dy;
                    double templengthY = cut->lengthY;
                    cut->dx = 0.;
                    cut->dy = 0.;

                    if (stName.compare(0, 3, "BOG") == 0) {
                        // make the cutouts a bit longer
                        cut->lengthY = templengthY + 31.;
                    }

                    cut->dx = tempdx;
                    cut->dy = tempdy;

                    if (std::abs(cut->dead1) > 1. && techname == "MDT03")
                        cut->dy = cut->dy + 15.0 * cos(cut->dead1 * Gaudi::Units::deg);
                    // should compensate for the dy position defined in amdb at the bottom of the foam in ML 1 of EMS1,3 and BOS 6
                    // can be applied only for layout >=r.04.04 in rel 15.6.X.Y due to the frozen Tier0 policy

                    cut->lengthY = templengthY;
                    // in thickness, cutout will coincide with component
                    // not needed (DHW)  double xposcut = 0.;  // rel. to component thickness
                    ncutouts++;
                    ATH_MSG_VERBOSE( "A new cutout for this component " );
                    ATH_MSG_VERBOSE( *cut );
                   
                    // Corrected cutout values for BMS7, BMS14
                    if (stName.compare(0, 3, "BMS") == 0) {
                        if (fi == 3) {               // stationPhi = 4
                            if (std::abs(zi) == 1) { // stationEta = +-1
                                double margin = 1.0; // make cutout a little bigger to avoid coincident boundaries

                                if (type == "RPC" || type == "DED") {
                                    cut->widthXl += 2 * margin;
                                    cut->widthXs += 2 * margin;
                                    cut->dx += margin;
                                    cut->lengthY += 2 * margin;

                                    if (zi > 0)
                                        cut->dy = -margin;
                                }
                            }

                            if (zi == -1) {
                                if (type == "MDT")
                                    cut->dy = 0.;
                            }
                        }
                    }

                    // the following is a fine tuning ----- MUST CHECK for a better solution
                    if (stName.compare(0, 3,"BOS") == 0 && zi == -6 && type == "MDT") {
                        cut->dy = c->dy - cut->dy - cut->lengthY - halfpitch;
                        cut->dead1 = 30.; // why this is not 30. or -30. already ?????
                        if (techname == "MDT03")
                            cut->dy = cut->dy + 30.0; // *cos(cut->dead1*Gaudi::Units::deg);
                        ATH_MSG_VERBOSE( "Cut dead1 for BOS 6 on C side is " << cut->dead1 );
                        
                    }

                    // this mirroring of the cutout is necessary only for barrel MDT chambers; for EC the cutout will be automatically mirrored
                    // this fix cannot be applied in 15.6.X.Y for layout < r.04.04 due to the frozen tier0 policy

                    if (type == "MDT" && (is_mirrored || zi < 0) && stName.compare(0, 1, "B") == 0) {
                        // MDT in chambers explicitly described at z<0 have to be
                        // rotated by 180deg to adj. tube staggering
                        // reverse the position (x amdb) of the cutout if the m_station is mirrored
                        Cutout *cutmirr = new Cutout(*cut);
                        cutmirr->dx = -cutmirr->dx;
                        // this way, after the rotation by 180 Gaudi::Units::deg, the cut will be at the same global phi
                        // it has for the m_station at z>0
                        vcutdef.push_back(cutmirr);
                        vcutdef_todel.emplace_back(cutmirr);
                        ATH_MSG_VERBOSE( "adding for application mirrored cut \n" << *cutmirr );
                        
                    } else if (type == "RPC" || type == "DED") {
                        Cutout *cutRpcType = new Cutout(*cut);
                        // temporary for testing fixes to r.03.09
                        if (stName.compare(0, 3, "BMS") == 0 && zi == 4 && (c->index == 20 || c->index == 21 || c->index == 24 || c->index == 25)) {
                            cutRpcType->dy = 1102.5;
                        }

                        if (stName.compare(0, 3, "BOS") == 0 && zi == 6 && type == "DED")
                            cutRpcType->dy = 706.;

                        cutRpcType->dy = cutRpcType->dy - c->posy;
                        cutRpcType->dx = cutRpcType->dx - c->posx;

                        if (type == "RPC") {
                            RpcComponent *rp = (RpcComponent *)c;
                            if (rp->iswap == -1) {
                                cutRpcType->dy = c->dy - (cutRpcType->dy + cutRpcType->lengthY);
                            }
                        }

                        ATH_MSG_VERBOSE( " Rpc or ded cutout redefined as follows \n" << *cutRpcType );
                        vcutdef.push_back(cutRpcType);
                        vcutdef_todel.emplace_back(cutRpcType);
                    } else if (type == "TGC") {
                        // In AMDB, y coordinates of cutout and component are given by
                        // radius from detector z-axis.  To get standard y value of cutout,
                        // subtract radius of component from radius of cutout
                        Cutout *tgccut = new Cutout(*cut);
                        tgccut->dy -= c->posy; //

                        ATH_MSG_VERBOSE( " Tgc cutout redefined as follows \n" << *tgccut );
                        vcutdef.push_back(tgccut);
                        vcutdef_todel.emplace_back(tgccut);
                    } else {
                        vcutdef.push_back(cut);
                    }
                }
            } // Loop over cutouts in m_station

            if (ncutouts > 0) {
                ATH_MSG_DEBUG( c->name << " of station " << stName << " at fi/zi " << fi + 1 << "/" << zi << " has " << ncutouts << " cutouts " );                
            }
            // define here the total transform that will be applied to component:
            GeoTrf::Transform3D htcomponent(GeoTrf::Transform3D::Identity());
            GeoAlignableTransform *xfaligncomponent{nullptr};
            // for RPCs we need a vector of transforms for M28 geometry...

            if (type == "CRO") {
                if (stName.compare(0, 1, "B") != 0 && is_mirrored)
                    mstat->setxAmdbCRO(-xpos);
                else
                    mstat->setxAmdbCRO(xpos);
            }

            if (type == "MDT") {
                MdtComponent *md= (MdtComponent *) c;
                htcomponent = GeoTrf::TranslateX3D(ypos) * GeoTrf::TranslateZ3D(zpos) * GeoTrf::TranslateY3D(xpos);

                if (zi < 0 && !is_mirrored && stName[0] == 'B') {
                    // this (rotation +  shift of halfpitch) will mirror the tube structure w.r.t. the chamber at z>0
                    htcomponent = htcomponent * GeoTrf::RotateX3D(180. * Gaudi::Units::deg);
                    htcomponent = htcomponent * GeoTrf::TranslateZ3D(halfpitch);
                }

                // ss - 24-05-2006 I don't really understand if this is needed at all
                //      it was introduced by Isabel T.
                if (zi < 0 && stName.compare(0, 3, "BOG") == 0 && is_mirrored) {
                    //      htcomponent = htcomponent*GeoTrf::RotateX3D(180.*Gaudi::Units::deg);
                    //      tubes OK but chambers wrong
                    //      htcomponent = GeoTrf::RotateX3D(180.*Gaudi::Units::deg)*htcomponent;
                    //      chambers OK but tubes wrong
                    htcomponent = GeoTrf::RotateX3D(180. * Gaudi::Units::deg) * htcomponent * GeoTrf::RotateX3D(180. * Gaudi::Units::deg); // turn chambers but go back for tubes
                } // ss - 24-05-2006 I don't really understand if this is needed at all

                std::string key =std::string( stName) + techname;
                xfaligncomponent = (*m_mapAXF)[key+"_"+std::to_string(zi)+"_"+std::to_string(fi)+"_"+std::to_string(md->index)];

                // for cutouts:
                // MDT cutouts for BOS1,5, BMS7,14, (problem with BMS4,10), EMS, BMG and BIS MDT14
                bool mdtCutoutFlag = ((stname == "BOS" && std::abs(zi) == 6) || stname == "BMG" || techname == "MDT14" || (stname == "BMS" && (std::abs(zi) == 1 && fi == 3)) ||
                                      (stname == "EMS" && (std::abs(zi) == 1 || std::abs(zi) == 3)));
                if (((manager->IncludeCutoutsFlag() && mdtCutoutFlag) || (manager->IncludeCutoutsBogFlag() && stName.compare(0, 3, "BOG") == 0)) && zi >= 0) {
                    key += "p" + buildString(mysql.allocPosFindSubtype(statType, fi, zi), 0) + "_" + buildString(mysql.allocPosFindCutout(statType, fi, zi), 0);
                } else if (((manager->IncludeCutoutsFlag() && mdtCutoutFlag) || (manager->IncludeCutoutsBogFlag() && stName.compare(0, 3, "BOG")) == 0) && zi < 0) {
                    key += "m" + buildString(mysql.allocPosFindSubtype(statType, fi, zi), 0) + "_" + buildString(mysql.allocPosFindCutout(statType, fi, zi), 0);
                }
		if (m_keySet->find(key)==m_keySet->end()) {
		  std::unique_ptr<Mdt> r = std::make_unique<Mdt>(mysql, c, stName + techname);
		  if ((manager->IncludeCutoutsFlag() && mdtCutoutFlag) || (manager->IncludeCutoutsBogFlag() && stName.compare(0, 3, "BOG") == 0)) {
		    if (!vcutdef.empty()) r->processCutouts(vcutdef);
		  }
		  m_keySet->insert(key);
		}
		lvm = (*m_mapFPV)[key+"_"+std::to_string(zi)+"_"+std::to_string(fi)+"_"+std::to_string(md->index)];

            } else if (type == "RPC") {
                // position stuff needed for cutout, used to be below:
                RpcComponent *rp = (RpcComponent *)c;
                int ndivy = rp->ndivy;
                int ndivz = rp->ndivz;

                if (ndivz != 1 || ndivy != 1) {
                    ATH_MSG_ERROR( " RPC segmentation z,y " << ndivz << " " << ndivy );
                }

                double xpos = c->posx;
                // implement really the mirror symmetry
                if (is_mirrored)
                    xpos = -xpos;

                ATH_MSG_VERBOSE( " In station " << stName << " with " << nDoubletR << " doubletR,"
                                << " RPC " << (c->name).substr(3, 2) << " has swap flag = " << rp->iswap << " ypos, zpos " << ypos << " " << zpos << " " );
                
                htcomponent = GeoTrf::TranslateX3D(ypos) * GeoTrf::TranslateY3D(xpos) * GeoTrf::TranslateZ3D(zpos);
                if (rp->iswap == -1) { // this is like amdb iswap
                    htcomponent = htcomponent * GeoTrf::RotateY3D(180 * Gaudi::Units::deg);
                }

                // end of position stuff

                bool rpcCutoutFlag = (stname == "BOS" && std::abs(zi) == 6) || (stname == "BMS" && (std::abs(zi) == 2 || std::abs(zi) == 4 || std::abs(zi) == 6)) ||
                                     (stname == "BMS" && std::abs(zi) == 1 && fi == 3);
                std::string key = stName + techname;
                if (((manager->IncludeCutoutsFlag() && rpcCutoutFlag) || (manager->IncludeCutoutsBogFlag() && stName.compare(0, 3, "BOG") == 0)) && zi >= 0) {
                    key += "p" + buildString(mysql.allocPosFindSubtype(statType, fi, zi), 0) + "_" + buildString(mysql.allocPosFindCutout(statType, fi, zi), 0) + "_" +
                           buildString(vcutdef.size(), 0) + "_" + buildString(rp->iswap, 0);
                } else if (((manager->IncludeCutoutsFlag() && rpcCutoutFlag) || (manager->IncludeCutoutsBogFlag() && stName.compare(0, 3, "BOG") == 0)) && zi < 0) {
                    key += "m" + buildString(mysql.allocPosFindSubtype(statType, fi, zi), 0) + "_" + buildString(mysql.allocPosFindCutout(statType, fi, zi), 0) + "_" +
                           buildString(vcutdef.size(), 0) + "_" + buildString(rp->iswap, 0);
                }
                xfaligncomponent = (*m_mapAXF)[key+"_"+std::to_string(zi)+"_"+std::to_string(fi)+"_"+std::to_string(rp->index)];
                lvr = (*m_mapFPV)[key+"_"+std::to_string(zi)+"_"+std::to_string(fi)+"_"+std::to_string(rp->index)];
            } else if (type == "TGC") {
                TgcComponent *tg = (TgcComponent *)m_station->GetComponent(i);
                TgcComponent *tgInner = (TgcComponent *)m_station->GetComponent(0);
                irad = tgInner->posy;
                TgcComponent *tgOuter = (TgcComponent *)m_station->GetComponent(m_station->GetNrOfComponents() - 1);
                double orad = tgOuter->posy + tgOuter->dy;
                double start = -(orad - irad) / 2. + (tg->posy - irad) + tg->dy / 2;
                double xstart = -thickness / 2. + tg->GetThickness(mysql) / 2.;
                htcomponent = GeoTrf::TranslateX3D(xstart + tg->posz) * GeoTrf::TranslateZ3D(start);

                // Define key for this TGC component
                std::string key = std::string(stName) + techname;
                if (manager->IncludeCutoutsFlag()) {
                    if (mysql.allocPosFindCutout(statType, fi, zi) > 0) {
                        // If there is a cutout for this chamber, give it a special key
                        if (zi >= 0) {
                            key += "p" + buildString(mysql.allocPosFindSubtype(statType, fi, zi), 0) + "_" + buildString(mysql.allocPosFindCutout(statType, fi, zi), 0);
                        } else if (zi < 0) {
                            key += "m" + buildString(mysql.allocPosFindSubtype(statType, fi, zi), 0) + "_" + buildString(mysql.allocPosFindCutout(statType, fi, zi), 0);
                        }
                    }
                }

                key += std::to_string(int(10 * c->dx1));
                xfaligncomponent = (*m_mapAXF)[key+"_"+std::to_string(zi)+"_"+std::to_string(fi)];

                lvt = (*m_mapFPV)[key+"_"+std::to_string(zi)+"_"+std::to_string(fi)];

            } else if (type == "CSC") {
                htcomponent = GeoTrf::TranslateX3D(ypos) * GeoTrf::TranslateZ3D(zpos);
                // Here define the key for this CSC component
                std::string key = std::string(stName) + techname;
                if (manager->IncludeCutoutsFlag() && zi >= 0) {
                    key += "p" + buildString(mysql.allocPosFindSubtype(statType, fi, zi), 0) + "_" + buildString(mysql.allocPosFindCutout(statType, fi, zi), 0);
                } else if (manager->IncludeCutoutsFlag() && zi < 0) {
                    key += "m" + buildString(mysql.allocPosFindSubtype(statType, fi, zi), 0) + "_" + buildString(mysql.allocPosFindCutout(statType, fi, zi), 0);
                }

                xfaligncomponent = (*m_mapAXF)[key+"_"+std::to_string(zi)+"_"+std::to_string(fi)];
                lvc=(*m_mapFPV)[key+"_"+std::to_string(zi)+"_"+std::to_string(fi)];
            } else {
                if (type != "MDT" && type != "RPC" && type != "TGC" && type != "SUP" && type != "DED" && type != "SPA" && type != "CHV" && type != "CRO" && type != "CMI" &&
                    type != "LB0" && type != "LBI") {
                   ATH_MSG_WARNING("Unknown component " << type );
                }
            }

            // Place components in chamber envelope
            if (lvm && manager->mdtIdHelper()) {
                int stationEta = zi;
                int stationPhi = fi + 1;
                int ml = 1;
                int tubel = 1;
                int tube = 1;
                if (ypos > 5.)
                    ml = 2; // Need >5 instead of >0 because BIS78 is not perfectly centered


                const MdtIdHelper *mdt_id = manager->mdtIdHelper();
                std::unique_ptr<MdtReadoutElement> det = std::make_unique<MdtReadoutElement>(lvm, stName, manager);
                Position ip = mysql.GetStationPosition(stName.substr(0, 3), fi, zi);
                setMdtReadoutGeom(mysql, det.get(), (MdtComponent *)c, ip);
                det->setHasCutouts(ncutouts > 0);
                det->setNMdtInStation(nMdt);
                Identifier id = mdt_id->channelID(stationType, stationEta, stationPhi, ml, tubel, tube);
                det->setIdentifier(id);
                det->setMultilayer(ml);
                det->setParentStationPV(PVConstLink(ptrd));
                det->setParentMuonStation(mstat);
                det->geoInitDone();

                if (ml == 1) {
                    // set fixed point for MDT deformations: s0,z0,t0 for the point at lowest t,z (z,y amdb) and s=x=0
                    mstat->setBlineFixedPointInAmdbLRS(c->posx, c->posy, c->posz);
                } else {
                    const Amg::Vector3D b0 = mstat->getBlineFixedPointInAmdbLRS();
                    if (c->posy < b0.y())
                        mstat->setBlineFixedPointInAmdbLRS(b0.x(), c->posy, b0.z());
                }

                int jobIndex = c->index;

                mstat->addMuonReadoutElementWithAlTransf(det.get(), xfaligncomponent, jobIndex);
                manager->addMdtReadoutElement(std::move(det));
            }

            if (lvc && manager->cscIdHelper()) {
                CscComponent *cs = (CscComponent *)m_station->GetComponent(i);
                int stationEta = zi;
                int stationPhi = fi + 1;
                int chamberLayer = 1;
                if (ypos > 0.)
                    chamberLayer = 2;

                std::unique_ptr<CscReadoutElement> det = std::make_unique<CscReadoutElement>(lvc, stName, manager);
                Position ip = mysql.GetStationPosition(stName.substr(0, 3), fi, zi);
                setCscReadoutGeom(mysql, det.get(), cs, ip);

                const CscIdHelper *csc_id = manager->cscIdHelper();
                det->setHasCutouts(ncutouts > 0);
                Identifier id = csc_id->channelID(stationType, stationEta, stationPhi, chamberLayer, 1, 0, 1);
                det->setIdentifier(id);

                det->setChamberLayer(chamberLayer);
                det->setParentStationPV(PVConstLink(ptrd));
                det->setParentMuonStation(mstat);

                int jobIndex = c->index;
                //
                mstat->addMuonReadoutElementWithAlTransf(det.get(), xfaligncomponent, jobIndex);

                

                // set alignment parameters for the wire layers
                manager->addCscReadoutElement(std::move(det));
            }

            if (lvt && manager->tgcIdHelper()) {
                
                ATH_MSG_DEBUG( " Adding a TGC chamber to the tree zi,fi, is_mirrored " << zi << " " << fi + 1 << " " << is_mirrored );
                

                TgcComponent *tg = (TgcComponent *)m_station->GetComponent(i);
                
                ATH_MSG_VERBOSE( "There's a TGC named " << techname << " of thickness " << tg->GetThickness(mysql) );
                
                const TgcIdHelper *tgc_id = manager->tgcIdHelper();
                int stationEta = 0;
                stationEta = tg->index;
                if (zi < 0)
                    stationEta = -stationEta;
                int stationPhi = 0;
                stationPhi = stationPhiTGC(stName, fi + 1,zi);
                                
                std::unique_ptr<TgcReadoutElement> det = std::make_unique<TgcReadoutElement>(lvt, stName, manager);
                Position ip = mysql.GetStationPosition(stName.substr(0, 3), fi, zi);
                setTgcReadoutGeom(mysql, det.get(), tg, ip, stName);
                det->setHasCutouts(ncutouts > 0);
                Identifier id = tgc_id->channelID(stationType, stationEta, stationPhi, 1, false, 1);
                det->setIdentifier(id);
                det->setParentStationPV(PVConstLink(ptrd));
                det->setParentMuonStation(mstat);

                int jobIndex = c->index;

                mstat->addMuonReadoutElementWithAlTransf(det.get(), xfaligncomponent, jobIndex);

                manager->addTgcReadoutElement(std::move(det));
            }
            if (lvr && RPCON && manager->rpcIdHelper()) {
                RpcComponent *rp = (RpcComponent *)c;
                int ndivy = rp->ndivy;
                int ndivz = rp->ndivz;

                if (ndivz != 1 || ndivy != 1) {
                    ATH_MSG_ERROR( " RPC segmentation z,y " << ndivz << " " << ndivy );
                }

                double zpos = -length / 2. + c->posy + c->dy / 2.;
                double xpos = c->posx;

                // implement really the mirror symmetry
                if (is_mirrored)
                    xpos = -xpos;
                // ... putting back to here!

                const RpcIdHelper *rpc_id = manager->rpcIdHelper();
                int stationEta = zi;
                int stationPhi = fi + 1;
                int doubletR = 1;
                int doubletZ = 1;

                if (nRpc > 1 && nDoubletR == 2 && ypos > 0.)
                    doubletR = 2;
                ndbz[doubletR - 1]++;

                // the BI RPCs are 3-gap RPCs mounted inside of the BI (s)MDTs
                if (stname.find("BI") != std::string::npos) {
                    if (stname.find("BIS") != std::string::npos) {
                        // for BIS78, there is a second RPC doubletZ at amdb-y (MuonGeoModel-z)=144mm inside the station
                        if (std::abs(stationEta)>= 7){
                           ATH_MSG_DEBUG("BIS78 station eta: "<<stationEta<<" phi: "<<stationPhi<<" dR: "<<doubletR<<" dZ:"<< doubletZ <<" rp: "<<rp->posz);
                        }
                        if (std::abs(stationEta) >= 7 && rp->posz > 80)
                            doubletZ = 2;
                        else
                            doubletZ = 1;
                    } else {
                        // for BIL/BIM/BIR, we have 10 RPCs put on 6 MDT stations, thus, need to exploit doubletZ as additional variable on top of stationEta
                        // only for BIL, there are sometimes 2 RPCs per 1 MDT station, namely for stationEta 1,3,4,6
                        if (stname.find("BIL") != std::string::npos && std::abs(stationEta) < 7 && std::abs(stationEta) != 2 && std::abs(stationEta) != 5) {
                            if (rp->posy > 1)
                                doubletZ = 2; // put the chamber with positive amdb-z to doubletZ=2
                        } else
                            doubletZ = 1;
                    }
                } else {
                    if (zi <= 0 && !is_mirrored) {
                        if (zpos < -100 * Gaudi::Units::mm)
                            doubletZ = 2;
                    } else {
                        if (zpos > 100 * Gaudi::Units::mm)
                            doubletZ = 2;
                    }
                }

                // BMS (BOG) RPCs can have |xpos|=950 (|xpos|=350)
                if (std::abs(xpos) > 100. * Gaudi::Units::mm) {
                    if (ndbz[doubletR - 1] > 2) {
                        doubletZ = 3;
                    }
                    ndbz[doubletR - 1]--;
                }

                int dbphi = 1;

                // this special patch is needed for BMS in the ribs where xpos is ~950mm;
                // the theshold to 100mm (too low) caused a bug
                // in BOG at eta +/-4 and stationEta 7 (not 6) ==>> 28 Jan 2016 raising the threshold to 400.mm
                // doublet phi not aware of pos. in space !!!
                if (xpos > 400. * Gaudi::Units::mm)
                    dbphi = 2;

                int doubletPhi = dbphi;
                //
                if (zi < 0 && is_mirrored && doubletZ == 3) {
                    doubletPhi++;
                    if (doubletPhi > 2)
                        doubletPhi = 1;
                } else if (zi < 0 && is_mirrored && doubletZ == 2 && doubletR == 1 && stName == "BMS6") {
                    doubletPhi++;
                    if (doubletPhi > 2)
                        doubletPhi = 1;
                }
                // never defined fields: set to the lower limit
                int gasGap = 1;
                int measuresPhi = 0;
                int strip = 1;

                int tag = doubletZ + doubletR * 100 + dbphi * 1000;
                if (rp->iswap == -1)
                    tag = -1 * tag;
                if (useAssemblies || isAssembly) {
          //
                } else {
                    int tag = rp->index + doubletR * 100 + dbphi * 1000;
                    if (rp->iswap == -1)
                        tag = -1 * tag;
                    
                }


                std::unique_ptr<RpcReadoutElement> det = std::make_unique<RpcReadoutElement>(lvr, stName, zi, fi + 1, is_mirrored, manager);
                Position ip = mysql.GetStationPosition(stName.substr(0, 3), fi, zi);
                setRpcReadoutGeom(mysql, det.get(), rp, ip);
                det->setHasCutouts(ncutouts > 0);
                Identifier id = rpc_id->channelID(stationType, stationEta, stationPhi, doubletR, doubletZ, doubletPhi, gasGap, measuresPhi, strip);
                det->setIdentifier(id);
                det->setDoubletR(doubletR);
                det->setDoubletZ(doubletZ);
                det->setDoubletPhi(doubletPhi);
                if (stName.find("BI") != std::string::npos)
                    det->setNumberOfLayers(3); // all BI RPCs always have 3 gas gaps
                det->setParentStationPV(PVConstLink(ptrd));
                det->setParentMuonStation(mstat);

                int jobIndex = c->index;

                mstat->addMuonReadoutElementWithAlTransf(det.get(), xfaligncomponent, jobIndex);
                


                if (stName.find("BI") != std::string::npos) {
                    std::map<std::string, float>::const_iterator yItr = rpcYTrans.find(techname);
                    if (yItr != rpcYTrans.end())
                        det->setYTranslation(yItr->second);
                    std::map<std::string, float>::const_iterator zItr = rpcZTrans.find(techname);
                    if (zItr != rpcZTrans.end())
                        det->setZTranslation(zItr->second);
                }

                det->fillCache();  // fill temporary cache (global position on known yet)
                det->initDesign(); ///  init design : design uses  global (converting back to local) positions
                det->clearCache(); // clear temporary cache
                manager->addRpcReadoutElement(std::move(det));

            } // if (lvr && RPCON && manager->rpcIdHelper()) {

        } // End big loop over components
        mstat->updateBlineFixedPointInAmdbLRS();

        return ptrd;
    }

    void MuonChamberLite::setCscReadoutGeom(const MYSQL& mysql,
                                        CscReadoutElement *re, const CscComponent *cc, const Position &ip) {
      
        re->m_Ssize = cc->dx1;
        re->m_LongSsize = cc->dx2;
        re->m_Rsize = cc->dy;
        re->m_LongRsize = cc->dy;
        re->m_Zsize = cc->GetThickness(mysql);
        re->m_LongZsize = cc->GetThickness(mysql);
        re->m_RlengthUpToMaxWidth = cc->maxwdy;
        re->m_excent = cc->excent;

        // Csc features specific to this readout element
        std::string tname = cc->name;
        re->setTechnologyName(tname);

        if (ip.isAssigned) {
            re->setStationS(ip.shift);
        } else {
      throw std::runtime_error(" MuonChamberLite::setCscReadoutGeom: position not found ");
        }

        const CSC *thisc = dynamic_cast<const CSC*>(mysql.GetTechnology(tname));
        re->m_anodecathode_distance = thisc->anocathodist;
        re->m_ngasgaps = thisc->numOfLayers;
        re->m_nstriplayers = thisc->numOfLayers;
        re->m_nwirelayers = thisc->numOfLayers;
        re->m_roxacellwidth = thisc->roxacellwith;
        re->m_nEtastripsperlayer = thisc->nEtastrips;
        re->m_nPhistripsperlayer = thisc->nPhistrips;
        re->m_Etastrippitch = thisc->cathreadoutpitch;
        re->m_Phistrippitch = thisc->phireadoutpitch;
        re->m_Etastripwidth = re->m_Etastrippitch;
        re->m_Phistripwidth = re->m_Phistrippitch;
    }

    void MuonChamberLite::setMdtReadoutGeom(const MYSQL& mysql,
                                        MdtReadoutElement *re, const MdtComponent *cc, const Position &ip) {
        re->m_Ssize = cc->dx1;
        re->m_LongSsize = cc->dx2;

        if (re->m_inBarrel) {
            re->m_Rsize = cc->GetThickness(mysql);
            re->m_LongRsize = cc->GetThickness(mysql);
            re->m_Zsize = cc->dy;
            re->m_LongZsize = cc->dy;
        } else {
            re->m_Rsize = cc->dy;
            re->m_LongRsize = cc->dy;
            re->m_Zsize = cc->GetThickness(mysql);
            re->m_LongZsize = cc->GetThickness(mysql);
        }

        re->m_cutoutShift = cc->cutoutTubeXShift;
        re->m_tubelenStepSize = cc->tubelenStepSize;

        if (ip.isAssigned) {
            re->setStationS(ip.shift);
        } else {
            throw std::runtime_error(" MuonChamberLite::setMdtReadoutGeom: position not found ");
        }

        std::string tname = cc->name;
        re->setTechnologyName(tname);
        const MDT *thism = dynamic_cast<const MDT*>(mysql.GetTechnology(tname));
        re->m_nlayers = thism->numOfLayers;
        re->m_tubepitch = thism->pitch;
        re->m_tubelayerpitch = thism->y[1] - thism->y[0];
        re->m_endpluglength = thism->tubeEndPlugLength;
        re->m_deadlength = cc->deadx; // thism->tubeDeadLength;
        re->m_innerRadius = thism->innerRadius;
        re->m_tubeWallThickness = thism->tubeWallThickness;

        if (re->m_inBarrel) {
            re->m_ntubesperlayer = int(re->m_Zsize / re->m_tubepitch);
            re->m_nsteps = 1; // all tubes have the same length
            re->m_ntubesinastep = re->m_ntubesperlayer;
            re->m_tubelength[0] = re->m_Ssize;
        } else {
            re->m_ntubesperlayer = int(re->m_Rsize / re->m_tubepitch);
            re->m_nsteps = int(re->m_Rsize / re->m_tubelenStepSize);
            re->m_ntubesinastep = int(re->m_tubelenStepSize / re->m_tubepitch);
            re->m_tubelength[0] = re->m_Ssize;
            double diff = (re->m_LongSsize - re->m_Ssize) * (re->m_LongRsize - re->m_tubepitch / 2.) / re->m_LongRsize;
            for (int is = 0; is < re->m_nsteps; ++is) {
                double len = re->m_Ssize + is * diff / re->m_nsteps;
                re->m_tubelength[is] = len;
            }
        }

        for (int tl = 0; tl < re->m_nlayers; ++tl) {
            re->m_firstwire_x[tl] = thism->x[tl];
            re->m_firstwire_y[tl] = thism->y[tl];
        }
    }

    void MuonChamberLite::setRpcReadoutGeom(const MYSQL& mysql,
                                            RpcReadoutElement *re, 
                                            const RpcComponent *cc, 
                                            const Position &ip) {
       
        re->m_Ssize = cc->dx1;
        re->m_LongSsize = cc->dx2;
        re->m_Rsize = cc->GetThickness(mysql);
        re->m_LongRsize = cc->GetThickness(mysql);
        re->m_Zsize = cc->dy;
        re->m_LongZsize = cc->dy;

        re->m_hasDEDontop = true;
        if (cc->iswap == -1)
            re->m_hasDEDontop = false;

        if (ip.isAssigned) {
            re->setStationS(ip.shift);
        } else {
            ATH_MSG_ERROR( " MuonChamber::setRpcReadoutGeom: position not found " );
            assert(0);
        }

        std::string tname = cc->name;
        re->setTechnologyName(tname);
        const RPC *thisr = dynamic_cast<const RPC*>(mysql.GetTechnology(tname));
        re->m_nphigasgaps = thisr->NGasGaps_in_s;
        re->m_gasgapssize = re->m_Ssize / re->m_nphigasgaps - 2. * thisr->bakeliteframesize;
        re->m_gasgapzsize = re->m_Zsize - 2. * thisr->bakeliteframesize;
        re->m_nphistrippanels = thisr->NstripPanels_in_s; 
        re->m_phistrippitch = thisr->stripPitchS;
        re->m_etastrippitch = thisr->stripPitchZ;

        if (re->getStationName().find("BI") != std::string::npos) {
            re->setNumberOfLayers(3); // all BI RPCs always have 3 gas gaps

        }
        const RPC* genericRpc = dynamic_cast<const RPC*>(mysql.GetATechnology("RPC0"));

        if (re->numberOfLayers() == 3) {
            constexpr double rpc3GapLayerThickness = 11.8;  // gas vol. + ( bakelite + graphite + PET )x2
            for (int gasGap =1 ; gasGap <= re->numberOfLayers(); ++gasGap) {
                /// the values from MuonGeoModel have an offset of 0.74, TO BE INVESTIGATED, cf. ATLASSIM-5021
                /// On the other hand this detector has never sent any good data. So the investigation doesn't really matter
                re->m_gasGap_xPos[gasGap -1] = (gasGap - 2) * rpc3GapLayerThickness - 0.74; 
            }
        } else {
            re->m_gasGap_xPos[0] = -re->m_Rsize / 2. + thisr->externalSupPanelThickness + genericRpc->stripPanelThickness
                                    + genericRpc->GasGapThickness / 2;
            re->m_gasGap_xPos[1] = re->m_gasGap_xPos[0] + genericRpc->rpcLayerThickness + 
                                   genericRpc->centralSupPanelThickness;
            if (!re->m_hasDEDontop) {
                std::swap(re->m_gasGap_xPos[0], re->m_gasGap_xPos[1]);
            }
        }

        re->m_phistripwidth = re->m_phistrippitch - genericRpc->stripSeparation;
        re->m_etastripwidth = re->m_etastrippitch - genericRpc->stripSeparation;
        re->m_nphistripsperpanel = int((re->m_Ssize / re->m_nphistrippanels) / re->m_phistrippitch);
        if (re->getStationName().compare(0, 3, "BME") != 0) {
            re->m_nphistripsperpanel-=(re->m_nphistripsperpanel % 8) ;
        }
        re->m_netastripsperpanel = int((re->m_Zsize) / re->m_etastrippitch);
        re->m_netastripsperpanel -= (re->m_netastripsperpanel % 8);

        re->m_phipaneldead = re->m_Ssize / re->m_nphistrippanels - 
                             re->m_nphistripsperpanel * re->m_phistrippitch +
                            genericRpc->stripSeparation;
        re->m_phipaneldead = re->m_phipaneldead / 2.;
        re->m_etapaneldead = re->m_Zsize - re->m_netastripsperpanel * re->m_etastrippitch + genericRpc->stripSeparation;
        re->m_etapaneldead = re->m_etapaneldead / 2.;
        re->m_phistriplength = re->m_LongZsize;
        re->m_etastriplength = re->m_LongSsize / re->m_nphistrippanels;

        re->m_first_phistrip_s[0] = -re->m_Ssize / 2. + re->m_phipaneldead + re->m_phistripwidth / 2.;
        if (re->m_nphistrippanels == 2) {
            re->m_first_phistrip_s[1] = re->m_phipaneldead + re->m_phistripwidth / 2.;
        }

        double offset = 0.;
        re->m_phistrip_z = -re->m_Zsize / 2. + offset + re->m_phistriplength / 2.;
       
        // first strip position on each eta panel
        re->m_first_etastrip_z = -re->m_Zsize / 2. + re->m_etapaneldead + re->m_etastripwidth / 2.;
        re->m_etastrip_s[0] = -re->m_Ssize / 2. + offset + re->m_etastriplength / 2.;
        if (re->m_nphistrippanels == 2) {
            re->m_etastrip_s[1] = re->m_Ssize / 2. - offset - re->m_etastriplength / 2.;
        }

    }

    void MuonChamberLite::setTgcReadoutGeom(const MYSQL& mysql,
                                            TgcReadoutElement *re, 
                                            const TgcComponent *cc, 
                                            const Position &ip, 
                                            const std::string& stName) {
       
        re->setSsize(cc->dx1);
        re->setLongSsize(cc->dx2);
        re->setRsize(cc->dy);
        re->setLongRsize(cc->dy);
        re->setZsize(cc->GetThickness(mysql));
        re->setLongZsize(cc->GetThickness(mysql));


        const TGC *genericTgc = dynamic_cast<const TGC*>(mysql.GetATechnology("TGC0"));
        re->setFrameThickness(genericTgc->frame_h,
                              genericTgc->frame_ab);


        const std::string &tname = cc->name;
        int tname_index = MuonGM::strtoint(tname, 3, 2);
        re->setTechnologyName(tname);

        if (ip.isAssigned) {
            re->setStationS(ip.shift);
        } else {
            throw std::runtime_error(" MuonChamberLite::setTgcReadoutGeom position not found ");
        }

        re->setReadOutName(stName.substr(0, 4) + '_' + std::to_string(cc->index));
        re->setReadOutParams(mysql.GetTgcRPars(tname_index));

        const TGC *thist = dynamic_cast<const TGC*>(mysql.GetTechnology(tname));
        const std::size_t ncomp = (thist->materials).size();
        
        unsigned int gasGap{0};
        for (std::size_t i = 0; i < ncomp; ++i) {
            double newpos = -re->getZsize() / 2. + thist->positions[i] + thist->tck[i] / 2.;

            if ( thist->materials[i].find("TGCGas") != std::string::npos) {
                re->setPlaneZ(newpos, ++gasGap);
            }
        }
    }

    void MuonChamberLite::print() const {
        ATH_MSG_INFO("MuonChamberLite " << name << " :" );
    }
} // namespace MuonGM
