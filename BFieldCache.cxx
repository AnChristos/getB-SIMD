/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "BFieldCache.h"
#include "vec.h"
#include <cmath>
#if defined(__GNUC__) && !defined(__clang__)
#define ATH_ENABLE_FUNCTION_VECTORIZATION                                      \
  __attribute__((optimize("tree-vectorize")))
#else
#define ATH_ENABLE_FUNCTION_VECTORIZATION
#endif

/// existing method
void
BFieldCache::getB(const double* ATH_RESTRICT xyz,
                  double r,
                  double phi,
                  double* ATH_RESTRICT B,
                  double* ATH_RESTRICT deriv) const
{

  const double x = xyz[0];
  const double y = xyz[1];
  const double z = xyz[2];

  // make sure phi is inside [m_phimin,m_phimax]
  if (phi < m_phimin) {
    phi += 2 * M_PI;
  }
  // fractional position inside this bin
  const double fz = (z - m_zmin) * m_invz;
  const double gz = 1.0 - fz;
  const double fr = (r - m_rmin) * m_invr;
  const double gr = 1.0 - fr;
  const double fphi = (phi - m_phimin) * m_invphi;
  const double gphi = 1.0 - fphi;
  const double scale = m_scale;
  // interpolate field values in z, r, phi
  double Bzrphi[3];
  for (int i = 0; i < 3; ++i) { // z, r, phi components
    const double* field = m_field[i];
    Bzrphi[i] = scale * (gz * (gr * (gphi * field[0] + fphi * field[1]) +
                               fr * (gphi * field[2] + fphi * field[3])) +
                         fz * (gr * (gphi * field[4] + fphi * field[5]) +
                               fr * (gphi * field[6] + fphi * field[7])));
  }
  // convert (Bz,Br,Bphi) to (Bx,By,Bz)
  double invr;
  double c;
  double s;
  if (r > 0.0) {
    invr = 1.0 / r;
    c = x * invr;
    s = y * invr;
  } else {
    invr = 0.0;
    c = cos(m_phimin);
    s = sin(m_phimin);
  }
  B[0] = Bzrphi[1] * c - Bzrphi[2] * s;
  B[1] = Bzrphi[1] * s + Bzrphi[2] * c;
  B[2] = Bzrphi[0];

  // compute field derivatives if requested
  if (deriv) {
    const double sz = m_scale * m_invz;
    const double sr = m_scale * m_invr;
    const double sphi = m_scale * m_invphi;

    std::array<double, 3> dBdz;
    std::array<double, 3> dBdr;
    std::array<double, 3> dBdphi;

    for (int j = 0; j < 3; ++j) { // Bz, Br, Bphi components
      const double* field = m_field[j];
      dBdz[j] =
        sz *
        (gr * (gphi * (field[4] - field[0]) + fphi * (field[5] - field[1])) +
         fr * (gphi * (field[6] - field[2]) + fphi * (field[7] - field[3])));
      dBdr[j] =
        sr *
        (gz * (gphi * (field[2] - field[0]) + fphi * (field[3] - field[1])) +
         fz * (gphi * (field[6] - field[4]) + fphi * (field[7] - field[5])));
      dBdphi[j] =
        sphi * (gz * (gr * (field[1] - field[0]) + fr * (field[3] - field[2])) +
                fz * (gr * (field[5] - field[4]) + fr * (field[7] - field[6])));
    }
    // convert to cartesian coordinates
    const double cc = c * c;
    const double cs = c * s;
    const double ss = s * s;
    const double ccinvr = cc * invr;
    const double csinvr = cs * invr;
    const double ssinvr = ss * invr;
    const double sinvr = s * invr;
    const double cinvr = c * invr;
    deriv[0] = cc * dBdr[1] - cs * dBdr[2] - csinvr * dBdphi[1] +
               ssinvr * dBdphi[2] + sinvr * B[1];
    deriv[1] = cs * dBdr[1] - ss * dBdr[2] + ccinvr * dBdphi[1] -
               csinvr * dBdphi[2] - cinvr * B[1];
    deriv[2] = c * dBdz[1] - s * dBdz[2];
    deriv[3] = cs * dBdr[1] + cc * dBdr[2] - ssinvr * dBdphi[1] -
               csinvr * dBdphi[2] - sinvr * B[0];
    deriv[4] = ss * dBdr[1] + cs * dBdr[2] + csinvr * dBdphi[1] +
               ccinvr * dBdphi[2] + cinvr * B[0];
    deriv[5] = s * dBdz[1] + c * dBdz[2];
    deriv[6] = c * dBdr[0] - sinvr * dBdphi[0];
    deriv[7] = s * dBdr[0] + cinvr * dBdphi[0];
    deriv[8] = dBdz[0];
  }
}

