/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Updates:
 * - 2022 Jan, Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *   Added checks to catch issues when quantities needed
 *   to build the readout geometry are not set correctly.
 */

#include "TileDetDescr/TileDetDescrManager.h"
#include "TileDetDescr/TileDddbManager.h"
#include "TileDetDescr/TileCellDim.h"
#include "GeoModelKernel/Units.h"

#include "CaloDetDescr/CaloDetDescriptor.h"
#include "CaloDetDescr/CaloDetectorElements.h"
#include "CaloIdentifier/TileID.h"
#include "TileIdentifier/TileHWID.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "AthenaKernel/getMessageSvc.h"

#include <iostream>
#include <iomanip>
#include <cmath>


// 23-feb-2005

/**
   Z-shift for ext.barrel introduced
   with Z-shift > 0, eta coordinate of the cell
   is shifted by
   Deta = tile_zshift * tanh(eta) / Zcen =
   = tile_zshift * tanh(eta) *
   tan(2 * atan(exp(-eta))) / Rcen
   where Zcen and Rcen are Z and R coordinate
   of the cell center

   After the shift cell width becomes
   smaller than nominal value (e.g. 0.098 instead of 0.1)

   What is the correct eta for gap/crack scintillators
   is an open question, in any case their sizes in eta/phi
   do not match sizes in R/Z so one can leave them without shift
   but for the moment Z-shift is applied to ITC/gap/crack
   in the same way as for all Ext.Barrel cells
*/

// Temporary solution for cell volumes
#include "CellVolumes.h"
// ------------- Temporary solution for cell volumes

TileDetDescrManager::TileDetDescrManager(TileDddbManager_ptr dbManager)
        : AthMessaging ("TileDetDescrManager")
        , m_dbManager(std::move(dbManager))
        , m_elements_created(false)
        , m_tile_id(0)
        , m_cell_id(0)
        , m_tile_hwid(0)
{
  setName("Tile");
}

TileDetDescrManager::~TileDetDescrManager()
{
  clear();
}

// -------- Access to raw geometry: -----------
unsigned int TileDetDescrManager::getNumTreeTops() const
{
  return m_treeTops.size();
}

PVConstLink TileDetDescrManager::getTreeTop(unsigned int i) const
{
  if (i<m_treeTops.size())
    return m_treeTops[i];
  else
    return 0;
}

// -------------- Add a Tree top: ------------
void TileDetDescrManager::addTreeTop(PVConstLink link)
{
  m_treeTops.push_back(link);
}

// --- Readonly access to TileDddbManager object ----
TileDddbManager* TileDetDescrManager::getDbManager() const
{
  return m_dbManager.get();
}

// --- Release DB Manager ---
void TileDetDescrManager::releaseDbManager()
{
  m_dbManager.reset();
}

void TileDetDescrManager::print() const
{
  std::cout << "TileDetDescrManager       : " << std::endl;
  std::cout << "   Number of Tile regions : " << m_tile_region_vec.size()  << std::endl;
  std::cout << "   Number of Tile descr   : " << m_tile_descr_vec.size()   << std::endl;

  tile_region_const_iterator    first = tile_region_begin();
  tile_region_const_iterator    last  = tile_region_end();

  for (; first != last; ++first) {
    (*first)->print();
  }

  std::cout << "   Number of Tile modules : "
            << m_tile_module_vec.size() << std::endl;

  calo_descr_const_iterator first1 = tile_module_begin();
  calo_descr_const_iterator last1  = tile_module_end();
  for (; first1 != last1; ++first1) {
    (*first1)->print();  // too many !
  }

  std::cout << "   Number of Tile cells : "
            << m_tile_cell_vec.size() << std::endl;

  calo_element_const_iterator   first2 = tile_cell_begin();
  calo_element_const_iterator   last2  = tile_cell_end();
  for (; first2 != last2; ++first2) {
    (*first2)->print();  // too many !
  }

  std::cout << std::endl;
}

