#ifndef args_h
#define args_h
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <iomanip>

static struct option long_options[] = {
  {"numBodies",    1, 0, 'n'},
  {"numTargets",   1, 0, 't'},
  {"ncrit",        1, 0, 'c'},
  {"nspawn",       1, 0, 's'},
  {"images",       1, 0, 'i'},
  {"theta",        1, 0, 'o'},
  {"mutual",       1, 0, 'm'},
  {"verbose",      1, 0, 'v'},
  {"distribution", 1, 0, 'd'},
  {"repeat",       1, 0, 'r'},
  {"help",         0, 0, 'h'},
  {0, 0, 0, 0}
};

class Args {
public:
  int numBodies;
  int numTargets;
  int ncrit;
  int nspawn;
  int images;
  double theta;
  int mutual;
  int verbose;
  const char * distribution;
  int repeat;

private:
  void usage(char * name) {
    fprintf(stderr,
            "Usage: %s [options]\n"
            "Option : Description (Default value):\n"
            " --numBodies : Number of bodies (%d)\n"
            " --numTargets : Number of targets for error checking (%d)\n"
            " --ncrit : Number of bodies per leaf cell (%d)\n"
            " --nspawn : Threshold for stopping thread spawning during recursion (%d)\n"
            " --images : Number of periodic image levels (%d)\n"
            " --theta : Multipole acceptance criterion (%f)\n"
            " --mutual [0/1] : Use mutual interaction (%d)\n"
	    " --verbose [0/1] : Print information to screen (%d)\n"
            " --distribution [l/c/s/p] : lattice, cube, sphere, plummer (%s)\n"
            " --repeat R : repeat R times\n"
            " --help : Show this help document\n",
            name,
            numBodies,
            numTargets,
            ncrit,
            nspawn,
            images,
            theta,
            mutual,
	    verbose,
            distribution);
  }

  const char * parse(const char * arg) {
    switch (arg[0]) {
    case 'l':
      return "lattice";
    case 'c':
      return "cube";
    case 's':
      return "sphere";
    case 'p':
      return "plummer";
    default:
      fprintf(stderr, "invalid distribution %s\n", arg);
      exit(0);
    }
    return "";
  }

public:
  Args(int argc=0, char ** argv=NULL) : numBodies(1000000), numTargets(100), ncrit(16), nspawn(1000), images(0),
					theta(.4), mutual(1), verbose(1), distribution("cube"), repeat(1) {
    while (1) {
      int option_index;
      int c = getopt_long(argc, argv, "", long_options, &option_index);
      if (c == -1) break;
      switch (c) {
      case 'n':
        numBodies = atoi(optarg);
        break;
      case 't':
        numTargets = atoi(optarg);
        break;
      case 'c':
        ncrit = atoi(optarg);
        break;
      case 's':
        nspawn = atoi(optarg);
        break;
      case 'i':
        images = atoi(optarg);
        break;
      case 'o':
        theta = atof(optarg);
        break;
      case 'm':
        mutual = atoi(optarg);
        break;
      case 'v':
	verbose= atoi(optarg);
	break;
      case 'd':
        distribution = parse(optarg);
        break;
      case 'r':
        repeat = atoi(optarg);
        break;
      case 'h':
        usage(argv[0]);
        exit(0);
      default:
        usage(argv[0]);
        exit(0);
      }
    }
  }

  void print(int stringLength, int P, int mpirank=-1) {
    if (verbose) {
      std::cout << std::setw(stringLength) << std::fixed << std::left// Set format
		<< "numBodies" << " : " << numBodies << std::endl // Print numBodies  
		<< std::setw(stringLength)                      // Set format
		<< "P" << " : " << P << std::endl               // Print P
		<< std::setw(stringLength)                      // Set format
		<< "theta" << " : " << theta << std::endl       // Print theta
		<< std::setw(stringLength)                      // Set format
		<< "ncrit" << " : " << ncrit << std::endl       // Print ncrit
		<< std::setw(stringLength)                      // Set format
		<< "nspawn" << " : " << nspawn << std::endl     // Print nspawn
		<< std::setw(stringLength)                      // Set format
		<< "images" << " : " << images << std::endl     // Print images
		<< std::setw(stringLength)                      // Set format
		<< "mutual" << " : " << mutual << std::endl     // Print mutual
		<< std::setw(stringLength)                      // Set format
		<< "verbose" << " : " << verbose << std::endl   // Print verbose
		<< std::setw(stringLength)                      // Set format
		<< "distribution" << " : " << distribution << std::endl// Print distribution
		<< std::setw(stringLength)                      // Set format
		<< "repeat" << " : " << repeat << std::endl;    // Print distribution
    } else if (mpirank == 0) {
      std::cout << std::setw(stringLength) << std::left         // Set format
		<< "numBodies" << " : " << numBodies << std::endl; // Print numBodies  
    }
  }
};
#endif
