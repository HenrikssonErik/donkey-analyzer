


#include "include/root_handler.hpp"
#include <string>
#include <set>
#include <unordered_set>

RootHandler* RootHandler::rootHandler;;

// Constructor implementation
RootHandler* RootHandler::getRootHandlerInstance(int rootListSize, FILE* sketchFile, int preGen, int sketchSize, int maxWindow, int decayInterval, double lambda) {
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

bool RootHandler::updateRoots(Root updateArray[], Root fromArray[], unsigned long edgeTme) {
		
    std::vector<Root> validPairs;  // Store non-zero unique pairs
    //std::vector<RootPair> zeroPairs;   // Store zero pairs
    std::unordered_set<unsigned long> seenRoots; // Track unique root values
    Root oldArray[this->rootListSize];
    std::memcpy(oldArray, updateArray, sizeof(oldArray));

    for (size_t i = 0; i < this->rootListSize; i++) {
        Root pair = fromArray[i];

        if (pair.root == 0) {
            break; //end of list reach if we see a 0 root
        } else if (seenRoots.find(pair.root) == seenRoots.end()) { //to avoid duplicate roots
            if (pair.tme < edgeTme){ //only update edges that has been created after the root was created
                seenRoots.insert(pair.root);  // Mark root as seen
                validPairs.push_back(pair);   // Store unique valid values
            }
        }
    }

    for (size_t i = 0; i < this->rootListSize; i++) {
        Root pair = {updateArray[i]};

        if (pair.root == 0) {
            break; //end of list reach if we see a 0 root
        } else if (seenRoots.find(pair.root) == seenRoots.end()) { //to avoid duplicate roots
            seenRoots.insert(pair.root);  // Mark root as seen
            validPairs.push_back(pair);   // Store unique valid values
        }
    }

    std::sort(validPairs.begin(), validPairs.end(), [](const Root& a, const Root& b) {
        return a.order < b.order;
    });

    size_t index = 0;
    size_t startIndex = std::max(validPairs.size(), static_cast<size_t>(this->rootListSize)) - this->rootListSize;  

    // Add the last ROOTS number of valid pairs (from the end of the list)
    for (size_t i = startIndex; i < validPairs.size(); ++i) {
        updateArray[index++] = validPairs[i];
    }

    // Step 4: Fill the remaining space with zero pairs if there is room, this could be done without the objects by just adding zeros to the array.
    for (size_t i = validPairs.size(); i < this->rootListSize; ++i) {
        updateArray[index++] = {0,0};   // Add zero for root and index
    }

    if (std::memcmp(oldArray, updateArray, sizeof(oldArray)) == 0) {
        //std::cout << "No changes detected.\n";
        return false; // No changes
    } else {
        //std::cout << "Array has changed.\n";
        return true; // Changes detected
    }
}

std::string RootHandler::rootToPrint( uint32_t currentRoot, Root roots[], const char* delimiter = ",") { 
    std::string result;
    bool firstElement = true;
    

    for (size_t i = 0; i < this->rootListSize; i++) {
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

void RootHandler::updateRootOrderAndAddToRoots(Root roots[], uint32_t rootToAdd, unsigned long tme) {
    static std::unordered_map<uint32_t, uint32_t> seenRoots;  // Map to store roots and their corresponding rootOrder
    //std::lock_guard<std::mutex> lock(rootOrderMutex);  // Lock mutex for both operations
    uint32_t rootOrderToAssign = 0;
    if (rootToAdd == 0) {
        //logstream(LOG_INFO) << "Root is zero, skipping it! For vertex: " << rootToAdd << std::endl;
        rootToAdd = 1; //if we encounter a correct id that is 0, we msut use a
        //return;
    }
    // Check if the root already exists in the set, if so use the existing rootOrder
    if (seenRoots.find(rootToAdd) != seenRoots.end()) {
        rootOrderToAssign = seenRoots[rootToAdd];  // Use the existing rootOrder for this root
    } else {
        rootOrderMutex.lock(); //lock rootOrder
        rootOrderToAssign = rootOrder;
        seenRoots[rootToAdd] = rootOrder;  // Store the new root and its rootOrder in the map
        rootOrder++;  // Safely increment rootOrder
        rootOrderMutex.unlock(); //unlock rootOrder
        logstream(LOG_INFO) << "Updated rootOrder: " << rootOrderToAssign << "For vertex: " << rootToAdd << std::endl;
    }
    Root newRoot = {rootToAdd, rootOrderToAssign, tme};
    roots[0] = newRoot;
}

void RootHandler::updateRootsForHist(Root roots[], bool base) {
    histRoot->decay(SFP_Root);
    for (size_t i = 0; i < ROOTS; ++i) {
        // Assuming nl.lb[0] is used in the update call, and rootHash is defined
        Root root = roots[i];
        if (root.root != 0){
            unsigned long rootHash = root.root;  // will convert the value to a unsinged long
            histRoot->update(rootHash, base);
        }else{
            break; //if we encounter a 0 root we have reached the end of line
        }
    }
}

void RootHandler::fixRoots(graphchi_vertex<VertexDataType, EdgeDataType> &vertex, graphchi_context &gcontext){ //take edge list
    bool updatedRoots = false;
    bool updateSelf = false;
    VertexDataType nl = vertex.get_data();

        if(!nl.rootChecked){
            if(isRoot(vertex)){
                updateRootOrderAndAddToRoots(nl.roots, vertex.id(), vertex.get_data().tm[0]);
                updatedRoots = true;
            }
            nl.rootChecked = true;
        }

        for (int i = 0; i < vertex.num_inedges(); i++) {
            graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(i);
            EdgeDataType el = in_edge->get_data();
            if(el.roots[0].root != 0){
                updatedRoots = updatedRoots || updateRoots(nl.roots, el.roots, el.tme[0]);
            }else{
                /*
                if(! gcontext.scheduler->is_scheduled(in_edge->vertex_id())){ //to avoid exessive scheduling
                gcontext.scheduler->add_task(in_edge->vertex_id(), true);
                }*/
            }
        }
    

        if (nl.roots[0].root != 0){ //unnecessary to run update algo on edges if we have no roots
            bool updatedSpecificEdge = false;
            for (int i = 0; i < vertex.num_outedges(); i++) {
                graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(i);
                EdgeDataType el = out_edge->get_data();
                updatedSpecificEdge =  updateRoots(el.roots, nl.roots, el.tme[0]);
                out_edge->set_data(el);
                /*if (updatedRoots && updatedSpecificEdge){ //some edges that hasnt recieved an update might be scheduled nonetheless. Should be ok
                    if(! gcontext.scheduler->is_scheduled(out_edge->vertex_id())){ //to avoid exessive scheduling
                        gcontext.scheduler->add_task(out_edge->vertex_id(), false);
                    }
                }*/
            }
        }

        /*if(updatedRoots){
            logstream(LOG_INFO) << "Roots have been updated! Re-scheduled out-edges! (Vertex " << vertex.id() << ")" << std::endl;
        }*/

            vertex.set_data(nl);
            //rootToPrint(vertex.id(), vertex.get_data().roots, gcontext);
    }

bool RootHandler::isRoot(graphchi_vertex<VertexDataType, EdgeDataType> &vertex){

    unsigned long smallestOut = 0;
    unsigned long smallestIn = 0;

    if (vertex.num_inedges() > 0){
        graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(0);
        EdgeDataType el = in_edge->get_data();
        smallestIn = el.tme[0];
        for (int i = 1; i < vertex.num_inedges(); i++) {
            graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(i);
            EdgeDataType el = in_edge->get_data();
            if (el.tme[0] < smallestIn){
                smallestIn = el.tme[0];
            }
        }
    }

    if (vertex.num_outedges() > 0){
        graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(0);
        EdgeDataType el = out_edge->get_data();
        smallestOut = el.tme[0];
        for (int i = 1; i < vertex.num_outedges(); i++) {
            graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(i);
            EdgeDataType el = out_edge->get_data();
            if (el.tme[0] < smallestOut){
                smallestOut = el.tme[0];
            }
        }
    }
    //should return true if, no inedges, inedge > outedge
    if((smallestOut != 0 && (smallestOut < smallestIn)) || vertex.num_inedges() == 0){
        return true;
     }
    return false;		
}
