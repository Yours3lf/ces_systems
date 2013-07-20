#include <iostream>
#include <list>

#include "object_manager.h"

//#define USE_TYPE_B
#ifdef USE_TYPE_B

/*
 * CES SYSTEM IMPLEMENTATION "B"
 * 
 * In type B:
 * Systems are separate, each contain the components they act on.
 * They are wrapped around by a system-manager.
 * Entities are stored in and entity-manager.
 * Each entity contains an ID that identifies which components belong to it.
 * Therefore finding each component of an entity takes a bit longer, but each component's type is 'known'
 * 
 * System Manager
 *   -Systems
 *     -Components
 *       -ID
 *       -entity-id
 *       -data
 *     -type-id
 * 
 * Entity manager
 *   -Entites
 *     -ID
 */

using namespace std;

/*
 * Components only contain data, no logic (only constructors/destructors maybe)
 */
namespace ces
{
namespace component
{
  class base
  {
  public:
    om::id_type id; //entity id, assigned by a system
  };

  class pos : public base
  {
  public:
    float x, y, z;
    pos(float a = float(), float b = float(), float c = float()) : x(a), y(b), z(c) {}
  };

  class name : public base
  {
  public:
    string str;
    name(const string& n = string()) : str(n) {}
  };
}

/*
 * Entities. In this type, they don't hold any data or logic, only an ID.
 */
namespace entity
{
  class base
  {
  public:
    om::id_type id;
  };

  class manager
  {
    om::object_manager< base > entities; //collection of entities
  private:
  protected:
    manager(){} //singleton
    manager(const manager&);
    manager(manager&&);
    manager& operator=(const manager&);
  public:
    om::id_type add()
    {
      return entities.add(base());
    }

    base& get(om::id_type id)
    {
      return entities.lookup(id);
    }

    void remove(om::id_type id)
    {
      entities.remove(id);
    }

    om::object_manager< base >& get_data()
    {
      return entities;
    }

    static manager& get()
    {
      static manager instance;
      return instance;
    }
  };
}

/*
 * In this type of CES, systems hold the data (components).
 */
namespace system
{
  class base
  {
  public:
    virtual void init(){}
    virtual void shutdown(){}
    virtual void update(){}
    //no need for type IDs
  };

  class pos : public base //there is a system for each component type
  {
    om::object_manager< component::pos > components;
  public:
    om::id_type add(om::id_type entity_id)
    {
      auto tmp = components.add(component::pos());
      components.lookup(tmp).id = entity_id;
      return tmp;
    }

    component::pos& get(om::id_type id)
    {
      return components.lookup(id);
    }

    void remove(om::id_type id)
    {
      components.remove(id);
    }

    void update()
    {
      for( auto c = components.begin(); c != components.end(); ++c )
      {
        cout << c->second.x << " " << c->second.y << " " << c->second.z << endl; //perform something on them
      }
    }
  };

  class name : public base
  {
    om::object_manager< component::name > components;
  public:
    om::id_type add(om::id_type entity_id)
    {
      auto tmp = components.add(component::name());
      components.lookup(tmp).id = entity_id;
      return tmp;
    }

    component::name& get(om::id_type id)
    {
      return components.lookup(id);
    }

    void remove(om::id_type id)
    {
      components.remove(id);
    }

    void update()
    {
      for( auto c = components.begin(); c != components.end(); ++c )
      {
        cout << c->second.str.c_str() << endl;
      }
    }
  };

  //this is needed so that we can neatly just call tell this manager to update/init etc., 
  //no need to know about the systems
  class manager
  {
    list< base* > systems; 
  private:
  protected:
    manager(){} //singleton
    manager(const manager&);
    manager(manager&&);
    manager& operator=(const manager&);
  public:
    void add( base* c )
    {
      systems.push_back(c);
    }

    void update()
    {
      for( auto c = systems.begin(); c != systems.end(); ++c )
        (*c)->update();
    }

    void init()
    {
      for( auto c = systems.begin(); c != systems.end(); ++c )
        (*c)->init();
    }

    void shutdown()
    {
      //destroy in reverse order
      for( auto c = systems.rbegin(); c != systems.rend(); ++c )
      {
        (*c)->shutdown();
        delete *c;
      }
    }

    static manager& get()
    {
      static manager instance;
      return instance;
    }
  };
}
}

//usage
int main()
{
  auto pos_sys = new ces::system::pos; //systems will either need to be stored in a map, or kept around to access them
  auto name_sys = new ces::system::name;
  ces::system::manager::get().add(pos_sys);
  ces::system::manager::get().add(name_sys);

  om::id_type entity_with_pos = ces::entity::manager::get().add();
  om::id_type pos_component1 = pos_sys->add(entity_with_pos);
  auto& pc1 = pos_sys->get(pos_component1);
  pc1.x = 1;
  pc1.y = 2;
  pc1.z = 3;

  om::id_type entity_with_name = ces::entity::manager::get().add();
  om::id_type name_component1 = name_sys->add(entity_with_name);
  auto& nc1 = name_sys->get(name_component1);
  nc1.str = "hello world";

  om::id_type entity_with_pos_and_name = ces::entity::manager::get().add();
  om::id_type pos_component2 = pos_sys->add(entity_with_pos_and_name);
  auto& pc2 = pos_sys->get(pos_component2);
  pc2.x = 4;
  pc2.y = 5;
  pc2.z = 6;
  om::id_type name_component2 = name_sys->add(entity_with_pos_and_name);
  auto& nc2 = name_sys->get(name_component2);
  nc2.str = "world hello lolwut?";

  ces::system::manager::get().init();
  ces::system::manager::get().update();
  ces::system::manager::get().shutdown();

	cin.get();
	return 0;
}

#endif