/**
 * D2MOO
 * Copyright (c) 2020-2023  The Phrozen Keep community
 *
 * This file belongs to D2MOO.
 * File authors: Lectem, Mir Drualga
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <cstddef>

/**
 * A block that is used to implement a cache with a least recently used
 * (LRU) block replacement policy.
 */
struct LRUBlock
{
	/**
	 * A copy of the data in the pValue member. Named lpData in assert
	 * strings.
	 */
	void* pData;

	/** The size, in bytes, of the data pointed to by pData. */
	size_t dwSize;

	/** A pointer to the original data copied to pData. */
	void* pSource;

	/** The next block in the LRU link. */
	LRUBlock* pNext;

	/** The previous block in the LRU link. */
	LRUBlock* pPrevious;
};

typedef void (__fastcall *DestroyBlockValueFunc)(LRUBlock* pBlock);

/**
 * A cache that implements a least recently used (LRU) block replacement
 * policy.
 */
struct LRUCache
{
	/**
	 * The Fog memory pool responsible for allocating the contiguous
	 * array of blocks.
	 */
	void* pMemPool;

	/**
	 * The total data allocation size limit for every committed block.
	 * Named dwMaxAllocated in assert strings.
	 */
	size_t dwAllocationLimit;

	/** The current total data allocation size for every committed block. */
	size_t dwAllocated;

	/** The total number of committed blocks. */
	size_t dwCommittedBlocks;

	/** A pointer to an array of contiguous blocks acting as a cache. */
	LRUBlock* pLRUBlocks;

	/** The limit on the number of committed blocks. */
	size_t dwCommittedBlocksLimit;

	/** The least recently used (LRU) block. */
	LRUBlock* pLRUCommittedBlock;

	/** The most recently used (MRU) block. */
	LRUBlock* pMRUCommittedBlock;

	/** The first free block that is ready to commit. */
	LRUBlock* pFirstFreeBlock;

	/** The last free block that is ready to commit. */
	LRUBlock* pLastFreeBlock;

	/**
	 * The function used to destroy the copy of the data stored by an
	 * LRUBlock.
	 */
	DestroyBlockValueFunc pfnDestroyBlockValue;

	/**
	 * The number of completed operations that resulted in one or more
	 * cache blocks being released, which would lead to cache misses.
	 */
	int nCacheMisses;

	/**
	 * The number of completed operations made to the committed blocks
	 * (e.g. insertions, removals).
	 */
	int nCacheAge;
};

/**
 * Initializes an LRU cache with the specified memory pool, total
 * allocation limit, block value destructor, and committed blocks
 * limit.
 *
 * 1.00: D2CMP.0x1000C930
 * 1,07 Beta: D2CMPd.0x6FC84A70
 * 1.07: D2CMP.0x6FE19070
 * 1.10: D2CMP.0x6FDF8B30
 * 1.13C: D2CMP.0x6FE27270
 */
void __fastcall LRUCache_Initialize(LRUCache* pLRUCache, void* pMemPool, size_t dwAllocationLimit, DestroyBlockValueFunc pfnDestroyBlockValue, size_t dwCommittedBlocksLimit);
