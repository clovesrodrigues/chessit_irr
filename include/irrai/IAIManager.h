#ifndef INC_IAIMANAGER_H
#define INC_IAIMANAGER_H

#include <irrlicht.h>
#include "IDebugFOVSceneNode.h"
#include "IPathFinder.h"

using namespace irr;

namespace IrrAI { 

class IAIEntity;
struct SAIEntityDesc;
class IPlayerAIEntity;
class INPC;
struct SNPCDesc;
class ICombatNPC;
struct SCombatNPCDesc;
class IFieldOfView;
class IAISensor;
class IWaypoint;


/*! Struct describing a group of waypoints */
struct SWaypointGroup {
	/**
	Constructor
	*/
	SWaypointGroup() {
		WaypointMeshNode = NULL;
		Name = "Group X";
		Colour = video::SColor(255,255,255,255);  
		WaypointSize = 5;             
	} 
	/**
	\brief Sets the name of the waypoint group
	\param str - the new name
	*/
	void setName(const c8* str) { Name = str; }
	/**
	\brief Returns the name of the waypoint group
	\return a c8*
	*/
	const c8* getName() { return Name.c_str(); }
	/**
	\brief Returns whether the specified waypoint is a member of this waypoint group
	\param waypoint - the waypoint to check for membership
	\return a bool
	*/
	bool contains(IWaypoint* waypoint);
	/**
	\brief Sets the colour of the group
	\param colour - the new colour
	*/
	void setColour(const video::SColor& colour);
	/**
	\brief Sets the size of the waypoints in the mesh

	Note that the waypoint mesh node will require rebuilding with IAIManager::createDebugWaypointMesh(SWaypointGroup*) for this to take effect
	\param size - the new size
	*/
	void setSize(s32 size);
  
	/**
	The waypoints of the group
	*/
	core::array<IWaypoint*> Waypoints;
	/**
	A scene node showing the positions and links of the waypoints, useful for debugging

	Note that this is initally NULL and must be created with IAIManager::createDebugWaypointMesh(SWaypointGroup*)
	*/
	scene::ISceneNode* WaypointMeshNode;
	/**
	The name of the group
	*/
	core::stringc Name;
	/**
	The colour of the group
	*/
	video::SColor Colour;  
	/**
	The size of the waypoints in the waypoint mesh node
	*/
	s32 WaypointSize;                                          
};

/*! Struct describing a group of entities*/
struct SEntityGroup {
	/**
	\brief Constructor
	*/
	SEntityGroup() {}
	/**
	\brief Adds the specified entity to this group
	\param entity - the entity to be added
	*/
	void addEntity(IAIEntity* entity) {
		Entities.push_back(entity);
	} 
                 
	/**
	The entities in the group
	*/
	core::array<IAIEntity*> Entities;
};

/*! Class to manage AI entities */
class IAIManager {

	public:
		/**
		\brief Constructor
		
		Do not use this to create an instance of the AI manager, rather use createAIManager(IrrlichtDevice* device)
		\param device - the Irrlicht Device you wish to use the AI manager with
		\see createAIManager(IrrlichtDevice* device)
		*/
		IAIManager(IrrlichtDevice* device) {
			Device = device;
			SceneManager = Device->getSceneManager();
			DebugVisible = false;
		}
		/**
		\brief Destructor
		*/
		virtual ~IAIManager() {}

