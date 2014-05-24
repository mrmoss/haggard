/** 
@file
@brief Producer-Consumer Queues
@ingroup Machine

This queue implementation enables a producer and a consumer to
communicate via a queue.  The queues are optimized for this situation,
they don't require any operating system locks (they do require 32-bit
reads and writes to be atomic.)  Cautions: there can only be one
producer, and one consumer.  These queues cannot store null pointers.
***************************************************************************/

/**
 * \addtogroup Machine
 * @{
 */

#ifndef __PCQUEUE__
#define __PCQUEUE__


/* Minimalist, stripped-down SMP memory interface. 
  Something like HP's Atomic_ops would be much better:
  	http://www.hpl.hp.com/research/linux/qprof/download.php4

  For rationale and other platforms, see:
	http://upc.gwu.edu/~upc/upcworkshop04/bonachea-consis-0904.pdf
*/

/* GCC x86-specific inline assembly implementation: */

/** Atomically increment this integer. */
#define CmiMemoryAtomicIncrement(someInt) asm("lock incl %0" :: "m" (someInt))
/** Atomically decrement this integer. */
#define CmiMemoryAtomicDecrement(someInt) asm("lock decl %0" :: "m" (someInt)) 

/** Make the changes at this address range visible to other CPUs. 
    (on x86, any locked instruction will do for reads or writes.) */
#define CmiMemoryWriteFence(startPtr,nBytes) asm("lock addl $0,(%esp)")	

/** Make any changes to this address range made by other CPUs visible to us. */
#define CmiMemoryReadFence(startPtr,nBytes) CmiMemoryWriteFence(startPtr,nBytes)

/** This data type is at least one cache line of padding, used to avoid
   cache line thrashing on SMP systems.  On x86, this is just for performance;
   on other CPUs, this can affect your choice of fence operations.
*/
typedef struct CmiMemorySMPSeparation_t {
	unsigned char padding[128];
} CmiMemorySMPSeparation_t;

/*****************************************************************************
 * #define PCQUEUE_LOCK
 * PCQueue doesn't need any lock, the lock here is only 
 * for debugging and testing purpose! it only make sense in smp version
 ****************************************************************************/
#undef PCQUEUE_LOCK

/**
  This is how many pointers are allocated per CircQueueStruct.
*/
#define PCQueueSize 0x100

/**
  This struct holds a fixed-size set of "char *data" pointers.
  The "Circ" name is somewhat misleading, since these
  objects do not form a circular queue, and are only 
  recycled if the 
*/
typedef struct CircQueueStruct
{
  /** The next entry in our linked list of structs.
      Written by Push when expanding the linked list.  
      Read by Pop when done reading each CircQueue. */
  struct CircQueueStruct *next;
  /** The next data index for Pop routine to read from. 
      If pull==PCQueueSize, 
      Written by Push only when expanding the queue.
      Read and written by Pop normally.
  */
  int pull;
  /** Pointers to user data.  Unused pointers are set to 0.
      Pointers are read from index pull by Pop routine, then set to zero.
      Pointers are written to index push by Push routine.
  */
  
  CmiMemorySMPSeparation_t pad1; /* separate "pull" from "push" */
  
  /** The next data index for Push routine to write into. 
      Read and written only by Push.
  */
  int push;
  
  CmiMemorySMPSeparation_t pad2; /* separate "push" from "data" */
  
  char *data[PCQueueSize];
  
}
*CircQueue;

/** 
  This struct holds a variable-size linked list of CircQueue structs.
  This is the only outside-visible name.
*/
typedef struct PCQueueStruct
{
  /**
    The first CircQueue in our linked list.  
    Read and written only by Pop, which reads data pointers from it.
  */
  CircQueue head;
  CmiMemorySMPSeparation_t pad1; /* separate "head" from "tail" */
  /**
    The last CircQueue in our linked list.
    Read and written only by Push, which writes data pointers into it.
    It is possible for head==tail.
  */
  CircQueue tail;
  /**
    The number of user data pointers in our queue.  This should equal
    (# of items in head) + (# of items between head and tail) + (# in tail)
    = (PCQueueSize - head->pull) + (PCQueueSize*(listlength-2)) + tail->push
    
    This field is incremented by Push and decremented by Pop.
  */
  int  len;
  
  /** An optional SMP lock, used for debugging PCQueue itself. */
#ifdef PCQUEUE_LOCK
  CmiNodeLock  lock;
#endif
}
*PCQueue;



/************* UNUSED CircQueue recycling ***************/
#if 0 /* recycle CircQueue structs, to speed up messaging. */
static CircQueue Cmi_freelist_circqueuestruct = 0;
static int freeCount = 0;

/* Make "dg" point to a new, zero-initialized CircQueue struct. */
#define MallocCircQueueStruct(dg) {\
  CircQueue d;\
  CmiMemLock();\
  d = Cmi_freelist_circqueuestruct;\
  if (d==(CircQueue)0){\
    d = ((CircQueue)calloc(1, sizeof(struct CircQueueStruct)));\
  }\
  else{\
    freeCount--;\
    Cmi_freelist_circqueuestruct = d->next;\
    }\
  dg = d;\
  CmiMemUnlock();\
}

