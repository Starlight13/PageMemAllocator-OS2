# PageMemAllocator-OS2
The class allocator which can select data in heap, with required size. Memory is divided into pages, every page has a size of 256 bytes. Every page can be divided into blocks of the same size (page_class). A page can have 3 states:
*FREE - meaning the block is not yet divided and is not occupied by a multy-page block
*BLOCK_DIVIDED - meaning it is divided into blocks.
*MULTIPLE_PAGE_BLOCK - meaning that it is occupied by a multy-page block

The allocator contains information about each page, such as:

```
State state - can be 1 of 3 options (FREE, BLOCK_DIVIDED, MULTIPLE_PAGE_BLOCK)
uint8_t* page_start - the address where the page starts
uint16_t page_class - the size of blocks in which the page is divided
uint8_t list_of_using[MAX_BLOCKS_ON_PAGE] - a list that shows free blocks
```

## Functions:

`uint16_t align(uint16_t size)` - aligns the size of allocated memory to the power of 2 and not less than 16.

`void* mem_alloc(uint16_t size)` – if the block is bigger than half of the page size, decide how many pages are needed for this block, than find required number of consecutive pages and write to state that they are MULTIPLE_PAGE_BLOCKs. If the block is less than half of the page size, look for a block with state BLOCK_DIVIDED with the same page_class as the align(size) and free blocks. While searching remember a free page if seen. If there is no suitible BLOCK_DIVIDED page, divide a free page if such was found. If none of the options are avaliable, return NULL.

`void* mem_alloc(void* adr, uint16_t size)` - change size of chosen block, can make it smaller or bigger. If the adr is NULL, than simply mem_alloc(size). If the align(size) is equal to old size than just return adr.Else, find the location of the block and free it. Try to mem_alloc(size), if it returns NULL, make needed changes to the old page and return the old address, else return the new address.

`void mem_free(void* adr)` – mark this block as free. Than call function `unite_page(i)`.

`void unite_page(page_index)` - joins page back if all its blocks are free.

## Examples:

#### Allocator creation
##### Code:
```c++
Allocator allocator;
allocator.init();
allocator.mem_dump();
```
##### Result:
```
==Allocator==
Page N1, Size: 256
Start: 0x102805200
State: FREE

Page N2, Size: 256
Start: 0x102805300
State: FREE

Page N3, Size: 256
Start: 0x102805400
State: FREE
...
```

#### Memory allocation
##### Code:
```c++
Allocator allocator;
allocator.init();
void * adr1 = allocator.mem_alloc(58);
void * adr2 = allocator.mem_alloc(34);
void * adr3 = allocator.mem_alloc(61);
allocator.mem_dump();
```
##### Result:
```
==Allocator==
Page N1, Size: 256
Start: 0x102007a00
State: Block divided
Class size: 64
Block N1, is used: 1
Block N2, is used: 1
Block N3, is used: 0
Block N4, is used: 0

Page N2, Size: 256
Start: 0x102007b00
State: Block divided
Class size: 32
Block N1, is used: 1
Block N2, is used: 0
Block N3, is used: 0
Block N4, is used: 0
Block N5, is used: 0
Block N6, is used: 0
Block N7, is used: 0
Block N8, is used: 0
...
```

#### Memory free
##### Code:
```c++
Allocator allocator;
allocator.init();
void * adr1 = allocator.mem_alloc(58);
void * adr2 = allocator.mem_alloc(34);
void * adr3 = allocator.mem_alloc(61);
allocator.mem_free(adr1);
allocator.mem_free(adr3);
allocator.mem_dump();
```
##### Result:
```
==Allocator==
Page N1, Size: 256
Start: 0x103003c00
State: Block divided
Class size: 64
Block N1, is used: 0
Block N2, is used: 1
Block N3, is used: 0
Block N4, is used: 0

Page N2, Size: 256
Start: 0x103003d00
State: FREE
...
```

#### Memory reallocation
##### Code:
```c++
Allocator allocator;
allocator.init();
void * adr1 = allocator.mem_alloc(58);
void * adr2 = allocator.mem_alloc(34);
void * adr3 = allocator.mem_alloc(61);
allocator.mem_realloc(adr1, 11);
allocator.mem_dump();
```
##### Result:
```
Page N1, Size: 256
Start: 0x10080b200
State: Block divided
Class size: 64
Block N1, is used: 0
Block N2, is used: 1
Block N3, is used: 0
Block N4, is used: 0

Page N2, Size: 256
Start: 0x10080b300
State: Block divided
Class size: 32
Block N1, is used: 1
Block N2, is used: 0
Block N3, is used: 0
Block N4, is used: 0
Block N5, is used: 0
Block N6, is used: 0
Block N7, is used: 0
Block N8, is used: 0

Page N3, Size: 256
Start: 0x10080b400
State: Block divided
Class size: 16
Block N1, is used: 1
Block N2, is used: 0
Block N3, is used: 0
Block N4, is used: 0
Block N5, is used: 0
Block N6, is used: 0
Block N7, is used: 0
Block N8, is used: 0
Block N9, is used: 0
Block N10, is used: 0
Block N11, is used: 0
Block N12, is used: 0
Block N13, is used: 0
Block N14, is used: 0
Block N15, is used: 0
Block N16, is used: 0
...
```