/// Explicitly vectorized version
void
BFieldCache::getBVec(const double* ATH_RESTRICT xyz,
                     double r,
                     double phi,
                     double* ATH_RESTRICT B,
                     double* ATH_RESTRICT deriv) const
{

  const double x = xyz[0];
  const double y = xyz[1];
  const double z = xyz[2];

  // make sure phi is inside [m_phimin,m_phimax]
  if (phi < m_phimin) {
    phi += 2 * M_PI;
  }
  // fractional position inside this bin
  const double fz = (z - m_zmin) * m_invz;
  const double gz = 1.0 - fz;
  const double fr = (r - m_rmin) * m_invr;
  const double gr = 1.0 - fr;
  const double fphi = (phi - m_phimin) * m_invphi;
  const double gphi = 1.0 - fphi;
  const double scale = m_scale;

  // interpolate field values in z, r, phi
  CxxUtils::vec<double, 2> interpCoeffphi1 = { gphi, fphi };
  CxxUtils::vec<double, 2> interpCoeffphi2 = { gphi, fphi };
  CxxUtils::vec<double, 2> interpCoeffphi3 = { gphi, fphi };
  CxxUtils::vec<double, 2> interpCoeffphi4 = { gphi, fphi };

  std::array<double, 3> Bzrphi{};
  for (int i = 0; i < 3; ++i) { // z, r, phi components

    CxxUtils::vec<double, 2> field1 = { m_field[i][0], m_field[i][1] };
    CxxUtils::vec<double, 2> field2 = { m_field[i][2], m_field[i][3] };
    CxxUtils::vec<double, 2> field3 = { m_field[i][4], m_field[i][5] };
    CxxUtils::vec<double, 2> field4 = { m_field[i][6], m_field[i][7] };

    CxxUtils::vec<double, 2> interp1 = field1 * interpCoeffphi1 * gr;

    CxxUtils::vec<double, 2> interp2 = field2 * interpCoeffphi2 * fr;

    CxxUtils::vec<double, 2> interp3 = field3 * interpCoeffphi3 * gr;

    CxxUtils::vec<double, 2> interp4 = field4 * interpCoeffphi4 * fr;

    CxxUtils::vec<double, 2> interp =
      (gz * (interp1 + interp2) + fz * (interp3 + interp4));

    Bzrphi[i] = scale * (interp[0] + interp[1]);
  }

  // convert (Bz,Br,Bphi) to (Bx,By,Bz)
  double invr;
  double c;
  double s;
  if (r > 0.0) {
    invr = 1.0 / r;
    c = x * invr;
    s = y * invr;
  } else {
    invr = 0.0;
    c = cos(m_phimin);
    s = sin(m_phimin);
  }
  B[0] = Bzrphi[1] * c - Bzrphi[2] * s;
  B[1] = Bzrphi[1] * s + Bzrphi[2] * c;
  B[2] = Bzrphi[0];

  // compute field derivatives if requested
  if (deriv) {
    const double sz = m_scale * m_invz;
    const double sr = m_scale * m_invr;
    const double sphi = m_scale * m_invphi;

    std::array<double, 3> dBdz;
    std::array<double, 3> dBdr;
    std::array<double, 3> dBdphi;

    for (int j = 0; j < 3; ++j) { // Bz, Br, Bphi components
      const double* field = m_field[j];
      dBdz[j] =
        sz *
        (gr * (gphi * (field[4] - field[0]) + fphi * (field[5] - field[1])) +
         fr * (gphi * (field[6] - field[2]) + fphi * (field[7] - field[3])));
      dBdr[j] =
        sr *
        (gz * (gphi * (field[2] - field[0]) + fphi * (field[3] - field[1])) +
         fz * (gphi * (field[6] - field[4]) + fphi * (field[7] - field[5])));
      dBdphi[j] =
        sphi * (gz * (gr * (field[1] - field[0]) + fr * (field[3] - field[2])) +
                fz * (gr * (field[5] - field[4]) + fr * (field[7] - field[6])));
    }
    // convert to cartesian coordinates
    const double cc = c * c;
    const double cs = c * s;
    const double ss = s * s;
    const double ccinvr = cc * invr;
    const double csinvr = cs * invr;
    const double ssinvr = ss * invr;
    const double sinvr = s * invr;
    const double cinvr = c * invr;
    deriv[0] = cc * dBdr[1] - cs * dBdr[2] - csinvr * dBdphi[1] +
               ssinvr * dBdphi[2] + sinvr * B[1];
    deriv[1] = cs * dBdr[1] - ss * dBdr[2] + ccinvr * dBdphi[1] -
               csinvr * dBdphi[2] - cinvr * B[1];
    deriv[2] = c * dBdz[1] - s * dBdz[2];
    deriv[3] = cs * dBdr[1] + cc * dBdr[2] - ssinvr * dBdphi[1] -
               csinvr * dBdphi[2] - sinvr * B[0];
    deriv[4] = ss * dBdr[1] + cs * dBdr[2] + csinvr * dBdphi[1] +
               ccinvr * dBdphi[2] + cinvr * B[0];
    deriv[5] = s * dBdz[1] + c * dBdz[2];
    deriv[6] = c * dBdr[0] - sinvr * dBdphi[0];
    deriv[7] = s * dBdr[0] + cinvr * dBdphi[0];
    deriv[8] = dBdz[0];
  }
}

