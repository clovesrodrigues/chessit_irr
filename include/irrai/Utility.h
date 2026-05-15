#ifndef INC_UTILITY_H
#define INC_UTILITY_H

#include <irrlicht.h>

using namespace irr;

namespace IrrAI {

/**
\brief Reads out four RGBA values from the specified buffer and stores them in the specified SColor
\param readBuffer - the buffer to read from
\param bufferSize - the size of the buffer
\param col - the receiving SColor
*/
void getColourFrom(const c8* readBuffer, s32 bufferSize, video::SColor* col);
/**
\brief Reads out three float values from the specified buffer and stores them in the specified vector3df
\param readBuffer - the buffer to read from
\param bufferSize - the size of the buffer
\param vec - the receiving vector3df
*/
void getVector3dfFrom(const c8* readBuffer, s32 bufferSize, core::vector3df* vec);
/**
\brief Reads out two float values from the specified buffer and stores them in the specified dimension2df
\param readBuffer - the buffer to read from
\param bufferSize - the size of the buffer
\param dim - the receiving dimension2df
*/
void getDimension2dfFrom(const c8* readBuffer, s32 bufferSize, core::dimension2df* dim);

} // end of IrrAI namespace

#endif /* INC_UTILITY_H */
