/*
 *
 * Author: Xueyuan Han <hanx@g.harvard.edu>
 *
 * Copyright (C) 2018-2020 Harvard University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 * 
 * ###################################################################
 * 
 * Adapted code from the Histogram.cpp in the Unicorn-Analyzer system.
 * Original Author: Xueyuan Han <hanx@g.harvard.edu>
 * Author of modifications: Sebastian Kvaldén <kvalden@chalmers.se>, Erik Henriksson <erhen@chalmers.se>
 * 
 * original Code: https://github.com/crimson-unicorn/analyzer
 * 
 * ###################################################################
 *
 */

#ifndef __ROOTHISTOGRAM_HPP__
#define __ROOTHISTOGRAM_HPP__

#include <iostream>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <math.h>
#include "root_def.hpp"
#include <set>

/* We use singleton design to create a single instance of a histogram.
 * This is not thread-safe. A proper locking mechanism is needed.
 * Current implementation uses an ordered Map as the histogram. */
class RootHistogram {
public:
    static RootHistogram* get_instance(FILE* sketchFile, int preGen = 10000, int sketchSize = 100, int maxWindow = 500, int decayInterval = 10, double lambda = 0.02);
    static RootHistogram* get_instance();
    ~RootHistogram();
    //struct hist_elem construct_hist_elem(unsigned long label);
    void decay();
    void update(unsigned long label, bool update_hash);
    void create_sketch();
    void record_sketch(std::vector<unsigned long> sketch);
    std::vector<unsigned long> get_sketch_copy();
    std::string print_histogram();

private:
    static RootHistogram* rootHistogram;

    RootHistogram(FILE* sketchFile, int preGen, int sketchSize, int maxWindow, int decayInterval, double lambda)
        : preGen(preGen), sketchSize(sketchSize), maxWindow(maxWindow), sketch_file_pointer(sketchFile), decayInterval(decayInterval), lambda(lambda) {
        this->t = 0;
        this->w = 0;
        this->powerful = pow(M_E, -lambda);
        
        // Initialize object length with zeros
        root_sketch = std::vector<unsigned long>(sketchSize, 0);
        hash = std::vector<double>(sketchSize, 0.0); 

        gamma_param = std::vector<std::vector<double>>(preGen, std::vector<double>(sketchSize, 0.0));
        r_beta_param = std::vector<std::vector<double>>(preGen, std::vector<double>(sketchSize, 0.0));
        power_r = std::vector<std::vector<double>>(preGen, std::vector<double>(sketchSize, 0.0));
    
    }

    std::map<unsigned long, double> histogram_map;
    std::vector<unsigned long> root_sketch;    // Dynamic size based on sketchSize
    std::vector<double> hash; 
    double powerful;
    int preGen; //Decide how many hash values to pre-generate
    int sketchSize; //Decide how many values each sketch should contain
    int maxWindow; //Decide how often sketches should be recorded
    int decayInterval; //Decide how much the histogram should decay with
    double lambda; // Lambda factor used during decay
    FILE* sketch_file_pointer;

    /* Pregenerated samples used for hasing in Sketch creation */
    std::vector<std::vector<double>> gamma_param;   // Vector of vectors of size preGen x sketchSize
    std::vector<std::vector<double>> r_beta_param;  // Vector of vectors of size preGen x sketchSize
    std::vector<std::vector<double>> power_r;

    int t; /* If t reaches decayIntervall, hashes is decayed. */
    int w; /* window (w) decides when a sketch should be created */
    
    /* The lock to update histogram map. */
    std::mutex histogram_root_map_lock;
    /* The lock to modify file. */
    std::mutex histogram_root_file_lock;
};

#include "../root_histogram.cpp"
#endif /* __HISTOGRAMROOT_HPP__ */
