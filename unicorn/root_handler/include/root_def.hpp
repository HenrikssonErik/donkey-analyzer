
#ifndef __ROOT_DEF_HPP__
#define __ROOT_DEF_HPP__

#include <random>

/*
struct hist_elem {
    double r[SKETCH_SIZE];
    double beta[SKETCH_SIZE];
    double c[SKETCH_SIZE]; 
};
*/

struct NodeInfo {
    uint32_t vertex_id = 0;
    bool checkedIfRoot = false;
    unsigned long node_ts = 0;
};

struct Root {
	uint32_t root = 0;
	uint32_t order = 0;
    unsigned long tme = 0;
};

/* Distribution used in locality-sensitive hashing. */
std::gamma_distribution<double> root_gamma_dist(2.0, 1.0);
std::uniform_real_distribution<double> root_uniform_dist(0.0, 1.0);

#endif