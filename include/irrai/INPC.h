#ifndef INC_INPC_H
#define INC_INPC_H

#include "IAIEntity.h"
#include "IWaypoint.h"

namespace IrrAI { 

/**
\brief Enum to describe an NPC's current state
*/
enum E_NPC_STATE_TYPE {
	/**
	The NPC is waiting/stationary
	*/
	ENST_WAITING,
	/**
	The NPC is moving
	*/
    ENST_MOVING,
	/*
	The NPC is following its path

	This indicates that the NPC has reached a waypoint along its path which is not the goal
	*/
    ENST_FOLLOWING_PATH,
	/**
	Not an actual state but a means to easily know the number of possible states
	*/
    NUM_STATE_TYPES   
};
 
/**
\brief Enum to describe an NPC event
*/
enum E_NPC_EVENT_TYPE {
	/**
	The NPC can see an enemy
	*/
	ENET_ENEMY_VISIBLE,
	/**
	The NPC can see an enemy and that enemy is in range
	*/
    ENET_ENEMY_IN_RANGE,
	/**
	The NPC can see an ally
	*/
    ENET_ALLY_VISIBLE,
	/**
	The NPC can see an ally and that ally is in range
	*/
    ENET_ALLY_IN_RANGE,
	/**
	The NPC has been hit by a weapon
	*/
    ENET_UNDER_ATTACK,
	/**
	The NPC had been informed of an enemy's position
	*/
    ENET_ENEMY_POSITION_KNOWN,
	/*
	The NPC has reached its goal
	*/
    ENET_AT_GOAL,
	/**
	Not an actual event but a means to easily know the number of possible events
	*/
    NUM_EVENT_TYPES     
};

/**
\brief Struct describing a non-player character
*/
struct SNPCDesc : public SAIEntityDesc {
    /**
	\brief Constructor
	*/
	SNPCDesc() : SAIEntityDesc() {
		WaypointGroupName = "";
        WaypointGroupIdx = 0;
        StartWaypointID = 0;
        MoveSpeed = 0.08f; 
        AtDestinationThreshold = 5.0f;
    }
	/**
	\brief Constructor which reads in parameters from the specified IrrXMLReader
	\param xml - the XML reader to read from
	*/
	SNPCDesc(io::IrrXMLReader* xml) : SAIEntityDesc(xml) {
		if (xml) {
			WaypointGroupName = xml->getAttributeValueSafe("waypointGroupName");
			StartWaypointID = xml->getAttributeValueAsInt("startWaypointID");
			MoveSpeed = xml->getAttributeValueAsFloat("moveSpeed");
			AtDestinationThreshold = xml->getAttributeValueAsFloat("atDestinationThreshold");
			WaypointGroupIdx = 0;
		}
	}
	/**
	The name of the waypoint group for the NPC

	In a PathFindingNPC and a CombatNPC the waypoint group index will have priority over the waypoint group name, you need only specify one of them
	*/
    core::stringc WaypointGroupName;
	/**
	The index of the waypoint group for the NPC

	In a PathFindingNPC and a CombatNPC the waypoint group index will have priority over the waypoint group name, you need only specify one of them
	*/
	s32 WaypointGroupIdx;
	/**
	The ID of the waypoint to start from, if -1 is specified then a random waypoint is picked
	*/
    s32 StartWaypointID;    
	/**
	The speed at which the NPC will move in units per second
	*/
    f32 MoveSpeed;
	/**
	The threshold at which the NPC will be considered to have reached a destination (e.g. a waypoint)

	You may wish to alter this based on the scale of your scene
	*/
    f32 AtDestinationThreshold;
};

/**
\brief Interface to implement non-player characters from
*/
class INPC : public IAIEntity {

