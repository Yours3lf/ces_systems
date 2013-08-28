#ifndef ces_callback_h
#define ces_callback_h

#include <vector>
#include <list>

namespace ces
{
  //60 bytes
  union callback_data
  {
    struct
    {
      char data[60];
    };

    struct
    {
      char core[32];
      char aux[28];
    };

    struct
    {
      unsigned v4[15];
    };

    struct
    {
      unsigned long long v8[7];
      unsigned dummy;
    };
  };

  //64 bytes
  struct callback_pack
  {
    unsigned type;
    callback_data cbd;
  };

  class callback_base
  {
  public:
    virtual bool operator()( const callback_pack& cbd ){ return false; }
    virtual callback_base* clone()
    {
      return 0;
    }
  };

  template< class t >
  class callback : public callback_base
  {
    t func;
  public:
    bool operator()( const callback_pack& cbd )
    {
      return func( cbd );
    }

    callback* clone()
    {
      return new callback( *this );
    }

    callback( t f ) : func( f ) {}
  };

  class callback_manager
  {
  private:
    std::list< callback_base* > callbacks;
    std::vector< callback_pack > events;
  protected:
    callback_manager(){}; //singleton
    callback_manager(const callback_manager&);
    callback_manager(callback_manager&&);
    callback_manager& operator=(const callback_manager&);
  public:
    template< class t >
    void add_callback( t cb )
    {
      callbacks.push_back( new callback< t >( cb ) );
    }

    void add_event( const callback_pack& cbp )
    {
      events.push_back( cbp );
    }

    void dispatch_callbacks()
    {
      auto cnt = 0;
      for( auto c = events.begin(); c != events.end(); ++cnt )
      {
        bool found = false;
        for( auto d = callbacks.begin(); d != callbacks.end(); ++d )
        {
          if( (**d)( *c ) ) //event handled
          {
            events.erase( c );

            if( events.size() > 0 )
            {
              c = events.begin();
              for( int i = 0; i < cnt; ++i );
            }
            else
            {
              c = events.begin();
            }

            found = true;

            break;
          }
        }

        if( !found )
        {
          ++c;
        }
      }
    }

    ~callback_manager()
    {
      for( auto c = callbacks.begin(); c != callbacks.end(); ++c )
      {
        delete *c;
      }
    }

    static callback_manager& callback_manager::get()
    {
      static callback_manager instance;
      return instance;
    }
  };
}

#endif