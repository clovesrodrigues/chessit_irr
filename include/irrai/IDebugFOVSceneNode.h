#ifndef INC_IDEBUGFOVSCENENODE_H
#define INC_IDEBUGFOVSCENENODE_H

#include <irrlicht.h>

using namespace irr;

namespace IrrAI {

/**
\brief Interface to use for visualising different types of field of view
*/
class IDebugFOVSceneNode : public scene::ISceneNode {

	public:
		/**
        \brief Constructor
        \param parent - parent node
        \param mgr - associated scene manager
        \param id - desired id
        */
		IDebugFOVSceneNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id) : scene::ISceneNode(parent, mgr, id) {};
		/**
        \brief Destructor
		*/
		virtual ~IDebugFOVSceneNode() {}
		/**
        \brief Returns the axis aligned bounding box of this node
        \return an core::aabbox3d<f32>
        */
		virtual const core::aabbox3d<f32>& getBoundingBox() const { return Box; }
		/**
		\brief Sets the dimensions of the field of view
		\param dim - the new dimensions
		*/
		virtual void setDimensions(const core::vector3df& dim) { Dimensions = dim; }

	protected:
        core::aabbox3d<f32> Box;
		core::vector3df Dimensions;

};

} // end of IrrAI namespace

#endif /* INC_IDEBUGFOVSCENENODE_H */
