#ifndef INC_IAIENTITY_H
#define INC_IAIENTITY_H

#include "IAIManager.h"
#include "Utility.h"

namespace IrrAI {

/**
\brief An enum describing the type of an entity
*/
enum E_AIENTITY_TYPE {
	/**
	an NPC entity which can pathfind and see other entities
	*/
	EAIET_COMBATNPC, 
	/**
	an NPC entity which can pathfind
	*/
	EAIET_PATHFINDINGNPC,
	/**
	a human player entity type
	*/
	EAIET_PLAYER,
	/**
	an AI sensor entity type which senses other entities entering or exiting its volume
	*/
	EAIET_ENTRYEXIT_SENSOR,
	/**
	an unknown entity type
	*/
	EAIET_UNKNOWN,
	/**
	Not an actual entity type but a means to easily know the number of possible entity types
	*/
	EAIET_NUM_TYPES  
};

/**
\brief Struct describing an AI entity
*/
struct SAIEntityDesc {
	/**
	\brief Constructor
	*/
	SAIEntityDesc() {
		Scale = core::vector3df(10,10,10);
		Offset = core::vector3df(0,5,0);
		UserData = NULL;
		Name = L"";
	}
	/**
	\brief Constructor which reads in parameters from the specified IrrXMLReader
	\param xml - the XML reader to read from
	*/
	SAIEntityDesc(io::IrrXMLReader* xml) {
		if (xml) {
			core::stringc strc;
			strc = xml->getAttributeValueSafe("scale");
			getVector3dfFrom(strc.c_str(), strc.size(), &Scale);
			Name = xml->getAttributeValueSafe("name");
			strc = xml->getAttributeValueSafe("offset");
			getVector3dfFrom(strc.c_str(), strc.size(), &Offset);
			UserData = NULL;
			strc = xml->getAttributeValueSafe("position");
			getVector3dfFrom(strc.c_str(), strc.size(), &Position);
			strc = xml->getAttributeValueSafe("rotation");
			getVector3dfFrom(strc.c_str(), strc.size(), &Rotation);
		}
	}
	/**
	The scale of the entity
	*/
	core::vector3df Scale;
	/**
	The name of the entity
	*/
	core::stringw Name;
	/**
	The user data of the entity
	*/
	void* UserData;
	/**
	The offset of the entity
	*/
	core::vector3df Offset;
	/**
	The position of the entity
	*/
	core::vector3df Position;
	/**
	The rotation of the entity
	*/
	core::vector3df Rotation;

};

/**
\brief Interface to implement AI entites from
*/
class IAIEntity {

