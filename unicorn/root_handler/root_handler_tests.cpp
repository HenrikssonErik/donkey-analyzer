#include "include/root_handler.hpp"
#include <gtest/gtest.h>
#include <vector>

class RootHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        int rootListSize = 10;
        rootHandler = RootHandler::getRootHandlerInstance(rootListSize, nullptr);
    }

    RootHandler* rootHandler;
};

TEST_F(RootHandlerTest, SingletonInstance) {
    int rootListSize = 10;
    RootHandler* instance1 = RootHandler::getRootHandlerInstance(rootListSize, nullptr);
    RootHandler* instance2 = RootHandler::getRootHandlerInstance();
    EXPECT_EQ(instance1, instance2);
}

TEST_F(RootHandlerTest, UpdateRoots_NoChange) {
    if (!rootHandler) {
        std::cerr << "rootHandler is null!" << std::endl;
        return;
    }

    Root updateArray[10] = {};
    Root fromArray[10] = {};

    // Make copies of the original arrays
    Root originalUpdateArray[10];
    Root originalFromArray[10];
    std::memcpy(originalUpdateArray, updateArray, sizeof(updateArray));
    std::memcpy(originalFromArray, fromArray, sizeof(fromArray));

    // Call the updateRoots function
    bool updated = rootHandler->updateRoots(updateArray, fromArray, 100);
    EXPECT_FALSE(updated);

    // Verify that the arrays are the same as their original copies
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(updateArray[i].root, originalUpdateArray[i].root) << "updateArray changed at index " << i;
        EXPECT_EQ(fromArray[i].root, originalFromArray[i].root) << "fromArray changed at index " << i;
    }
}

TEST_F(RootHandlerTest, UpdateRoots_ValidUpdate) {
    Root updateArray[10] = {{1, 1, 50}, {2, 2, 60}};
    Root fromArray[10] = {{3, 3, 40}, {4, 4, 70}};
    EXPECT_TRUE(rootHandler->updateRoots(updateArray, fromArray, 100));

    EXPECT_EQ(updateArray[0].root, fromArray[0].root);
    EXPECT_EQ(updateArray[1].root, 1);
    EXPECT_EQ(updateArray[2].root, 2);
    EXPECT_EQ(updateArray[3].root, fromArray[1].root);
    EXPECT_EQ(updateArray[4].root, 0);

    EXPECT_EQ(fromArray[0].root, 3);
    EXPECT_EQ(fromArray[1].root, 4);
    EXPECT_EQ(fromArray[2].root, 0);
    EXPECT_EQ(fromArray[3].root, 0);
}

TEST_F(RootHandlerTest, UpdateRoots_emptyFrom) {
    Root updateArray[10] = {{1, 1, 50}, {2, 2, 60}};
    Root fromArray[10] = {};
    EXPECT_FALSE(rootHandler->updateRoots(updateArray, fromArray, 100));

    EXPECT_EQ(updateArray[0].root, 1);
    EXPECT_EQ(updateArray[1].root, 2);
    EXPECT_EQ(updateArray[2].root, 0);
    EXPECT_EQ(updateArray[3].root,0);
    EXPECT_EQ(updateArray[4].root, 0);

    EXPECT_EQ(fromArray[0].root, 0);
    EXPECT_EQ(fromArray[1].root, 0);
    EXPECT_EQ(fromArray[2].root, 0);
    EXPECT_EQ(fromArray[3].root, 0);
}

TEST_F(RootHandlerTest, UpdateRoots_emptyTo) {
    Root updateArray[10] = {};
    Root fromArray[10] = {{3, 3, 40}, {4, 4, 70}};
    EXPECT_TRUE(rootHandler->updateRoots(updateArray, fromArray, 100));

    EXPECT_EQ(updateArray[0].root, fromArray[0].root);
    EXPECT_EQ(updateArray[1].root, fromArray[1].root);
    EXPECT_EQ(updateArray[2].root, 0);
    EXPECT_EQ(updateArray[3].root, 0);
    EXPECT_EQ(updateArray[4].root, 0);

    EXPECT_EQ(fromArray[0].root, 3);
    EXPECT_EQ(fromArray[1].root, 4);
    EXPECT_EQ(fromArray[2].root, 0);
    EXPECT_EQ(fromArray[3].root, 0);
}

TEST_F(RootHandlerTest, UpdateRoots_LowerTS_From) {
    Root updateArray[10] = {{1, 1, 50}, {2, 2, 60}};
    Root fromArray[10] = {{3, 3, 40}, {4, 4, 70}};
    EXPECT_TRUE(rootHandler->updateRoots(updateArray, fromArray, 40));

    EXPECT_EQ(updateArray[0].root, fromArray[0].root);
    EXPECT_EQ(updateArray[1].root, 0);
    EXPECT_EQ(updateArray[2].root, 0);
    EXPECT_EQ(updateArray[3].root, 0);
    EXPECT_EQ(updateArray[4].root, 0);

    EXPECT_EQ(fromArray[0].root, 3);
    EXPECT_EQ(fromArray[1].root, 4);
    EXPECT_EQ(fromArray[2].root, 0);
    EXPECT_EQ(fromArray[3].root, 0);
}


