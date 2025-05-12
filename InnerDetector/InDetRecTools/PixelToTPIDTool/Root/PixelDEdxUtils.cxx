#include "PixelToTPIDTool/PixelDEdxUtils.h"

namespace PixelDEdx {
  
  /// The functions below take PixelClusterStructs as input, a simple struct defined to abstract away the two EDMs.
  /// This prevents the duplication of the truncated mean logic, as well as the cluster & track angle cuts.
  /// The number of good pixel hits (hits considered for truncated mean calc) is passed by ref & incremented.
  /// As is the number of IBL hits in overflow (again, only if they are considered for the trunc mean calc).

  /// If good measurement, update cluster raw cluster dE/dx, passdEdxCutsLoose, and passdEdxCutsTight.
  /// Also, increment nUsedIBLOverflowHits.
  /// If bad measurement, keep default negative value for dE/dx, don't increment.
  void getClusterdEdx(PixelClusterStruct& cluster,
                      int& nUsedIBLOverflowHits,
                      bool tightClusterCleaning) {    

    float dEdxValue;

    //////////////////
    /// Loose cuts ///
    //////////////////

    /// Remove clusters if track is too shallow.
    if ( std::abs(cluster.cosalpha) < 0.16 ) {
      //msg << MSG::DEBUG << "Path through sensor is too shallow for good dE/dx measurement: cos(alpha) = " << cluster.cosalpha << endmsg;
      /// Do not update cluster.dEdx from default negative value.
      /// Do not update cluster.passdEdxCutsLoose or cluster.passdEdxCutsTight from default false values.
      return;
    }

    /// Now check layer & barrel vs endcap, applying local (x,y) cuts.
    if (cluster.isIBL) { // check if IBL      
      if (((cluster.eta_module >= -10 && cluster.eta_module <= -7) ||
           (cluster.eta_module >= 6 && cluster.eta_module <= 9)) &&
          (fabs(cluster.locy) < 10. &&
           (cluster.locx > -8.33 &&
            cluster.locx < 8.3))) { // check if IBL 3D and good cluster selection

        dEdxValue = cluster.charge * cluster.cosalpha * conversionfactor / IBL_3D_sensorthickness;
        cluster.passdEdxCutsLoose = true;
      } else if ((cluster.eta_module >= -6 && cluster.eta_module <= 5) &&
                 (fabs(cluster.locy) < 20. &&
                  (cluster.locx > -8.33 &&
                   cluster.locx < 8.3))) { // check if IBL planar and good cluster

        dEdxValue = cluster.charge * cluster.cosalpha * conversionfactor / IBL_PLANAR_sensorthickness;
        cluster.passdEdxCutsLoose = true;
      } else { // IBL cluster fails
        /// Do not update cluster.dEdx from default negative value.
        /// Do not update cluster.passdEdxCutsLoose or cluster.passdEdxCutsTight from default false values.
        return;
      }
    }
    /// PIXEL BARREL
    else if(cluster.bec==0 && fabs(cluster.locy)<30. &&  ((cluster.locx>-8.20 && cluster.locx<-0.60) || (cluster.locx>0.50 && cluster.locx<8.10))) {
      dEdxValue = cluster.charge * cluster.cosalpha * conversionfactor / Pixel_sensorthickness;
      cluster.passdEdxCutsLoose = true;
    }
    /// PIXEL ENDCAP
    else if (std::abs(cluster.bec)==2 && fabs(cluster.locy)<30. && ((cluster.locx>-8.15 && cluster.locx<-0.55) || (cluster.locx>0.55 && cluster.locx<8.15))) {
      dEdxValue = cluster.charge * cluster.cosalpha * conversionfactor / Pixel_sensorthickness;
      cluster.passdEdxCutsLoose = true;
    }
    else{ // Cluster fails.
      //msg << MSG::DEBUG << "Cluster fails loose cuts. bec: " << cluster.bec << ", layer: " << cluster.layer << ", locx: " << cluster.locx << ", locy: " << cluster.locy << endmsg;
      /// Do not update cluster.dEdx from default negative value.
      /// Do not update cluster.passdEdxCutsLoose or cluster.passdEdxCutsTight from default false values.
      return;
    }

    /// Should not pass this point if cluster.passdEdxCutsLoose == false.
    
    //////////////////
    /// Tight cuts ///
    //////////////////

    ///  Apply extra cluster cleaning cuts for improved dE/dx measurements.
    if (tightClusterCleaning) {

      /// TODO!
      /// Add extra cleaning cuts here when ready.
      /// Exploring cuts on cluster size to remove e.g. delta rays.

      /// Now set cluster.passdEdxCutsTight appropriately.

    }

    ////////////////////////////////////
    // Update counters & assign dE/dx //
    ////////////////////////////////////

    if (tightClusterCleaning) { // Applying tight cluster cleaning on top of the (nominal) loose cuts on (x,y) and cos(alpha).
      if(cluster.passdEdxCutsLoose && cluster.passdEdxCutsTight) { // technically shouldn't have to check cluster.passdEdxCutsLoose 
        /// Update counters & assign dE/dx.
        if (cluster.isIBL && cluster.iblOverflow) {
          nUsedIBLOverflowHits++;
        }
        cluster.dEdx = dEdxValue;
        return;
      }
      else {
        /// Do not update cluster.dEdx from default negative value.
        /// Do not update cluster.passdEdxCutsLoose or cluster.passdEdxCutsTight from default false values.
        return;
      }
    }
    else { // Only applying (nominal) loose cuts on (x,y) and cos(alpha).
      if(cluster.passdEdxCutsLoose) { // technically shouldn't have to check cluster.passdEdxCutsLoose 
        /// Update counters & assign dE/dx.
        if (cluster.isIBL && cluster.iblOverflow) {
          nUsedIBLOverflowHits++;
        }
        cluster.dEdx = dEdxValue;
        return;
      }
      else {
        /// Do not update cluster.dEdx from default negative value.
        /// Do not update cluster.passdEdxCutsLoose or cluster.passdEdxCutsTight from default false values.
        return;
      }
    }
  }