	public:
		/**
		\brief Constructor
		\param desc - description of the entity to be created
		\param aimgr - the ai manager which will manage the entity
		\param smgr - the scene manager to use the entity with
		\param type - the entity type
		\param id - id of the entity
		*/
		IAIEntity(const SAIEntityDesc& desc, IAIManager* aimgr, scene::ISceneManager* smgr, E_AIENTITY_TYPE type, s32 id) {

			AIManager = aimgr;
			SceneManager = smgr;
			Type = type;
			ID = id;
			UserData = desc.UserData;
			Name = desc.Name;
			NodeOffset = desc.Offset;
			VisibleToOtherEntity = false;
			EnemyGroup = NULL;
			AllyGroup = NULL;
  
			// Add a simple cube to represent the entity, used for FOV checking etc.
			Node = SceneManager->addCubeSceneNode(1, AIManager->getDebugRootNode(), ID);
			if (!Node) {
				printf("entity node creation failed\n");
				return;
			}
			Node->grab();
			Node->setScale(desc.Scale);
			Node->setRotation(desc.Rotation);
			Node->setMaterialFlag(video::EMF_LIGHTING, false);
			Node->setMaterialFlag(video::EMF_WIREFRAME, true);
			setPosition(desc.Position);
  
			TextNode = smgr->addTextSceneNode(SceneManager->getGUIEnvironment()->getSkin()->getFont(), core::stringw(Name.c_str()).c_str(), video::SColor(255,255,255,255), Node);

		}
		/**
		\brief Destructor
		
		Removes this entity from its registered enemy/ally groups and also the node representing the entity
		*/
		virtual ~IAIEntity() {

			if (Node) {
				Node->drop();
				Node->remove();
				Node = NULL;
			}
            
			if (EnemyGroup) { // removing it from here will remove it from enemy's entity groups
				for (u32 i = 0 ; i < EnemyGroup->Entities.size() ; ++i) { 
					if (EnemyGroup->Entities[i] == this) {
						EnemyGroup->Entities.erase(i);
					}
				}
			}
               
			if (AllyGroup) { // removing it from here will remove it from enemy's entity groups
				for (u32 i = 0 ; i < AllyGroup->Entities.size() ; ++i) { 
					if (AllyGroup->Entities[i] == this) {
						AllyGroup->Entities.erase(i);
					}
				}
			}
	
		}
		/**
		\brief Resets the entity
		\internal this function is for internal use
		*/
		virtual void reset() {
			setVisibleToOtherEntity(false); 
		}
		/**
		\brief Updates the entity
		\param elapsedTime - the time elapsed since last update
		*/
		virtual void update(s32 elapsedTime) {
			Node->updateAbsolutePosition();
		}
		/**
		\brief Writes out descriptive data to the specified arrays. Used for saving entity descriptions to file
		\param names - the names of the parameters
		\param values - the values of the parameters
		\internal this function is for internal use
		*/
		virtual void writeOutXMLDescription(core::array<core::stringw>& names, core::array<core::stringw>& values) const {
			core::stringw strw;
			names.push_back(core::stringw(L"scale"));
			strw = core::stringw(Node->getScale().X); 
			strw += ",";
			strw += Node->getScale().Y; 
			strw += ",";
			strw += Node->getScale().Z; 
			values.push_back(strw);
			names.push_back(L"name");
			values.push_back(Name);
			names.push_back(core::stringw(L"offset"));
			strw = core::stringw(NodeOffset.X); 
			strw += ",";
			strw += NodeOffset.Y; 
			strw += ",";
			strw += NodeOffset.Z; 
			values.push_back(strw);
			names.push_back(core::stringw(L"position"));
			core::vector3df pos = Node->getPosition() - NodeOffset;
			strw = core::stringw(pos.X); 
			strw += ",";
			strw += pos.Y; 
			strw += ",";
			strw += pos.Z; 
			values.push_back(strw);
			names.push_back(core::stringw(L"rotation"));
			core::vector3df rot = Node->getRotation();
			strw = core::stringw(rot.X); 
			strw += ",";
			strw += rot.Y; 
			strw += ",";
			strw += rot.Z; 
			values.push_back(strw);
		}
		/**
		\brief Rotates the entity to face the specified position
		\param targetPosition - the position to face
		*/
		virtual void rotateToFace(const core::vector3df& targetPosition) {
  
			core::vector3df r = targetPosition - Node->getAbsolutePosition();
			core::vector3df angle;
   
			angle.Y = atan2 (r.X, r.Z);
			angle.Y *= (180 / core::PI);
   
			if(angle.Y < 0) angle.Y += 360;
			if(angle.Y >= 360) angle.Y -= 360;
  
			angle -= core::vector3df(0,90,0);
   
			Node->setRotation(angle);
  
		}
		/**
		\brief Sets the offset position of the entity 

		The offset is used to position the entity so that it will appear to be standing on the ground
		\param vec - the new offset
		*/
		virtual void setOffset(const core::vector3df& vec) {

			core::vector3df oldPos = Node->getAbsolutePosition();
			core::vector3df currPos = oldPos - NodeOffset;
			NodeOffset = vec;
			Node->setPosition(currPos + NodeOffset);

		}
		/**
		\brief Returns the entity's offset

		The offset is used to position the entity so that it will appear to be standing on the ground
		\return a vector3df
		*/
		virtual const core::vector3df& getOffset() const {
			return NodeOffset;
		}
		/**
		\brief Sets the position of the entity

		If your entity is able to navigate through your world then using this function may cause unexpected effects
		\param vec - the new position
		*/
		virtual void setPosition(const core::vector3df& vec) {
			Node->setPosition(vec + NodeOffset);
			Node->updateAbsolutePosition();
		}
		/**
		\brief Returns the absolute position of the entity
		\return a vector3df
		*/
		virtual core::vector3df getAbsolutePosition() const {
			return Node->getAbsolutePosition();
		}
		/**
		\brief Sets the entity group to be considered enemies
		\param grp - the entity group
		*/
		virtual void setEnemyGroup(SEntityGroup* grp) {
			EnemyGroup = grp;
		}
		/**
		\brief Sets the entity group to be considered allies
		\param grp - the entity group
		*/
		virtual void setAllyGroup(SEntityGroup* grp) {
			AllyGroup = grp;
		}  
		/**
		\brief Returns the type of the entity
		\return an E_AIENTITY_TYPE
		*/  
		virtual E_AIENTITY_TYPE getType() const {
			return Type;
		}
		/**
		\brief Returns whether the entity is known to be visible to any other entity
		\return a bool
		*/
		virtual bool isVisibleToOtherEntity() const {
			return VisibleToOtherEntity;
		}
		/**
		\brief Sets whether the entity is known to be visible to any other entity or not
		\param val - whether the entity is visible to another entity
		*/
		virtual void setVisibleToOtherEntity(bool val) {
			VisibleToOtherEntity = val;
		}
		/**
		\brief Sets the entity's user data
		\param data - the data to set
		*/
		virtual void setUserData(void* data) {
			UserData = data;
		}
		/**
		\brief Returns the user data which can be used for anything the user wishes

		A good use of this would be a pointer to the application's owner of the entity, such as a class for the renderable representation of the entity
		\return a void*
		*/
		virtual void* getUserData() const {
			return UserData;
		}
		/**
		\brief Returns the entity's enemy group, or NULL if this has not been set
		\return an SEntityGroup*
		*/
		virtual SEntityGroup* getEnemyGroup() const {
			return EnemyGroup;
		}
		/**
		\brief Returns the entity's ally group, or NULL if this has not been set
		\return an SEntityGroup*
		*/
		virtual SEntityGroup* getAllyGroup() const {
			return AllyGroup;
		}
		/**
		\brief Returns the entity's ID
		\return an s32
		*/
		virtual s32 getID() const {
			return ID;
		}
		/**
		\brief Returns the entity's name
		\return a const wchar_t*
		*/
		virtual const wchar_t* getName() const {
			return Name.c_str();
		}
		/**
		\brief Sets the entity's name to the specified string
		\param str - the name to set
		*/
		virtual void setName(const wchar_t* str) {
			Name = str;
			TextNode->setText(Name.c_str());
		}
		/**
		\brief Returns the node representing the entity
		\return a ISceneNode
		*/
		virtual scene::ISceneNode* getNode() {
			return Node;
		}
		/**
		\brief Sets the scale of the node representing the entity
		\param vec - the new scale
		*/
		virtual void setScale(const core::vector3df& vec) {
			Node->setScale(vec);
		}
        
	protected:
		IAIManager* AIManager;
		scene::ISceneManager* SceneManager;
		scene::ISceneNode* Node;
		scene::ITextSceneNode* TextNode;
		core::vector3df NodeOffset;
		SEntityGroup* EnemyGroup,* AllyGroup;
		void* UserData;
		bool VisibleToOtherEntity;
		E_AIENTITY_TYPE Type;
		s32 ID;
		core::stringw Name;
    
};

} // end of IrrAI namespace

#endif /* INC_IAIENTITY_H */