void TileDetDescrManager::create_elements()
{
  create_elements (m_tile_id->do_checks());
}
void TileDetDescrManager::create_elements(bool checks)
{
  ATH_MSG_INFO( "Entering create_elements()" );

  // resize vectors :
  m_tile_module_vec.resize( (int) m_tile_id->module_hash_max(),0);
  m_tile_cell_vec.resize( (int) m_tile_id->cell_hash_max(),0);

  IdContext module_context = m_tile_id->module_context();
  IdContext cell_context   = m_tile_id->cell_context();

  tile_descr_const_iterator first = tile_descriptors_begin();
  tile_descr_const_iterator last  = tile_descriptors_end();

  int n_regions = 0;
  int n_modules = 0;
  int n_cells = 0;

  // For each descriptor :
  ATH_MSG_DEBUG( "Looping over descriptors..." );

  for (; first != last; ++first) {

    TileDetDescriptor* descr = *first;
    Identifier reg_id = descr->identify();

    int section = m_tile_id->section(reg_id);
    int side    = m_tile_id->side(reg_id);

    double zshift = descr->zshift();
    bool doZshift = (zshift != 0.0);

    int etasign = descr->sign_eta();
    if (side != etasign) {
      ATH_MSG_ERROR( "side and eta sign in TileDetDescriptor[" << n_regions
                     << "] do not match" );
    }
    ++n_regions;

    ATH_MSG_DEBUG( "descriptor - " << reg_id << ", " << section << ", " << side
                   << ", " << zshift << ", " << doZshift
                   << ", " << etasign << ", " << n_regions );

    int nsamp = descr->n_samp();

    int    nphi = descr->n_phi();
    double dphi = descr->dphi();
    double phi_min = descr->phi_min();
    double phi_max = descr->phi_max();

    // create calo descriptors per one calo sample firsrt ( 6 * 3 = 18 in total)
    CaloDetDescriptor* caloDescr;

    int calo_sample0;
    if ( m_tile_id->is_tile_barrel(reg_id))
      calo_sample0 = (int)CaloCell_ID::TileBar0;
    else if ( m_tile_id->is_tile_extbarrel(reg_id))
      calo_sample0 = (int)CaloCell_ID::TileExt0;
    else if ( m_tile_id->is_tile_gap(reg_id))
      calo_sample0 = (int)CaloCell_ID::TileGap1 - 1;
    else
      calo_sample0 = CaloCell_ID::Unknown;
    std::vector<double> depth_in(1);
    std::vector<double> depth_out(1);

    bool gap = (nsamp > (int)TileID::SAMP_E);
    for (int isamp=0; isamp<(int)TileID::SAMP_E; ++isamp) {

      int neta = descr->n_eta(isamp);

      if (neta>0) {
        int neta = descr->n_eta(isamp);
        double deta = descr->deta(isamp);
        double emin = descr->eta_min(isamp);
        double emax = descr->eta_max(isamp);
        double rmin = descr->rcenter(isamp)-descr->dr(isamp)/2;
        double rmax = descr->rcenter(isamp)+descr->dr(isamp)/2;
        double zmin = descr->zcenter(isamp)-descr->dz(isamp)/2;
        double zmax = descr->zcenter(isamp)+descr->dz(isamp)/2;
        depth_in[0] = rmin;
        depth_out[0] = rmax;

        if ((zmax+zmin)/2. < 0) {
          double ztmp = zmax;
          zmax = -zmin;
          zmin = -ztmp;
        }

        CaloCell_ID::CaloSample sample = (CaloCell_ID::CaloSample)(calo_sample0 + isamp);

        caloDescr = new CaloDetDescriptor(reg_id,static_cast<const AtlasDetectorID *>(m_tile_id),m_cell_id,sample,isamp);
        // caloDescr->set_cylindric(emin,emax,phi_min,phi_max,rmin,rmax,zmin,zmax,gap);
        // --
        caloDescr->setCaloEtaMin(emin);
        caloDescr->setCaloEtaMax(emax);
        caloDescr->setCaloPhiMin(phi_min);
        caloDescr->setCaloPhiMax(phi_max);
        caloDescr->setCaloRMin(rmin);
        caloDescr->setCaloRMax(rmax);
        caloDescr->setCaloZMin(zmin);
        caloDescr->setCaloZMax(zmax);
        // --

        caloDescr->set_eta_phi_granularity(neta,deta,nphi,dphi);
        caloDescr->set_n_calo_depth(1);
        caloDescr->set_depth_in(depth_in);
        caloDescr->set_depth_out(depth_out);
        add_calodescr(caloDescr);
      }
    }

    // special case - all gap scin in one sample
    if (gap) {
      double emin = 1.e+10;
      double emax =-1.e+10;
      double rmin = 1.e+10;
      double rmax =-1.e+10;
      double zmin = 1.e+10;
      double zmax =-1.e+10;

      for (int isamp=TileID::SAMP_E; isamp<nsamp; ++isamp) {

        int neta = descr->n_eta(isamp);

        if (neta>0) {
          emin = std::min(emin,(double)descr->eta_min(isamp));
          emax = std::max(emax,(double)descr->eta_max(isamp));
          rmin = std::min(rmin,(double)(descr->rcenter(isamp)-descr->dr(isamp)/2));
          rmax = std::max(rmax,(double)(descr->rcenter(isamp)+descr->dr(isamp)/2));
          zmin = std::min(zmin,(double)(descr->zcenter(isamp)-descr->dz(isamp)/2));
          zmax = std::max(zmax,(double)(descr->zcenter(isamp)+descr->dz(isamp)/2));
        }
      }

      if ((zmax+zmin)/2. < 0) {
        double ztmp = zmax;
        zmax = -zmin;
        zmin = -ztmp;
      }

      double deta    = 0.1; // fixed deta for gap scin
      int    neta    = (emax>1.65) ? 7 : 6;   // fixed number of eta bins for gap scin - 6 for RUN1/RUN2, 7 for RUN3/RUN4
      depth_in[0] = zmin;
      depth_out[0] = zmax;
      CaloCell_ID::CaloSample sample = CaloCell_ID::TileGap3;

      caloDescr = new CaloDetDescriptor(reg_id,static_cast<const AtlasDetectorID *>(m_tile_id),m_cell_id,sample,TileID::SAMP_E);
      // caloDescr->set_cylindric(emin,emax,phi_min,phi_max,rmin,rmax,zmin,zmax,true);
      // --
      caloDescr->setCaloEtaMin(emin);
      caloDescr->setCaloEtaMax(emax);
      caloDescr->setCaloPhiMin(phi_min);
      caloDescr->setCaloPhiMax(phi_max);
      caloDescr->setCaloRMin(rmin);
      caloDescr->setCaloRMax(rmax);
      caloDescr->setCaloZMin(zmin);
      caloDescr->setCaloZMax(zmax);
      // --

      caloDescr->set_eta_phi_granularity(neta,deta,nphi,dphi);
      caloDescr->set_n_calo_depth(1);
      caloDescr->set_depth_in(depth_in);
      caloDescr->set_depth_out(depth_out);
      add_calodescr(caloDescr);
    }


    double emin = 99999.0;
    double emax =-99999.0;
    double rmin = 99999.0;
    double rmax =-99999.0;
    std::vector<double> delr;

    for (int isamp=0; isamp<nsamp; ++isamp) {
      delr.push_back(descr->dr(isamp));
      emin = std::min(emin,(double)descr->eta_min(isamp));
      emax = std::max(emax,(double)descr->eta_max(isamp));
      rmin = std::min(rmin,(double)(descr->rcenter(isamp)-descr->dr(isamp)/2));
      rmax = std::max(rmax,(double)(descr->rcenter(isamp)+descr->dr(isamp)/2));
    }
    if (emin < 0.0 ) emin = 0.0; // avoid emin = -0.1 because of D0 in barrel
    if (etasign < 0) {
      double etmp = emax;
      emax = -emin;
      emin = -etmp;
    }

    // and now create calo descriptors per every module ( 6 * 64 = 384 in total)
    double phi  = descr->phi_min() + dphi/2.;

    for (int iphi=0; iphi<nphi; ++iphi) {

      int module = iphi; // we count modules from 0 to N always

      // Temporary solution for cell volumes
      int volumeIndex = 0;
      // ------------ Temporary solution for cell volumes

      CaloDetDescriptor* modDescr;

      try {
        Identifier id = m_tile_id->module_id(section,side,module, checks);
        IdentifierHash idhash;
        /* int result = */ m_tile_id->get_hash(id,idhash,&module_context);
        modDescr = new CaloDetDescriptor(id,static_cast<const AtlasDetectorID *>(m_tile_id),m_cell_id);
        // modDescr->set_cylindric(emin,emax,phi-dphi/2.,phi+dphi/2,rmin,rmax);
        // --
        modDescr->setCaloEtaMin(emin);
        modDescr->setCaloEtaMax(emax);
        modDescr->setCaloPhiMin(phi-dphi/2.);
        modDescr->setCaloPhiMax(phi+dphi/2.);
        modDescr->setCaloRMin(rmin);
        modDescr->setCaloRMax(rmax);
        modDescr->setCaloZMin(rmin*sinh(emin));
        modDescr->setCaloZMax(rmax*sinh(emax));

        // --
        // modDescr->set_depth(delr,nsamp);
        modDescr->set_n_calo_depth(nsamp);
        modDescr->set_depth_in(delr);
        add_module(idhash,modDescr);
        ++n_modules;
      } catch ( const TileID_Exception& ) {
        ATH_MSG_ERROR( "can't build module ID from ("
                       << section << ","
                       << side << ","
                       << module << ")" );
        continue;
      }

      for (int isamp=0; isamp<nsamp; ++isamp) {

        int neta = descr->n_eta(isamp);

        if (neta>0) {

          int sample = std::min(isamp,(int)TileID::SAMP_E); // all gap scin are in sampling 3
          double rcenter = descr->rcenter(isamp);
          double dr      = descr->dr(isamp);

          double deta = descr->deta(isamp);
          double eta  = descr->eta_min(isamp) + deta/2.; // this is unsigned eta

          for (int ieta=0; ieta<neta; ++ieta) {

            int tower = (sample == (int)TileID::SAMP_E) ? isamp : (int)((eta + 0.01) * 10); // tower number in 0.1 granularity

            try {
              Identifier id = m_tile_id->cell_id(section,side,module,tower,sample,checks);
              IdentifierHash idhash;
              /* int result = */ m_tile_id->get_hash(id,idhash,&cell_context);

              TileDetectorElement* elt = new TileDetectorElement(
                  idhash, TileHWID::NOT_VALID_HASH, TileHWID::NOT_VALID_HASH, modDescr);

              TileCellDim* cell_dim = get_cell_dim(id);

              // ideal eta/phi for all the cells
              elt->set_cylindric_raw(eta*etasign,phi,rcenter);

              if (doZshift) {

                double eta1 = shiftEta(eta-deta/2,rcenter,zshift);
                double eta2 = shiftEta(eta+deta/2,rcenter,zshift);

                if (0 == iphi) {
                  ATH_MSG_VERBOSE( "side/sec/mod/twr/samp="
                                   <<side<<"/"<<section<<"/"<<module<<"/"<<tower<<"/"<<sample
                                   << " rcen = "<<rcenter
                                   << " dr = "<<dr
                                   << " eta = "  << eta
                                   << " eta1 = " << eta1
                                   << " eta2 = " << eta2
                                   << " eta' = " << (eta1+eta2)/2.
                                   << " deta = " << (eta2-eta1)
                                   << " iphi = " << iphi
                                   << " dphi = " << dphi );
                }

                elt->set_cylindric((eta1+eta2)/2.*etasign,phi,rcenter);
//                elt->set_cylindric_size((eta2-eta1),dphi,dr);
// keep ideal deta for the cell (0.1 or 0.2) instead of (eta2-eta1),
// otherwise algorithms which use "eta_raw" and "deta" might fail
//                elt->set_cylindric_size(deta,dphi,dr);
                elt->set_deta(deta);
                elt->set_dphi(dphi);
                elt->set_dr(dr);

              } else {

                if (0 == iphi) {
                  ATH_MSG_VERBOSE( "side/sec/mod/twr/samp="
                                   <<side<<"/"<<section<<"/"<<module<<"/"<<tower<<"/"<<sample
                                   << " rcen = "<<rcenter
                                   << " dr = "<<dr
                                   << " eta = "  << eta
                                   << " deta = " << deta
                                   << " iphi = " << iphi
                                   << " dphi = " << dphi );
                }

                elt->set_cylindric(eta*etasign,phi,rcenter);
                // elt->set_cylindric_size(deta,dphi,dr);
                elt->set_deta(deta);
                elt->set_dphi(dphi);
                elt->set_dr(dr);
              }

              // Temporary solution for cell volumes
              if (section == TileID::BARREL && side == -1 && ieta == 0 && sample == 2)
                ++volumeIndex; // skip D0 in negative side
              if (section == TileID::BARREL)
                elt->set_volume(vBarrelCells[volumeIndex++]);
              else if (section == TileID::EXTBAR)
                elt->set_volume(vExtendedCells[volumeIndex++]);
              else if (section == TileID::GAPDET)
                elt->set_volume(vItcGapCells[volumeIndex++]);

              // ----------------- Final solution for cell volumes
              if (cell_dim) {

                double oldz = elt->z();
                double olddz = elt->dz();
                int ic=cell_dim->getNRows()-1;
                double z1=0,z2=0;
                if (side < 0) {
                  z1 = cell_dim->getZMax(0);
                  for ( ; ic>=0; --ic) {
                    z2 = cell_dim->getZMin(ic);
                    if (0 == iphi)
                      ATH_MSG_DEBUG( "z2: " << z2 << ", ZMax: " << cell_dim->getZMax(ic) << ", diff: " << z2-cell_dim->getZMax(ic) );
                    if (fabs(z2-cell_dim->getZMax(ic))>0.1) break;
                  }
                } else {
                  z1 = cell_dim->getZMin(0);
                  for ( ; ic>=0; --ic) {
                    z2 = cell_dim->getZMax(ic);
                    if (0 == iphi)
                      ATH_MSG_DEBUG( "z2: " << z2 << ", ZMin: " << cell_dim->getZMin(ic) << ", diff: " << z2-cell_dim->getZMin(ic) );
                    if (fabs(z2-cell_dim->getZMin(ic))>0.1) break;
                  }
                }

                if (ic<0) {
                  ATH_MSG_WARNING( "TileDetDescrManager -- ic < 0! Expect crashes or misbehavior! ==> This should be checked, because 'ic' should be related to the numbers of rows!! Note: 'ic' gets < 0 when z2-cell_dim is too small and does not make the above loop break; that can be caused, for example, if 'barrelPeriodThickness' and 'extendedPeriodThickness' are not set (or set to the default 0. value) and, as a result, ZMax is not properly set." );
                }

                double z = (z1+z2)/2.;

                // D-layer has cells that are centered at 0.
                // For these cells, the above calculation will usually
                // come out to ~ 1e-14, the exact value varying depending
                // on platform.  For reproducibility, force very small
                // numbers to 0.
                if (std::abs(z) < 1e-8 * Gaudi::Units::mm) {
                  if (0 == iphi)
                    ATH_MSG_DEBUG( "Cell D0 - put cell center at Z=0" );
                  z = 0;
                }

                double dz = 0.5 * fabs(cell_dim->getZMax(0)     // special
                                       -cell_dim->getZMin(0)    // calculations
                                       +cell_dim->getZMax(ic)   // to get BC cells
                                       -cell_dim->getZMin(ic)); // size and position right

                if (section == TileID::BARREL && sample==1 && tower < 8) {
                  if (msgLvl (MSG::VERBOSE) && 0 == iphi) {
                    double z1  = 0.5*(cell_dim->getZMax(0)+cell_dim->getZMin(0));
                    double dz1 = fabs(cell_dim->getZMax(0)-cell_dim->getZMin(0));
                    double z2  = 0.5*(cell_dim->getZMax(ic)+cell_dim->getZMin(ic));
                    double dz2 = fabs(cell_dim->getZMax(ic)-cell_dim->getZMin(ic));
                    ATH_MSG_VERBOSE( "old z/dz: " << oldz << " " << olddz << std::endl
                                     <<"new z/dz: " << z << " " << dz << " ( B: " << z1 << " " << dz1 << "  C: " << z2 << " " << dz2 << " ) "
                                     << z/oldz*100-100 << " % diff "
                                     <<"do not change z/dz for BC cells in barrel" );
                  }
                } else if ( (section == TileID::GAPDET) && (sample == TileID::SAMP_E) ) {

                  elt->set_z( descr->zcenter(isamp) );
                  elt->set_dz( descr->dz(isamp) );
                  if (0 == iphi) {
                    ATH_MSG_VERBOSE( "old z/dz: " << oldz << " " << olddz << std::endl
                                     <<"new z/dz: " << elt->z() << " " << elt->dz() << " "
                                     << elt->z()/oldz*100-100 << " % diff "
                                     <<"use z/dz from descriptor for E cells" );
                  }

                } else {
                  // elt->set_z_pos_and_size(z,dz);
                  elt->set_z(z);
                  elt->set_dz(dz);
                  if (0 == iphi) {
                    ATH_MSG_VERBOSE( "old z/dz: " << oldz << " " << olddz << std::endl
                                     <<"new z/dz: " << elt->z() << " " << elt->dz() << " "
                                     << elt->z()/(oldz+1.e-10)*100-100 << " % diff" );
                  }
                }


                double oldr = elt->r();
                double olddr = elt->dr();
                double r1 = cell_dim->getRMin(0);
                double r2 = cell_dim->getRMax(ic);
                // elt->set_r_pos_and_size((r1+r2)/2.,fabs(r2-r1));

                elt->set_r((r1+r2)/2.);
                elt->set_dr(fabs(r2-r1));
                if (0 == iphi) {
                  ATH_MSG_VERBOSE( "old r/dr: " << oldr << " " << olddr << std::endl
                                   <<"new r/dr: " << elt->r() << " " << elt->dr() << " "
                                   << elt->r()/(oldr+1.e-10)*100-100 << " % diff" );
                }

                double oldv=elt->volume();
                double newv = cell_dim->getVolume();   // ps  cell volume to correct for special cells
                elt->set_volume(newv);
                //
                // ps cutout region in ext. barrel
                //
                int ModuleNcp = module + 1;

                if ( ( section == TileID::EXTBAR ) && ( TileID::SAMP_A == sample ) && ((ModuleNcp>=35 && ModuleNcp<=37) || (ModuleNcp>=60 && ModuleNcp<=62)) && ( (tower == 11) || (tower == 12) || (tower == 15) ) )
                  {
                    double oldv=elt->volume();

                    if (msgLvl (MSG::VERBOSE)) {
                      ATH_MSG_VERBOSE( "CUTOUT CELL VOLUME UPDATE"<< std::endl
                                       <<"old volume: " << oldv << std::endl
                                       << " iphi = " << iphi
                                       << " phi = " << (module + 0.5)*dphi  <<" phi2 = "<<elt->phi()
                                       << std::endl
                                       << " Mod# = " << ModuleNcp <<" module = "<<module<<" tower = "<<tower
                                       << std::endl
                                       <<"sample = "<<sample
                                       <<" A = " << (TileID::SAMP_A  == sample)
                                       <<" BC = "<< (TileID::SAMP_BC == sample)
                                       <<" D = " << (TileID::SAMP_D  == sample)
                                       <<" ---------------------" );
                      cell_dim->print();
                    }

                    double  Radius2HL = tan ( M_PI / 64. );

                    m_dbManager->SetCurrentSection(TileID::EXTBAR);
                    double epThickness = 0.0; // only used for tower == 15
                    double epVolume    = 0.0; // only used for tower == 15
                    if ( tower == 15 ) epThickness = m_dbManager->TILBdzend2() * Gaudi::Units::cm;

                    double volumeInRow[5];  // book large array
                    for (int iRow = 0; iRow < 5; iRow++) volumeInRow[iRow] = 0.;


                    for (unsigned int iRow = 0; iRow < cell_dim->getNRows(); iRow++)
                      {
                        double rowVolume = cell_dim->computeRowVolume(iRow);
                        double oldrv = rowVolume;

                        ATH_MSG_VERBOSE( " *** rowVolume = "<<rowVolume<<" volumeInRow = "<< volumeInRow[iRow] );

                        ATH_MSG_DEBUG( "Computing radMax and deltaZ..." );
                        double radMax =  cell_dim->getRMax(iRow), radMin = cell_dim->getRMin(iRow);
                        double deltaZ =  cell_dim->getZMax(iRow) - cell_dim->getZMin(iRow);

                        ATH_MSG_VERBOSE( "deltaz = "<<deltaZ<<" rmax = "<<radMax<<" rmin = "<<radMin<<" rowVolume = "<<rowVolume<<" oldrv = "<<oldrv<<" irow = "<<iRow<<" tower = "<<tower );

                        if ( 15 == tower )
                          {
                            if ( m_dbManager->addPlatesToCell() )
                              {
                                epVolume   = ( rowVolume/deltaZ ) * epThickness;
                                rowVolume -= epVolume;
                                deltaZ -= epThickness;

                                ATH_MSG_VERBOSE( " \t\t epV = "<<epVolume<<" epT = "<<epThickness << " diff = "<<(volumeInRow[iRow] - oldrv)/oldrv*100. );
                              }

                            volumeInRow[iRow] += (rowVolume * (32./48) );  // the remaining 32 periods which are not cutted out
                            deltaZ    *= 16./48.;                          // dz of the cutted part
                            rowVolume *= 16./48.;                          // volume of the cutted part of the cell, but before cut
                            ATH_MSG_VERBOSE( " *** 15 rowVolume = "<<rowVolume<<" volumeInRow = "<< volumeInRow[iRow] <<" diff = "<<(volumeInRow[iRow] - oldrv)/oldrv*100. );
                          }
                        else if ( 12 == tower )
                          {
                            volumeInRow[iRow] += (rowVolume * (2./25) );   // the remaining 2 periods which are not cutted out
                            deltaZ    *= 23./25.;                          // dz of the cutted part
                            rowVolume *= 23./25.;                          // volume of the cutted part of the cell, but before cut
                            ATH_MSG_VERBOSE( " *** 12 rowVolume = "<<rowVolume<<" volumeInRow = "<< volumeInRow[iRow] << " % = "<<(volumeInRow[iRow] - oldrv)/oldrv*100. );
                          }

                        ATH_MSG_VERBOSE( "deltaz = "<<deltaZ<<" rmax = "<<radMax<<" rmin = "<<radMin<<" changed dz ?"<<(volumeInRow[iRow] - oldrv)/oldrv*100. );

                        if ( (ModuleNcp == 61) || (ModuleNcp == 36) )
                          {
                            if ( iRow == 0 )
                              rowVolume = 0.;
                            else if ( iRow == 1 )
                              {
                                if (15 != tower)
                                  {
                                    rowVolume = 2.*radMax + radMin;
                                    rowVolume *= 0.5*(radMax - radMin);
                                    rowVolume *= Radius2HL * deltaZ;
                                  }
                                else
                                  {
                                    rowVolume += epVolume;
                                  }
                              }
                            else if (15 == tower) rowVolume += epVolume;

                            volumeInRow[iRow] += rowVolume;

                            ATH_MSG_VERBOSE( " *** rowVolume = "<<rowVolume<<" volumeInRow = "<< volumeInRow[iRow] << " % = "<<(volumeInRow[iRow] - oldrv)/oldrv*100. );
                          } // Module 61  36

                        if ( (ModuleNcp == 62) || (ModuleNcp == 35) )
                          {
                            if ( iRow == 0 )
                              {
                                if (15 == tower)
                                  {
                                    rowVolume = radMax + radMin;
                                    rowVolume *= (radMax - radMin);
                                    rowVolume -= 0.5*radMin*(27*Gaudi::Units::mm);
                                    rowVolume *= Radius2HL *  ( deltaZ + epThickness ); // adding volume of cutted endplate
                                  }
                                else
                                  {
                                    rowVolume = radMax - radMin - 35*Gaudi::Units::mm;
                                    rowVolume *= (radMax + radMin);
                                    rowVolume *= Radius2HL * deltaZ;
                                  }
                              }
                            else if (15 == tower) rowVolume += epVolume;

                            volumeInRow[iRow] += rowVolume;
                            ATH_MSG_VERBOSE( " *** rowVolume = "<<rowVolume<<" volumeInRow = "<< volumeInRow[iRow] << " % = "<<(volumeInRow[iRow] - oldrv)/oldrv*100. );
                          } // Module 62  35

                        if ( (ModuleNcp == 60) || (ModuleNcp == 37) )
                          {
                            double deltax = 38.7*std::cos(25.3125*Gaudi::Units::deg);
                            double pstan  = std::tan(25.3125*Gaudi::Units::deg);
                            double inv_pstan = 1. / pstan;
                            if ( ( 15 == tower ) )
                              {
                                if ( iRow < 2 )
                                  rowVolume = 0;
                                else
                                  rowVolume += epVolume;
                              }
                            else if ( iRow == 0 )
                              {
                                rowVolume  = (radMax  + radMin) * Radius2HL ;

                                rowVolume +=  2.*deltax + (radMax - radMin)* pstan ;

                                // std::cout <<"\t\t\t test = "<<( 2.*deltax + (radMax - radMin)* pstan)/(radMax  + radMin)* Radius2HL<<" dx = "<<deltax<<" rmin = "<<radMin<<" rmax = "<<radMax<<" pstan = "<<pstan<<std::endl;

                                rowVolume *= 0.5 * (radMax - radMin) * deltaZ;

                                // rowVolume *=  (radMax - radMin) * Radius2HL * deltaZ;
                              }
                            else if ( iRow == 1 )
                              {
                                double radMin0 = cell_dim->getRMin(0);
                                rowVolume  = (radMax  + radMin) * Radius2HL;
                                rowVolume += 2.*deltax + (radMax + radMin - 2.*radMin0 )* pstan ;
                                rowVolume *= 0.5 * (radMax - radMin) ;
                                rowVolume -= 0.5 * std::pow( deltax + (radMax - radMin0) * pstan - radMax * Radius2HL, 2) * inv_pstan;
                                rowVolume *= deltaZ;
                              }

                            volumeInRow[iRow] += rowVolume;
                            ATH_MSG_VERBOSE( " *** rowVolume = "<<rowVolume<<" volumeInRow = "<< volumeInRow[iRow] << " % = "<<(volumeInRow[iRow] - oldrv)/oldrv*100. );
                          } // Module 60  37

                        ATH_MSG_VERBOSE( " *** rowVolume = "<<rowVolume<<" volumeInRow = "<< volumeInRow[iRow] << " % = "<<(volumeInRow[iRow] - oldrv)/oldrv*100. );
                      } // for iRow


                    double totalVolume = 0;
                    for (unsigned int iRow = 0; iRow < cell_dim->getNRows(); iRow++) totalVolume += volumeInRow[iRow];
                    elt->set_volume( totalVolume );

                    ATH_MSG_VERBOSE( "\n total volume = "
                                     << elt->volume() << " cell_dim xCheck ( "
                                     <<  cell_dim->getVolume() <<" ) "
                                     << elt->volume()/oldv*100-100 << " % diff" );


                    ATH_MSG_VERBOSE( "-----------------------------------------------" );
                  }
                //
                // ps special D4
                //
                if ( (section == TileID::GAPDET) && (sample == TileID::SAMP_D) )
                  {
                    if (side == -1) m_dbManager->SetCurrentEnvByType(4);
                    if (side ==  1) m_dbManager->SetCurrentEnvByType(5);

                    m_dbManager->SetCurrentModuleByIndex(module);
                    m_dbManager->SetCurrentSection(TileDddbManager::TILE_PLUG1);
                    int Id4 = int(fmod(m_dbManager->GetModType(),10));
                    m_dbManager->SetCurrentSection(Id4);

                    double standardD4dz = elt->dz();
                    double specialD4dz = m_dbManager->TILBdzmodul()*Gaudi::Units::cm;
                    if (! m_dbManager->addPlatesToCell()) specialD4dz -= m_dbManager->TILBdzend1()*Gaudi::Units::cm;
                    // special case of non-existing D4 in EBA15 and EBC18
                    // if ( Id4 == 8 ) specialD4dz = 0.; commented out - don't set size to zero - it'll be used in TICAL derivation code

                    if (elt->z() > 0.)
                      elt->set_z(elt->z()+0.5*(standardD4dz-specialD4dz));
                    else
                      elt->set_z(elt->z()-0.5*(standardD4dz-specialD4dz));
                    elt->set_dz(specialD4dz);
                    elt->set_volume(newv * specialD4dz/(standardD4dz));

                    if (Id4 > 3) {
                      ATH_MSG_VERBOSE( "spD4 old z/dz: " << oldz << " " << standardD4dz << std::endl
                                       <<"spD4 new z/dz: " << elt->z() << " " << elt->dz() << " "
                                       << elt->z()/(oldz+1.e-10)*100-100 << " % diff" );

                      ATH_MSG_VERBOSE( "spD4 old volume: " << oldv << std::endl
                                       <<"spD4 new volume: " << elt->volume() << " "
                                       << elt->volume()/oldv*100-100 << " % diff"
                                       << " iphi = " << iphi
                                       << " dphi = " << dphi << " phi = " << (module + 0.5)*dphi - M_PI <<" phi2 = "<<elt->phi() );
                    }
                  }
                //
                // ps special C10
                //

                /*
                  if ( (section == TileID::GAPDET) && (sample == TileID::SAMP_C) )
                  {
                  if (side == -1) m_dbManager->SetCurrentEnvByType(4);
                  if (side ==  1) m_dbManager->SetCurrentEnvByType(5);

                  m_dbManager->SetCurrentModuleByIndex(module);
                  m_dbManager->SetCurrentSection(TileDddbManager::TILE_PLUG2);
                  int Ic10 = int(fmod(m_dbManager->GetModType()/100,100));

                  double standardC10dz = elt->dz();
                  double standardC10vol = elt->volume();

                  if ( Ic10 == 14 )
                  {
                  m_dbManager->SetCurrentSection( Ic10 );
                  m_dbManager->SetCurrentScin( m_dbManager->TILBcurscint() );

                  double specialC10dz = m_dbManager->SCNTdt();
                  elt->set_dz(specialC10dz);
                  elt->set_volume(standardC10vol * specialC10dz/standardC10dz );

                  if ( m_verbose )
                  std::cout<<"C10 old z/dz: " << oldz << " " << standardC10dz << std::endl
                  <<"C10 new z/dz: " << elt->z() << " " << elt->dz() << " "
                  << elt->z()/(oldz+1.e-10)*100-100 << " % diff" << std::endl;
                  if ( m_verbose )
                  std::cout<<"spC10 old volume: " << newv << std::endl
                  <<"spC10 new volume: " << elt->volume() << " "
                  << elt->volume()/oldv*100-100 << " % diff"
                  << " iphi = " << iphi
                  << " dphi = " << dphi
                  << std::endl;
                  }
                  }
                */

                //
                // ps special D5+D4
                //
                if ( (section == TileID::EXTBAR) && (sample == TileID::SAMP_D) && (tower == 10) )
                  {
                    // this is needed to read special D4 type and its size
                    if (side == -1) m_dbManager->SetCurrentEnvByType(4);
                    if (side ==  1) m_dbManager->SetCurrentEnvByType(5);
                    m_dbManager->SetCurrentModuleByIndex(module);
                    m_dbManager->SetCurrentSection(TileDddbManager::TILE_PLUG1);
                    int Id4 = int(fmod(m_dbManager->GetModType(),10));
                    m_dbManager->SetCurrentSection(Id4);

                    if ( Id4 == 8 )
                      {
                        oldz = elt->z();
                        double standardD5dz = elt->dz();
                        double specialD4dz = m_dbManager->TILBdzmodul()*Gaudi::Units::cm;
                        double specialD4Rin = m_dbManager->TILBrmin()*Gaudi::Units::cm;
                        if (m_dbManager->addPlatesToCell()) {
                          specialD4Rin -= m_dbManager->TILBdrfront()*Gaudi::Units::cm;
                        } else {
                          specialD4dz -= m_dbManager->TILBdzend1()*Gaudi::Units::cm;
                        }
                        double newS = (pow((elt->r() + elt->dr()/2.),2) - specialD4Rin*specialD4Rin)/(2.*elt->r()*elt->dr());
                        elt->set_volume(elt->volume()* (1. + newS*specialD4dz/standardD5dz));
                        /* commented out - don't change z and dz for special D5+D4
                        elt->set_dz(specialD4dz + standardD5dz);
                        if (elt->z() > 0.)
                          elt->set_z(elt->z()-0.5*specialD4dz);
                        else
                          elt->set_z(elt->z()+0.5*specialD4dz);
                        */

                        ATH_MSG_VERBOSE( "D5    old z/dz: " << oldz << " " << standardD5dz << "  spD4 dz: " << specialD4dz );
                        ATH_MSG_VERBOSE( "D5+D4 new z/dz: " << elt->z() << " " << elt->dz() << " " );
                        ATH_MSG_VERBOSE( "D5+D4 old volume: " << newv << std::endl
                                         <<"D5+D4 new volume: " << elt->volume() << " "
                                         << elt->volume()/oldv*100-100 << " % diff"
                                         << " iphi = " << iphi
                                         << " dphi = " << dphi <<" Id4 = "<< Id4 <<" modType = "<<m_dbManager->GetModType()
                                         << " phi = " << (module + 0.5)*dphi - M_PI<<" phi2 = "<<elt->phi() );
                        ATH_MSG_VERBOSE( "  module = "<< module
                                         << "  sample = " << sample
                                         << "  section = " << section
                                         << "  tower = " << tower );
                      }
                  }

                if (0 == iphi) {
                  ATH_MSG_VERBOSE( "old volume: " << oldv << std::endl
                                   <<"new volume: " << elt->volume() << " "
                                   << elt->volume()/oldv*100-100 << " % diff"
                                   << " iphi = " << iphi
                                   << " dphi = " << dphi );
                }
              }

              add_cell(elt);
              ++n_cells;
            } catch ( const TileID_Exception& ) {
              ATH_MSG_ERROR( "can't build cell ID from ("
                             << section << "," << side << "," << module << ","
                             << tower << "," << sample << ")" );
            }
            eta += deta;
          }
        }
      }
      phi += dphi;
    }
  }

  ATH_MSG_DEBUG( n_cells << " cells and "
                 << n_modules << " half-modules were created for "
                 << n_regions << " Tile Regions" );
}


