#ifndef INC_IPATHFINDER_H
#define INC_IPATHFINDER_H

#include "IWaypoint.h"

namespace IrrAI {

/*! Struct for use in search algorithms */
struct SSearchNode {
	/**
    \brief Constructor
    \param p - parent search node
    \param w - waypoint node being represented
    */
    SSearchNode (SSearchNode* p, IWaypoint* w) {
		Parent = p;
        Waypoint = w;
    } 
	/**
	The parent of this node
	*/
    SSearchNode* Parent;
	/**
	The waypoint of this node
	*/
    IWaypoint* Waypoint; 
};

/*! Interface for a path finder class */
class IPathFinder {

	public: 
		/**
		\brief Constructor
		*/ 
		IPathFinder() {}
		/**
		\brief Destructor
		*/
		virtual ~IPathFinder() {}
		/**
		\brief Returns true if a path was found between the specified nodes, false otherwise
		\param startNode - the waypoint to start the search from
		\param goalNode - the waypoint to fine a path to
		\param path - the array to receive the path
		\return a bool
		*/
		virtual bool findPath(IWaypoint* startNode, IWaypoint* goalNode, core::array<IWaypoint*>& path) = 0;
    
  
	protected:
		/**
		\brief Deletes the search nodes in the specified array
		\param arr - the array of search nodes
		*/
		virtual void deleteSearchNodes(const core::array<SSearchNode*>& arr) {
			for (u32 i = 0 ; i < arr.size() ; ++i)
				delete arr[i];
		}
		/**
		\brief Returns true if the specified waypoint is a member of the specified array of search nodes, false otherwise
		\param arr - the array of search nodes
		\param node - the node to check for
		\return a bool
		*/
		virtual bool contains(const core::array<SSearchNode*>& arr, IWaypoint* node) {       
			for (u32 i = 0 ; i < arr.size() ; ++i)
				if (node == arr[i]->Waypoint) return true;
			return false; 
		}
		/**
		\brief Retrieves the reverse path from the search node structure and returns true if a full path was constructed
		\param sNode - the search node that has reached the goal node
		\param path - the array to receieve the path
		\return a bool
		*/
		virtual  bool getPath(SSearchNode* sNode, core::array<IWaypoint*>& path) {
			path.clear(); // clear out any previous paths, we want a fresh one now!
			const SSearchNode* pSNode = sNode;
			// Create path (backwards)
			while (pSNode != NULL) {
				path.push_back(pSNode->Waypoint);
				pSNode = pSNode->Parent;
			}   
			return pSNode == NULL; // the current node should not have a parent, it should be the start node
		}
    
};

} // end of IrrAI namespace

#endif /* INC_IPATHFINDER_H */