//TODO: add test for outgoing roots
TEST_F(RootHandlerTest, UpdateRoots_Incoming_notRoot) {
    NodeInfo nodeInfo;
    nodeInfo.checkedIfRoot = true;
    nodeInfo.vertex_id = 10;
    nodeInfo.node_ts =40; 
   
    Root incoming1[10] = {{1, 1, 50}, {2, 2, 60}};
    Root incoming2[10] = {{3, 3, 40}, {4, 4, 70}};
    Root incoming3[10] = {{5, 5, 20}, {4, 4, 70}};
    Root ownRoots[10] = {};
    
    rootHandler->updateRootsFromInEdges(nodeInfo, incoming1, ownRoots);

    EXPECT_EQ(ownRoots[0].root, incoming1[0].root);
    EXPECT_EQ(ownRoots[1].root, incoming1[1].root);
    EXPECT_EQ(ownRoots[2].root, 0);
    EXPECT_EQ(ownRoots[3].root, 0);
    EXPECT_EQ(ownRoots[4].root, 0);

    rootHandler->updateRootsFromInEdges(nodeInfo, incoming2, ownRoots);

    EXPECT_EQ(ownRoots[0].root, incoming2[0].root);
    EXPECT_EQ(ownRoots[1].root, incoming1[0].root);
    EXPECT_EQ(ownRoots[2].root, incoming1[1].root);
    EXPECT_EQ(ownRoots[3].root, incoming2[1].root);
    EXPECT_EQ(ownRoots[4].root, 0);

    rootHandler->updateRootsFromInEdges(nodeInfo, incoming3, ownRoots);

    EXPECT_EQ(ownRoots[0].root, incoming3[0].root);
    EXPECT_EQ(ownRoots[1].root, incoming2[0].root);
    EXPECT_EQ(ownRoots[2].root, incoming1[0].root);
    EXPECT_EQ(ownRoots[3].root, incoming1[1].root);
    EXPECT_EQ(ownRoots[4].root, incoming2[1].root);
    EXPECT_EQ(ownRoots[5].root, 0);
}

TEST_F(RootHandlerTest, UpdateRoots_Incoming_to_many_roots) {
    NodeInfo nodeInfo;
    nodeInfo.checkedIfRoot = true;
    nodeInfo.vertex_id = 10;
    nodeInfo.node_ts =40; 
   
    Root incoming1[10] = {{1, 1, 50}, {2, 2, 60}, {3, 3, 40}, {4, 4, 70}, {5, 5, 20}, {6, 6, 70}};
    Root ownRoots[10] = {{8, 8, 51}, {9, 9, 59}, {10, 10, 43}, {11, 11, 72}, {12, 12, 22}, {13, 13, 65}};
    
    rootHandler->updateRootsFromInEdges(nodeInfo, incoming1, ownRoots);

    EXPECT_EQ(ownRoots[0].root, 3);
    EXPECT_EQ(ownRoots[1].root, 10);
    EXPECT_EQ(ownRoots[2].root, 1);
    EXPECT_EQ(ownRoots[3].root, 8);
    EXPECT_EQ(ownRoots[4].root, 9);
    EXPECT_EQ(ownRoots[5].root, 2);
    EXPECT_EQ(ownRoots[6].root, 13);
    EXPECT_EQ(ownRoots[7].root, 4);
    EXPECT_EQ(ownRoots[8].root, 6);
    EXPECT_EQ(ownRoots[9].root, 11);
}

TEST_F(RootHandlerTest, RootToPrint) {
    Root roots[3] = {{1, 10}, {2, 20}, {0, 0}};
    std::string result = rootHandler->rootToPrint(1, roots, ",");
    EXPECT_EQ(result, "1:10,2:20");
}


TEST_F(RootHandlerTest, CheckAndAssignRoot_Out) {
    NodeInfo nodeInfo = {};
    nodeInfo.vertex_id = 0;
    nodeInfo.node_ts = 40;
    Root nodeRoots[10] = {};
    unsigned long inEdges[] = {60};
    unsigned long outEdges[] = {40};

    rootHandler->checkAndAssignRoot(nodeInfo, nodeRoots, inEdges, 1, outEdges, 1);
    EXPECT_EQ(nodeRoots[0].root, 1);
    EXPECT_EQ(nodeRoots[0].order, 1);
    EXPECT_EQ(nodeRoots[0].tme, 40);
    EXPECT_TRUE(nodeInfo.checkedIfRoot);
}

TEST_F(RootHandlerTest, CheckAndAssignRoot_In) {
    NodeInfo nodeInfo = {};
    nodeInfo.vertex_id = 5;
    nodeInfo.node_ts = 1;
    Root nodeRoots[10] = {};
    unsigned long inEdges_ts[] = {1};
    unsigned long outEdges_ts[] = {5};

    rootHandler->checkAndAssignRoot(nodeInfo, nodeRoots, inEdges_ts, 1, outEdges_ts, 1);
    EXPECT_EQ(nodeRoots[0].root, 0);
    EXPECT_EQ(nodeRoots[0].order, 0);
    EXPECT_EQ(nodeRoots[0].tme, 0);
    EXPECT_TRUE(nodeInfo.checkedIfRoot);
}

/*
TEST_F(RootHandlerTest, IsRoot_True) {
    unsigned long inEdges[] = {};
    unsigned long outEdges[] = {50};
    EXPECT_TRUE(rootHandler->isRoot(inEdges, 0, outEdges, 1));
}

TEST_F(RootHandlerTest, IsRoot_False) {
    unsigned long inEdges[] = {40};
    unsigned long outEdges[] = {50};
    EXPECT_FALSE(rootHandler->isRoot(inEdges, 1, outEdges, 1));
} */