  //////////////////
  //////////////////
  //////////////////

  /// Returns the truncated mean over the track. 
  /// If equalize == true, it will use the equalized cluster dE/dx measurement in the calculation.
  /// NB:  nUsedHits (divisor of trunc mean) is passed by reference and updated.  Do not call this function multiple times with the same counter.
  /*
  float getTruncatedMean(const std::vector<PixelClusterStruct>& clusters,
                         int& nUsedHits, 
                         bool equalize) {

    int pixelhits = clusters.size();
    /// Get the dEdxMap.
    /// First in pair is the dE/dx (raw or equalized).  Second indicates if it's a IBL cluster in with ToT in overflow.
    /// Multimaps  will automatically sort based on the first element in the pair.  Useful for truncated mean alg.
    std::multimap<float,bool> dEdxMap;
    for (const auto& cluster : clusters) {
      if(equalize) {
        dEdxMap.insert(std::pair<float, bool>(cluster.dEdxEq, cluster.iblOverflow));
      }
      else {
        dEdxMap.insert(std::pair<float, bool>(cluster.dEdx, cluster.iblOverflow));
      }
    }

    /// Now calculate the truncated mean.
    float averagedEdx=0.;
    nUsedHits=0;
    int IBLOverflow=0;
    for (std::pair<float,int> itdEdx : dEdxMap) {
      if (itdEdx.second==0) {
        averagedEdx += itdEdx.first;
        nUsedHits++;
      }
      if (itdEdx.second>0) { IBLOverflow++; }

      //break, skipping last or the two last elements depending on total measurements
      if (((int)pixelhits>=5) and ((int)nUsedHits>=(int)pixelhits-2)) { break; }

      //break, IBL Overflow case pixelhits==3 and 4
      if ((int)IBLOverflow>0 and ((int)pixelhits==3) and (int)nUsedHits==1) { break; }
      if ((int)IBLOverflow>0 and ((int)pixelhits==4) and (int)nUsedHits==2) { break; }

      if (((int)pixelhits > 1) and ((int)nUsedHits >=(int)pixelhits-1)) { break; }

      if ((int)IBLOverflow>0 and (int)pixelhits==1) { //only IBL in overflow
        averagedEdx=itdEdx.first;
        break;
      }
    }

    if (nUsedHits>0 or (nUsedHits==0 and(int)IBLOverflow>0 and (int)pixelhits==1)) {
      if (nUsedHits>0) { averagedEdx=averagedEdx/nUsedHits; }

      //msg << MSG::DEBUG << "Truncated mean dEdx = " << averagedEdx << endmsg;
      //msg << MSG::DEBUG << "Used hits: " << nUsedHits << ", IBL overflows: " << IBLOverflow << endmsg;
      //msg << MSG::DEBUG << "Number of good measurements = " << pixelhits << endmsg;
      return averagedEdx;
    }
    return -1;
  }
*/

