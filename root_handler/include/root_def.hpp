
#include <random>

/*
struct hist_elem {
    double r[SKETCH_SIZE];
    double beta[SKETCH_SIZE];
    double c[SKETCH_SIZE]; 
};
*/

struct Root {
	uint32_t root;
	uint32_t order;
    unsigned long tme;
};

/* Distribution used in locality-sensitive hashing. */
std::gamma_distribution<double> gamma_dist(2.0, 1.0);
std::uniform_real_distribution<double> uniform_dist(0.0, 1.0);