
#ifndef __ROOT_DEF_HPP__
#define __ROOT_DEF_HPP__

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

#endif