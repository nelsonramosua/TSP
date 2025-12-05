// LowerBound_HeldKarp.h - Held-Karp's Lagrangian Relaxation Lower Bound header.
//
// Nelson Ramos, 124921.
//
// November, 2025.
//
// You may freely use and change this code, it has no warranty, and it is not necessary to give me credit.

// https://youtu.be/GiDsjIBOVoA?si=zEtD3WFBlUYQO_LN
// Read README.md.

#ifndef _LOWER_BOUND_HELD_KARP_H_
#define _LOWER_BOUND_HELD_KARP_H_

#include "../TravelingSalesmanProblem.h"

// Held-Karp's Lagrangian Relaxation Lower Bound Configuration
#define MAX_ITERS 200          // number of subgradient iterations
#define STEP0 1.0              // initial step size factor
#define FIXED_START_ROOT 0     // fixed start root 

#endif // _LOWER_BOUND_HELD_KARP_H_