#ifndef INC_ICOMBATNPC_H
#define INC_ICOMBATNPC_H

#include "INPC.h"
#include "IFieldOfView.h"

namespace IrrAI {

/**
\brief Struct describing a combat style non-player character
*/
struct SCombatNPCDesc : public SNPCDesc {
    /**
	\brief Constructor
	*/
	SCombatNPCDesc() : SNPCDesc() {
        FovDimensions = core::vector3df(200,200,0);
        Range = 200.0f;    
        FovOcclusionCheck = true;
		CheckFovForEnemies = true;
		CheckFovForAllies = false;
    }
	/**
	\brief Constructor which reads in parameters from the specified IrrXMLReader
	\param xml - the XML reader to read from
	*/
	SCombatNPCDesc(io::IrrXMLReader* xml) : SNPCDesc(xml) {
		if (xml) {
			core::stringc strc;
			strc = xml->getAttributeValueSafe("fovDimensions");
			getVector3dfFrom(strc.c_str(), strc.size(), &FovDimensions);
			Range = xml->getAttributeValueAsFloat("range");
			FovOcclusionCheck = xml->getAttributeValueAsInt("fovOcclusionCheck") != 0;
			CheckFovForEnemies = xml->getAttributeValueAsInt("checkFovForEnemies") != 0;
			CheckFovForAllies = xml->getAttributeValueAsInt("checkFovForAllies") != 0;
		}
	}
	/**
	The dimensions of the NPC's field of view (if applicable)
	*/
    core::vector3df FovDimensions;
	/**
	The range of the NPC's weapon (or similar) (if applicable)

	This should be less than or equal to the 'height' of the field of view
	*/
    f32 Range;  
	/**
	Whether the NPC can see through walls or not (if applicable)

	Setting this to true will mean the NPC cannot see through walls
	*/
    bool FovOcclusionCheck;
	/**
	Whether the NPC should check its field of view for enemies (if applicable)
	*/
	bool CheckFovForEnemies;
	/**
	Whether the NPC should check its field of view for allies (if applicable)
	*/
	bool CheckFovForAllies;
};

/**
\brief Interface for Combat style NPCs which can pathfind and see enemies with their field of view
*/
class ICombatNPC : public INPC {

	public:
		/**
		\brief Constructor
		\param desc - description of the NPC
		\param aimgr - the AI manager to manage the NPC
		\param smgr - the scene manager of the scene the NPC will be added to
		\param id - the NPC's id
		*/
		ICombatNPC(const SCombatNPCDesc& desc, IAIManager* aimgr, scene::ISceneManager* smgr, s32 id) : INPC(desc, aimgr, smgr, EAIET_COMBATNPC, id) {
			FieldOfView = NULL;
			DebugFOV = NULL;
			Range = desc.Range;
			FovOcclusionCheck = desc.FovOcclusionCheck;
			CheckFovForEnemies = desc.CheckFovForEnemies;
			CheckFovForAllies = desc.CheckFovForAllies;
		}
		/**
		\brief Destructor
		*/
		virtual ~ICombatNPC() {}
		/**
		\brief Resets the NPC
		*/
		virtual void reset() { INPC::reset(); }
		/**
		\brief Sets the 'awareness' range of the NPC

		This could be used for the range of a weapon so that the NPC can alert the controlling user-character that an enemy is within range of their weapon
		\param range - the range
		*/
		virtual void setRange(f32 range) { Range = range; }
		/**
		\brief Returns the 'awareness' range of the NPC

		This could be used for the range of a weapon so that the NPC can alert the controlling user-character that an enemy is within range of their weapon
		\return a f32
		*/
		virtual f32 getRange() { return Range; }
		/**
		\brief Sets the NPC's field of view dimensions
		\param dim - the new dimensions
		*/
		virtual void setFOVDimensions(const core::vector3df& dim) {
			if (FieldOfView) FieldOfView->setDimensions(dim);
			if (DebugFOV) DebugFOV->setDimensions(dim);
		}
		/**
		\brief Returns the NPC's field of view dimensions
		\return a vector3df
		*/
		virtual void getFOVDimensions(core::vector3df& dim) { if (FieldOfView) dim = FieldOfView->getDimensions(); }
		/**
		\brief Returns whether the NPC is set to check occlusion with its field of view
		\return a bool
		*/
		virtual bool isUsingFOVOcclusion() { return FovOcclusionCheck; }
		/**
		\brief Sets whether the NPC should check occlusion with its field of view
		\param val - whether to check occlusion or not
		*/
		virtual void setUsesFOVOcclusion(bool val) { FovOcclusionCheck = val; }
		/**
		\brief Returns whether the NPC is set to check its field of view for enemies
		\return a bool
		*/
		virtual bool checksFOVForEnemies() { return CheckFovForEnemies; }
		/**
		\brief Sets whether the NPC should check its field of view for enemies
		\return val - whether to check for enemies or not
		*/
		virtual void setChecksFOVForEnemies(bool val) { CheckFovForEnemies = val; }
		/**
		\brief Returns whether the NPC is set to check its field of view for allies
		\return a bool
		*/
		virtual bool checksFOVForAllies() { return CheckFovForAllies; }
		/**
		\brief Sets whether the NPC should check its field of view for allies
		\return val - whether to check for allies or not
		*/
		virtual void setChecksFOVForAllies(bool val) { CheckFovForAllies = val; }
						
		/**
		\brief Returns whether the specified entity is visible to this NPC
		\param entity - the entity to check for
		*/
		virtual bool isVisibleToNPC(IAIEntity* entity) = 0;

	protected:
		f32 Range;
		bool FovOcclusionCheck;
		bool CheckFovForEnemies;
		bool CheckFovForAllies;
		IFieldOfView* FieldOfView;
		IDebugFOVSceneNode* DebugFOV;

};

} // end of IrrAI namespace

#endif /* INC_ICOMBATNPC_H */
