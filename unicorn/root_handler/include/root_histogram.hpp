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
    static RootHistogram* get_instance(FILE* sketchFile, int preGen = 10000, int sketchSize = 100, int maxWindow = 50, int decayInterval = 10, float lambda = 0.02);
    static RootHistogram* get_instance();
    ~RootHistogram();
    struct hist_elem construct_hist_elem(unsigned long label);
    void decay();
    void update(unsigned long label, bool update_hash);
    void create_root_sketch();
    void record_sketch(unsigned long* sketch);
    unsigned long* get_sketch_copy();
    std::string print_histogram();
    int get_sketch_size();

private:
    static RootHistogram* rootHistogram;
    void record_sketch_internal_nolock(unsigned long sketch[]);

    RootHistogram(FILE* sketchFile, int preGen, int sketchSize, int maxWindow, int decayInterval, float lambda)
        : preGen(preGen), sketchSize(sketchSize), maxWindow(maxWindow), sketch_file_pointer(sketchFile), decayInterval(decayInterval), lambda(lambda) {
        
        this->t = 0;
        this->w = 0;
        this->powerful = pow(M_E, -this->lambda);
        
        // Initialize object length with zeros
        //root_sketch = std::vector<unsigned long>(sketchSize);
        //hash = std::vector<double>(sketchSize);

        root_sketch = new unsigned long [this->sketchSize];
        hash = new double[this->sketchSize];
        

        //gamma_param = std::vector<std::vector<double>>(preGen, std::vector<double>(sketchSize, 0.0));
        //r_beta_param = std::vector<std::vector<double>>(preGen, std::vector<double>(sketchSize, 0.0));
        //power_r = std::vector<std::vector<double>>(preGen, std::vector<double>(sketchSize, 0.0));
        
        gamma_param = new double*[this-> preGen];
        r_beta_param = new double*[this-> preGen];
        power_r = new double*[this-> preGen];
        for (int i = 0; i < preGen; ++i) {
            gamma_param[i] = new double[this->sketchSize];
            r_beta_param[i] = new double[this->sketchSize];
            power_r[i] = new double[this->sketchSize];
        }

    }

    std::map<unsigned long, double> histogram_map;
    //std::vector<unsigned long> root_sketch;    // Dynamic size based on sketchSize
    //std::vector<double> hash;
    unsigned long* root_sketch;
    double* hash;

    double powerful;
    int preGen; //Decide how many hash values to pre-generate
    int sketchSize; //Decide how many values each sketch should contain
    int maxWindow; //Decide how often sketches should be recorded
    int decayInterval; //Decide how much the histogram should decay with
    float lambda; // Lambda factor used during decay
    FILE* sketch_file_pointer;
    bool sketch_initialized = false;

    /* Pregenerated samples used for hasing in Sketch creation */
    //std::vector<std::vector<double>> gamma_param;   // Vector of vectors of size preGen x sketchSize
    //std::vector<std::vector<double>> r_beta_param;  // Vector of vectors of size preGen x sketchSize
    //std::vector<std::vector<double>> power_r;
    double** gamma_param;
    double** r_beta_param;
    double** power_r;

    int t; /* If t reaches decayIntervall, hashes is decayed. */
    int w; /* window (w) decides when a sketch should be created */

    /*struct hist_elem {
        double* r;
        double* beta;
        double* c; 
    };*/
    
    /* The lock to update histogram map. */
    std::mutex histogram_root_map_lock;
    /* The lock to modify file. */
    std::mutex histogram_root_file_lock;
};

#include "../root_histogram.cpp"
#endif /* __HISTOGRAMROOT_HPP__ */
