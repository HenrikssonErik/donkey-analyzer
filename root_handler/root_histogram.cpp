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
 
 /* Decay values in the histogram map, and record the sketch to the 
  * file @fp, if WINDOW updates have performed (if WINDOW is used). */
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
        record_sketch();
        this->w = 0; /* Reset the timer. */
     }
     this->histogram_root_map_lock.unlock();
 }
 /* Insert @label to the histogram if it does not exist; otherwise, update its value.
  * If @base true, we do not update hash value; we only update them during streaming.
  * We do not decay the histogram or the sketch in this function. */
 void RootHistogram::update(unsigned long label, bool update_hash=true) {
 
     this->histogram_root_map_lock.lock();
     /* We add the new element or update the existing element in the
      * histogram. This is done both in base and stream graph. */
     std::pair<std::map<unsigned long, double>::iterator, bool> rst;
     double counter = 1;
     rst = this->histogram_map.insert(std::pair<unsigned long, double>(label, counter));
     if (rst.second == false) {
 
         (rst.first)->second++;
     }
     /* Update the hash if needed. */

     if(update_hash){
        srand(label);
        int pos1 = rand() % preGen;
        int pos2 = rand() % preGen;
        for (int i = 0; i < sketchSize; i++) {
                /* Compute the new hash value using picked random variables. */
                double c = this->gamma_param[pos2][i];
                double y = (rst.first)->second / this->r_beta_param[pos1][i];
                double a = c / (y * this->power_r[pos1][i]);
                /* If the hash value is smaller than the existing value,
            * we replace the hash value and change the sketch value. */
            if (a < this->hash[i]) {
                    this->hash[i] = a;
            this->sketch[i] = (rst.first)->first;
            }
        }
    }
     this->histogram_root_map_lock.unlock();
     return;
 }
 
 /* Create and initialize  a sketch.
  * This function shoudl only be called once during.  */
 void RootHistogram::create_sketch() {
     this->histogram_root_map_lock.lock();
     /* Pre-sample random variables. */
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
     /* Initialize sketch. */
     for (int i = 0; i < sketchSize; i++) {
         std::map<unsigned long, double>::iterator histoit = this->histogram_map.begin();
     unsigned long label = histoit->first;
 
     srand(label);
     int pos1 = rand() % preGen; /* For gamma and uniform distribution. */
     int pos2 = rand() % preGen; /* For the other gamma distribution. */
 
     double y = histoit->second / this->r_beta_param[pos1][i];
     double a_i = this->gamma_param[pos2][i] / (y * this->power_r[pos1][i]);
     unsigned long s_i = histoit->first;
     for (histoit = this->histogram_map.begin(); histoit != this->histogram_map.end(); histoit++) {
             label = histoit->first;
 
         srand(label);
         pos1 = rand() % PREGEN;
         pos2 = rand() % PREGEN;
 
         y = histoit->second / this->r_beta_param[pos1][i];
         double a = this->gamma_param[pos2][i] / (y * this->power_r[pos1][i]);
         if (a < a_i) {
                 a_i = a;
         s_i = histoit->first;
         }
     }
     this->sketch[i] = s_i;
     this->hash[i] = a_i;
     }
 
     this->histogram_root_map_lock.unlock();
     return;
 }
 
 /* Write the sketch to the file @fp. */
 void RootHistogram::record_sketch() {
     this->histogram_root_map_lock.lock();
     for (int i = 0; i < sketchSize; i++) {
         fprintf(this->sketch_file_pointer,"%lu ", this->sketch[i]);
     }
     fprintf(this->sketch_file_pointer, "\n");
     this->histogram_root_map_lock.unlock();
     return;
 }
 

 