		/**
		\brief Reinitialises the AI manager

		This is necessary after a call to ISceneManager::clear()
		*/
		virtual void reinit() = 0;
		/**
		\brief Updates the AI manager
		\param elapsedTime - time elapsed since last update
		*/
		virtual void update(s32 elapsedTime) = 0;
		/**
		\brief Removes AI entities and waypoints
		*/
		virtual void clear() = 0;
		/**
		\brief Reads waypoints & entities from the specified IrrAI format file
		\param fileName - the file to read from
		*/
		virtual bool loadAI(const c8* fileName) = 0;
		/**
		\brief Writes waypoints & entities to the specified IrrAI format file
		\param fileName - the file to write to
		*/
		virtual bool saveAI(const c8* fileName) = 0; 
		/**
		\brief Returns the waypoint with the specified ID if it's a member of the specified waypoint group, otherwise returns NULL
		\param group - the waypoint group which contains the required waypoint
		\param id - the ID of the required waypoint
		\return a IWaypoint*
		*/
		virtual IWaypoint* getWaypointFromId(SWaypointGroup* group, s32 id) = 0; 
		/**
		\brief Adds a path finding NPC entity to the AI manager with the specified description

		The pathfinding NPC can do pathfinding over a waypoint group
		\param desc - description of the NPC to be added
		\return a INPC*
		*/
		virtual INPC* createPathFindingNPC(const SNPCDesc& desc) = 0;
		/**
		\brief Adds a combat NPC entity to the AI manager with the specified description

		The combat NPC can do pathfinding over a waypoint group and also see enemies and allies with its field of view
		\param desc - description of the NPC to be added
		\return a INPC*
		*/
		virtual ICombatNPC* createCombatNPC(const SCombatNPCDesc& desc) = 0; 
		/**
		\brief Adds a Player entity to the AI manager with the specified description
		\param desc - description of the Player to be added
		\return a IPlayerAIEntity*
		*/
		virtual IPlayerAIEntity* createPlayerAIEntity(const SAIEntityDesc& desc) = 0;
		/**
		\brief Creates an empty SEntityGroup
		\return an SEntityGroup
		*/ 
		virtual SEntityGroup* createEntityGroup() = 0; 
		/**
		\brief Creates a waypoint at the specified position, adding it to the specified waypoint group
		\param group - the waypoint group to add the waypoint to
		\param pos - the position to put the waypoint at
		\return a IWaypoint*
		*/
		virtual IWaypoint* createWaypoint(SWaypointGroup* group, const core::vector3df& pos = core::vector3df(0,0,0)) = 0; 
		/**
		\brief Removes the specified waypoint from the specified waypoint group
		\param group - the group the waypoint is a member of
		\param waypoint - the waypoint to be removed
		*/
		virtual void removeWaypoint(SWaypointGroup* group, IWaypoint* waypoint) = 0; 
		/**
		\brief Removes the specified waypoint group and all its waypoints
		\param group - the waypoint group to be removed
		*/ 
		virtual void removeWaypointGroup(SWaypointGroup* group) = 0; 
		/**
		\brief Creates a new, empty waypoint group
		\return a SWaypointGroup*
		*/
		virtual SWaypointGroup* createWaypointGroup() = 0; 
		/**
		\brief Returns the waypoint group with the specified name or NULL if it does not exist
		\param name - the name of the waypoint group to be returned
		\return a SWaypointGroup*
		*/
		virtual SWaypointGroup* getWaypointGroupFromName(const c8* name) = 0; 
		/**
		\brief Removes the specified AI entity
		\param entity - the entity to be removed
		*/
		virtual void removeAIEntity(IAIEntity* entity) = 0; 
		/**
		\brief Sets the debug render data to the specified visibility
		\param val - the desired visibility
		*/
		virtual void setDebugVisible(bool val) = 0; 
		/**
		\brief Creates a mesh showing the locations and linkings of waypoints of the specified waypoint group for debug purposes
		
		Debug data must be set to visible with setDebugVisible(true) to see these
		\param group - the waypoint group to create the mesh for
		*/
		virtual void createDebugWaypointMesh(SWaypointGroup* group) = 0;
		/**
		\brief Creates a meshes showing the locations and linkings of waypoints in all waypoint group for debug purposes
		
		Debug data must be set to visible with setDebugVisible(true) to see these
		*/
		virtual void createDebugWaypointMeshes() = 0; 
		/**
		\brief Removes any previously created debug meshes for all waypoint groups
		*/
		virtual void removeDebugWaypointMeshes() = 0;
		/**
		\brief Returns the nearest waypoint in the specified group to the specified position
		\param group - the group to check
		\param pos - the position to check for
		\return a IWaypoint*
		*/
		virtual IWaypoint* getNearestWaypoint(SWaypointGroup* group, core::vector3df pos) = 0;
		/**
		\brief Returns the IAIEntity with the specified name.
		\param name - The name of the desired entity
		\return an IAIEntity*
		*/
		virtual IAIEntity* getEntityFromName(const wchar_t* name) = 0;
		/**
		\brief Creates a cone shaped field of view of the specified dimensions

		Note that only the X and Y coordinates of the specified dimensions are used. The X coordinate defines the width and the Z coordinate defines the depth.
		\param dim - the dimensions of the field of view
		\param occlusionCheck - whether the field of view should check occlusion
		\return a IFieldOfView*
		*/
		virtual IFieldOfView* createConeFieldOfView(const core::vector3df& dim, bool occlusionCheck) = 0;
		/**
		\brief Removes the specified field of view
		\param fov - the field of view to be removed
		*/
		virtual void removeFieldOfView(IFieldOfView* fov) = 0;
		/**
		\brief Creates a cone shaped debug field of view scene node to visually represent a field of view

		Note that only the X and Y coordinates of the specified dimensions are used. The X coordinate defines the width and the Z coordinate defines the depth.
		\param dim - the dimensions of the field of view
		*/
		virtual IDebugFOVSceneNode* createDebugConeFOVSceneNode(const core::vector3df& dim) = 0;
		/**
		\brief Removes the specified debug field of view scene node
		\param fovNode - the node to be removed
		*/
		virtual void removeDebugFOVScenNode(IDebugFOVSceneNode* fovNode) = 0;
		/**
		\brief Creates a path finder which uses an A* algorithm
		\return a IPathFinder*
		*/
		virtual IPathFinder* createAStarPathFinder() = 0;
		/**
		\brief Creates a path finder which uses a breadth first algorithm
		\return a IPathFinder*
		*/
		virtual IPathFinder* createBreadthFirstPathFinder() = 0;
		/**
		\brief Removes the specified path finder
		\param pathFinder - the path finder to be removed
		*/
		virtual void removePathFinder(IPathFinder* pathFinder) = 0;
		/**
		\brief Creates an AI sensor which reports when an entity (that has been registered with the sensor) enters or exits the sensor's volume
		\param desc - description of the sensor to be added
		\return a IAISensor*
		*/
		virtual IAISensor* createEntryExitSensor(const SAIEntityDesc& desc) = 0;
		/**
		\brief Removes the specified sensor
		\param sensor - the sensor to be removed
		*/
		virtual void removeSensor(IAISensor* sensor) = 0;
		/**
		\brief Returns the total number of waypoints
		\return a s32
		*/
		virtual s32 getNumWaypoints() const = 0;
		/**
		\brief Returns the total number of entities
		\return a s32
		*/
		virtual s32 getNumEntities() const = 0;
		/**
		\brief Returns the waypoint group with the specified index.
		\param idx - the index of the required waypoint group
		\return a SWaypointGroup*
		*/
		virtual SWaypointGroup* getWaypointGroupFromIndex(u32 idx) = 0;
		/**  
		\brief Returns access to the full set of waypoint groups
		\return a core::array<SWaypointGroup*>
		*/
		virtual const core::array<SWaypointGroup*>& getWaypointGroups() = 0;
		/**
		\brief Returns an SEntityGroup containing all the entities
		\return an SEntityGroup
		*/
		virtual const SEntityGroup* getEntities() const = 0;
		/**
		\brief Returns whether the debug render data is set to visible or not
		\return a bool
		*/
		virtual bool isDebugVisible() const = 0;
		/**
		\brief Sets the function to use for occlusion querys
		\param cb - the callback function to use
		*/
		virtual void setOcclusionQueryCallback(bool (*cb)(const core::line3df& ray)) = 0;
		/**
		\brief Returns whether the specified ray collides with the user's physical world or not, otherwise returns false

		Note that an occlusion query callback function must be specified with IAIManager::setOcclusionQueryCallback(bool (*cb)(const core::line3df& ray)). If this has not be done then false will be returned
		\param ray - the ray to use in the occlusion query
		\return a bool
		*/
		virtual bool occlusionQuery(const core::line3df& ray) = 0;
		/**
		\brief Adds the specified entity to the list of entities managed by this AI manager

		This should be used if you've created your own IAIEntity implementation and want the AI manager to update it
		\param entity - the entity to be added
		*/
		virtual void registerAIEntity(IAIEntity* entity) = 0;
		/**
		\brief Returns the node used to parent all debug nodes to make it easy to turn their visibility on or off
		\return a ISceneNode*
		*/
		virtual scene::ISceneNode* getDebugRootNode() = 0;
            
	protected:
		IrrlichtDevice* Device;
		scene::ISceneManager* SceneManager;
		bool DebugVisible;
        
};

} // end of IrrAI namespace

#endif /* INC_IAIMANAGER_H */
