/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

//
// BFieldZone.h
//
// A "zone" inside the toroid field map
//
// Masahiro Morii, Harvard University
//
#ifndef BFIELDZONE_H
#define BFIELDZONE_H

#include "BFieldMesh.h"
#include <vector>

class BFieldZone : public BFieldMesh<short>
{
public:
  // constructor
  BFieldZone(int id,
             double zmin,
             double zmax,
             double rmin,
             double rmax,
             double phimin,
             double phimax,
             double scale)
    : BFieldMesh<short>(zmin, zmax, rmin, rmax, phimin, phimax, scale)
    , m_id(id)
  {
    ;
  }
  // scale B field by a multiplicative factor: RDS 2019/09 - no longer used.
  // Scaling is done in cachec
  void scaleField(double factor)
  {
    scaleBscale(factor);
  }
  // accessors
  int id() const { return m_id; }
  // adjust the min/max edges to a new value
  void adjustMin(int i, double x)
  {
    m_min[i] = x;
    m_mesh[i].front() = x;
  }
  void adjustMax(int i, double x)
  {
    m_max[i] = x;
    m_mesh[i].back() = x;
  }

private:
  int m_id;                       // zone ID number
};


#endif

