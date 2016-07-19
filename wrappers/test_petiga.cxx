#include <mpi.h>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <complex>
//using namespace exafmm;

#if EXAFMM_SINGLE
  typedef float real_t;                                         //!< Floating point type is single precision
#else
  typedef double real_t;                                        //!< Floating point type is double precision
#endif
  typedef std::complex<real_t> complex_t;                       //!< Complex type


extern "C" void FMM_Init(double eps2, double kreal, double kimag, int ncrit, int threads,
                         int nb, double * xb, double * yb, double * zb, complex_t * vb,
                         int nv, double * xv, double * yv, double * zv, complex_t * vv);
extern "C" void FMM_Finalize();
extern "C" void FMM_Partition(int & nb, double * xb, double * yb, double * zb, complex_t * vb,
                              int & nv, double * xv, double * yv, double * zv, complex_t * vv);
extern "C" void FMM_BuildTree();
extern "C" void FMM_B2B(complex_t * vi, complex_t * vb, bool verbose);
extern "C" void FMM_V2B(complex_t * vb, complex_t * vv, bool verbose);
extern "C" void FMM_B2V(complex_t * vv, complex_t * vb, bool verbose);
extern "C" void FMM_V2V(complex_t * vi, complex_t * vv, bool verbose);
extern "C" void Direct(int nb, double * xb, double * yb, double * zb, complex_t * vb,
                       int nv, double * xv, double * yv, double * zv, complex_t * vv);

void Validate(int n, complex_t * vb, complex_t * vd, int verbose) {
  complex_t diff1 = 0, norm1 = 0, diff2 = 0, norm2 = 0;
  for (int i=0; i<n; i++) {
    diff1 += (vb[i] - vd[i]) * (vb[i] - vd[i]);
    norm1 += vd[i] * vd[i];
  }
  MPI_Reduce(&diff1, &diff2, 1, MPI_COMPLEX, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&norm1, &norm2, 1, MPI_COMPLEX, MPI_SUM, 0, MPI_COMM_WORLD);
  if (verbose) {
    std::cout << "--- FMM vs. direct ---------------" << std::endl;
    std::cout << std::setw(20) << std::left << std::scientific
              << "Rel. L2 Error" << " : " << std::sqrt(diff2/norm2) << std::endl;
  }
}

int main(int argc, char ** argv) {
  const int Nmax = 10000000;
  const int ncrit = 1000;
  const int threads = 16;
  const double eps2 = 0.0;
  const double kreal = 1.0;
  const double kimag = 0.1;
  double * xb = new double [Nmax];
  double * yb = new double [Nmax];
  double * zb = new double [Nmax];
  complex_t * vb = new complex_t [Nmax];
  double * xv = new double [Nmax];
  double * yv = new double [Nmax];
  double * zv = new double [Nmax];
  complex_t * vv = new complex_t [Nmax];
  complex_t * vd = new complex_t [Nmax];
  complex_t * vi = new complex_t [Nmax];

  int mpisize, mpirank;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &mpisize);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpirank);
  int nb = 50000 / mpisize;
  int nv = 100000 / mpisize;

  srand48(mpirank);
  for (int i=0; i<nb; i++) {
    xb[i] = drand48() - .5;
    yb[i] = drand48() - .5;
    zb[i] = drand48() - .5;
  }
  for (int i=0; i<nv; i++) {
    xv[i] = drand48() - .5;
    yv[i] = drand48() - .5;
    zv[i] = drand48() - .5;
  }

  FMM_Init(eps2, kreal, kimag, ncrit, threads, nb, xb, yb, zb, vb, nv, xv, yv, zv, vv);
  FMM_Partition(nb, xb, yb, zb, vb, nv, xv, yv, zv, vv);
  FMM_BuildTree();

  for (int i=0; i<nb; i++) {
    vb[i] = 1.0 / nb;
    vi[i] = 0;
    vd[i] = 0;
  }
  FMM_B2B(vi, vb, 1);
  Direct(100, xb, yb, zb, vd, nb, xb, yb, zb, vb);
  Validate(100, vi, vd, mpirank == 0);

  for (int i=0; i<nb; i++) {
    vb[i] = 0;
    vd[i] = 0;
  }
  for (int i=0; i<nv; i++) {
    vv[i] = 1.0 / nv;
  }
  FMM_V2B(vb, vv, 1);
  Direct(100, xb, yb, zb, vd, nv, xv, yv, zv, vv);
  Validate(100, vb, vd, mpirank == 0);

  for (int i=0; i<nb; i++) {
    vb[i] = 1.0 / nb;
  }
  for (int i=0; i<nv; i++) {
    vv[i] = 0;
    vd[i] = 0;
  }
  FMM_B2V(vv, vb, 1);
  Direct(100, xv, yv, zv, vd, nb, xb, yb, zb, vb);
  Validate(100, vv, vd, mpirank == 0);

  for (int i=0; i<nv; i++) {
    vv[i] = 1.0 / nv;
    vi[i] = 0;
    vd[i] = 0;
  }
  FMM_V2V(vi, vv, 1);
  Direct(100, xv, yv, zv, vd, nv, xv, yv, zv, vv);
  Validate(100, vi, vd, mpirank == 0);

  FMM_Finalize();
  MPI_Finalize();
  delete[] xb;
  delete[] yb;
  delete[] zb;
  delete[] vb;
  delete[] xv;
  delete[] yv;
  delete[] zv;
  delete[] vv;
  delete[] vd;
  delete[] vi;
}
