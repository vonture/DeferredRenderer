#pragma once

#include "PCH.h"

typedef UINT EntityID;
typedef std::set<EntityID> EntitySet;

class EntitySystem
{
private:
	EntityID _nextId;
		
	typedef size_t ComponentTypeID;
	typedef std::map<EntityID, void*> EntityComponentMap;
	typedef std::map<ComponentTypeID, EntityComponentMap> ComponentMap;

	EntitySet _entities;
	ComponentMap _components;
	
	template <class componentType>
	ComponentTypeID getComponentTypeID()
	{
		return typeid(componentType).hash_code();
	}

public:
	EntitySystem() 
		: _nextId(0)
	{
	}

	~EntitySystem()
	{
		// Kill all the entities to make sure their components are removed
		while (_entities.size() > 0)
		{
			KillEntity(*_entities.begin());
		}
	}

	EntityID SpawnEntity()
	{
		EntityID id = _nextId++;
		_entities.insert(id);
		return id;
	}

	void KillEntity(EntityID id)
	{
		if (_entities.find(id) != _entities.end())
		{
			// Clear its components, iterate over the component types
			for (ComponentMap::iterator it = _components.begin(); it != _components.end(); it++)
			{
				// Check if this entity has a component of this type
				EntityComponentMap::iterator componentIt = it->second.find(id);
				if (componentIt != it->second.end())
				{
					// Found one, remove it
					delete componentIt->second;
					it->second.erase(componentIt);
				}
			}

			// Finally, remove the entity 
			_entities.erase(id);
		}
	}

	template <class componentType>
	componentType* AddComponent(EntityID id)
	{
		// Make sure this entity exists
		if (_entities.find(id) != _entities.end())
		{
			ComponentTypeID compId = getComponentTypeID<componentType>();

			// Create the component and insert it
			componentType* newComp = new componentType();
			_components[compId][id] = newComp;

			return newComp;
		}

		// Can't make a component for an entity that doesn't exist
		return NULL;		
	}

	template <class componentType>
	void RemoveComponent(EntityID id)
	{
		ComponentTypeID compId = getComponentTypeID<componentType>();

		// Check if this entity has a component of this type
		ComponentMap::iterator i = _components.find(compId);
		if (i != _components.end())
		{
			EntityComponentMap::iterator j = i->second.find(id);
			if (j != i->second.end())			
			{
				// Found the component, delete it and clear the pair
				delete j->second;
				i->second.erase(j);
			}
		}		
	}

	template <class componentType>
	componentType* GetComponent(EntityID id)
	{
		ComponentTypeID compId = getComponentTypeID<componentType>();
		ComponentMap::iterator mapIt = _components.find(compId);
		if (mapIt != _components.end())
		{
			// This type of component is in the map, return the entities in it
			EntityComponentMap::iterator comp = mapIt->second.find(id);
			if (comp != mapIt->second.end())
			{
				return static_cast<componentType*>(comp->second);
			}
		}

		return NULL;
	}

	template <class componentType>
	void GetEntities(EntitySet& result)
	{
		ComponentTypeID compId = getComponentTypeID<componentType>();

		// Look to make sure there is a bin for this type of component
		ComponentMap::iterator compMap = _components.find(compId);
		if (compMap != _components.end())
		{
			// Add all entities from the map
			for (EntityComponentMap::iterator it = compMap->second.begin(); it != compMap->second.end(); it++)
			{
				result.insert(it->first);
			}
		}
	}
};