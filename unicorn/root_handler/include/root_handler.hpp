#ifndef ROOTHANDLER  // Include guard to prevent multiple inclusions
#define ROOTHANDLER

#include "root_def.hpp"
#include "root_histogram.hpp"
#include <string>
#include <vector>

class RootHandler {
private:
    static RootHandler* rootHandler;
    int rootListSize;

    RootHandler(int rootListSize, FILE* sketchFile, int preGen, int sketchSize, int maxWindow, int decayInterval, double lambda)
        :rootListSize(rootListSize){
        //Initiate the histogram singleton
        RootHistogram::get_instance(sketchFile, preGen, sketchSize, maxWindow, decayInterval, lambda);
    }

    RootHandler(int rootListSize, FILE* sketchFile)
        :rootListSize(rootListSize){
         //Initiate the histogram singleton
        RootHistogram::get_instance(sketchFile);
    }

    //Private helper methods
    bool updateRoots(Root updateArray[], Root fromArray[], unsigned long edgeTme);

    void updateRootOrderAndAddToRoots(Root roots[], uint32_t rootToAdd, unsigned long tme);

    bool isRoot(unsigned long in_edges_ts[], size_t in_edges_size, unsigned long out_edges_ts[], size_t out_edges_size);

public:
    
    void updateRootsForHist(Root roots[], bool base);

    static RootHandler* getRootHandlerInstance(int rootListSize, FILE* sketchFile, int preGen, int sketchSize, int maxWindow, int decayInterval, double lambda);

    static RootHandler* getRootHandlerInstance(int rootListSize, FILE* sketchFile);

    static RootHandler* getRootHandlerInstance();

    std::string rootToPrint( uint32_t currentRoot, Root roots[], const char* delimiter = ",");

    void checkAndAssignRoot(NodeInfo& nodeInfo, Root nodeRoots[], unsigned long in_edges_ts[], size_t in_edges_size, unsigned long out_edges_ts[], size_t out_edges_size);

    void updateRootsFromInEdges(NodeInfo current_node_info, Root inedge_roots[],Root node_roots[]);

    void updateOutedgeFromNode(unsigned long edge_ts, Root outedge_roots[],Root node_roots[]);

    void createSketch(){
        RootHistogram* rootHistogram = RootHistogram::get_instance();
        rootHistogram->create_sketch();
    }

    void recordSketch(){
        RootHistogram* rootHistogram = RootHistogram::get_instance();
        std::vector<unsigned long> sketch_copy = rootHistogram->get_sketch_copy();
        rootHistogram->record_sketch(sketch_copy);
    }

};

#include "../root_handler.cpp"
#endif 
