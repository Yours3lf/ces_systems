#include <iostream>
#include <list>

#include "object_manager.h"

/*
 * CES SYSTEM IMPLEMENTATION
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
    om::id_type id;
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
    om::object_manager< component::base* > components;
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
  };

  class manager
  {
    om::object_manager< base > entities;
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

  class pos : public base
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
      c->id = typ();
      return c;
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
            component::pos* p = static_cast<component::pos*>(d->second);
            cout << p->x << " " << p->y << " " << p->z << endl;
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
          }
        }
      }
    }

    om::id_type get_typeid()
    {
      return typ();
    }
  };

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
  ces::system::manager::get().shutdown();

	cin.get();
	return 0;
}