/// Auto vectorized version
ATH_ENABLE_FUNCTION_VECTORIZATION void
BFieldCache::getBAutoVec(const double* ATH_RESTRICT xyz,
                         double r,
                         double phi,
                         double* ATH_RESTRICT B,
                         double* ATH_RESTRICT deriv) const
{

  const double x = xyz[0];
  const double y = xyz[1];
  const double z = xyz[2];

  // make sure phi is inside [m_phimin,m_phimax]
  if (phi < m_phimin) {
    phi += 2 * M_PI;
  }
  // fractional position inside this bin
  const double fz = (z - m_zmin) * m_invz;
  const double gz = 1.0 - fz;
  const double fr = (r - m_rmin) * m_invr;
  const double gr = 1.0 - fr;
  const double fphi = (phi - m_phimin) * m_invphi;
  const double gphi = 1.0 - fphi;
  const double scale = m_scale;
  // interpolate field values in z, r, phi
  double Bzrphi[3];
  for (int i = 0; i < 3; ++i) { // z, r, phi components
    const double* field = m_field[i];
    Bzrphi[i] = scale * (gz * (gr * (gphi * field[0] + fphi * field[1]) +
                               fr * (gphi * field[2] + fphi * field[3])) +
                         fz * (gr * (gphi * field[4] + fphi * field[5]) +
                               fr * (gphi * field[6] + fphi * field[7])));
  }
  // convert (Bz,Br,Bphi) to (Bx,By,Bz)
  double invr;
  double c;
  double s;
  if (r > 0.0) {
    invr = 1.0 / r;
    c = x * invr;
    s = y * invr;
  } else {
    invr = 0.0;
    c = cos(m_phimin);
    s = sin(m_phimin);
  }
  B[0] = Bzrphi[1] * c - Bzrphi[2] * s;
  B[1] = Bzrphi[1] * s + Bzrphi[2] * c;
  B[2] = Bzrphi[0];

  // compute field derivatives if requested
  if (deriv) {
    const double sz = m_scale * m_invz;
    const double sr = m_scale * m_invr;
    const double sphi = m_scale * m_invphi;

    std::array<double, 3> dBdz;
    std::array<double, 3> dBdr;
    std::array<double, 3> dBdphi;

    for (int j = 0; j < 3; ++j) { // Bz, Br, Bphi components
      const double* field = m_field[j];
      dBdz[j] =
        sz *
        (gr * (gphi * (field[4] - field[0]) + fphi * (field[5] - field[1])) +
         fr * (gphi * (field[6] - field[2]) + fphi * (field[7] - field[3])));
      dBdr[j] =
        sr *
        (gz * (gphi * (field[2] - field[0]) + fphi * (field[3] - field[1])) +
         fz * (gphi * (field[6] - field[4]) + fphi * (field[7] - field[5])));
      dBdphi[j] =
        sphi * (gz * (gr * (field[1] - field[0]) + fr * (field[3] - field[2])) +
                fz * (gr * (field[5] - field[4]) + fr * (field[7] - field[6])));
    }
    // convert to cartesian coordinates
    const double cc = c * c;
    const double cs = c * s;
    const double ss = s * s;
    const double ccinvr = cc * invr;
    const double csinvr = cs * invr;
    const double ssinvr = ss * invr;
    const double sinvr = s * invr;
    const double cinvr = c * invr;
    deriv[0] = cc * dBdr[1] - cs * dBdr[2] - csinvr * dBdphi[1] +
               ssinvr * dBdphi[2] + sinvr * B[1];
    deriv[1] = cs * dBdr[1] - ss * dBdr[2] + ccinvr * dBdphi[1] -
               csinvr * dBdphi[2] - cinvr * B[1];
    deriv[2] = c * dBdz[1] - s * dBdz[2];
    deriv[3] = cs * dBdr[1] + cc * dBdr[2] - ssinvr * dBdphi[1] -
               csinvr * dBdphi[2] - sinvr * B[0];
    deriv[4] = ss * dBdr[1] + cs * dBdr[2] + csinvr * dBdphi[1] +
               ccinvr * dBdphi[2] + cinvr * B[0];
    deriv[5] = s * dBdz[1] + c * dBdz[2];
    deriv[6] = c * dBdr[0] - sinvr * dBdphi[0];
    deriv[7] = s * dBdr[0] + cinvr * dBdphi[0];
    deriv[8] = dBdz[0];
  }
}