void TileDetDescrManager::add(TileDetDescrRegion* region)
{
  m_tile_region_vec.push_back(region);
  m_tile_region_map[region->identify().get_identifier32().get_compact()] = region;
}

void TileDetDescrManager::add(TileDetDescriptor* descriptor)
{
  m_tile_descr_vec.push_back(descriptor);
}

void TileDetDescrManager::add_calodescr(CaloDetDescriptor* descr)
{
  int index = descr->getSampling(0)*descr->calo_sign();
  m_calo_descr_vec.push_back(descr);
  m_calo_descr_map[index] = descr;

  ATH_MSG_VERBOSE( "new Tile CaloDetDescriptor" );
  ATH_MSG_VERBOSE( " index " << index );
  ATH_MSG_VERBOSE( " calo_sample " << descr->getSampling(0) );
  ATH_MSG_VERBOSE( " calo_sign " << descr->calo_sign() );
  ATH_MSG_VERBOSE( " layer " << descr->layer() );
  ATH_MSG_VERBOSE( " is tile " << ((descr->is_tile()) ? "true" : "false" ) );
  // ATH_MSG_VERBOSE( " is cylindric " << (descr->is_cylindric() ? "true" : "false") );
  // ATH_MSG_VERBOSE( " is ec_cylindric " << (descr->is_ec_cylindric() ? "true" : "false") );
  ATH_MSG_VERBOSE( " deta " << descr->deta() );
  ATH_MSG_VERBOSE( " dphi " << descr->dphi() );
  ATH_MSG_VERBOSE( " n_eta " << descr->n_eta() );
  ATH_MSG_VERBOSE( " n_phi " << descr->n_phi() );
  ATH_MSG_VERBOSE( " calo_eta_min " << descr->calo_eta_min() );
  ATH_MSG_VERBOSE( " calo_eta_max " << descr->calo_eta_max() );
  ATH_MSG_VERBOSE( " calo_phi_min " << descr->calo_phi_min() );
  ATH_MSG_VERBOSE( " calo_phi_max " << descr->calo_phi_max() );
  ATH_MSG_VERBOSE( " calo_r_min " << descr->calo_r_min() );
  ATH_MSG_VERBOSE( " calo_r_max " << descr->calo_r_max() );
  ATH_MSG_VERBOSE( " calo_z_min " << descr->calo_z_min() );
  ATH_MSG_VERBOSE( " calo_z_max " << descr->calo_z_max() );
}

