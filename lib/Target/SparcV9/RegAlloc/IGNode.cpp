//===-- IGNode.cpp -------------------------------------------------------===//
// 
//  class IGNode for coloring-based register allocation for LLVM.
// 
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/IGNode.h"
#include <algorithm>
#include <iostream>
using std::cerr;

//-----------------------------------------------------------------------------
// Sets this IGNode on stack and reduce the degree of neighbors  
//-----------------------------------------------------------------------------

void IGNode::pushOnStack() {
  OnStack = true; 
  int neighs = AdjList.size();

  if (neighs < 0) {
    cerr << "\nAdj List size = " << neighs;
    assert(0 && "Invalid adj list size");
  }

  for(int i=0; i < neighs; i++)
    AdjList[i]->decCurDegree();
}
 
//-----------------------------------------------------------------------------
// Deletes an adjacency node. IGNodes are deleted when coalescing merges
// two IGNodes together.
//-----------------------------------------------------------------------------

void IGNode::delAdjIGNode(const IGNode *Node) {
  std::vector<IGNode *>::iterator It=find(AdjList.begin(), AdjList.end(), Node);
  assert( It != AdjList.end() );      // the node must be there
  AdjList.erase(It);
}

//-----------------------------------------------------------------------------
// Get the number of unique neighbors if these two nodes are merged
//-----------------------------------------------------------------------------

unsigned
IGNode::getCombinedDegree(const IGNode* otherNode) const
{
  std::vector<IGNode*> nbrs(AdjList);
  nbrs.insert(nbrs.end(), otherNode->AdjList.begin(), otherNode->AdjList.end());
  sort(nbrs.begin(), nbrs.end());
  std::vector<IGNode*>::iterator new_end = unique(nbrs.begin(), nbrs.end());
  return new_end - nbrs.begin();
}


