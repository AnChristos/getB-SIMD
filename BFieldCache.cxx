/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "BFieldCache.h"
#include "vec.h"
#include <cmath>

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
    Bzrphi[i] = scale * (gz * (gr * (gphi * field[0] + fphi * field[4]) +
                               fr * (gphi * field[1] + fphi * field[5])) +
                         fz * (gr * (gphi * field[2] + fphi * field[6]) +
                               fr * (gphi * field[3] + fphi * field[7])));
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
        (gr * (gphi * (field[2] - field[0]) + fphi * (field[6] - field[4])) +
         fr * (gphi * (field[3] - field[1]) + fphi * (field[7] - field[5])));
      dBdr[j] =
        sr *
        (gz * (gphi * (field[1] - field[0]) + fphi * (field[5] - field[4])) +
         fz * (gphi * (field[3] - field[2]) + fphi * (field[7] - field[6])));
      dBdphi[j] =
        sphi * (gz * (gr * (field[4] - field[0]) + fr * (field[5] - field[1])) +
                fz * (gr * (field[6] - field[2]) + fr * (field[7] - field[3])));
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

  CxxUtils::vec<double, 4> rInterCoeff = { gr, fr, gr, fr };

  // Load  Bz at 8 corners of the bin
  CxxUtils::vec<double, 4> field1_z = {
    m_field[0][0], m_field[0][1], m_field[0][2], m_field[0][3]
  };
  CxxUtils::vec<double, 4> field2_z = {
    m_field[0][4], m_field[0][5], m_field[0][6], m_field[0][7]
  };
  // Load Br at 8 corners of the bin
  CxxUtils::vec<double, 4> field1_r = {
    m_field[1][0], m_field[1][1], m_field[1][2], m_field[1][3]
  };
  CxxUtils::vec<double, 4> field2_r = {
    m_field[1][4], m_field[1][5], m_field[1][6], m_field[1][7]
  };
  // Load Bphi at 8 corners of the bin
  CxxUtils::vec<double, 4> field1_phi = {
    m_field[2][0], m_field[2][1], m_field[2][2], m_field[2][3]
  };
  CxxUtils::vec<double, 4> field2_phi = {
    m_field[2][4], m_field[2][5], m_field[2][6], m_field[2][7]
  };

  CxxUtils::vec<double, 4> gPhiM_z = field1_z * gphi;
  CxxUtils::vec<double, 4> fPhiM_z = field2_z * fphi;
  CxxUtils::vec<double, 4> interp_z = (gPhiM_z + fPhiM_z) * rInterCoeff;

  CxxUtils::vec<double, 4> gPhiM_r = field1_r * gphi;
  CxxUtils::vec<double, 4> fPhiM_r = field2_r * fphi;
  CxxUtils::vec<double, 4> interp_r = (gPhiM_r + fPhiM_r) * rInterCoeff;

  CxxUtils::vec<double, 4> gPhiM_phi = field1_phi * gphi;
  CxxUtils::vec<double, 4> fPhiM_phi = field2_phi * fphi;
  CxxUtils::vec<double, 4> interp_phi = (gPhiM_phi + fPhiM_phi) * rInterCoeff;

  //  We end up with
  //  3 (z,r,phi) size 4 SIMD vectors :
  //  The entries of each of the 3 SIMD vectors are :
  //  0 :  gr * (gphi * field[0] + fphi * field[4]) ,
  //  1 : fr * (gphi * field[1] + fphi * field[5]) ,
  //  2 : gr * (gphi * field[2] + fphi * field[6]) ,
  //  3 :3 fr * (gphi * field[3] + fphi * field[7]) ,

  // We want to retain also binary compatibility
  // Switch to 4 vector of 3 entries (z,r,phi)

  CxxUtils::vec<double, 4> Bzrphivec0 = {
    interp_z[0], interp_r[0], interp_phi[0], 0
  };
  CxxUtils::vec<double, 4> Bzrphivec1 = {
    interp_z[1], interp_r[1], interp_phi[1], 0
  };
  CxxUtils::vec<double, 4> Bzrphivec2 = {
    interp_z[2], interp_r[2], interp_phi[2], 0
  };
  CxxUtils::vec<double, 4> Bzrphivec3 = {
    interp_z[3], interp_r[3], interp_phi[3], 0
  };

  // Do the final interpolation ster for all 3 Bz,Br,Bphi
  CxxUtils::vec<double, 4> Bzrphi1 = (Bzrphivec0 + Bzrphivec1) * gz;
  CxxUtils::vec<double, 4> Bzrphi2 = (Bzrphivec2 + Bzrphivec3) * fz;
  // now create the final (r,z,phi) values
  CxxUtils::vec<double, 4> Bzrphi = (Bzrphi1 + Bzrphi2) * m_scale;

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

    CxxUtils::vec<double, 4> field_0 = {
      m_field[0][0], m_field[1][0], m_field[2][0], 0
    };
    CxxUtils::vec<double, 4> field_1 = {
      m_field[0][1], m_field[1][1], m_field[2][1], 0
    };
    CxxUtils::vec<double, 4> field_2 = {
      m_field[0][2], m_field[1][2], m_field[2][2], 0
    };
    CxxUtils::vec<double, 4> field_3 = {
      m_field[0][3], m_field[1][3], m_field[2][3], 0
    };
    CxxUtils::vec<double, 4> field_4 = {
      m_field[0][4], m_field[1][4], m_field[2][4], 0
    };
    CxxUtils::vec<double, 4> field_5 = {
      m_field[0][5], m_field[1][5], m_field[2][5], 0
    };
    CxxUtils::vec<double, 4> field_6 = {
      m_field[0][6], m_field[1][6], m_field[2][6], 0
    };
    CxxUtils::vec<double, 4> field_7 = {
      m_field[0][7], m_field[1][7], m_field[2][7], 0
    };

    CxxUtils::vec<double, 4> dBdz;
    CxxUtils::vec<double, 4> dBdr;
    CxxUtils::vec<double, 4> dBdphi;
    dBdz =
      sz * (gr * (gphi * (field_2 - field_0) + fphi * (field_6 - field_4)) +
            fr * (gphi * (field_3 - field_1) + fphi * (field_7 - field_5)));
    dBdr =
      sr * (gz * (gphi * (field_1 - field_0) + fphi * (field_5 - field_4)) +
            fz * (gphi * (field_3 - field_2) + fphi * (field_7 - field_6)));
    dBdphi =
      sphi * (gz * (gr * (field_4 - field_0) + fr * (field_5 - field_1)) +
              fz * (gr * (field_6 - field_2) + fr * (field_7 - field_3)));

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

