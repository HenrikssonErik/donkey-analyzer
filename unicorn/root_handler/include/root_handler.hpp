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

    RootHandler(int rootListSize, FILE* sketchFile, int preGen, int sketchSize, int maxWindow, int decayInterval, float lambda)
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

    void updateRootOrderAndAddToRoots(Root roots[], uint32_t rootToAdd, unsigned long tme);

    bool isRoot(unsigned long in_edges_ts[], size_t in_edges_size, unsigned long out_edges_ts[], size_t out_edges_size);

public:
    bool updateRoots(Root updateArray[], Root fromArray[], unsigned long edgeTme);
    
    void updateRootsForHist(Root roots[], bool base, bool decay);

    static RootHandler* getRootHandlerInstance(int rootListSize, FILE* sketchFile, int preGen, int sketchSize, int maxWindow, int decayInterval, float lambda);

    static RootHandler* getRootHandlerInstance(int rootListSize, FILE* sketchFile);

    static RootHandler* getRootHandlerInstance();

    std::string rootToPrint( uint32_t currentRoot, Root roots[], const char* delimiter = ",");

    void checkAndAssignRoot(NodeInfo& nodeInfo, Root nodeRoots[], unsigned long in_edges_ts[], size_t in_edges_size, unsigned long out_edges_ts[], size_t out_edges_size);

    void updateRootsFromInEdges(NodeInfo current_node_info, Root inedge_roots[],Root node_roots[]);

    void updateOutedgeFromNode(unsigned long edge_ts, Root outedge_roots[],Root node_roots[]);

    void createSketch();

    void recordSketch();

    std::string printHistogram(){
        RootHistogram* rootHistogram = RootHistogram::get_instance();
        return rootHistogram->print_histogram();
    }

    
std::string sketchToString(const std::vector<unsigned long>& vec) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i];
        if (i != vec.size() - 1) {
            oss << " "; // Separate elements with a space
        }
    }
    return oss.str();
}

std::string printSketch() {
    RootHistogram* rootHistogram = RootHistogram::get_instance();
    unsigned long* sketch = rootHistogram->get_sketch_copy();
    int sketch_size = rootHistogram->get_sketch_size();
    std::ostringstream oss;
    for (size_t i = 0; i < sketch_size; ++i) {
        oss << sketch[i];
        if (i != sketch_size - 1) {
            oss << ","; // Separate elements
        }
    }
    return oss.str();
}

};

#include "../root_handler.cpp"
#endif 