	public:
		/**
		\brief Constructor
		\param desc - description of the NPC
		\param aimgr - the AI manager to manage the NPC
		\param smgr - the scene manager of the scene for the NPC
		\param type - the type of the NPC
		\param id - the id of the NPC
		*/
		INPC(const SNPCDesc& desc, IAIManager* aimgr, scene::ISceneManager* smgr, E_AIENTITY_TYPE type, s32 id) : IAIEntity(desc, aimgr, smgr, type, id) {
			StateChangedCallbackPtr = NULL;
			EventCallbackPtr = NULL;

			if (desc.WaypointGroupName.size() == 0) WaypointGroup = aimgr->getWaypointGroupFromIndex(desc.WaypointGroupIdx);
			else WaypointGroup = aimgr->getWaypointGroupFromName(desc.WaypointGroupName.c_str());
			MoveSpeed = desc.MoveSpeed;
			AtDestinationThreshold = desc.AtDestinationThreshold;
		}
		/**
		\brief Desctructor
		*/
		virtual ~INPC() {}
		/**
		\brief If true is passed then the NPC will stop in its tracks, if false is passed it will continue on its previous path
		\param sp - whether to stay put or not
		*/
		virtual void setStayPut(bool sp) {
			StayPut = sp;
			if (StayPut)
				changeState(ENST_WAITING); 
		}
		/**
		\brief Returns whether the NPC is set to stay put
		\return a bool
		*/
		virtual bool isStayingPut() {
			return StayPut;
		}
		/**
		\brief Returns the NPC's waypoint group
		\return a SWaypointGroup*
		*/
		virtual SWaypointGroup* getWaypointGroup() {
			return WaypointGroup; 
		}
		/**
		\brief Sets the NPC's waypoint group to the specified group
		\param group - the new waypoint group
		*/
		virtual void setWaypointGroup(SWaypointGroup* group) {
			WaypointGroup = group;
		}
		/**
		\brief Sets the callback function to receive alerts to the NPC changing its state

		Note: do not store the userData pointer returned by this callback as it may be a pointer to a tempory variable
		\param cb - the callback function
		*/
		virtual void setStateChangedCallback(void (*cb)(E_NPC_STATE_TYPE, void*)) {
			StateChangedCallbackPtr = cb;
		}
		/**
		\brief Sets the callback function to receive NPC events
		\param cb - the callback function
		*/
		virtual void setEventCallback(void (*cb)(E_NPC_EVENT_TYPE, void*, void*)) {
			EventCallbackPtr = cb;
		}
		/**
		\brief Returns the NPC's destination waypoint for the path they are currently following
		\return a IWaypoint*
		*/
		virtual IWaypoint* getDestinationWaypoint() { 
			return PathDestinationWaypoint;
		}
		/**
		\brief Returns the NPC's current waypoint.

		If the NPC is between waypoints then the current waypoint will be the one it is heading for
		\return a IWaypoint*
		*/
		virtual IWaypoint* getCurrentWaypoint() {
			return CurrentWaypoint;
		}
		/**
		\brief Sends the specified event to the NPC

		This can be used to inform the NPC that it is under attack by the user application or for allowing a player to give commands to friendly NPCs
		\param event - the event
		\param eventData - data useful to the event
		*/
		virtual void sendEventToNPC(E_NPC_EVENT_TYPE event, void* eventData) {
			switch (event) {
				case ENET_UNDER_ATTACK:
					sendEvent(event, NULL);
					break;
				case ENET_ENEMY_POSITION_KNOWN:
					sendEvent(event, eventData);
					break;
				default:
					break;       
			}
		}
		/**
		\brief Returns the movement speed of the NPC, in units per millisecond
		\return a f32
		*/
		virtual f32 getMoveSpeed() const { return MoveSpeed; }
		/**
		\brief Sets the movement speed of the NPC, in units per millisecond
		\param speed - the speed to set
		*/
		virtual void setMoveSpeed(f32 speed) { MoveSpeed = speed; }
		/**
		\brief Returns the 'at destination' threshold of the NPC

		The 'at destination' threshold is the distance the NPC must be from a waypoint to be considered to have reached the waypoint
		\return a f32
		*/
		virtual f32 getAtDestinationThreshold() const { return AtDestinationThreshold; }
		/**
		\brief Sets the 'at destination' threshold of the NPC

		The 'at destination' threshold is the distance the NPC must be from a waypoint to be considered to have reached the waypoint
		\param threshold - the threshold
		*/
		virtual void setAtDestinationThreshold(f32 threshold) { AtDestinationThreshold = threshold; }

		/**
		\brief Sets the destination of the NPC to the specified waypoint
		\param dest - the destination waypoint
		*/
		virtual void setDestination(IWaypoint* dest) = 0;
		/**
		\brief Sets the destination of the NPC to the waypoint with the specified index
		\param destIdx - the destination waypoint's index
		*/
		virtual void setDestination(s32 destIdx) = 0;
		/**
		\brief Updates the NPC
		\param elapsedTime - the time elapsed since the last update
		*/
		virtual void update(s32 elapsedTime) = 0;
		/**
		\brief Sets the position of the NPC to the specified waypoint
		\param w - waypoint to set the NPC's position to
		*/
		virtual void setPosition(IWaypoint* w) = 0;

		/**
		\brief Returns true if the specified NPC list contains the specified NPC
		\param vec - the list to search
		\param npc - the NPC to search for
		\return a bool
		*/
		static bool contains(const core::list<INPC*>& vec, INPC* npc) {
			core::list<INPC*>::ConstIterator iter = vec.begin();
			while(iter != vec.end()) {
				if (npc == *iter) return true;
				iter++;
			}
			return false; 
		}
            
	protected:
		SWaypointGroup* WaypointGroup;
		core::array<IWaypoint*> PathToDestination;
		IWaypoint* PathDestinationWaypoint;
		IWaypoint* CurrentWaypoint;
		IWaypoint* DestinationWaypoint;
		IPathFinder* PathFinder;
		f32 MoveSpeed, AtDestinationThreshold;
		bool StayPut;
		E_NPC_STATE_TYPE State;
    
		void (*StateChangedCallbackPtr)(E_NPC_STATE_TYPE, void*);
		void (*EventCallbackPtr)(E_NPC_EVENT_TYPE, void*, void*);

		/**
		\brief Changes the state of the NPC to the specified state and calls the state changed callback function (if set)
		\param state - the new state
		*/
		virtual void changeState(E_NPC_STATE_TYPE state) {
			State = state; 
			if (StateChangedCallbackPtr)
				StateChangedCallbackPtr(State, UserData);  
		}
		/**
		\brief Sends the specified event to the event callback function (if set)
		\param event - the event
		\param eventData - the data for the event, if any
		*/
		virtual void sendEvent(E_NPC_EVENT_TYPE event, void* eventData) {
			if (EventCallbackPtr)
				EventCallbackPtr(event, UserData, eventData);   
		}
                    
};

} // end of IrrAI namespace

#endif /* INC_INPC_H */