/* Dispose of this CircQueue struct. */
#define FreeCircQueueStruct(dg) {\
  CircQueue d;\
  CmiMemLock();\
  d=(dg);\
  d->next = Cmi_freelist_circqueuestruct;\
  Cmi_freelist_circqueuestruct = d;\
  freeCount++;\
  CmiMemUnlock();\
}

#else /* common case--no CircQueue recycling. */
#define MallocCircQueueStruct(dg) dg=(CircQueue)calloc(1, sizeof(struct CircQueueStruct))
#define FreeCircQueueStruct(dg) free(dg)
#endif
/************* end UNUSED recycling code *******************/

/**
  Create a new PCQueue object.  
  
  This routine cannot execute concurrently with either Push or Pop.
*/
PCQueue PCQueueCreate(void)
{
  CircQueue circ;
  PCQueue Q;

  MallocCircQueueStruct(circ);
  
  Q = (PCQueue)malloc(sizeof(struct PCQueueStruct));
  Q->head = circ;
  Q->tail = circ;
  Q->len = 0;
#ifdef PCQUEUE_LOCK
  Q->lock = CmiCreateLock();
#endif
  return Q;
}

/**
  Return 1 if this PCQueue has no data pointers.
  This function can execute concurrently with any other function.
*/
int PCQueueEmpty(PCQueue Q)
{
  CircQueue circ = Q->head;
  char *data = circ->data[circ->pull];
  return (data == 0);
}

/**
  Return the length of this PCQueue.
  This function can execute concurrently with any other function.
*/
int PCQueueLength(PCQueue Q)
{
  return Q->len;
}

/**
  Return the next user pointer in the PCQueue "Q".
  The returned pointer value will only be NULL if the queue is empty.
  Multiple threads cannot "Pop" the same queue simultaniously, but
  "Push" and "Pop" can be run simultaniously.
*/
char *PCQueuePop(PCQueue Q)
{
  CircQueue circ; int pull; char *data;

#ifdef PCQUEUE_LOCK
    CmiLock(Q->lock);
#endif
    circ = Q->head;
    
    /* This fence is needed so we can see the modified "data" pointer
       written by the "Push" function. */
    CmiMemoryReadFence(circ,sizeof(*circ));
    
    pull = circ->pull;
    data = circ->data[pull];
    if (data) 
    { /* the queue is not empty--advance over this data pointer */
      circ->pull = (pull + 1);
      circ->data[pull] = 0;
      if (pull == PCQueueSize - 1) { /* just pulled the data from the last slot
                                     of this buffer */
        /* This fence is needed so we can see the modified "next" pointer
          written by the "Push" function. */
        CmiMemoryReadFence(circ,sizeof(*circ));
        Q->head = circ-> next; /* next buffer must exist, because "Push"  */
	                       /* links in the next buffer *before* filling */
                               /* in the last slot. See below. */
        FreeCircQueueStruct(circ);
      }
      CmiMemoryAtomicDecrement(Q->len);
    }
    /* else data==0, so queue seems to be empty. 
       The producer may be adding something to it, 
       but its ok to report queue is empty. */
#ifdef PCQUEUE_LOCK
    CmiUnlock(Q->lock);
#endif
    return data;
}

/**
  Add the user pointer "data" to the PCQueue "Q".
  "data" cannot be NULL.
  Multiple threads cannot "Push" into the same queue simultaniously, but
  "Push" and "Pop" can be run simultaniously.
*/
void PCQueuePush(PCQueue Q, char *data)
{
  CircQueue circ1; int push;
  
#ifdef PCQUEUE_LOCK
  CmiLock(Q->lock);
#endif
  circ1 = Q->tail;
  push = circ1->push;
  if (push == (PCQueueSize -1)) { /* last slot is about to be filled */
    /* this way, the next buffer is linked in before data is filled in 
       in the last slot of this buffer */
    CircQueue newcirc;
    MallocCircQueueStruct(newcirc);
    /* We need to make sure the new struct's freshly-zeroed
       "next", "pull", and "data" pointers are visible
       before we can link the struct in from outside. */
    CmiMemoryWriteFence(newcirc,sizeof(*newcirc));
    
    Q->tail->next = newcirc;
    Q->tail = newcirc;
    /* Now we need to make sure our modification to "next" is visible 
       before we can push the last slot full. */
    CmiMemoryWriteFence(Q,sizeof(*Q));
  }
  circ1->data[push] = data;
  circ1->push = (push + 1);
  CmiMemoryAtomicIncrement(Q->len);
  /* Eventually we need to make our "data" pointer modification visible */
  CmiMemoryWriteFence(Q,sizeof(*Q));
#ifdef PCQUEUE_LOCK
  CmiUnlock(Q->lock);
#endif
}


#endif

/*@}*/
