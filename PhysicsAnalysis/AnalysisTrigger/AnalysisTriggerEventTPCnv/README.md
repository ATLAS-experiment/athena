# AnalysisTriggerEventTPCnv Package

Author Attila Kraznahorkay Jr. <Attila.Krasznahorkay@cern.ch>
Author Wolfgang Ehrenfeld <Wolfgang.Ehrenfeld@desy.de>
Author Alan Watson <Alan.Watson@cern.ch>

## Introduction

This package contains the persistent data classes of AnalysisTriggerEvent
and the corresponding T/P converter.

## Class Overview
  The AnalysisTriggerEventTPCnv package contains of following persistent classes:

  - CTP_Decision_p1: first version of persistent class for CTP_Decision class
  - CTP_Decision_p2: second version of persistent class for CTP_Decision class
  - LVL1_ROI_p1: first version of persisten class of LVL1_ROI class

  The LVL1_ROI and hence the LVL1_ROI_p1 class contains different LVL1 RoIs. 
  See the class description for more details.

  The AnalysisTriggerEventTPCnv package contains of following T/P converter:
  - CTP_DecisionCnv_p1: T/P converter between CTP_Decision and CTP_Decision_p1
  - CTP_DecisionCnv_p2: T/P converter between CTP_Decision and CTP_Decision_p2
  - LVL1_ROICnv_p1: T/P converter between LVL1_ROI and LVL1_ROI_p1
