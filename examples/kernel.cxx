#include "args.h"
#include <fstream>
#include "kernel.h"
#include <vector>
#include "verify.h"
using namespace exafmm;
vec3 KernelBase::Xperiodic = 0;
real_t KernelBase::eps2 = 0.0;
complex_t KernelBase::wavek = complex_t(10.,1.) / real_t(2 * M_PI);

template<Equation equation>
void initSource(Body<equation> & body) {
  body.SRC = 1;
}
template<>
void initSource<BiotSavart>(Body<BiotSavart> & body) {
  body.SRC[0] = drand48();
  body.SRC[1] = drand48();
  body.SRC[2] = drand48();
  body.SRC[3] = 0.1;
}

template<typename Kernel>
void fmm(Args args) {
  typedef typename Kernel::Bodies Bodies;                       //!< Vector of bodies
  typedef typename Kernel::Cells Cells;                         //!< Vector of cells
  typedef typename Kernel::B_iter B_iter;                       //!< Iterator of body vector
  typedef typename Kernel::C_iter C_iter;                       //!< Iterator of cell vector

  Bodies bodies(1), bodies2(1), jbodies(1);
  Kernel::init();
  logger::verbose = true;

  const int NTERM = Kernel::NTERM;
  Cells cells(4);
  Verify<Kernel> verify;
  jbodies[0].X = 2;
  initSource<Kernel::equation>(jbodies[0]);
  C_iter Cj = cells.begin();
  Cj->X = 1;
  Cj->X[0] = 3;
  Cj->SCALE = 2;
  Cj->BODY = jbodies.begin();
  Cj->NBODY = jbodies.size();
  Cj->M = 0;
  Kernel::P2M(Cj);

#if 1
  C_iter CJ = cells.begin()+1;
  CJ->ICHILD = Cj-cells.begin();
  CJ->NCHILD = 1;
  CJ->X = 0;
  CJ->X[0] = 4;
  CJ->SCALE = 4;
  CJ->M = 0;
  Kernel::M2M(CJ, cells.begin());

  C_iter CI = cells.begin()+2;
  CI->X = 0;
  CI->X[0] = -4;
  CI->SCALE = 4;
  CI->M = 1;
  CI->L = 0;
  Kernel::M2L(CI, CJ, false);

  C_iter Ci = cells.begin()+3;
  Ci->X = 1;
  Ci->X[0] = -3;
  Ci->SCALE = 2;
  Ci->IPARENT = 2;
  Ci->M = 1;
  Ci->L = 0;
  Kernel::L2L(Ci, cells.begin());
#else
  C_iter Ci = cells.begin()+3;
  Ci->X = 1;
  Ci->X[0] = -3;
  Ci->SCALE = 2;
  Ci->M = 1;
  Ci->L = 0;
  Kernel::M2L(Ci, Cj, false);
#endif

  bodies[0].X = 2;
  bodies[0].X[0] = -2;
  bodies[0].SRC = 1;
  bodies[0].TRG = 0;
  Ci->BODY = bodies.begin();
  Ci->NBODY = bodies.size();
  Kernel::L2P(Ci);

  for (B_iter B=bodies2.begin(); B!=bodies2.end(); B++) {
    *B = bodies[B-bodies2.begin()];
    B->TRG = 0;
  }
  Cj->NBODY = jbodies.size();
  Ci->NBODY = bodies2.size();
  Ci->BODY = bodies2.begin();
  Kernel::P2P(Ci, Cj, false);
  for (B_iter B=bodies2.begin(); B!=bodies2.end(); B++) {
    B->TRG /= B->SRC;
  }

  std::fstream file;
  file.open("kernel.dat", std::ios::out | std::ios::app);
  double potDif = verify.getDifScalar(bodies, bodies2);
  double potNrm = verify.getNrmScalar(bodies);
  double accDif = verify.getDifVector(bodies, bodies2);
  double accNrm = verify.getNrmVector(bodies);
  std::cout << args.P << " " << std::sqrt(potDif/potNrm) << "  " << std::sqrt(accDif/accNrm) << std::endl;
  double potRel = std::sqrt(potDif/potNrm);
  double accRel = std::sqrt(accDif/accNrm);
  verify.print("Rel. L2 Error (pot)",potRel);
  verify.print("Rel. L2 Error (acc)",accRel);
  file << args.P << " " << std::sqrt(potDif/potNrm) << "  " << std::sqrt(accDif/accNrm) << std::endl;
  file.close();
  Kernel::finalize();
}

int main(int argc, char ** argv) {
  Args args(argc, argv);                                        // Argument parser class
  switch (args.equation[0]) {                                   // Case switch for equation
  case 'L':                                                     // Laplace equation
    fmm<LaplaceKernel<Pmax> >(args);                            //  Call Laplace kernel
    break;                                                      // Break Laplace equation
  case 'H':                                                     // Helmholtz equation
    fmm<HelmholtzKernel<2*Pmax> >(args);                        //  Call Helmholtz kernel
    break;                                                      // Break Helmholtz equation
  case 'B':                                                     // Biot-Savart equation
    fmm<BiotSavartKernel<Pmax> >(args);                         //  Call Biot-Savart kernel
    break;                                                      // Break Biot-Savart equation
  default:                                                      // No matching case
    fprintf(stderr,"No matching equation\n");                   //  Print error message
    abort();                                                    //  Abort execution
  }                                                             // End case switch for equation
  return 0;
}
