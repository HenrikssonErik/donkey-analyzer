#include <cstdint>
#include <unordered_set>
#include <algorithm>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <vector>
#include <sstream>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <gtest/gtest.h>
#include <include/def.hpp>

static int ROOTS = 5;

void fixRoots(graphchi_vertex<VertexDataType, EdgeDataType> &vertex, graphchi_context &gcontext){
    VertexDataType nl = vertex.get_data();

    if(vertex.id() == 9130 || vertex.id() == 9131){
        rootBreak();
    }

    unsigned long in_edges_ts[vertex.num_inedges()];
    unsigned long out_edges_ts[vertex.num_outedges()];
    
    if(!nl.nodeInfo.checkedIfRoot){
        //initiate node info variables

        int minIn = std::numeric_limits<unsigned long>::max();

        for (int i = 0; i < vertex.num_inedges(); i++) {
            graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(i);
            EdgeDataType el = in_edge->get_data();
            in_edges_ts[i] = el.tme[0];
            if(minIn > el.tme[0]){
                minIn = el.tme[0];
            }
        }

        int minOut = std::numeric_limits<unsigned long>::max();

        for (int i = 0; i < vertex.num_outedges(); i++) {
            graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(i);
            EdgeDataType el = out_edge->get_data();
            out_edges_ts[i] = el.tme[0];
            if(minOut > el.tme[0]){
                minOut = el.tme[0];
            }
        }
        nl.nodeInfo.node_ts = std::min(minIn, minOut);
        nl.nodeInfo.vertex_id = vertex.id();

        root_handler->checkAndAssignRoot(nl.nodeInfo, nl.roots,in_edges_ts , vertex.num_inedges(), out_edges_ts, vertex.num_outedges());
        vertex.set_data(nl);
    }

        for (int i = 0; i < vertex.num_inedges(); i++) {
            graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(i);
            EdgeDataType el = in_edge->get_data();
            root_handler->updateRootsFromInEdges(nl.nodeInfo, el.roots, nl.roots);
        }
    

        if (nl.roots[0].root != 0){ //unnecessary to run update algo on edges if we have no roots
            for (int i = 0; i < vertex.num_outedges(); i++) {
                graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(i);
                EdgeDataType el = out_edge->get_data();
                root_handler->updateOutedgeFromNode(el.tme[0],el.roots, nl.roots);
                out_edge->set_data(el);
            }
        }
        vertex.set_data(nl);
    }


int main(int argc, char **argv) {
    //uint32_t updateArray[ROOTS * 2] = {0};
    RootPair updateArray[ROOTS] = {{6000, 3}, {2000, 5}, {3000, 6}, {4000, 9}, {5000, 10}}; // New roots with increasing order
    RootPair fromArray[ROOTS] = {{7000, 4}, {1000, 1}, {8000, 2}, {9000, 8}, {10000, 7}};
    //uint32_t fromArray[ROOTS * 2] = {1000, 1, 2000, 2, 3000, 4, 4000, 7, 5000, 8}; // New roots with increasing order

    updateRoots(updateArray, fromArray);

    std::cout << "Updated roots: ";
    for (size_t i = 0; i < ROOTS; i++) {
        std::cout << updateArray[i].root << ":" << updateArray[i].order << " ";
    }
    std::cout << std::endl;

    return 0;
}