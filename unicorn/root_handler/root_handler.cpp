


#include "include/root_handler.hpp"
#include "include/root_histogram.hpp"
#include <string>
#include <set>
#include <unordered_set>
#include <cstring>
#include <algorithm>
#include <unordered_map>


RootHandler* RootHandler::rootHandler;
std::mutex rootOrderMutex;

// Constructor implementation
RootHandler* RootHandler::getRootHandlerInstance(int rootListSize, FILE* sketchFile, int preGen, int sketchSize, int maxWindow, int decayInterval, float lambda) {
    if (!rootHandler)
        rootHandler = new RootHandler(rootListSize, sketchFile, preGen, sketchSize, maxWindow, decayInterval, lambda);
    return rootHandler;
}

//Initiates with defualt values
RootHandler* RootHandler::getRootHandlerInstance(int rootListSize, FILE* sketchFile) {
    if (!rootHandler)
        rootHandler = new RootHandler(rootListSize, sketchFile);
    return rootHandler;
}

RootHandler* RootHandler::getRootHandlerInstance() {
    if (!rootHandler) {
        throw std::runtime_error("RootHandler instance does not exist.");
    }
    return rootHandler;
}

bool RootHandler::updateRoots(Root updateArray[], Root fromArray[], unsigned long compare_ts) {

    std::vector<Root> validPairs;  // Store non-zero unique pairs
    //std::vector<RootPair> zeroPairs;   // Store zero pairs
    std::unordered_set<unsigned long> seenRoots; // Track unique root values
    Root oldArray[this->rootListSize];
    std::memcpy(oldArray, updateArray, sizeof(oldArray));

    for (int i = 0; i < this->rootListSize; i++) {
        Root pair = fromArray[i];

        if (pair.root == 0) {
            break; //end of list reach if we see a 0 root
        } else if (seenRoots.find(pair.root) == seenRoots.end()) { //to avoid duplicate roots
            if (pair.tme <= compare_ts){ 
                seenRoots.insert(pair.root);  // Mark root as seen
                validPairs.push_back(pair);   // Store unique valid values
            }
        }
    }

    for (int i = 0; i < this->rootListSize; i++) {
        Root pair = {updateArray[i]};

        if (pair.root == 0) {
            break; //end of list reach if we see a 0 root
        } else if (seenRoots.find(pair.root) == seenRoots.end()) { //to avoid duplicate roots
            if (pair.tme <= compare_ts){ 
                seenRoots.insert(pair.root);  // Mark root as seen
                validPairs.push_back(pair);   // Store unique valid values
            }
        }
    }

    std::sort(validPairs.begin(), validPairs.end(), [](const Root& a, const Root& b) {
        return a.tme < b.tme;
    });

    int index = 0;
    size_t startIndex = std::max(validPairs.size(), static_cast<size_t>(this->rootListSize)) - static_cast<size_t>(this->rootListSize);  

    // Add the last ROOTS number of valid pairs (from the end of the list)
    for (size_t i = startIndex; i < validPairs.size(); ++i) {
        updateArray[index++] = validPairs[i];
    }

    // Fill the remaining space with zero pairs if there is room, this could be done without the objects by just adding zeros to the array.
    for (int i = validPairs.size(); i < this->rootListSize; ++i) {
        updateArray[index++] = {};   // Add zero for root and index
    }

    if (std::memcmp(oldArray, updateArray, sizeof(oldArray)) == 0) {
        //std::cout << "No changes detected.\n";
        return false; // No changes
    } else {
        //std::cout << "Array has changed.\n";
        return true; // Changes detected
    }
}

std::string RootHandler::rootToPrint( uint32_t currentRoot, Root roots[], const char* delimiter) { 
    std::string result;
    bool firstElement = true;
    

    for (int i = 0; i < this->rootListSize; i++) {
        if (roots[i].root == 0){
            break;
        }  // Skip zero values

        if (!firstElement) {
            result += delimiter;  // Add delimiter **only after the first element**
        } else {
            firstElement = false;  // Mark that the first element has been added
        }

        result += std::to_string(roots[i].root) + ":" + std::to_string(roots[i].order); //For nice printing
    }
    return result;
}

