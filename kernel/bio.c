// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 13
#define HASH(x) ((x) % (NBUCKET))

struct {
  struct spinlock lock[NBUCKET];
  struct buf buf[NBUF];
  struct spinlock eviction_lock;

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head[NBUCKET];
} bcache;

void
move_to_bucket(struct buf *b, uint blockno)
{
  int hash = HASH(blockno);
  b->next->prev = b->prev;
  b->prev->next = b->next;
  b->next = bcache.head[hash].next;
  b->prev = &bcache.head[hash];
  bcache.head[hash].next->prev = b;
  bcache.head[hash].next = b;
}

void
binit(void)
{
  struct buf *b;

  // Create linked list of buffers
  // bcache.head.prev = &bcache.head;
  // bcache.head.next = &bcache.head;
  for (int bid = 0; bid < NBUCKET; bid ++) {
    initlock(&bcache.lock[bid], "bcache.bucket");
  }
  initlock(&bcache.eviction_lock, "bcache eviction");

  for (int bid = 0; bid < NBUCKET; bid ++) {
    bcache.head[bid].prev = &bcache.head[bid];
    bcache.head[bid].next = &bcache.head[bid];
  }

  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head[0].next;
    b->prev = &bcache.head[0];
    initsleeplock(&b->lock, "buffer");
    b->time_stamp = ticks;
    bcache.head[0].next->prev = b;
    bcache.head[0].next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  uint hash = HASH(blockno); // hashcode
  acquire(&bcache.lock[hash]);

  // Is the block already cached?
  for(b = bcache.head[hash].next; b != &bcache.head[hash]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[hash]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  release(&bcache.lock[hash]);
  acquire(&bcache.eviction_lock);
  acquire(&bcache.lock[hash]);

  for(b = bcache.head[hash].next; b != &bcache.head[hash]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[hash]);
      release(&bcache.eviction_lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  for(b = bcache.buf; b != bcache.buf + NBUF; b++) {
    int ehash = HASH(b->blockno);
    if(hash != ehash)
      acquire(&bcache.lock[ehash]);
    if(b->refcnt == 0) {
      b->dev = dev;
      if(hash != ehash) {
        move_to_bucket(b, blockno);
        release(&bcache.lock[ehash]);
      }
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.lock[hash]);
      release(&bcache.eviction_lock);
      acquiresleep(&b->lock);
      return b;
    }
    if(hash != ehash)
      release(&bcache.lock[ehash]);
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  // acquire(&bcache.lock);
  uint hash = HASH(b->blockno);
  acquire(&bcache.lock[hash]);
  b->refcnt--;
  release(&bcache.lock[hash]);
}

void
bpin(struct buf *b) {
  uint hash = HASH(b->blockno);
  acquire(&bcache.lock[hash]);
  b->refcnt++;
  release(&bcache.lock[hash]);
}

void
bunpin(struct buf *b) {
  uint hash = HASH(b->blockno);
  acquire(&bcache.lock[hash]);
  b->refcnt--;
  release(&bcache.lock[hash]);
}


