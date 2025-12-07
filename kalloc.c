// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

// --- COW additions ---
static ushort refcnt[(PHYSTOP - KERNBASE) / PGSIZE];
static struct spinlock ref_lock;

// helper: convertir una dirección física a índice del arreglo
static inline int pa2idx(uint pa) {
  return (pa - KERNBASE) / PGSIZE;
}
// --- fin COW additions ---

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  initlock(&ref_lock, "refcnt");   // <-- NUEVO
  kmem.use_lock = 0;

  memset(refcnt, 0, sizeof(refcnt));  // <-- NUEVO

  freerange(vstart, vend);
}


void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  uint pa = V2P(v);
  int idx = pa2idx(pa);

  // --- COW logic: decrement refcount ---
  acquire(&ref_lock);
  if(refcnt[idx] > 1){
    refcnt[idx]--;          // todavía hay procesos usando esta página
    release(&ref_lock);
    return;                 // NO liberamos físicamente
  }
  // si es 0 o 1, liberamos de verdad
  refcnt[idx] = 0;
  release(&ref_lock);

  // --- liberar físicamente ---
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  if(kmem.use_lock)
    release(&kmem.lock);

  if(r){
    uint pa = V2P(r);
    int idx = pa2idx(pa);
    acquire(&ref_lock);
    refcnt[idx] = 1;   // <-- cada vez que se asigna, refcount = 1
    release(&ref_lock);
  }

  return (char*)r;
}


void
page_ref_inc(uint pa)
{
  int idx = pa2idx(pa);
  acquire(&ref_lock);
  refcnt[idx]++;
  release(&ref_lock);
}

// decrementa y devuelve el nuevo valor
int
page_ref_dec(uint pa)
{
  int idx = pa2idx(pa);
  int v;
  acquire(&ref_lock);
  if(refcnt[idx] > 0)
    refcnt[idx]--;
  v = refcnt[idx];
  release(&ref_lock);
  return v;
}

int
page_ref_get(uint pa)
{
  int idx = pa2idx(pa);
  int v;
  acquire(&ref_lock);
  v = refcnt[idx];
  release(&ref_lock);
  return v;
}
