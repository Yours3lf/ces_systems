#include <iostream>
#include <list>

#include "object_manager.h"
#include "ces_callback.h"

#define USE_TYPE_A
#ifdef USE_TYPE_A

/*
 * CES SYSTEM IMPLEMENTATION "A"
 *
 * In type A:
 * Systems are separate, they only contain logic, no data. They are wrapped around by a system-manager.
 * Entities are stored in an entity-manager.
 * Each entity contains a vector of components, and an ID that identifies the entity.
 * Systems ask the entity-manager for entities to process each time update is called on them by the system-manager.
 * Note that entities and components don't store their ID directly, they are rather just identified by systems and other objects by it.
 *
 * System Manager
 *   -Systems
 *     -type-id
 *
 * Entity manager
 *   -Entites
 *     -ID
 *     -Components
 *       -ID
 *       -type-id
 *       -data
 */

using namespace std;

enum
{
  EVENT_TYPE_ONE, EVENT_TYPE_TWO
};

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
    om::id_type id; //type id, assigned by a system
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
 * Entities. In this type, they hold all the data, but no logic, systems process them.
 */
namespace entity
{
  class base
  {
    om::object_manager< component::base* > components; //collection of components
  public:
    om::id_type add(component::base* c)
    {
      return components.add(c);
    }

    component::base*& get(om::id_type id)
    {
      return components.lookup(id);
    }

    void remove(om::id_type id)
    {
      delete components.lookup(id);
      components.remove(id);
    }

    om::object_manager< component::base* >& get_data()
    {
      return components;
    }

    void shutdown()
    {
      for( auto c = components.begin(); c != components.end(); ++c )
      {
        delete c->second;
      }
    }
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
    
    void shutdown()
    {
      for( auto c = entities.begin(); c != entities.end(); ++c )
      {
        c->second.shutdown();
      }
    }

    static manager& get()
    {
      static manager instance;
      return instance;
    }
  };
}

/*
 * Systems should contain all the logic to make components work
 * In this type of CES, systems don't hold the data (components), rather entitys hold them.
 */
namespace system
{
  class base
  {
  public:
    virtual void init(){}
    virtual void shutdown(){}
    virtual void update(){}
    virtual om::id_type get_typeid(){return om::id_type();}
  };

  class pos : public base //there is a system for each component type
  {
    static om::id_type typ()
    {
      static char type;
      return om::id_type(&type);
    }
  public:
    static component::pos* create()
    {
      component::pos* c = new component::pos;
      c->id = typ(); //add type information to a component
      return c;
    }

    void init()
    {
      callback_manager::get().add_callback( [&]( callback_pack d )
      {
        if( d.type == EVENT_TYPE_TWO )
        {
          std::cout << "Event two: " << d.cbd.data << std::endl;
          return true;
        }

        return false;
      } );
    }

    void update()
    {
      for( auto c = ces::entity::manager::get().get_data().begin(); //go through all entities
           c != ces::entity::manager::get().get_data().end(); ++c )
      {
        for( auto d = c->second.get_data().begin(); //go through all components
             d != c->second.get_data().end(); ++d )
        {
          if(d->second->id == typ()) //if the type is the same
          {
            component::pos* p = static_cast<component::pos*>(d->second); //then cast the component to the right type
            cout << p->x << " " << p->y << " " << p->z << endl; //and perform something on them

            //send an event
            callback_pack cbp;
            cbp.type = EVENT_TYPE_ONE;
            cbp.cbd.v4[0] = p->x;
            cbp.cbd.v4[1] = p->y;
            callback_manager::get().add_event( cbp );
          }
        }
      }
    }

    om::id_type get_typeid()
    {
      return typ();
    }
  };

  class name : public base
  {
    static om::id_type typ()
    {
      static char type;
      return om::id_type(&type);
    }
  public:
    static component::name* create()
    {
      component::name* c = new component::name;
      c->id = typ();
      return c;
    }

    void init()
    {
      callback_manager::get().add_callback( [&]( callback_pack d )
      {
        if( d.type == EVENT_TYPE_ONE )
        {
          std::cout << "Event one: " << d.cbd.v4[0] << " " << d.cbd.v4[1] << std::endl;
          return true;
        }

        return false;
      } );
    }

    void update()
    {
      for( auto c = ces::entity::manager::get().get_data().begin();
           c != ces::entity::manager::get().get_data().end(); ++c )
      {
        for( auto d = c->second.get_data().begin();
             d != c->second.get_data().end(); ++d )
        {
          if(d->second->id == typ())
          {
            component::name* p = static_cast<component::name*>(d->second);
            cout << p->str.c_str() << endl;

            //send an event
            callback_pack cbp;
            cbp.type = EVENT_TYPE_TWO;
            memcpy( cbp.cbd.data, p->str.c_str(), p->str.size() + 1 );
            callback_manager::get().add_event( cbp );
          }
        }
      }
    }

    om::id_type get_typeid()
    {
      return typ();
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
  ces::system::manager::get().add(new ces::system::pos);
  ces::system::manager::get().add(new ces::system::name);

  om::id_type entity_with_pos = ces::entity::manager::get().add();
  auto pos_component1 = ces::system::pos::create();
  pos_component1->x = 1;
  pos_component1->y = 2;
  pos_component1->z = 3;
  ces::entity::manager::get().get(entity_with_pos).add(pos_component1);

  om::id_type entity_with_name = ces::entity::manager::get().add();
  auto name_component1 = ces::system::name::create();
  name_component1->str = "hello world";
  ces::entity::manager::get().get(entity_with_name).add(name_component1);

  om::id_type entity_with_pos_and_name = ces::entity::manager::get().add();
  auto pos_component2 = ces::system::pos::create();
  pos_component2->x = 4;
  pos_component2->y = 5;
  pos_component2->z = 6;
  auto name_component2 = ces::system::name::create();
  name_component2->str = "world hello lolwut?";
  ces::entity::manager::get().get(entity_with_pos_and_name).add(pos_component2);
  ces::entity::manager::get().get(entity_with_pos_and_name).add(name_component2);

  ces::system::manager::get().init();
  ces::system::manager::get().update();
  ces::callback_manager::get().dispatch_callbacks();
  ces::system::manager::get().shutdown();

  ces::entity::manager::get().shutdown();

  writeout_bits(entity_with_pos);

	cin.get();
	return 0;
}

#endif