#ifndef object_manager_h
#define object_manager_h

#include <vector>

namespace om
{

/**/
//64 bit IDs
#define INDEX_MASK 0xffffffff
#define INNER_MASK UINT_MAX
#define NEW_OBJECT_ID_ADD 0x100000000
typedef unsigned long long int id_type;
typedef unsigned long int inner_id_type;
/**/

/**
//32 bit IDs 
#define INDEX_MASK 0xffff
#define INNER_MASK USHRT_MAX
#define NEW_OBJECT_ID_ADD 0x10000
typedef unsigned long int id_type;
typedef unsigned short int inner_id_type;
/**/

//32 + 16 + 16 bits
//OR
//64 + 32 + 32 bits
struct index
{
	id_type id; //unique object identifier
	inner_id_type idx; //index to the object buffer
	inner_id_type next; //next free index
	index( id_type i = INDEX_MASK, inner_id_type n = INDEX_MASK, inner_id_type ix = INDEX_MASK ) :
		id( i ), idx( ix ), next( n ) {}
};

static void writeout_bits( om::id_type id )
{
  using namespace std;
  om::id_type siz = sizeof( id ) * 8;
  om::id_type one = 1;

  cout << id << ": ";

  for( om::id_type c = 0; c < siz; ++c )
  {
    cout << bool( id & ( one << ( siz - 1 - c ) ) );
  }

  cout << endl;
}

template< class t >
class object_manager
{
private:
	typedef std::pair< id_type, t > stored_type;
	std::vector< stored_type > objects;
	std::vector< index > indices;
	inner_id_type freelist_enqueue;
	inner_id_type freelist_dequeue;
protected:
public:
  typedef typename std::vector< stored_type >::iterator iter;

	bool has( id_type id )
	{
		index& in = indices[id & INDEX_MASK];
		return in.id == id && in.idx != INNER_MASK;
	}

	t& lookup( id_type id )
	{
		return objects[indices[id & INDEX_MASK].idx].second;
	}

	id_type add( const t& d )
	{
		//no indices stored, or no space left for more indices
		if( freelist_dequeue == INNER_MASK || freelist_dequeue == indices.size() )
		{
			freelist_dequeue = indices.size();
			indices.push_back( index( indices.size(), indices.size() + 1 ) );
		}

		index& in = indices[freelist_dequeue];
		freelist_dequeue = in.next;
		in.id += NEW_OBJECT_ID_ADD;
		in.idx = objects.size();
		objects.push_back( stored_type( 0, d ) );
		stored_type& o = objects[in.idx];
		o.first = in.id;
		return in.id;
	}

	void remove( id_type id )
	{
		index& in = indices[id & INDEX_MASK];

		stored_type& o = objects[in.idx];
		o = objects[objects.size() - 1];
		indices[o.first & INDEX_MASK].idx = in.idx;
		objects.resize( objects.size() - 1 );
		in.idx = INNER_MASK;

		//no indices stored, or no space left for more indices
		if( freelist_enqueue == INNER_MASK || freelist_enqueue == indices.size() )
		{
			freelist_enqueue = indices.size() - 1;
		}

		indices[freelist_enqueue].next = id & INDEX_MASK;
		freelist_enqueue = id & INDEX_MASK;
	}

	std::vector< stored_type >& get_objects()
	{
		return objects;
	}

  iter begin()
  {
    return objects.begin();
  }

  iter end()
  {
    return objects.end();
  }

	object_manager()
	{
		freelist_enqueue = INNER_MASK;
		freelist_dequeue = INNER_MASK;
	}
};

}

#endif