//#include <sdsl/rank_support_v.hpp>
#include <vector>
#include "xxhash.h"
#include <time.h>
#include "hw1/rank_support/rank_support.hpp"
#include <unordered_set>
#include <unordered_map>

using namespace std;
//using namespace sdsl;

typedef uint64_t size_type;

template <class KeyType> 
class bbhash;

static const size_type MAX_LEVEL = 25;

template <class KeyType> 
class bb_level
{
public:
	size_type size;
	size_type seed;
	bit_vector A;
	bit_vector C;
	
	rank_support r;


	// DEBUG
//	vector <KeyType> keys;
	
//	bbhash *bb;

	vector<KeyType> init(vector<KeyType> keys, double gamma)
	{
		this->size = keys.size() * gamma;
		this->seed = rand(); // TODO

		size_type sz = size;
		if (size < 4)
			sz = 4;
		A = bit_vector(sz, 0);
		C = bit_vector(sz, 0);

//		this->keys = keys;

		vector<KeyType> collisions;

		for (auto key: keys)
		{
			size_type i = get_hash(key);
//			cerr << i << endl;
			if (A[i] == 0 && C[i] == 0)
				A[i] = 1;
			else if (A[i] == 1) 
			{
				assert (C[i] == 0);
				A[i] = 0;
				C[i] = 1;
			}
		}

		for (auto key: keys)
		{
			size_type i = get_hash(key)% size;
			if (C[i] == 1)
				collisions.push_back(key);
		}
		//util::init_support(r, &A);

		r = rank_support(&A);

		return collisions;

	}


	size_type get_hash(KeyType key) // hash % size
	{
		ostringstream o;
		o << key;
		string s = o.str();

		XXH64_hash_t hash = XXH64(s.c_str(), s.size(), seed);	
		return hash % size;
	}

	size_type weight()
	{
		/*
		cerr << A << endl;
		cerr << r.size() << endl;
		cerr << r.rank(0) << endl;
		cerr << r.rank(1) << endl;
		cerr << r.rank(2) << endl;
		cerr << r.rank(3) << endl;
		cerr << r.rank(4) << endl;
		*/
	//	cerr << r.b->size() << " " << size-1 << endl;
		return r.rank1(size-1);
	}

	
};

//	
template <class KeyType>
class bbhash
{
	public:
    double gamma;
	size_type n;
	//vector<KeyType> keys;

	vector<bb_level<KeyType> > level;

	unordered_map<KeyType, size_type> terminal_map;
	


	bbhash(vector <KeyType> keys, double gamma)
    {
        {
            // better collision check TODO
            unordered_set<KeyType> uq(keys.begin(), keys.end());
            assert(uq.size() == keys.size());
        }
		//srand(time(NULL));
		n = keys.size();
//		this->keys = keys;
		this->gamma = gamma;

		assert (n > 0);

		vector <KeyType> collisions = keys;
		for (size_t i = 0; i < MAX_LEVEL && !collisions.empty(); i++)
		{
//			cerr << collisions.size() << endl;
			level.push_back(bb_level<KeyType>());
			collisions = level.back().init(collisions, gamma);
		}

		size_type all_weights = keys.size()-collisions.size();
		if (!collisions.empty())
        {
		    for (auto key: collisions)
                if (terminal_map.find(key) == terminal_map.end())
                {
                    size_type ind = terminal_map.size();
                    terminal_map[key] = all_weights+ind;
                }
        }

		//cerr << level.size() << endl;
//		for (auto l: level)
//		{
//			cerr << l.keys.size() << " ";
//			for(auto key: l.keys)
//				cerr << key << " ";
//			cerr << endl;
//		}
		/*
		for (size_t k = 0; k < level.size(); k++)
		{
			cerr << "X" << level[k].r.b->size() << endl;
			cerr << "D" << level[k].A.size() << endl;
		}
		*/
//		cout << "Test" << endl;
		
	}

	size_type get(KeyType key)
	{
		size_type sum = 0;
		for (size_t i = 0; i < level.size(); i++)
		{
			size_type ind = level[i].get_hash(key);
//			cerr << i << " " << ind << " " << level[i].A.size() << " " << level[i].r.b->size() << endl;
			if (level[i].A[ind] == 1)
			{
				return sum + level[i].r.rank1(ind)-1;
			}
			sum += level[i].weight();
		}
        if (terminal_map.find(key) != terminal_map.end())
            return terminal_map[key];
        sum += terminal_map.size();

        cerr << "Key not found: " << key << endl;
		return sum;
	}


	// in bytes
	size_type overhead_bitvectors()
    {
        size_type sum = 0;
        for (auto l: level)
            sum += l.size;
        return sum/8;
    }

	size_type overhead()
    {
	    size_type th = 0;
	    if (is_same<KeyType, string>::value)
	        for (auto it: terminal_map)
	            th += it.first.size()+sizeof(size_type);
	    else
            th = terminal_map.size() * (sizeof(KeyType) + sizeof(size_type));
	    return overhead_bitvectors() + th;
    }


	void print()
    {
	    cout << "n:" << n << " , gamma:" << gamma << endl;
	    cout << "levels:" << level.size() << endl;
	    for (size_t i = 0; i < level.size(); i++)
	        cout << "level-" << i << " : " << level[i].weight() << endl;
        cout << "terminal hash table size: " << terminal_map.size() << endl;

        cout << "bitvector overhead: " << overhead_bitvectors() << endl;
        cout << "all overhead: " << overhead() << endl;
    }

};

