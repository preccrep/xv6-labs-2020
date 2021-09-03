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

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  int rc[PHYSTOP / PGSIZE];  //reference count
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    kmem.rc[(uint64)p / PGSIZE] = 1;
    kfree(p);
  }
}

void
increase_rc(uint64 pa) {
  acquire(&kmem.lock);
  kmem.rc[pa / PGSIZE]++;
  release(&kmem.lock);
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
  acquire(&kmem.lock);
  kmem.rc[(uint64)pa / PGSIZE]--;
  if(kmem.rc[(uint64)pa / PGSIZE] <= 0) {
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);
    r = (struct run*)pa;
    r->next = kmem.freelist;
    kmem.freelist = r;
  }
  release(&kmem.lock);
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
  if(r) {
    kmem.freelist = r->next;
    kmem.rc[(uint64)r / PGSIZE] = 1;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

int
cow_alloc(pagetable_t pgt, uint64 va) {
  uint64 pa;
  uint64 mem;
  pte_t *pte;
  if (va >= MAXVA)
    return -1;
  va = PGROUNDDOWN(va);
  pte = walk(pgt, va, 0);
  if (pte == 0) {
    return -1;
  }
  // not a valid cow page
  if (!(*pte & PTE_V)) {
    return -2;
  }
  pa = PTE2PA(*pte);

  // only one rf, make it writable
  acquire(&kmem.lock);
  if (kmem.rc[pa / PGSIZE] == 1) {
    *pte &= ~PTE_COW;
    *pte |= PTE_W;
    release(&kmem.lock);
    return 0;
  }
  release(&kmem.lock);
  if ((mem = (uint64)kalloc()) == 0){
    return -3;
  }
  memmove((void *)mem, (void *)pa, PGSIZE);
  *pte = ((PA2PTE(mem) | PTE_FLAGS(*pte) | PTE_W) & (~PTE_COW));
  // decrease rc
  kfree((void *)pa);
  return 0;
}