  void getdEdxMetrics(const std::vector<PixelClusterStruct>& clusters,
                         float& averagedEdx, 
                         float& sigmadEdx, 
                         int& nUsedHits, 
                         bool equalize) {

    int pixelhits = clusters.size();
    /// Get the dEdxMap.
    /// First in pair is the dE/dx (raw or equalized).  Second indicates if it's a IBL cluster in with ToT in overflow.
    /// Multimaps  will automatically sort based on the first element in the pair.  Useful for truncated mean alg.
    std::multimap<float,bool> dEdxMap;
    for (const auto& cluster : clusters) {
      if(equalize) {
        dEdxMap.insert(std::pair<float, bool>(cluster.dEdxEq, cluster.iblOverflow));
      }
      else {
        dEdxMap.insert(std::pair<float, bool>(cluster.dEdx, cluster.iblOverflow));
      }
    }

    /// Now calculate the truncated mean.
    averagedEdx=0.;
    sigmadEdx=0.;
    nUsedHits=0;
    std::vector<float> sigmadEdxVec = {};
    int IBLOverflow=0;
    for (std::pair<float,int> itdEdx : dEdxMap) {
      if (itdEdx.second==0) {
        averagedEdx += itdEdx.first;
        sigmadEdxVec.push_back(itdEdx.first);
        nUsedHits++;
      }
      if (itdEdx.second>0) { IBLOverflow++; }

      //break, skipping last or the two last elements depending on total measurements
      if (((int)pixelhits>=5) and ((int)nUsedHits>=(int)pixelhits-2)) { break; }

      //break, IBL Overflow case pixelhits==3 and 4
      if ((int)IBLOverflow>0 and ((int)pixelhits==3) and (int)nUsedHits==1) { break; }
      if ((int)IBLOverflow>0 and ((int)pixelhits==4) and (int)nUsedHits==2) { break; }

      if (((int)pixelhits > 1) and ((int)nUsedHits >=(int)pixelhits-1)) { break; }

      if ((int)IBLOverflow>0 and (int)pixelhits==1) { //only IBL in overflow
        averagedEdx=itdEdx.first;
        /// Leave sigmadEdx at it's default value of 0.
        break;
      }
    }

    /// If it's a good track measurement:
    /// At least one good non-IBL-overflow hit...
    /// Or only 1 hit exactly, and it's an IBL overflow hit..
    if (nUsedHits>0 or (nUsedHits==0 and(int)IBLOverflow>0 and (int)pixelhits==1)) {
      if (nUsedHits>0) {
        averagedEdx=averagedEdx/nUsedHits;
        for (auto val : sigmadEdxVec) {
          sigmadEdx += (val - averagedEdx)*(val - averagedEdx);
        }
        sigmadEdx = sigmadEdx / nUsedHits;
        sigmadEdx = sqrt(sigmadEdx);
      }
      /// Good measurements, so return with updated metric values.
      /// If only 1 hit, sigmadEdx = 0. 
      return;
    }
    /// Insufficient cluster measurements --> bad track measurements.
    averagedEdx = -1;
    sigmadEdx  = -1;
    return;
  }

} // namespace PixelDEdx