/// Both auto vectorized and use vector version
ATH_ENABLE_FUNCTION_VECTORIZATION void
BFieldCache::getBBothVec(const double* ATH_RESTRICT xyz,
                         double r,
                         double phi,
                         double* ATH_RESTRICT B,
                         double* ATH_RESTRICT deriv) const
{

  const double x = xyz[0];
  const double y = xyz[1];
  const double z = xyz[2];

  // make sure phi is inside [m_phimin,m_phimax]
  if (phi < m_phimin) {
    phi += 2 * M_PI;
  }
  // fractional position inside this bin
  const double fz = (z - m_zmin) * m_invz;
  const double gz = 1.0 - fz;
  const double fr = (r - m_rmin) * m_invr;
  const double gr = 1.0 - fr;
  const double fphi = (phi - m_phimin) * m_invphi;
  const double gphi = 1.0 - fphi;
  const double scale = m_scale;

  // interpolate field values in z, r, phi
  CxxUtils::vec<double, 2> interpCoeffphi1 = { gphi, fphi };
  CxxUtils::vec<double, 2> interpCoeffphi2 = { gphi, fphi };
  CxxUtils::vec<double, 2> interpCoeffphi3 = { gphi, fphi };
  CxxUtils::vec<double, 2> interpCoeffphi4 = { gphi, fphi };

  std::array<double, 3> Bzrphi{};
  for (int i = 0; i < 3; ++i) { // z, r, phi components

    CxxUtils::vec<double, 2> field1 = { m_field[i][0], m_field[i][1] };
    CxxUtils::vec<double, 2> field2 = { m_field[i][2], m_field[i][3] };
    CxxUtils::vec<double, 2> field3 = { m_field[i][4], m_field[i][5] };
    CxxUtils::vec<double, 2> field4 = { m_field[i][6], m_field[i][7] };

    CxxUtils::vec<double, 2> interp1 = field1 * interpCoeffphi1 * gr;

    CxxUtils::vec<double, 2> interp2 = field2 * interpCoeffphi2 * fr;

    CxxUtils::vec<double, 2> interp3 = field3 * interpCoeffphi3 * gr;

    CxxUtils::vec<double, 2> interp4 = field4 * interpCoeffphi4 * fr;

    CxxUtils::vec<double, 2> interp =
      (gz * (interp1 + interp2) + fz * (interp3 + interp4));

    Bzrphi[i] = scale * (interp[0] + interp[1]);
  }

  // convert (Bz,Br,Bphi) to (Bx,By,Bz)
  double invr;
  double c;
  double s;
  if (r > 0.0) {
    invr = 1.0 / r;
    c = x * invr;
    s = y * invr;
  } else {
    invr = 0.0;
    c = cos(m_phimin);
    s = sin(m_phimin);
  }
  B[0] = Bzrphi[1] * c - Bzrphi[2] * s;
  B[1] = Bzrphi[1] * s + Bzrphi[2] * c;
  B[2] = Bzrphi[0];

  // compute field derivatives if requested
  if (deriv) {
    const double sz = m_scale * m_invz;
    const double sr = m_scale * m_invr;
    const double sphi = m_scale * m_invphi;

    std::array<double, 3> dBdz;
    std::array<double, 3> dBdr;
    std::array<double, 3> dBdphi;

    for (int j = 0; j < 3; ++j) { // Bz, Br, Bphi components
      const double* field = m_field[j];
      dBdz[j] =
        sz *
        (gr * (gphi * (field[4] - field[0]) + fphi * (field[5] - field[1])) +
         fr * (gphi * (field[6] - field[2]) + fphi * (field[7] - field[3])));
      dBdr[j] =
        sr *
        (gz * (gphi * (field[2] - field[0]) + fphi * (field[3] - field[1])) +
         fz * (gphi * (field[6] - field[4]) + fphi * (field[7] - field[5])));
      dBdphi[j] =
        sphi * (gz * (gr * (field[1] - field[0]) + fr * (field[3] - field[2])) +
                fz * (gr * (field[5] - field[4]) + fr * (field[7] - field[6])));
    }
    // convert to cartesian coordinates
    const double cc = c * c;
    const double cs = c * s;
    const double ss = s * s;
    const double ccinvr = cc * invr;
    const double csinvr = cs * invr;
    const double ssinvr = ss * invr;
    const double sinvr = s * invr;
    const double cinvr = c * invr;
    deriv[0] = cc * dBdr[1] - cs * dBdr[2] - csinvr * dBdphi[1] +
               ssinvr * dBdphi[2] + sinvr * B[1];
    deriv[1] = cs * dBdr[1] - ss * dBdr[2] + ccinvr * dBdphi[1] -
               csinvr * dBdphi[2] - cinvr * B[1];
    deriv[2] = c * dBdz[1] - s * dBdz[2];
    deriv[3] = cs * dBdr[1] + cc * dBdr[2] - ssinvr * dBdphi[1] -
               csinvr * dBdphi[2] - sinvr * B[0];
    deriv[4] = ss * dBdr[1] + cs * dBdr[2] + csinvr * dBdphi[1] +
               ccinvr * dBdphi[2] + cinvr * B[0];
    deriv[5] = s * dBdz[1] + c * dBdz[2];
    deriv[6] = c * dBdr[0] - sinvr * dBdphi[0];
    deriv[7] = s * dBdr[0] + cinvr * dBdphi[0];
    deriv[8] = dBdz[0];
  }
}

