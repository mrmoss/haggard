/*
Orion's Standard Library
Orion Sky Lawlor, olawlor@acm.org, 5/4/2002

Simple hashtable class.  Maps a pointer-to-key
to a pointer-to-object.
*/

#ifndef __OSL_HASHTABLE_H
#define __OSL_HASHTABLE_H

namespace osl {
	
class hashtableEntry {
	const void *key;
	void *object;
public:
	hashtableEntry() {empty();}

	inline void empty(void) {key=0; object=0;}
	inline bool isEmpty(void) const {return key==0;}

	inline void setKey(const void *k) {key=k;}
	inline void setObject(void *o) {object=o;}
	
	inline const void *getKey(void) const {return key;}
	inline void *getObject(void) const {return object;}
};

class hashtableIterator {
	int cur,len;
	const hashtableEntry *table;
public:
	hashtableIterator(const hashtableEntry *table_,int len_)
		:cur(0),len(len_),table(table_) {}
	void *next(const void **nextKey=0);
	inline void *next(void **nextKey) {
		return next((const void **)nextKey);
	}
};

///This function maps the user's key to a random integer
typedef unsigned int hashcode;
typedef hashcode (*hashFn)(const void *keyPtr);

///This function compares two user keys
typedef bool (*compareFn)(const void *key1,const void *key2);

class hashtable {
	int nEnt,resizeEnt; //Number of entries, when to resize at
	float loadFactor; //Fraction of entries to actually use
	hashFn hash; compareFn compare;
	hashtableEntry *allocTable; //Heap-allocated table, or 0
	
	
	void freeOld(void); //Free old table
	void build(int newLen); //Build a new table
	void resize(int newLen); //Change the size 

protected:
	int len; //Number of entries in table
	hashtableEntry *table;
	///Increment i around the table
	int inc(int i) const {
		i++;
		if (i==len) return 0;
		return i;
	}
public:
	hashtable(int len_,float loadFactor_,
		hashFn hash_,compareFn compare_,
		hashtableEntry *table_=0);
	~hashtable();

	//Low-level lookup:
	hashtableEntry *lookup(const void *key,bool addIfMissing);
	
	///Lookup the object associated with this key
	inline void *get(const void *key) {
		hashtableEntry *e=lookup(key,false);
		if (e) return e->getObject();
		else return 0;
	}
	
	///Associate this object with this key.  Returns the
	/// old object, or NULL if there was none.
	inline void *set(const void *key,void *obj) {
		hashtableEntry *e=lookup(key,true);
		void *oldObj=e->getObject();
		e->setObject(obj);
		return oldObj;
	}
	
	///Synonym for set
	inline void *put(const void *key,void *obj) {
		return set(key,obj);
	}

	///Remove this key from the table
	void remove(const void *key);

	///Return the number of keys in the table:
	int getKeys(void) const {return nEnt;}
	
	hashtableIterator getIterator(void) const {
		return hashtableIterator(table,len);
	}
};

template<class KEY,class OBJ,class INLINENAMESPACE>
class hashtableT : public hashtable {
public:
	hashtableT(int len_,float loadFactor_,
		hashFn hash_,compareFn compare_,
		hashtableEntry *table_=0)
		:hashtable(len_,loadFactor_,hash_,compare_,table_) {}
	
	inline OBJ *get(const KEY *key) {
		return (OBJ *)hashtable::get(key);
	}
	inline OBJ *set(const KEY *key,OBJ *obj) {
		return (OBJ *)hashtable::set(key,obj);
	}
	
	inline OBJ *getInline(const KEY *key) {
	  //typename INLINENAMESPACE;
	  int i=INLINENAMESPACE::hash(key)%len;
	  while (1) {
		hashtableEntry *slot=&table[i];
		if (slot->isEmpty()) 
			return 0;
		if (INLINENAMESPACE::compare(key,slot->getKey()))
			return (OBJ *)slot->getObject();
		i=inc(i); //Look in the next slot
	 }
	 return 0;
	}
};


};

#endif /* __OSL_H*/

