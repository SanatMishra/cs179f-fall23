// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct {
  struct spinlock lock;
  uint64 ref[MAXPAGES];
} refcnt;

void
refcntAL() {
  acquire(&refcnt.lock);
}

void
refcntRL() {
  release(&refcnt.lock);
}

int
refcntInd(void* arg) {
  return (PGROUNDDOWN((uint64)arg) - PGROUNDUP((uint64)end))/PGSIZE;
}

void
refcntInc(void* arg, int k) {
  refcnt.ref[refcntInd(arg)] += k;
}

uint64
refcntGet(int k) {
  uint64 ret = refcnt.ref[k];
  return ret;
}

void
refcntSet(int k, uint64 m) {
  refcnt.ref[k] = m;
}

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&refcnt.lock, "refcnt");
  acquire(&refcnt.lock);
  for (int i = 0; i <= refcntInd((void*)PHYSTOP); i++) {
    refcntSet(i, 1); // to make kfree think we're freeing pages
  }
  release(&refcnt.lock);
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&refcnt.lock);
  uint64 ri = refcntInd(pa);

  if (refcntGet(ri) < 1)
    panic("kfree refcnt");

  refcntInc(pa, -1);
  if (refcntGet(ri) == 0) {    
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);
  
    r = (struct run*)pa;
  
    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
  release(&refcnt.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    acquire(&refcnt.lock);
    if (refcntGet(refcntInd((void*)r)) != 0)
      panic("kalloc refcnt");
    refcntSet(refcntInd((void*)r), 1);
    memset((char*)r, 5, PGSIZE); // fill with junk
    release(&refcnt.lock);
  }
  return (void*)r;
}
