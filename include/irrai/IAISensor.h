#ifndef INC_IAISENSOR_H
#define INC_IAISENSOR_H

#include "IAIEntity.h"

namespace IrrAI {

/**
\brief Enum describing AI sensor event types
*/
enum E_AISENSOR_EVENT_TYPE {

	/**
	An event to specify that an entity has entered the sensor
	*/
	EAISET_ENTER,
	/**
	An event to specify that an entity has exited the sensor
	*/
	EAISET_EXIT,
	/**
	Not an actual event type but a means to easily know the number of possible event types
	*/
	E_NUM_AISENSOR_EVENT_TYPES
};

/**
\brief Interface to implement AI sensors from
*/
class IAISensor : public IAIEntity {
  
	public:
		/**
		\brief Constructor
		\param desc - description of the sensor
		\param aimgr - the AI manager managing this sensor
		\param smgr - the scene manager of the scene the sensor will be added to
		\param id - the id of the entity
		*/
		IAISensor(const SAIEntityDesc& desc, IAIManager* aimgr, scene::ISceneManager* smgr, E_AIENTITY_TYPE type, s32 id) : IAIEntity(desc, aimgr, smgr, type, id) {
			CallbackPtr = NULL;
		}
		/**
		\brief Destructor
		*/
		virtual ~IAISensor() {
			for (u32 i = 0 ; i < Entities.size() ; ++i)
				delete Entities[i];
			Entities.clear();
		}
		/**
		\brief Sets the callback function to receive sensor alerts
		\param cb - the callback
		*/
		virtual void setCallback(void (*cb)(IAISensor*,IAIEntity*,E_AISENSOR_EVENT_TYPE)) { CallbackPtr = cb; }
		/**
		\brief Adds the specified entity to the group of entities that the sensor should be sensing
		\param entity - the entity
		*/
		virtual void addEntity(IAIEntity* entity) {
			if (!entity) return;
			SSensorData* data = new SSensorData;
			data->Entity = entity;
			Entities.push_back(data);
		}
		/**
		\brief Removes the specified entity from the group of entities that the sensor should be sensing
		\param entity - the entity
		*/
		virtual void removeEntity(IAIEntity* entity) {
			for (u32 i = 0 ; i < Entities.size() ; ++i)
				if (Entities[i]->Entity == entity) {
					delete Entities[i];
					Entities.erase(i);
					break;
				}
		}
    
	protected:
		struct SSensorData {
			SSensorData() {
				Entity = NULL;
			}
			IAIEntity* Entity;
		};

		core::array<SSensorData*> Entities;      
          
		void (*CallbackPtr)(IAISensor*,IAIEntity*,E_AISENSOR_EVENT_TYPE);
      
};

} // end of IrrAI namespace

#endif /* INC_IAISENSOR_H */
