# getB-SIMD
Playground for using SIMD for the magnetic field cache interpolation

# First clone/install

Assuming you want also the google bench mark code
``git clone --recursive https://github.com/AnChristos/getB-SIMD.git``
``cd  cd getB-SIMD/``
``source buildGoogleBenchmark.sh``

# Run the test
Sanity test to see things working
``source build_test.sh ``
``./getB_test`` 
Ouput
```
get field std: i, bxyz 0 -2.83727e-07, 9.47007e-08, 0.00308551 fractional diff gt 10^-5: 0, 0, 0
get field std: i, bxyz 1 -2.81403e-07, 7.49033e-08, 0.00255923 fractional diff gt 10^-5: 0, 0, 0
get field std: i, bxyz 2 -2.79079e-07, 5.51058e-08, 0.00203296 fractional diff gt 10^-5: 0, 0, 0
get field std: i, bxyz 3 -2.76755e-07, 3.53084e-08, 0.00150669 fractional diff gt 10^-5: 0, 0, 0
get field std: i, bxyz 4 -2.74431e-07, 1.5511e-08, 0.000980422 fractional diff gt 10^-5: 0, 0, 0
get field std: i, bxyz 5 -2.72107e-07, -4.28645e-09, 0.000454151 fractional diff gt 10^-5: 0, 0, 0
get field std: i, bxyz 6 -2.69782e-07, -2.40839e-08, -7.21201e-05 fractional diff gt 10^-5: 0, 0, 0
get field std: i, bxyz 7 -2.67458e-07, -4.38813e-08, -0.000598391 fractional diff gt 10^-5: 0, 0, 0
get field std: i, bxyz 8 -2.65134e-07, -6.36787e-08, -0.00112466 fractional diff gt 10^-5: 0, 0, 0
get field std: i, bxyz 9 -2.6281e-07, -8.34762e-08, -0.00165093 fractional diff gt 10^-5: 0, 0, 0
runTest: status 0
```