void TileDetDescrManager::add_module(IdentifierHash idhash, CaloDetDescriptor* module)
{
  m_tile_module_vec[idhash] = module;
  m_tile_module_map[module->identify().get_identifier32().get_compact()] = module;
}

void TileDetDescrManager::add_cell(CaloDetDescrElement* cell)
{
  m_tile_cell_vec[cell->subcalo_hash()] = cell;
  unsigned int id = m_tile_id->cell_id(cell->subcalo_hash()).get_identifier32().get_compact();
  m_tile_cell_map[id] = cell;
}

void TileDetDescrManager::add_cellDim(int section, int side, int tower, int sample, TileCellDim* cellDim)
{
  unsigned int ID = m_tile_id->cell_id(section,side,0,tower,sample).get_identifier32().get_compact();
  m_cell_dim_map[ID] = cellDim;
}

TileCellDim* TileDetDescrManager::get_cell_dim(const Identifier& cell_id) const
{
  int section = m_tile_id->section(cell_id);
  if (section == Tile_Base_ID::AUXDET) return NULL;
  int side = m_tile_id->side(cell_id);
  int tower = m_tile_id->tower(cell_id);
  int sample = m_tile_id->sample(cell_id);

  unsigned int newCell = m_tile_id->cell_id(section,side,0,tower,sample).get_identifier32().get_compact();
  if (m_cell_dim_map.find(newCell) != m_cell_dim_map.end()) {
    return (*m_cell_dim_map.find(newCell)).second;
  } else {
    return NULL;
  }
}

