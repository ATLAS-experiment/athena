/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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
        CalibFrontEndInfo (int MODid, int FEid, const std::string & MODid_str, const std::string & RODid_str) {m_MODid =MODid; m_FEid=FEid; m_MODid_str =MODid_str; m_RODid_str =RODid_str;};
        ~ CalibFrontEndInfo (){};
        
        //Setters for the parameters
        void set_MODid (int x){m_MODid   = x;}
        void set_FEid  (int x){m_FEid    = x;}
        
        void set_NormalThreshold (int x){m_NormalThreshold  = x;}
        void set_NormalRms       (int x){m_NormalRms        = x;}
        void set_NormalNoise     (int x){m_NormalNoise      = x;}
        void set_NormalIntime    (int x){m_NormalIntime     = x;}
        
        void set_LongThreshold  (int x){m_LongThreshold    = x;}
        void set_LongRms        (int x){m_LongRms          = x;}
        void set_LongNoise      (int x){m_LongNoise        = x;}
        void set_LongIntime     (int x){m_LongIntime       = x;}
        
        void set_GangedThreshold (int x){m_GangedThreshold  = x;}
        void set_GangedRms       (int x){m_GangedRms        = x;}
        void set_GangedNoise     (int x){m_GangedNoise      = x;}
        void set_GangedIntime    (int x){m_GangedIntime     = x;}
        
        void set_times_fitted   (int x){m_times_fitted     = x;}
        
        //Setters for the fits
        void set_NormalParams (const std::vector<float> & x){m_NormalFitParams = x; }
        void set_LongParams   (const std::vector<float> & x){m_LongFitParams   = x; }
        void set_SigParams    (const std::vector<float> & x){m_SigFitParams    = x; }
        
        void set_NormalParamsQuality (const std::vector<float> & x){m_NormalFitParamsQuality = x; }
        void set_LongParamsQuality   (const std::vector<float> & x){m_LongFitParamsQuality   = x; }
        void set_SigParamsQuality    (const std::vector<float> & x){m_SigFitParamsQuality    = x; }
        
        
        //Getters for the parameters - coming soon
        int MODid()         const {return m_MODid;            };
        int FEid()          const {return m_FEid;             };
        std::string MODid_str()     const {return m_MODid_str;        };
        std::string FEid_str()      const {return m_RODid_str;        };
        
        int normThreshold() const {return m_NormalThreshold; };
        int normRms()       const {return m_NormalRms;       };
        int normNoise()     const {return m_NormalNoise;     };
        int normIntime()    const {return m_NormalIntime;    };
        
        int longThreshold() const {return m_LongThreshold;   };
        int longRms()       const {return m_LongRms;         };
        int longNoise()     const {return m_LongNoise;       };
        int longIntime()    const {return m_LongIntime;      };
        
        int gangThreshold() const {return m_GangedThreshold; };
        int gangRms()       const {return m_GangedRms;       };
        int gangNoise()     const {return m_GangedNoise;     };
        int gangIntime()    const {return m_GangedIntime;    };
        
        
        //Prints the information stored in case of need.
        std::stringstream printDBformat()    const;
        void printBeautyformat()const;
        void printVals()        const;
        
        //Prints the information stored in case of error - needs more implementation.
        void printMODerr()      const;
        
        
    private:
    
        
        std::string m_MODid_str  = "";
        std::string m_RODid_str  = "";
        
        int   m_MODid            = -1;
        int   m_FEid             = -1;
        
        int m_NormalThreshold    = -1;
        int m_NormalRms          = -1;
        int m_NormalNoise        = -1;
        int m_NormalIntime       = -1;
        
        int m_LongThreshold      = -1;
        int m_LongRms            = -1;
        int m_LongNoise          = -1;
        int m_LongIntime         = -1;
        
        int m_GangedThreshold    = -1;
        int m_GangedRms          = -1;
        int m_GangedNoise        = -1;
        int m_GangedIntime       = -1;
        
        int m_times_fitted       = -1;
        
        std::vector<float> m_NormalFitParams = {0,0,0};
        std::vector<float> m_LongFitParams   = {0,0,0};
        std::vector<float> m_SigFitParams    = {0,0};
        
        std::vector<float> m_NormalFitParamsQuality = {0,0};
        std::vector<float> m_LongFitParamsQuality   = {0,0};
        std::vector<float> m_SigFitParamsQuality    = {0,0};
        
        
};

#endif
