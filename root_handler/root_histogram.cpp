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
 */

 #include <fstream>
 #include <math.h>
 #include <random>
 #include <cstdlib>
 #include <string>
 #include "include/root_histogram.hpp"
 
 RootHistogram* RootHistogram::rootHistogram;
 
//Singleton
RootHistogram* RootHistogram::get_instance(FILE* sketchFile, int preGen = 10000, int sketchSize = 50, int maxWindow = 500, int decayInterval = 10, double lambda = 0.02) {
     if (!rootHistogram)
        rootHistogram = new RootHistogram(sketchFile, preGen, sketchSize, maxWindow, decayInterval, lambda);
     return rootHistogram;
 }
 
 RootHistogram::~RootHistogram() {
     delete rootHistogram;
 }
 
 
 /* Sample random values for hashing histogram. */
 /*struct hist_elem HistogramRoot::construct_hist_elem(unsigned long label) {
     struct hist_elem new_elem;
     std::default_random_engine r_generator(label);
     std::default_random_engine c_generator(label / 2);
     std::default_random_engine beta_generator(label);
     for (int i = 0; i < sketchSize; i++) {
         new_elem.r[i] = gamma_dist(r_generator);
         new_elem.beta[i] = uniform_dist(beta_generator);
         new_elem.c[i] = gamma_dist(c_generator);
     }
     gamma_dist.reset();
     return new_elem;
 } */
 
 /* Decay values in the histogram map, and record sketch */
 void RootHistogram::decay() {
     this->histogram_root_map_lock.lock();
     this->t++;
     this->w++;
     /* Decay only when t == DECAY. */
     if (this->t >= this->decayInterval) {
         std::map<unsigned long, double>::iterator it;
     /* Decay histogram values. */
         for (it = this->histogram_map.begin(); it != this->histogram_map.end(); it++)
             it->second *= this->powerful;
     /* Decay sketch values. */
         for (int i = 0; i < sketchSize; i++)
             this->hash[i] *= this->powerful;
         this->t = 0;  /* Reset the timer. */
     }
     /* Record sketch only when t == WINDOW if we use
      * WINDOW as frequency to generate sketches. */
     if (this->w >= maxWindow) {
        record_sketch(this->root_sketch);
        this->w = 0; /* Reset the timer. */
     }
     this->histogram_root_map_lock.unlock();
 }
 /* Update Root Histogram with new label
    Will not decay or  create sketch
    Set update_has = false to avoid updating hashvalues. Usefull if the histogram should be prefilled with values*/
 void RootHistogram::update(unsigned long label, bool update_hash=true) {
     std::pair<std::map<unsigned long, double>::iterator, bool> root_iterator;
     double counter = 1;
     this->histogram_root_map_lock.lock();
     root_iterator = this->histogram_map.insert(std::pair<unsigned long, double>(label, counter));
     if (root_iterator.second == false) {
         (root_iterator.first)->second++;
     }
     /* Update the hash if needed. */

     if(update_hash){
        srand(label);
        int betaPos = rand() % preGen;
        int gammaPos = rand() % preGen;
        for (int i = 0; i < sketchSize; i++) {
                /* Compute the new hash value using picked random variables. */
                double histValue = (root_iterator.first)->second;
                double c = this->gamma_param[gammaPos][i];
                double rb = this->r_beta_param[betaPos][i];
                double pwr_r = this->power_r[betaPos][i];
                double y = histValue / rb;
                double hashValue = this->gamma_param[gammaPos][i] / (y * pwr_r);
                /* If the hash is smaller than the existing value,
            * we replace the hash value and change the sketch value. */
            if (hashValue < this->hash[i]) {
                    this->hash[i] = hashValue;
            this->root_sketch[i] = (root_iterator.first)->first;
            }
        }
    }
     this->histogram_root_map_lock.unlock();
     return;
 }
 
 /* Create and initialize  a sketch.
  * This function should only be called once during.  */
 void RootHistogram::create_sketch() {
     /* Sample variables. */
     srand(36); /* Set a seed. */
     for (unsigned long i = 0; i < preGen; i++) {
         int randomized_i = rand();
     std::default_random_engine r_generator(randomized_i);
     std::default_random_engine beta_generator(randomized_i);
 
     for (int j = 0; j < sketchSize; j++) {
         this->gamma_param[i][j] = gamma_dist(r_generator);
         double uniform_param = uniform_dist(beta_generator);
         this->r_beta_param[i][j] = pow(M_E, this->gamma_param[i][j] * uniform_param);
         this->power_r[i][j] = pow(M_E, this->gamma_param[i][j]);
     }
     gamma_dist.reset();
     }
     this->histogram_root_map_lock.lock();
     /* Build sketch. */
     for (int i = 0; i < sketchSize; i++) {
         std::map<unsigned long, double>::iterator iterator = this->histogram_map.begin();
     unsigned long label = iterator->first;
 
     srand(label);
     int betaPos = rand() % preGen;
     int gammaPos = rand() % preGen;
    
     double rb = this->r_beta_param[betaPos][i];
     double c = this->gamma_param[gammaPos][i];

     double y = iterator->second / rb;
     double pw_r = this->power_r[betaPos][i];
     double a_i = c / (y * pw_r);
     unsigned long s_i = iterator->first;
     for (iterator = this->histogram_map.begin(); iterator != this->histogram_map.end(); iterator++) {
             label = iterator->first;
 
         srand(label);
         betaPos = rand() % preGen;
         gammaPos = rand() % preGen;
         rb = this->r_beta_param[betaPos][i];
         c = this->gamma_param[gammaPos][i];
         y = iterator->second / rb;
         pw_r = this->power_r[betaPos][i];
 
         y = iterator->second / rb;
         double a = c / (y * pw_r);
         if (a < a_i) {
                 a_i = a;
         s_i = iterator->first;
         }
     }
     this->root_sketch[i] = s_i;
     this->hash[i] = a_i;
     }
 
     this->histogram_root_map_lock.unlock();
     return;
 }
 
 //Record to file
 void RootHistogram::record_sketch(std::vector<unsigned long> sketch) {

    this->histogram_root_file_lock.lock();
    for (int i = 0; i < sketchSize; i++) {
        fprintf(this->sketch_file_pointer, "%lu ", sketch[i]);
    }
    fprintf(this->sketch_file_pointer, "\n");
    this-> histogram_root_file_lock.unlock();
}

 

 