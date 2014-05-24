/**
 Airspace and airspace restrictions.
 
 Orion Sky Lawlor, olawlor@acm.org, 2007-01-30 (Public Domain)
*/
#ifndef __OSL_REFCOUNTED_H
#define __OSL_REFCOUNTED_H

namespace osl {

/** A class that can be reference counted. 
	Classes are created with a reference count of zero.
	Calls to ref increment the reference count.
	Calls to unref decrement the reference count.
	When the reference count reaches zero, the class deletes itself.

Objects of type RefCounted:
	- Should only be allocated with "new", and deleted only with unref.
	- Should preferably be held inside a RefPtr.
*/
class RefCounted {
public:
	inline RefCounted() :refcount(0) {}
	inline ~RefCounted() {refcount=-9999;}
	
	/// Increment our reference count.
	inline void ref(void) {refcount++;}
	/// Decrement our reference count.
	/// When the reference count reaches zero, the class deletes itself.
	inline void unref(void) {if (--refcount==0) delete this;}
protected:
	int refcount;
};

/** A "smart pointer" that points to a RefCounted object or NULL. 
*/
template <class RefCountedType>
class RefPtr {
public:
	/// Constructor calls ref.
	RefPtr(RefCountedType *ptr_) :ptr(ptr_) {if (ptr) ptr->ref();}
	/// Copy constructor calls ref.
	RefPtr(const RefPtr<RefCountedType> &src) :ptr(src.ptr) {if (ptr) ptr->ref();}
	
	/// Destructor calls unref.
	~RefPtr() {if (ptr) ptr->unref();}
	
	/// Assignment operator releases old reference, adds new one.
	RefPtr<RefCountedType> &operator=(RefCountedType *newVal) {
		if (newVal) newVal->ref();
		if (ptr) ptr->unref();
		ptr=newVal;
		return *this;
	}
	
	/// Allow RefPtr to be used basically like a pointer:
	operator RefCountedType *() {return ptr;}
	operator const RefCountedType *() const {return ptr;}
	RefCountedType *operator->() {return ptr;}
	const RefCountedType *operator->() const {return ptr;}
private:
	RefCountedType *ptr;
};

};

#endif