/*tme = node creation time*/
void RootHandler::updateRootOrderAndAddToRoots(Root roots[], uint32_t rootToAdd, unsigned long tme) {
    static uint32_t rootOrder = 1;
    static std::unordered_map<uint32_t, uint32_t> seenRoots;  // Map to store roots and their corresponding rootOrder

    uint32_t rootOrderToAssign = 0;
    if (rootToAdd == 0) {
        rootToAdd = 1; //if we encounter a correct id that is 0, we msut use a
        //return;
    }
    // Check if the root already exists in the set, if so use the existing rootOrder
    if (seenRoots.find(rootToAdd) != seenRoots.end()) {
        rootOrderToAssign = seenRoots[rootToAdd];  // Use the existing rootOrder for this root
    }else {
        rootOrderMutex.lock(); //lock rootOrder
        rootOrderToAssign = rootOrder;
        seenRoots[rootToAdd] = rootOrder;  // Store the new root and its rootOrder in the map
        rootOrder++;  // Safely increment rootOrder
        rootOrderMutex.unlock(); //unlock rootOrder
    }
    Root newRoot = {};
    newRoot.root = rootToAdd;
    newRoot.order = rootOrderToAssign;
    newRoot.tme = tme;
    roots[0] = newRoot;
}

void RootHandler::updateRootsForHist(Root roots[], bool update_hash, bool decay) {
    RootHistogram* rootHistogram = RootHistogram::get_instance();
    if(decay){
        rootHistogram->decay();
    }
    for (int i = 0; i < this->rootListSize; ++i) {
        // Assuming nl.lb[0] is used in the update call, and rootHash is defined
        Root root = roots[i];
        if (root.root != 0){
            //unsigned long rootHash = root.root;  // convert the value to a unsinged long
            unsigned long rootHash = static_cast<unsigned long>(root.root); 
            rootHistogram->update(rootHash, update_hash);
        }else{
            break; //if we encounter a 0 root we have reached the end of line
        }
    }
}

//Public
void RootHandler::checkAndAssignRoot(NodeInfo& nodeInfo, Root nodeRoots[], unsigned long in_edges_ts[], size_t in_edges_size, unsigned long out_edges_ts[], size_t out_edges_size){
    if(!nodeInfo.checkedIfRoot){
        if(this->isRoot(in_edges_ts, in_edges_size, out_edges_ts, out_edges_size)){
            updateRootOrderAndAddToRoots(nodeRoots, nodeInfo.vertex_id, nodeInfo.node_ts);
        }
        nodeInfo.checkedIfRoot = true;
    }
}

bool RootHandler::isRoot(unsigned long in_edges_ts[], size_t in_edges_size, unsigned long out_edges_ts[], size_t out_edges_size){

    unsigned long smallestOut = std::numeric_limits<unsigned long>::max();;
    unsigned long smallestIn = std::numeric_limits<unsigned long>::max();

    if (in_edges_size > 0){
        smallestIn = in_edges_ts[0];
        for (int i = 1; i < in_edges_size; i++) {
            if (in_edges_ts[i] < smallestIn){
                smallestIn = in_edges_ts[i];
            }
        }
    }

    if (out_edges_size > 0){
        smallestOut = out_edges_ts[0];
        for (int i = 1; i < out_edges_size; i++) {
            if (out_edges_ts[i] < smallestOut){
                smallestOut = out_edges_ts[i];
            }
        }
    }
    //should return true if, no inedges, inedge > outedge
    if( (smallestOut <= smallestIn) || in_edges_size == 0){
        return true;
     }
    return false;		
}

/*Takes the node roots and rotos from one inedge
Updates the root nodes depending on */
void RootHandler::updateRootsFromInEdges(NodeInfo current_node_info, Root inedge_roots[],Root node_roots[]){
    if(inedge_roots[0].root != 0){
        this->updateRoots(node_roots, inedge_roots, std::numeric_limits<unsigned long>::max()); //we use max as ts since we want to take input from all incoming edges
    }
}

/*Takes the node roots and rotos from one inedge
Updates the root nodes depending on */
void RootHandler::updateOutedgeFromNode(unsigned long edge_ts, Root outedge_roots[],Root node_roots[]){
    if(node_roots[0].root != 0){
        this->updateRoots(outedge_roots, node_roots, edge_ts); //here we dont use max to only push roots to nodes that appeared after it was created
    }
}

void RootHandler::createSketch(){
    RootHistogram* rootHistogram = RootHistogram::get_instance();
    rootHistogram->create_root_sketch();
}

void RootHandler::recordSketch(){
    RootHistogram* rootHistogram = RootHistogram::get_instance();
    unsigned long* sketch_copy = rootHistogram->get_sketch_copy();
    rootHistogram->record_sketch(sketch_copy);
}