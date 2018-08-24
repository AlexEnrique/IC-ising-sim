#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// random number generator file
#include "random_generator.h"

// structure of all observables
// #include "observables.h"

// those variables are used in the function header
double dE;
double beta; // beta == 1/(kT)
unsigned int jPos = 1;
// functions used
#include "functions.h"

// Consants declared with #define used in the program
#include "extern_defs_var.h"

#include "lattice_structure.h"
#include "observables_structure.h"

int main (int argc, char *argv[]) {
  unsigned int n = MAX_N;
  double T, dT, minT;
	char *filename = malloc(BUFF_SIZE * sizeof(*filename));
	char *dir = malloc(BUFF_SIZE * sizeof(*dir));
	char *command = malloc(BUFF_SIZE * sizeof(*command));

	// T = atof(*(argv + 1));
  // int j = atoi(*(argv + 1));

  startRNG(); // Starts the random number generator
  SpinsLattice lattice = createLattice(n, n);
  Observables observables = createObservables();

  // Initialization of variables
  dT = DELTA_T;
  T = INITIAL_TEMPERATURE + dT; // dT é descontado no inicio do 'while' abaixo
  minT = MIN_TEMPERATURE;

  // Initialization of structures
  lattice.memSpinsAlloc(&lattice);
  lattice.initSpinsInline(&lattice);
  // lattice.initSpinsRandomly(&lattice);

  // File to output the data calculated
  #define OBSRV_FOLDER "FinalObservables3"
	snprintf(dir, BUFF_SIZE, "%s", OBSRV_FOLDER);
	snprintf(command, BUFF_SIZE, "if [ ! -d \"%s\" ]; then mkdir %s; fi", dir, dir);
	system(command);

	snprintf(dir, BUFF_SIZE, "%s/j=%uf", OBSRV_FOLDER, j);

	snprintf(command, BUFF_SIZE, "if [ ! -d \"%s\" ]; then mkdir %s; fi", dir, dir);
	system(command);

	snprintf(filename, BUFF_SIZE, "%s/simulation.dat", dir);
  FILE *filePtr = fopen(filename, "w");

  // Formating output file
  fprintf(filePtr, "# i = %d\tj = %d\n", lattice.pos.i, lattice.pos.j);
  fprintf(filePtr, "# T(K)   <E>   |<M>|   Szi   SziSzj  Corr(Szi,Szj)\n");
  fprintf(filePtr, "# ---  ------  -----  -----  ------  -------------\n");

  showCriticalTemperature(0); // 0 == no, 1 == yes
  // for (int j = 1; j < n; j++) {
  while ((T -= dT) > minT) {
    beta = 1/T; // k == 1
    lattice.pos.j = j;

    // Float the spins for disregarding transient states
    lattice.floatSpins(&lattice);

    // Calculate the observables before the MC loop (for some temperature)
    observables.energy = totalEnergy(lattice);
    observables.magnetization = totalMagnetization(lattice);
    observables.Szi = totalSzi(lattice);
    observables.Szj = totalSzj(lattice);
    observables.SziSzj = totalSziSzj(lattice);
    observables.Sxi = totalSxi(lattice);
    observables.SxiSxj = totalSxiSxj(lattice);
    observables.Z1X2 = totalZ1X2(lattice);
    observables.X1Z2 = totalX1Z2(lattice);

    // Monte Carlo loop
    for (unsigned int i = 0; i < MAX_MC_LOOPS; i++) {
      // Metropolis loop
      for (unsigned int j = 0; j < lattice.size; j++) {
        lattice.choseRandomPosition(&lattice);
        if ( lattice.flipSpin(&lattice) )
          observables.adjust(lattice, &observables);
      } // end Metropolis loop

      // Store the new observables
      observables.E[i] = observables.energy;
      observables.M[i] = observables.magnetization;
      observables.SziArr[i] = observables.Szi;
      observables.SzjArr[i] = observables.Szj;
      observables.SziSzjArr[i] = observables.SziSzj;
      observables.SxiArr[i] = observables.Sxi;
      observables.SxiSxjArr[i] = observables.SxiSxj;
      observables.Z1X2Arr[i] = observables.Z1X2;
      observables.X1Z2Arr[i] = observables.X1Z2;
    } // end monte carlo loop

    observables.average(lattice, &observables); // calcula avgE e avgM (e outros)

    // output data
    // printing T, <E>, |<M>|
    fprintf(filePtr, " %.2lf  %u  %.3lf  %.3lf  %.3lf  %.3lf  %.3lf  %.3lf\n", T, j, observables.avgE, fabs(observables.avgM), observables.avgSzi, observables.avgSziSzj, observables.avgSziSzj - observables.avgSzi * observables.avgSzj, observables.avgSxi);
    // printf(" %.2lf  %u  %.3lf  %.3lf  %.3lf  %.3lf  %.3lf  %.3lf\n", T, j, observables.avgE, fabs(observables.avgM), observables.avgSzi, observables.avgSziSzj, observables.avgSzi * observables.avgSziSzj - observables.avgSziSzj, observables.avgSxi); // this print is just to see in which Temperature is the program
  } // end while

  // the following commands desalocates the memory used
  stopRNG(); // stops random number generator
  lattice.freeMemory(&lattice);
  observables.freeMemory(&observables);
  fclose(filePtr);
  // printf("\a");

  return 0;
}