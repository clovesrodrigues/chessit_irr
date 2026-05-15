#ifndef INC_IFIELDOFVIEW_H
#define INC_IFIELDOFVIEW_H

#include "IAIManager.h"

namespace IrrAI {

/**
\brief Interface for an entity's field of view, used to determine what other entities are visible to them 
*/
class IFieldOfView {
      
	public:
		/**
		\brief Constructor
		\param aimgr - the AI Manager to use for occlusion checks
		\param occlusionCheck - whether or not to check for occlusion
		\param dimensions - the dimensions of the field of view
		*/
		IFieldOfView(IAIManager* aimgr, bool occlusionCheck, const core::vector3df& dimensions) { 
			AIManager = aimgr;
			OcclusionCheck = occlusionCheck;
			Dimensions = dimensions;
		}
		/**
		\brief Destructor
		*/
		virtual ~IFieldOfView() {}

		/**
		\brief Returns true if the specified box is within the field of view, false otherwise
		\param box - the box to check
		\param boxPos - position of the box
		\return a bool
		*/
  		virtual bool isInFOV(const core::aabbox3df& box, const core::vector3df& boxPos) = 0;
  	  
  		/**
  		\brief Sets the position of the field of view
  		\param vec - position to be set
  		*/
  		virtual void setPosition(const core::vector3df& vec) { Position = vec; }
  		/**
  		\brief Returns the position of the field of view
  		\return a vector3df
  		*/
  		virtual const core::vector3df& getPosition() { return Position; }
  		/**
  		\brief Sets the rotation of the field of view
  		\param vec - rotation to be set
  		*/
  		virtual void setRotation(const core::vector3df& vec) { Rotation = vec; }
  		/**
  		\brief Returns the rotation of the field of view
  		\return a vector3df
  		*/
  		virtual const core::vector3df& getRotation() { return Rotation; }
  		/**
  		\brief Sets whether the field of view should check for occlusion or not
  		\param val - whether to check for occlusion
  		*/
  		virtual void setOcclusionCheck(bool val) { OcclusionCheck = val; }
  		/**
  		\brief Returns whether the field of view should check for occlusion or not
  		\return a bool
  		*/
  		virtual bool getOcclusionCheck() { return OcclusionCheck; }
		/**
		\brief Sets the dimensions of the field of view
		\param dim - the new dimensions
		*/
		virtual void setDimensions(const core::vector3df& dim) { Dimensions = dim; }
		/**
		\brief Returns the dimensions of the field of view
		\return a vector3df
		*/
		virtual const core::vector3df& getDimensions() { return Dimensions; }
		
	protected:
		IAIManager* AIManager;
		core::vector3df Position;
		core::vector3df Rotation;		
		core::vector3df Dimensions;
		bool OcclusionCheck;

};

} // end of IrrAI namespace

#endif /* INC_IFIELDOFVIEW_H */