void TileDetDescrManager::clear()
{
  // Remove regions and descriptors
  tile_region_const_iterator first = m_tile_region_vec.begin();
  tile_region_const_iterator last  = m_tile_region_vec.end();
  for (; first != last; ++first) delete (*first);
  m_tile_region_vec.clear();

  tile_descr_const_iterator first2 = m_tile_descr_vec.begin();
  tile_descr_const_iterator last2  = m_tile_descr_vec.end();
  for (; first2 != last2; ++first2) delete (*first2);
  m_tile_descr_vec.clear();

  calo_descr_const_iterator first3 = m_tile_module_vec.begin();
  calo_descr_const_iterator last3  = m_tile_module_vec.end();
  for (; first3 != last3; ++first3) delete (*first3);
  m_tile_module_vec.clear();

  calo_element_const_iterator first4 = m_tile_cell_vec.begin();
  calo_element_const_iterator last4  = m_tile_cell_vec.end();
  for (; first4 != last4; ++first4) delete (*first4);
  m_tile_cell_vec.clear();

  // Remove Cell Dim's
  cell_dim_map::const_iterator first5 = m_cell_dim_map.begin();
  cell_dim_map::const_iterator last5 = m_cell_dim_map.end();
  for (; first5 != last5; ++first5) delete (*first5).second;

  // clear cell descriptors used in CaloDetDescrManager
  calo_descr_const_iterator first6 = m_calo_descr_vec.begin();
  calo_descr_const_iterator last6  = m_calo_descr_vec.end();
  for (; first6 != last6; ++first6) delete (*first6);
  m_calo_descr_vec.clear();
}
