/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrackClusterAssValidationUtils_H
#define TrackClusterAssValidationUtils_H

namespace InDet {

  /////////////////////////////////////////////////////////////////////////////////
  //  Header file for class PartPropCache
  /////////////////////////////////////////////////////////////////////////////////

  class PartPropCache
    {
      ///////////////////////////////////////////////////////////////////
      // Public methods:
      ///////////////////////////////////////////////////////////////////
    public:
      ///default c'tor
      PartPropCache()  {};
      ///copy c'tor defaulted
      PartPropCache(const PartPropCache&) = default;
      ///c'tor 
      PartPropCache(int uidCharge,int rapidity);
      ///destructor does nothing
      ~PartPropCache() = default;
      ///assignment defaulted
      PartPropCache& operator = (const PartPropCache&) = default;
      ///getters
      int id () const {return abs(m_uidCharge);}
      int charge  () const {return ((m_uidCharge>0) - (m_uidCharge<0));} //returns 1, -1 or 0 depending on sign
      int rapidity() const {return m_rapidity;}

    protected:
      int m_uidCharge{};
      int m_rapidity{};
    };

 /////////////////////////////////////////////////////////////////////////////////
  // Inline methods
  /////////////////////////////////////////////////////////////////////////////////

  inline PartPropCache::PartPropCache (int uidCharge,int rapidity)
    {
      m_uidCharge = uidCharge ;
      m_rapidity  = rapidity;
    }


  struct TrackCollectionStat_t {
     public:
        int                                m_efficiency   [6]={}  ;
        int                                m_efficiencyN  [6][5]={};
        int                                m_efficiencyBTE[6][5][4]={};
        int                                m_efficiencyPOS[6]={}  ;
        int                                m_efficiencyNEG[6]={}  ;
        int                                m_ntracksPOSB{}       ;
        int                                m_ntracksPOSE{}       ;
        int                                m_ntracksPOSFWD{};
        int                                m_ntracksNEGB{}       ;
        int                                m_ntracksNEGE{}       ;
        int                                m_ntracksNEGFWD{};
        int                                m_total        [50]={} ;
        int                                m_fake         [50]={} ;
        int                                m_events{}                 ;
        int                                m_eventsPOS{}              ;
        int                                m_eventsNEG{}              ;
        int                                m_eventsBTE[4]={0}           ;


        TrackCollectionStat_t &operator+=(const TrackCollectionStat_t &a_stat) {
            for (int i=0; i<6; ++i) { m_efficiency[i]+=a_stat.m_efficiency[i];}
            for (int i=0; i<6; ++i) { for (int j=0; j<5; ++j) { m_efficiencyN[i][j]+=a_stat.m_efficiencyN[i][j];}}
            for (int i=0; i<6; ++i) { for (int j=0; j<5; ++j) { for (int k=0; k<4; ++k) { m_efficiencyBTE[i][j][k]+=a_stat.m_efficiencyBTE[i][j][k];} } }
            for (int i=0; i<6; ++i) { m_efficiencyPOS[i]+=a_stat.m_efficiencyPOS[i];}
            for (int i=0; i<6; ++i) { m_efficiencyNEG[i]+=a_stat.m_efficiencyNEG[i];}
            m_ntracksPOSB+=a_stat.m_ntracksPOSB       ;
            m_ntracksPOSE+=a_stat.m_ntracksPOSE       ;
            m_ntracksPOSFWD+=a_stat.m_ntracksPOSFWD;
            m_ntracksNEGB+=a_stat.m_ntracksNEGB       ;
            m_ntracksNEGE+=a_stat.m_ntracksNEGE       ;
            m_ntracksNEGFWD+=a_stat.m_ntracksNEGFWD;
            for (int i=0; i<50; ++i) { m_total[i]+=a_stat.m_total[i];}
            for (int i=0; i<50; ++i) { m_fake[i]+=a_stat.m_fake[i];}

            return *this;
        }

  };


  struct EventStat_t {
      public:
        int                                m_events{}      ;
        int                                m_eventsPOS{}   ;
        int                                m_eventsNEG{}   ;
        int                                m_eventsBTE[4]={};

        int                                m_particleClusters   [50]={};
        int                                m_particleSpacePoints[50]={};
        int                                m_particleClustersBTE   [50][4]={};
        int                                m_particleSpacePointsBTE[50][4]={};

        int                                m_nclustersPosBP{}         ;
        int                                m_nclustersPosBS{}         ;
        int                                m_nclustersPosEP{}         ;
        int                                m_nclustersPosES{}         ;
        int                                m_nclustersNegBP{}         ;
        int                                m_nclustersNegBS{}         ;
        int                                m_nclustersNegEP{}         ;
        int                                m_nclustersNegES{}         ;
        //
        int                                m_nclustersPTOT{}          ;
        int                                m_nclustersPTOTt{}         ;
        int                                m_nclustersSTOT{}          ;
        int                                m_nclustersSTOTt{}         ;

       
        EventStat_t &operator+=(const EventStat_t &a_stat) {
          m_events += a_stat.m_events;
          m_eventsPOS += a_stat.m_eventsPOS;
          m_eventsNEG += a_stat.m_eventsNEG;
          for (int i=0; i<4; ++i)  { m_eventsBTE[i] += a_stat.m_eventsBTE[i]; }
          for (int i=0; i<50; ++i) {m_particleClusters[i] += a_stat.m_particleClusters[i]; };
          for (int i=0; i<50; ++i) {m_particleSpacePoints[i] += a_stat.m_particleSpacePoints[i]; };
          for (int i=0; i<50; ++i) {for (int j=0; j<4; ++j) { m_particleClustersBTE[i][j] += a_stat.m_particleClustersBTE[i][j]; } }
          for (int i=0; i<50; ++i) {for (int j=0; j<4; ++j) { m_particleSpacePointsBTE[i][j] += a_stat.m_particleSpacePointsBTE[i][j];} }
          m_nclustersPosBP  += m_nclustersPosBP;
          m_nclustersPosBS  += m_nclustersPosBS;
          m_nclustersPosEP  += m_nclustersPosEP;
          m_nclustersPosES  += m_nclustersPosES;
          m_nclustersNegBP  += m_nclustersNegBP;
          m_nclustersNegBS  += m_nclustersNegBS;
          m_nclustersNegEP  += m_nclustersNegEP;
          m_nclustersNegES  += m_nclustersNegES;
          m_nclustersPTOT   +=a_stat.m_nclustersPTOT;
	  m_nclustersPTOTt  +=a_stat.m_nclustersPTOTt;
	  m_nclustersSTOT   +=a_stat.m_nclustersSTOT;
	  m_nclustersSTOTt  +=a_stat.m_nclustersSTOTt;
          return *this;
        }
      };

}
#endif // TrackClusterAssValidationUtils_H

