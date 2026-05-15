#ifndef INC_IWAYPOINT_H
#define INC_IWAYPOINT_H

#include <irrlicht.h>

using namespace irr;

namespace IrrAI {

class IWaypoint;

/**
\brief Struct representing a waypoint's neighbour
*/
struct SNeighbour {
	/**
	\brief Constructor
	*/
	SNeighbour() {
		Waypoint = NULL;
        Distance = -1;
    }
	/**
	The waypoint which is the neighbour
	*/
    IWaypoint* Waypoint;
	/**
	The distance to the neighbour
	*/
    f32 Distance;     
};

/**
\brief Interface to implement waypoints from
*/
class IWaypoint {
      
	public:
		/**
		\brief Constructor
		\param id - the ID of the waypoint
		\param position - the position of the waypoint
		*/
		IWaypoint(s32 id, const core::vector3df& position = core::vector3df(0,0,0)) {
			ID = id;
			Position = position;
		}
		/**
		\brief Destructor
		*/
		virtual ~IWaypoint() {
			remove();
		}
		/**
		\brief Removes the waypoint, at the same time removing itself from any neighbours
		*/
		virtual void remove() {
			core::list<SNeighbour*>::Iterator iter = Neighbours.begin();
			while (iter != Neighbours.end()) {
				if ((*iter)->Waypoint) (*iter)->Waypoint->removeNeighbour(this);
				++iter;
			}
			Neighbours.clear();
		}
		/**
		\brief Returns the list of this waypoint's neighbours
		\return a core::list
		*/
        virtual const core::list<SNeighbour*>& getNeighbours() {
			return Neighbours;
		}
		/**
		\brief Returns whether the two waypoints are equal to each other

		Note that this function checks for equality by checking that both waypoints have the same ID
		\param waypoint - the waypoint to check equality with
		\return a bool
		*/
        virtual bool equals(IWaypoint* waypoint) const {
			return ID == waypoint->getID();
		}
		/**
		\brief Returns the position of this waypoint
		\return a core::vector3df
		*/
        virtual const core::vector3df& getPosition() const {
			return Position;
		}
		/**
		\brief Sets the position of this waypoint
		\param pos - the new position
		*/
        virtual void setPosition(const core::vector3df& pos) {
			Position = pos;
		}
		/**
		\brief Returns the ID of this waypoint
		\return a s32
		*/
        virtual s32 getID() const {
			return ID;
		}
		/**
		\brief Returns whether this waypoint is 'bigger than' the specified waypoint

		Note that in this case 'bigger than' means the waypoint's ID is bigger than the other's
		\param wypt - the waypoint to check against
		\return a bool
		*/
        virtual bool operator>(const IWaypoint* wypt) const {
			return ID > wypt->getID();
		} 
				
		/**
		\brief Adds the specified waypoint as a neighbour to this one
		\param w - the neighbour to add
		*/
		virtual void addNeighbour(IWaypoint* w) = 0;
		/**
		\brief Removes the specified waypoint from being a neighbour to this one
		\param w - the neighbour to remove
		*/
        virtual void removeNeighbour(IWaypoint* w) = 0;
		/**
		\brief Returns whether the specified waypoint is a neighbour to this one
		\param w - the neighbour to check for
		\return a bool
		*/
        virtual bool hasNeighbour(IWaypoint* w) const = 0;

		/**
		\brief Returns whether the specified array of waypoints contains the specified waypoint
		\param arr - the arry to check
		\param waypoint - the waypoint to check for
		\return a bool
		*/
		static bool contains(const core::array<IWaypoint*>& arr, IWaypoint* waypoint) {
			if (!waypoint) return false;
			//if (arr.binary_search_const(waypoint) != -1) return true;
			for (u32 i = 0 ; i < arr.size() ; ++i)
				if (arr[i] == waypoint) return true;
			return false; 
		}
		/**
		\brief Returns whether the specified list of neighbours contains the specified waypoint
		\param list - the list to check
		\param waypoint - the waypoint to check for
		\return a bool
		*/
        static bool contains(const core::list<SNeighbour*>& list, IWaypoint* waypoint) {
			core::list<SNeighbour*>::ConstIterator iter = list.begin();
			while(iter != list.end()) {
				if (waypoint == (*iter)->Waypoint) return true;
				++iter;
			}
			return false; 
		}
		/**
		\brief Prints out the IDs of the waypoints in the specified array
		\param arr - the arry to print
		*/
		static void printWaypointIDs(const core::array<IWaypoint*>& arr) {
			if (arr.size() == 0) return;
			for (u32 i = 0 ; i < arr.size() ; ++i) 
				printf("%i ", arr[i]->getID());     
			printf("\n");       
		}
                        
	protected:
        s32 ID;
        core::vector3df Position;
		core::list<SNeighbour*> Neighbours;
		core::stringc NeighbourString;
		
};

} // end of IrrAI namespace

#endif /* INC_IWAYPOINT_H */
