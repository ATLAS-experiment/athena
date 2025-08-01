/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
 * File:   TrackTools.cxx
 * Author: Marco van Woerden <mvanwoer@cern.ch>, Archil Durglishvili <archil.durglishvili@cern.ch>
 * Description: Track tools.
 *
 * Created in February 2013.
 * Updated in November 2014
 */
#include "TrackTools.h"
#include "CaloIdentifier/TileID.h"
#include "TileDetDescr/TileCellDim.h"
#include "TileDetDescr/TileDetDescrManager.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkParametersIdentificationHelpers/TrackParametersIdHelper.h"
#include "TrkCaloExtension/CaloExtensionHelpers.h"

namespace TileCal {

//==================================
StatusCode TrackTools::initialize(){
//=================================

  ATH_MSG_INFO( "Initializing TrackTools" );

  if (m_isCollision) {
    ATH_MSG_INFO( "Beam type = Collision" );
  } else {
    ATH_MSG_INFO( "Beam type = Cosmic" );
  }

  ATH_CHECK(m_caloExtensionTool.retrieve());

  ATH_CHECK( detStore()->retrieve(m_tileID) );

  //=== TileDetDescrManager
  ATH_CHECK( detStore()->retrieve(m_tileMgr) );

  return StatusCode::SUCCESS;
} // TrackTools::initialize

//================================
StatusCode TrackTools::finalize(){
//===============================
  ATH_MSG_INFO( "Finalizing TrackTools" );
  return StatusCode::SUCCESS;
} // TRACKTOOLS::FINALIZE

//==================================================================================================
std::unique_ptr<const Trk::TrackParameters> TrackTools::getTrackInCellSampling(const TRACK* track, CaloSampling::CaloSample sampling) const {
//==================================================================================================
  if( !m_caloExtensionTool.empty() ){
    std::unique_ptr<Trk::CaloExtension> extension =
      m_caloExtensionTool->caloExtension(Gaudi::Hive::currentContext(), *track);
    if (!extension)
      return nullptr;

    Trk::TrackParametersIdHelper  parsIdHelper;

    std::map<CaloSampling::CaloSample,const Trk::CurvilinearParameters*> Samplings;

    // loop over calo layers, keep track of previous layer
    auto cur = extension->caloLayerIntersections().begin();
    auto prev = cur;
    for( ; cur != extension->caloLayerIntersections().end() ; ++cur ){
      // check that prev and cur are not the same, if not fill if the previous was an entry layer
      if( prev != cur && parsIdHelper.isEntryToVolume((*prev).cIdentifier()) )
      {
        TrackParametersIdentifier id =  (*prev).cIdentifier();
        CaloSampling::CaloSample sample = parsIdHelper.caloSample(id);
        Samplings[sample] = &(*prev);
      }
      prev=cur;
    }
    if(!Samplings[sampling])  {return nullptr ;}
    else {return std::make_unique<const Trk::CurvilinearParameters> (*(Samplings[sampling]));}
  }
  return nullptr;
}

//==================================================================================================
std::vector< double > TrackTools::getXYZEtaPhiInCellSampling(const TRACK* track, CaloSampling::CaloSample sampling) const {
//==================================================================================================
  std::vector<double> coordinates;
  if( !m_caloExtensionTool.empty() ){
    std::unique_ptr<Trk::CaloExtension> extension =
      m_caloExtensionTool->caloExtension(Gaudi::Hive::currentContext(), *track);
    if (!extension)
      return coordinates;

    Trk::TrackParametersIdHelper  parsIdHelper;

    std::map<CaloSampling::CaloSample,const Trk::TrackParameters*> Samplings;

    // loop over calo layers, keep track of previous layer
    auto cur = extension->caloLayerIntersections().begin();
    auto prev = cur;
    for( ; cur != extension->caloLayerIntersections().end() ; ++cur ){
      // check that prev and cur are not the same, if not fill if the previous was an entry layer
      if( prev != cur && parsIdHelper.isEntryToVolume((*prev).cIdentifier()) )
      {
        TrackParametersIdentifier id =  (*prev).cIdentifier();
        CaloSampling::CaloSample sample = parsIdHelper.caloSample(id);
        Samplings[sample] = &(*prev);
      }
      prev=cur;
    }
    if(!(Samplings[sampling])) return coordinates;
    coordinates.push_back(Samplings[sampling]->position().x());
    coordinates.push_back(Samplings[sampling]->position().y());
    coordinates.push_back(Samplings[sampling]->position().z());
    coordinates.push_back(Samplings[sampling]->position().eta());
    coordinates.push_back(Samplings[sampling]->position().phi());
  }
  return coordinates;
} // getXYZEtaPhiInCellSampling


//==========================================================================================================
std::vector< double > TrackTools::getXYZEtaPhiInCellSampling(const TRACK* track, const CaloCell *cell) const {
//==========================================================================================================
  std::vector<double> coordinates;

  if(!cell || !track) return coordinates;
  // GET CELL DESCRIPTOR AND SAMPLING
  const CaloDetDescrElement* dde = cell->caloDDE();
  if(!dde) return coordinates;
  CaloSampling::CaloSample sampling = dde->getSampling();

  return getXYZEtaPhiInCellSampling(track,sampling);
} // getXYZEtaPhiInCellSampling


//=====================================================================================================
std::vector< std::vector<double> > TrackTools::getXYZEtaPhiPerLayer(const TRACK* track) const {
//====================================================================================================
  std::vector< std::vector<double> > coordinates(11);

  for(unsigned int sample=0 ; sample<21; ++sample) //Samplings:http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/Calorimeter/CaloGeoHelpers/CaloGeoHelpers/CaloSampling.def
  {
    std::vector<double> TrkPars(5);
    std::vector<double> XYZEtaPhi = getXYZEtaPhiInCellSampling( track, (CaloSampling::CaloSample)sample );
    TrkPars[0] = ( XYZEtaPhi.size()!=5 ) ? -9999. : XYZEtaPhi[0] ;
    TrkPars[1] = ( XYZEtaPhi.size()!=5 ) ? -9999. : XYZEtaPhi[1] ;
    TrkPars[2] = ( XYZEtaPhi.size()!=5 ) ? -9999. : XYZEtaPhi[2] ;
    TrkPars[3] = ( XYZEtaPhi.size()!=5 ) ? -9999. : XYZEtaPhi[3] ;
    TrkPars[4] = ( XYZEtaPhi.size()!=5 ) ? -9999. : XYZEtaPhi[4] ;
    int lay=-1;
    if     (sample==0  || sample==4 ){lay=0;}
    else if(sample==1  || sample==5 ){lay=1;}
    else if(sample==2  || sample==6 ){lay=2;}
    else if(sample==3  || sample==7 ){lay=3;}
    else if(sample==12 || sample==18){lay=4;}
    else if(sample==13 || sample==19){lay=5;}
    else if(sample==15 || sample==17){lay=5;}
    else if(sample==14 || sample==20){lay=6;}
    else if(sample==16)                {lay=6;}
    else if(sample==8                 ){lay=7;}
    else if(sample==9                 ){lay=8;}
    else if(sample==10                ){lay=9;}
    else if(sample==11                ){lay=10;}
    if(lay!=-1) coordinates[lay] = TrkPars;
  } // FOR

  return coordinates;
} // TrackTools::getXYZEtaPhiPerLayer

//=====================================================================================================
std::vector< std::vector<double> > TrackTools::getXYZEtaPhiPerSampling(const TRACK* track) const {
//====================================================================================================
  std::vector< std::vector<double> > coordinates;

  for(unsigned int s=0 ; s<21; ++s) //Samplings: http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/Calorimeter/CaloGeoHelpers/CaloGeoHelpers/CaloSampling.def
  {
    std::vector<double> TrkPars(5);
    std::vector<double> XYZEtaPhi = getXYZEtaPhiInCellSampling( track, (CaloSampling::CaloSample)s );
    TrkPars[0] = ( XYZEtaPhi.size()!=5 ) ? -9999. : XYZEtaPhi[0] ;
    TrkPars[1] = ( XYZEtaPhi.size()!=5 ) ? -9999. : XYZEtaPhi[1] ;
    TrkPars[2] = ( XYZEtaPhi.size()!=5 ) ? -9999. : XYZEtaPhi[2] ;
    TrkPars[3] = ( XYZEtaPhi.size()!=5 ) ? -9999. : XYZEtaPhi[3] ;
    TrkPars[4] = ( XYZEtaPhi.size()!=5 ) ? -9999. : XYZEtaPhi[4] ;
    coordinates.push_back(TrkPars);
  } // FOR

  return coordinates;
} // TrackTools::getXYZEtaPhiPerSampling


//======================================================================
void TrackTools::getCellsWithinConeAroundTrack(const TRACK* track,
                                               const CaloCellContainer* input,
                                               ConstDataVector<CaloCellContainer>* output,
                                               double cone,
                                               bool includelar) const {
//======================================================================
  // CELLS WITHIN CONE AROUND TRACK
  CaloCellContainer::const_iterator icell = input->begin();
  CaloCellContainer::const_iterator end   = input->end();
  for(;icell!=end;++icell){
    const CaloCell* cell = (*icell);
    const CaloDetDescrElement* dde = cell->caloDDE();

    // REASONS TO SKIP THIS CELL OR BREAK THE LOOP
    if(!dde) continue;
    if(!includelar && dde->getSubCalo() == CaloCell_ID::LAREM) continue;
    if(dde->getSubCalo() != CaloCell_ID::LAREM && dde->getSubCalo() != CaloCell_ID::TILE) continue;

    std::vector<double> coordinates = getXYZEtaPhiInCellSampling(track,dde->getSampling());
    if(coordinates.size()!=5) continue;

    double deltaR = KinematicUtils::deltaR( cell->eta(),coordinates[3], cell->phi(),coordinates[4]);

    if(deltaR<=cone)
    {
      unsigned int i=0;
      while(i!=output->size()){if(output->at(i)==cell){break;}i++;}
      if(i==output->size())
      {
        output->push_back(cell);
      }
    }
  } // FOR
} // TrackTools::getCellsWithinConeAroundTrack

//======================================================================================
double TrackTools::getPathInsideCell(const TRACK *track, const CaloCell *cell) const {
//======================================================================================

  ATH_MSG_DEBUG("in TrackInCaloTools::getPathInsideCell" );



  // GET CELL DESCRIPTOR AND SAMPLING
  const CaloDetDescrElement* dde = cell->caloDDE();
  if(!dde) return 0.;
  int sampling = dde->getSampling();
  int sampling_entrance = 0;
  int sampling_exit     = 0;
  int cell_tower        = m_tileID->tower(cell->ID());

  // The type of physics (collision or cosmic) determines the entrance and exit cell for the muons, therefore these different switches are defined accordingly.
  if (m_isCollision) {
    switch(sampling){
    case 12:
      sampling_entrance = 12;
      if      (cell_tower>=0 && cell_tower<=6) sampling_exit = 14;
      else if (cell_tower==7) sampling_exit = 13;                  // for A8, the exit is BC8
      else if (cell_tower>=8 && cell_tower<=9) sampling_exit = 20; // for A9 and A10, the exit is D5
      break;
    case 13:
      sampling_entrance = 12;
      if      (cell_tower>=0 && cell_tower<=6) sampling_exit = 14;
      else if (cell_tower==7)                  sampling_exit = 13; // for BC8, the exit is BC8
      else if (cell_tower==8)                  sampling_exit = 20; // for B9, the exit is D5
      break;
    case 14: sampling_entrance = 12; sampling_exit = 14; break;
    case 15: sampling_entrance = 12; sampling_exit = 20; break; // for C10, the entrance is A10, the exit is D5
    case 16: sampling_entrance = 12; sampling_exit = 13; break;
    case 17:
      sampling_entrance = 17;
      if (cell_tower == 10) {        // E1
        sampling_exit = 19;          // the exit is B11
      } else if (cell_tower == 11) { // E2
        if (getTrackInCellSampling(track, CaloSampling::CaloSample::TileExt0)) sampling_exit = 18; // the exit is A12
        else sampling_exit = 19;     // the exit is B11
       } else if ((cell_tower == 13) || (cell_tower == 15)) { // E3 or E4
        if (getTrackInCellSampling(track, CaloSampling::CaloSample::PreSamplerE)) sampling_exit = 4; // the exit is PreSamplerE
        else sampling_exit = 18;     // the exit is TileExt0
      }
      break;
    case 18:
      sampling_entrance = 18;
      if      (cell_tower>=11 && cell_tower<=13) sampling_exit = 20;
      else if (cell_tower>=14 && cell_tower<=15) sampling_exit = 19; // for A15 and A16, the exit is B15
      break;
    case 19:
      if      (cell_tower==10)                   {sampling_entrance = 19; sampling_exit = 20;} // for B11, the entrance is B11, the exit is D5
      else if (cell_tower>=11 && cell_tower<=13) {sampling_entrance = 18; sampling_exit = 20;} // for B12-B14, the entrance is EBA cells, the exit is EBD cells
      else if (cell_tower==14)                   {sampling_entrance = 18; sampling_exit = 19;} // for B15, the entrance is A15, the exit is B15
      break;
    case 20:
      if      (cell_tower==10)                   {sampling_entrance = 19; sampling_exit = 20;} // for D5, the entrance is B11, the exit is D5
      else if (cell_tower==12)                   {sampling_entrance = 18; sampling_exit = 20;} // for D6, the entrance is A13 cells, the exit is D6
      break;
    default: return 0.;
    } // SWITCH
  }
  else {
    switch(sampling){
    case 12: sampling_entrance = 12; sampling_exit = 14; break;
    case 13: sampling_entrance = 12; sampling_exit = 14; break;
    case 14: sampling_entrance = 12; sampling_exit = 14; break;
    case 15: sampling_entrance = 13; sampling_exit = 14; break;
    case 16: sampling_entrance = 12; sampling_exit = 13; break;
    case 17: sampling_entrance = 17; sampling_exit = 19; break;
    case 18: sampling_entrance = 18; sampling_exit = 20; break;
    case 19: sampling_entrance = 18; sampling_exit = 20; break;
    case 20: sampling_entrance = 18; sampling_exit = 20; break;
    default: return 0.;
    } // SWITCH
  }

  std::unique_ptr<const Trk::TrackParameters> pars_entrance =
    getTrackInCellSampling(track, (CaloSampling::CaloSample)sampling_entrance);
  std::unique_ptr<const Trk::TrackParameters> pars_exit =
    getTrackInCellSampling(track, (CaloSampling::CaloSample)sampling_exit);

  if( !pars_entrance || !pars_exit ) return 0.;

  return getPath(cell, pars_entrance.get(), pars_exit.get());
} // TrackTools::getPathInsideCell

//=====================================================================================================================================
double TrackTools::getPath(const CaloCell* cell, const Trk::TrackParameters *entrance, const Trk::TrackParameters *exit) const {
//====================================================================================================================================
    ATH_MSG_DEBUG("IN getPath...");

    // OBTAIN LAYER INDICES FOR LINEAR INTERPOLATION
    unsigned int sampleID = cell->caloDDE()->getSampling();

    // OBTAIN TRACK AND CELL PARAMETERS
    double pathl = 0.;
    double layer1X = exit->position().x();
    double layer1Y = exit->position().y();
    double layer1Z = exit->position().z();
    double layer2X = entrance->position().x();
    double layer2Y = entrance->position().y();
    double layer2Z = entrance->position().z();

    double cellPhi = cell->caloDDE()->phi();
    double cellDPhi = cell->caloDDE()->dphi();
    double cellPhimin = cellPhi - cellDPhi / 2.;
    double cellPhimax = cellPhi + cellDPhi / 2.;
    double cellZ = cell->caloDDE()->z();
    double cellDZ = cell->caloDDE()->dz();
    double cellZmin = cellZ - cellDZ / 2.;
    double cellZmax = cellZ + cellDZ / 2.;
    double cellR = cell->caloDDE()->r();
    double cellDR = cell->caloDDE()->dr();
    double cellRmin = cellR - cellDR / 2.;
    double cellRmax = cellR + cellDR / 2.;

    double cellXimp[2], cellYimp[2], cellZimp[2];
    double x(0), y(0), z(0), r(0), phi(0);
    double deltaPhi;

    // COMPUTE PATH
    bool compute = true;
    int lBC(0);
    while(compute){
        if ((lBC == 1) && (sampleID == 13) && (m_tileID->tower(cell->ID()) == 8)) break; // B9
        int np = 0;
        if(std::sqrt((layer1X - layer2X) * (layer1X - layer2X) + (layer1Y - layer2Y) * (layer1Y - layer2Y)) < 3818.5){
            if(sampleID == 13){
                TileCellDim* cellDim = m_tileMgr->get_cell_dim(cell->ID());
                if(lBC == 0){
                  cellRmin = cellDim->getRMin(TILE_RAW_FIRST);
                  cellRmax = cellDim->getRMax(TILE_RAW_THIRD);
                  cellZmin = cellDim->getZMin(TILE_RAW_SECOND);
                  cellZmax = cellDim->getZMax(TILE_RAW_SECOND);
                } else if(lBC == 1){
                  cellRmin = cellDim->getRMin(TILE_RAW_FOURTH);
                  cellRmax = cellDim->getRMax(TILE_RAW_SIXTH);
                  cellZmin = cellDim->getZMin(TILE_RAW_FIFTH);
                  cellZmax = cellDim->getZMax(TILE_RAW_FIFTH);
                }
            }
            // CALCULATE POINTS OF INTERSECTION
            // INTERSECTIONS R PLANES
            double radius(cellRmin);

            double x0int = exit->position().x();
            double x1int = entrance->position().x();
            double y0int = exit->position().y();
            double y1int = entrance->position().y();
            double z0int = exit->position().z();
            double z1int = entrance->position().z();
            double s = (y1int - y0int) / (x1int - x0int);
            double a = 1 + s * s;
            double b = 2 * s * y0int - 2 * s * s * x0int;
            double c = y0int * y0int - radius * radius + s * s * x0int * x0int - 2 * y0int * s * x0int;
            double x1 = (-b + std::sqrt(b * b - 4 * a * c)) / (2 * a);
            double x2 = (-b - std::sqrt(b * b - 4 * a * c)) / (2 * a);
            double y1 = y0int + s * (x1 - x0int);
            double y2 = y0int + s * (x2 - x0int);
            double s1 = (z1int - z0int) / (x1int - x0int);
            double z1 = z0int + s1 * (x1 - x0int);
            double z2 = z0int + s1 * (x2 - x0int);

            x = x1;
            y = y1;
            z = z1;

            if( ((x1 - x0int) * (x1 - x0int) + (y1 - y0int) * (y1 - y0int) + (z1 - z0int) * (z1 - z0int)) >
               ((x2 - x0int) * (x2 - x0int) + (y2 - y0int) * (y2 - y0int) + (z2 - z0int) * (z2 - z0int)) ){
                x = x2;
                y = y2;
                z = z2;
            } // IF

            phi = std::acos(x / std::sqrt(x * x + y * y));
            if(y <= 0) phi = -phi;
            r = cellRmin;

            if(z >= cellZmin && z <= cellZmax && phi >= cellPhimin && phi <= cellPhimax){
                cellXimp[np] = x;
                cellYimp[np] = y;
                cellZimp[np] = z;
                np = np + 1;

            } // IF

            radius = cellRmax;

            c  = y0int * y0int - radius * radius + s * s * x0int * x0int - 2 * y0int * s * x0int;
            x1 = ((-b + std::sqrt(b * b - 4 * a * c)) / (2 * a));
            x2 = ((-b - std::sqrt(b * b - 4 * a * c)) / (2 * a));
            y1 = (y0int + s * (x1 - x0int));
            y2 = (y0int + s * (x2 - x0int));
            z1 = (z0int + s1 * (x1 - x0int));
            z2 = (z0int + s1 * (x2 - x0int));
            s1 = ((z1int - z0int) / (x1int - x0int));

            x = x1;
            y = y1;
            z = z1;

            if( ((x1 - x0int) * (x1 - x0int) + (y1 - y0int) * (y1 - y0int) + (z1 - z0int) * (z1 - z0int)) >
               ((x2 - x0int) * (x2 - x0int) + (y2 - y0int) * (y2 - y0int) + (z2 - z0int) * (z2 - z0int)) ){
                x = x2;
                y = y2;
                z = z2;
            } // IF

            phi = std::acos(x / std::sqrt(x * x + y * y));
            if (y <= 0) phi = -phi;
            r = cellRmax;

            if(z >= cellZmin && z <= cellZmax && phi >= cellPhimin && phi <= cellPhimax){
                cellXimp[np] = x;
                cellYimp[np] = y;
                cellZimp[np] = z;
                np=np + 1;

            } // IF

            // INTERSECTIONS Z PLANES
            if(np < 2){
                double sxz = (layer2X - layer1X) / (layer2Z - layer1Z);
                double syz = (layer2Y - layer1Y) / (layer2Z - layer1Z);
                z = cellZmin;
                x = layer1X + sxz * (z - layer1Z);
                y = layer1Y + syz * (z - layer1Z);
                r = std::sqrt(x * x + y * y);
                phi = std::acos(x / r);
                if(y <= 0) phi=-phi;
                if(r >= cellRmin && r <= cellRmax && phi >= cellPhimin && phi <= cellPhimax){
                    cellXimp[np] = x;
                    cellYimp[np] = y;
                    cellZimp[np] = z;
                    np=np + 1;

                } // IF
            } // IF

            if(np < 2){
                double sxz = (layer2X - layer1X) / (layer2Z - layer1Z);
                double syz = (layer2Y - layer1Y) / (layer2Z - layer1Z);
                z = cellZmax;
                x = layer1X + sxz * (z - layer1Z);
                y = layer1Y + syz * (z - layer1Z);
                r = std::sqrt(x * x + y * y);
                phi = std::acos(x / r);
                if(y <= 0) phi = -phi;
                if(r >= cellRmin && r <= cellRmax && phi >= cellPhimin && phi <= cellPhimax){
                    cellXimp[np] = x;
                    cellYimp[np] = y;
                    cellZimp[np] = z;
                    np = np + 1;

                } // IF
            } // IF

            // INTERSECTIONS PHI PLANES
            if(np < 2){
                double sxy = (layer2X - layer1X) / (layer2Y - layer1Y);
                double sxz = (layer2X - layer1X) / (layer2Z - layer1Z);
                x = (layer1X - sxy * layer1Y) / (1 - sxy * tan(cellPhimin));
                y = x * std::tan(cellPhimin);
                z = layer1Z + (1 / sxz) * (x - layer1X);
                r = std::sqrt(x * x + y * y);
                phi = std::acos(x / r);
                if(y <= 0) phi = -phi;
                deltaPhi = std::abs(phi - cellPhimin);
                if(deltaPhi > 3.141593) deltaPhi = std::abs(phi + cellPhimin);
                if(r >= cellRmin && r <= cellRmax && z >= cellZmin && z <= cellZmax && deltaPhi < 0.0001){
                    cellXimp[np] = x;
                    cellYimp[np] = y;
                    cellZimp[np] = z;
                    np = np + 1;
                } // IF
            } // IF
            if(np < 2){
                double sxy = (layer2X - layer1X) / (layer2Y - layer1Y);
                double sxz = (layer2X - layer1X) / (layer2Z - layer1Z);
                x = (layer1X - sxy * layer1Y) / (1 - sxy * tan(cellPhimax));
                y = x * std::tan(cellPhimax);
                z = layer1Z + (1 / sxz) * (x - layer1X);
                r = std::sqrt(x * x + y * y);
                phi = std::acos(x / r);
                if(y <= 0) phi = -phi;
                deltaPhi = std::abs(phi - cellPhimax);
                if(deltaPhi > 3.141593) deltaPhi = std::abs(phi + cellPhimax);
                if(r >= cellRmin && r <= cellRmax && z >= cellZmin && z <= cellZmax && deltaPhi < 0.0001){
                    cellXimp[np] = x;
                    cellYimp[np] = y;
                    cellZimp[np] = z;
                    np = np + 1;
                } // IF
            } // IF

            // CALCULATE PATH IF TWO INTERSECTIONS WERE FOUND
            if(np == 2){

                pathl += std::sqrt( (cellXimp[0] - cellXimp[1]) * (cellXimp[0] - cellXimp[1]) +
                                    (cellYimp[0] - cellYimp[1]) * (cellYimp[0] - cellYimp[1]) +
                                    (cellZimp[0] - cellZimp[1]) * (cellZimp[0] - cellZimp[1]) );
            } // IF
        } // IF
        if(sampleID == 13 && lBC == 0) ++lBC;
        else compute = false;
    } // WHILE (FOR LBBC LAYER)

    return pathl;
} // TrackTools::getPath



std::vector<float> TrackTools::getEnergyInCones(const xAOD::TrackParticle* track,
                                                const xAOD::CaloClusterContainer* clusters,
                                                const std::set<xAOD::CaloCluster::CaloSample>& samplings,
                                                const std::vector<double>& drCones,
                                                const EventContext& ctx) const {

  std::vector<float> energyInCone(drCones.size(), 0.F);

  std::unique_ptr<Trk::CaloExtension> extension = m_caloExtensionTool->caloExtension(ctx, *track);
  CaloExtensionHelpers::EntryExitLayerMap entryExitLayerMap;
  CaloExtensionHelpers::entryExitLayerMap(*extension, entryExitLayerMap);

  for (const xAOD::CaloCluster* cluster : *clusters) {

    // Find the most energetic layer of the cluster
    xAOD::CaloCluster::CaloSample mostEnergeticLayer = xAOD::CaloCluster::CaloSample::Unknown;
    double maxLayerClusterEnergy = std::numeric_limits<double>::min();
    for (unsigned int sampling = 0; sampling < xAOD::CaloCluster::CaloSample::FCAL0; ++sampling) {
      if(sampling == xAOD::CaloCluster::CaloSample::TileGap3) continue;
      double clusterLayerEnergy = cluster->eSample(static_cast<xAOD::CaloCluster::CaloSample>(sampling));
      if(clusterLayerEnergy > maxLayerClusterEnergy) {
        maxLayerClusterEnergy = clusterLayerEnergy;
        mostEnergeticLayer = static_cast<xAOD::CaloCluster::CaloSample>(sampling);
      }
    }

    if(mostEnergeticLayer == xAOD::CaloCluster::CaloSample::Unknown) continue;

    double clusterEta = cluster->rawEta();
    double clusterPhi = cluster->rawPhi();

    auto entryExit = entryExitLayerMap.find(mostEnergeticLayer);
    if (entryExit == entryExitLayerMap.end()) continue;
    double trackEta = entryExit->second.first.eta();
    double trackPhi = entryExit->second.first.phi();

    double deltaR = KinematicUtils::deltaR(clusterEta, trackEta, clusterPhi, trackPhi);

    for (unsigned int icone = 0; icone < drCones.size(); ++icone) {
      if (deltaR < drCones[icone]) {
        for (xAOD::CaloCluster::CaloSample sampling : samplings) {
          energyInCone[icone] += cluster->eSample(sampling);
        }
      }
    }
  }

  return energyInCone;
}




//=====================================================
int TrackTools::retrieveIndex(int sampling, float eta) const {
//====================================================
    // STORE ETA MAP
    float etamap[81] = { -0.95,-0.85,-0.75,-0.65,-0.55,-0.45,-0.35,-0.25,-0.15,-0.05,  // CELLS A-10 TO A-1  (SAMPLING 12) INDICES  0:9
        0.05, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.75, 0.85, 0.95,  // CELLS A1 TO A10    (SAMPLING 12) INDICES 10:19
        -0.85,-0.75,-0.65,-0.55,-0.45,-0.35,-0.25,-0.15,-0.05,  // CELLS BC-9 TO BC-1 (SAMPLING 13) INDICES 20:28
        0.05, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.75, 0.85,  // CELLS BC1 TO BC9   (SAMPLING 13) INDICES 29:37
        -0.60,-0.40,-0.20,  // CELLS D-3 TO D-1   (SAMPLING 14) INDICES 38:40
        0.00, 0.20, 0.40, 0.60,  // CELLS D0 TO D3     (SAMPLING 14) INDICES 41:44
        -0.956279,0.9583722,  // CELLS C-10 TO C10  (SAMPLING 15) INDICES 45:46
        -0.855940,0.8579205,  // CELLS D-4 TO D4    (SAMPLING 16) INDICES 47:48
        -1.507772,-1.307385,-1.156978,-1.056676,  // CELLS E-4 TO E-1   (SAMPLING 17) INDICES 49:52
        1.0589020,1.1593041,1.3098471,1.5103633,  // CELLS E1 TO E4     (SAMPLING 17) INDICES 53:56
        -1.554988,-1.455460,-1.355965,-1.256501,-1.157065,  // CELLS A-16 TO A-12 (SAMPLING 18) INDICES 57:61
        1.1594202,1.258668,1.3579534,1.4572804,1.5566510,  // CELLS A12 TO A16   (SAMPLING 18) INDICES 62:66
        -1.454651,-1.355081,-1.255538,-1.156018,-1.056519,  // CELLS B-15 TO B-11 (SAMPLING 19) INDICES 67:71
        1.0586925,1.1580252,1.2573844,1.3567756,1.4562022,  // CELLS B11 TO B15   (SAMPLING 19) INDICES 72:76
        -1.204743,-1.005559,  // CELLS D-6 TO D-5   (SAMPLING 20) INDICES 77:78
        1.0074122,1.2063241}; // CELLS D5 TO D6     (SAMPLING 20) INDICES 79:80
    // CALCULATE INDEX
    int index(999),i_start(999),i_end(999);
    switch(sampling){
        case 12: i_start = 0;  i_end = 19; break;
        case 13: i_start = 20; i_end = 37; break;
        case 14: i_start = 38; i_end = 44; break;
        case 15: i_start = 45; i_end = 46; break;
        case 16: i_start = 47; i_end = 48; break;
        case 17: i_start = 49; i_end = 56; break;
        case 18: i_start = 57; i_end = 66; break;
        case 19: i_start = 67; i_end = 76; break;
        case 20: i_start = 77; i_end = 80; break;
        default: break;
    } // SWITCH

    if(i_start==999 || i_end==999) return -1;

    index = i_start;

    for(int i=i_start;i <= i_end;++i) index = std::abs(eta-etamap[i]) <= std::abs(eta-etamap[index]) ? i : index;
    return index;
} // TRACKTOOLS::RETRIEVEINDEX

} // TileCal namespace
