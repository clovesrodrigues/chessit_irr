#ifndef INC_IPLAYERAIENTITY_H
#define INC_IPLAYERAIENTITY_H

#include "IAIEntity.h"

namespace IrrAI { 

/**
\brief Interface to implement player entites from
*/
class IPlayerAIEntity : public IAIEntity {

	public:    
		/**
		\brief Constructor
		\param desc - the description of the player entity
		\param aimgr - the AI manager which will manage the entity
		\param smgr - the scene which the entity will be in
		\param id - the id for the player
		*/
		IPlayerAIEntity(const SAIEntityDesc& desc, IAIManager* aimgr, scene::ISceneManager* smgr, s32 id) : IAIEntity(desc, aimgr, smgr, EAIET_PLAYER, id) {}
                          
};

} // end of IrrAI namespace

#endif /* INC_IPLAYERAIENTITY_H */
