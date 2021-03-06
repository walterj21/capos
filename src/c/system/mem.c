#include <stddef.h>
#include <stdint.h>
#include "include/multiboot.h"
#include "include/mem.h"
#include "include/util.h"

struct free_hop free_origin;	// global, head of free hop list
size_t mem_total;		// set in mmap_init()

void init_mmap(mmap_entry_t *mmap_addr, multiboot_uint32_t mmap_length)
{
	mmap_entry_t *entry = mmap_addr;	// multiboot memory info
	struct free_hop *p = &free_origin;	// for setting initial hops
	size_t available_regions = 0;		// tracked for iteration
	mem_total = 0;

	/* make space for all regions to have enough for available regions */
	mmap_entry_t *available_mem[mmap_length / sizeof(mmap_entry_t)];

	/* iterate through memory regions, keeping track of available space */
	/* valid free space must be above low memory (> 1MB) */
	while (entry < mmap_addr + mmap_length) {
		if (entry->type == 1 && entry->base_addr_low >= 0x100000) {
			available_mem[available_regions] = entry;
			++available_regions;
		}

		entry = (mmap_entry_t *) ((unsigned int) entry + entry->size +
					                  sizeof(entry->size));
	}

	/* set free origin values, then iterate through available mem in 32 bit space */
	p->size = 0;
	for (size_t i = 0; i < available_regions; ++i) {
		if (available_mem[i]->base_addr_low < 0xffffffff) {
			p->fw = (struct free_hop *)available_mem[i]->base_addr_low;

			p->fw->size = (size_t)available_mem[i]->length_low;
			p->fw->fw = NULL;
			p = p->fw;
			mem_total += available_mem[i]->length_low;
		}
	}

	return;
}

void *cmalloc(size_t size)
{
	struct free_hop *p = &free_origin;
	size = (size + sizeof(struct busy_hop) + 7) & (-8);

	/* if there is a memory region of equal size, pop out and fix links */
	while (p->fw != NULL) {
		if (p->fw->size == size) {
			struct busy_hop *busy = (struct busy_hop *)p->fw;
			p->fw = p->fw->fw;
			busy->size = size;
			return &(busy->data);
		}

		p = p->fw;
	}

	/* otherwise, split the first larger region and fix links */
	p = &free_origin;
	while (p->fw != NULL) {
		if (p->fw->size - sizeof(struct free_hop) > size) {
			struct busy_hop *busy = (struct busy_hop *)p->fw;
			p->fw = (struct free_hop *)((uint8_t *)p->fw + size);
			p->fw->fw = ((struct free_hop *)busy)->fw;
			p->fw->size = (((struct free_hop *)busy)->size - size);
			busy->size = size;
			return &(busy->data);
		}

		p = p->fw;
	}

	return NULL;	// memory machine broke
}

void cfree(void *mem)
{
	/* reassemble busy hop structure and yoink memory region size */
	struct busy_hop *busy = (struct busy_hop *)((uint8_t *)mem - offsetof(struct busy_hop, data));
	size_t size = busy->size;

	/* recast to free hop structure and link to beginning of chain */
	struct free_hop *p = (struct free_hop *)busy;
	p->size = size;
	struct free_hop *tmp = free_origin.fw;
	free_origin.fw = p;
	p->fw = tmp;

	/* if you don't mind the extra overhead, you could invoke */
	/* defragment() here or do a single run through the free list to */
	/* check if the hop can be collapsed into another */

	return;
}

void *crealloc(void *mem, size_t size)
{
	void *nmem = cmalloc(size);
	memcpy(nmem, mem, size);
	cfree(mem);
	return nmem;
}

size_t mem_status(void)
{
	struct free_hop *p = free_origin.fw;
	size_t free = 0;

	while (p != NULL) {
		free += p->size;
		p = p->fw;
	}

	return free;
}

void defragment(void)
{
	struct free_hop *p = free_origin.fw;
	struct free_hop *cmp = p;

	/* search for adjacent hops, then collapse them together */
	while (p != NULL) {
		while (cmp->fw != NULL) {
			if (cmp->fw == (struct free_hop *)((uint8_t *)p + p->size)) {
				p->size = (p->size + cmp->fw->size);
				cmp->fw = cmp->fw->fw;
			}

			cmp = cmp->fw;
		}

		p = p->fw;
		cmp = &free_origin;
	}

	return;
}

