/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//***********************************************************************************************
//                   CalibFrontEndInfo - Class to store the calibration information
//                           ---------------------------------------
//     begin                : 01 11 2023
//     email                : sergi.rodriguez@cern.ch
//***********************************************************************************************

#ifndef PIXCALIBFRONTENDINFO_H
#define PIXCALIBFRONTENDINFO_H


#include <sstream>
#include <iostream>
#include <string>
#include <vector>

class CalibFrontEndInfo {
    public:
        CalibFrontEndInfo () {};
        CalibFrontEndInfo (int MODid, int FEid, const std::string & MODid_str, const std::string & RODid_str) :
          m_MODid_str (MODid_str),
          m_RODid_str (RODid_str),
          m_MODid (MODid),
          m_FEid (FEid)
        {}
        ~ CalibFrontEndInfo (){};
        
        //Setters for the parameters
        void set_MODid (int x){m_MODid   = x;}
        void set_FEid  (int x){m_FEid    = x;}
        
        void set_NormalTheshold (int x){m_NormalTheshold   = x;}
        void set_NormalRms      (int x){m_NormalRms        = x;}
        void set_NormalNoise    (int x){m_NormalNoise      = x;}
        void set_NormalIntime   (int x){m_NormalIntime     = x;}
        
        void set_LongTheshold   (int x){m_LongTheshold     = x;}
        void set_LongRms        (int x){m_LongRms          = x;}
        void set_LongNoise      (int x){m_LongNoise        = x;}
        void set_LongIntime     (int x){m_LongIntime       = x;}
        
        void set_GangedTheshold (int x){m_GangedTheshold   = x;}
        void set_GangedRms      (int x){m_GangedRms        = x;}
        void set_GangedNoise    (int x){m_GangedNoise      = x;}
        void set_GangedIntime   (int x){m_GangedIntime     = x;}
        
        void set_times_fitted   (int x){m_times_fitted     = x;}
        
        //Setters for the fits
        void set_NormalParams (const std::vector<float> &x){m_NormalFitParams = x; }
        void set_LongParams   (const std::vector<float> & x){m_LongFitParams   = x; }
        void set_SigParams    (const std::vector<float> & x){m_SigFitParams    = x; }
        
        void set_NormalParamsQuality (const std::vector<float> & x){m_NormalFitParamsQuality = x; }
        void set_LongParamsQuality   (const std::vector<float> & x){m_LongFitParamsQuality   = x; }
        void set_SigParamsQuality    (const std::vector<float> & x){m_SigFitParamsQuality    = x; }
        
        
        //Getters for the parameters - coming soon
        //int MODid()         const {return m_MODid;            };
        //int FEid()          const {return m_FEid;             };
        
        //int norTheshold()  const {return m_NormalTheshold;  };
        //int norRms()       const {return m_NormalRms;       };
        //int norNoise()     const {return m_NormalNoise;     };
        //int norIntime()    const {return m_NormalIntime;    };
        
        //int lonTheshold()  const {return m_LongTheshold;    };
        //int lonRms()       const {return m_LongRms;         };
        //int lonNoise()     const {return m_LongNoise;       };
        //int lonIntime()    const {return m_LongIntime;      };
        
        //int ganTheshold()  const {return m_GangedTheshold;  };
        //int ganRms()       const {return m_GangedRms;       };
        //int ganNoise()     const {return m_GangedNoise;     };
        //int ganIntime()    const {return m_GangedIntime;    };
        
        
        //Prints the information stored in case of need.
        std::stringstream printDBformat()    const;
        void printBeautyformat()const;
        void printVals()        const;
        
        //Prints the information stored in case of error - needs more implementation.
        void printMODerr()      const;
        
        
    private:
    
        
        std::string m_MODid_str   = "";
        std::string m_RODid_str   = "";
        
        int   m_MODid             = -1;
        int   m_FEid              = -1;
        
        int m_NormalTheshold     = -1;
        int m_NormalRms          = -1;
        int m_NormalNoise        = -1;
        int m_NormalIntime       = -1;
        
        int m_LongTheshold       = -1;
        int m_LongRms            = -1;
        int m_LongNoise          = -1;
        int m_LongIntime         = -1;
        
        int m_GangedTheshold     = -1;
        int m_GangedRms          = -1;
        int m_GangedNoise        = -1;
        int m_GangedIntime       = -1;
        
        int m_times_fitted        = -1;
        
        std::vector<float> m_NormalFitParams;
        std::vector<float> m_LongFitParams;
        std::vector<float> m_SigFitParams;
        
        std::vector<float> m_NormalFitParamsQuality;
        std::vector<float> m_LongFitParamsQuality;
        std::vector<float> m_SigFitParamsQuality;
        
        
};

#endif
