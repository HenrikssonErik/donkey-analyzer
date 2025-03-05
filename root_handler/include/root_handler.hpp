#ifndef ROOTHANDLER  // Include guard to prevent multiple inclusions
#define ROOTHANDLER

#include "root_def.hpp"
#include "root_histogram.hpp"
#include <string>

class RootHandler {
private:
    static RootHandler* rootHandler;
    static RootHistogram* rootHistogram;
    int rootListSize;

    RootHandler(int rootListSize, FILE* sketchFile, int preGen, int sketchSize, int maxWindow, int decayInterval, double lambda)
        :rootListSize(rootListSize){
        rootHistogram = RootHistogram::get_instance(sketchFile, preGen, sketchSize, maxWindow, decayInterval, lambda);
    }

    RootHandler(int rootListSize, FILE* sketchFile)
        :rootListSize(rootListSize){
        rootHistogram = RootHistogram::get_instance(sketchFile);
    }

    //Private helper methods
    bool updateRoots(Root updateArray[], Root fromArray[], unsigned long edgeTme);

    void updateRootOrderAndAddToRoots(Root roots[], uint32_t rootToAdd, unsigned long tme);

    //TODO: make private
    void updateRootsForHist(Root roots[], bool base);

    //TODO: make private
    bool isRoot(graphchi_vertex<VertexDataType, EdgeDataType> &vertex);

public:
    // Constructor, use singleton, take variables needed for histogram, overload getInstance method: one for creating/getting and one getting/null

    
    static RootHandler* getRootHandlerInstance(int rootListSize, FILE* sketchFile, int preGen, int sketchSize, int maxWindow, int decayInterval, double lambda);

    static RootHandler* getRootHandlerInstance(int rootListSize, FILE* sketchFile);

    std::string rootToPrint( uint32_t currentRoot, Root roots[], const char* delimiter = ",");


    //TODO: rename
    void fixRoots(graphchi_vertex<VertexDataType, EdgeDataType> &vertex, graphchi_context &gcontext);

};

#endif 
