/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HITMANAGEMENT_HitIdHelper_H
#define HITMANAGEMENT_HitIdHelper_H

#include <string>
#include <vector>

typedef int HitID;

struct IdField {
  std::string fieldName;
  int vMin = 0;
  int vMax = 0;
  int nBits = 0;
  int firstBit = 0;
};

typedef std::vector<IdField> IdFields;

class HitIdHelper {
public:
  HitIdHelper(): m_currentStorage(0)  {}
  virtual ~HitIdHelper() {}
  void InitializeField(const std::string& n, int vmn, int vmx);
  void InitializeField(const std::string& n, int nb);
  void SetFieldValue(const std::string& name, int n, HitID& targetID) const;
  int  GetFieldValue(const std::string& name, HitID targetID) const;
  void PrintFields();
  static void Print(int id);
protected:
  IdFields m_IDs;
  int m_currentStorage;
  static void SetBit(int& i, int bitpos) ;
  static void UnsetBit(int& i, int bitpos) ;
  static void Store(int& id, int value, int first, int field) ;
  static int  Retrieve(int id, int first, int field) ;
};

#endif
