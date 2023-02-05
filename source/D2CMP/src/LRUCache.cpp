/**
 * D2MOO
 * Copyright (c) 2020-2023  The Phrozen Keep community
 *
 * This file belongs to D2MOO.
 * File authors: Mir Drualga
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

#include "LRUCache.h"

#include <cstddef>
#include <cstring>

#include "Fog.h"

/**
 * Pushes a free block to the end of the free block queue.
 *
 * 1.00: Inline
 * 1.07 Beta: D2CMPd.0x6FC85230
 * 1.07: Inline
 * 1.10: Inline
 * 1.13C: Inline
 *
 * This implementation outputs effectively the same binary from 1.00, 1.07,
 * and 1.10 (via LRUCache_FreeBlock).
 */
static inline void __fastcall LRUCache_QueueFreeBlock(LRUCache* ptCache, LRUBlock* ptBlock)
{
	if (ptCache->ptLastFreeBlock == NULL)
	{
		ptCache->ptFirstFreeBlock = ptBlock;
		ptCache->ptLastFreeBlock = ptBlock;
	}
	else
	{
		ptCache->ptLastFreeBlock->ptNext = ptBlock;
		ptBlock->ptPrevious = ptCache->ptLastFreeBlock;
		ptCache->ptLastFreeBlock = ptBlock;
	}
}

/**
 * Decommits the least recently used block from the committed blocks
 * list.
 *
 * 1.00: Inline
 * 1.07 Beta: D2CMPd.0x6FC852A0
 * 1.07: Inline
 * 1.10: Inline
 * 1.13C: D2CMP.0x6FE27170
 *
 * This implementation outputs effectively the same binary from 1.00, 1.07,
 * and 1.10 (via LRUCache_FreeBlock).
 */
static inline LRUBlock* __fastcall LRUCache_DecommitLRUBlock(LRUCache* ptCache)
{
	if (ptCache->ptLRUCommittedBlock == NULL)
	{
		return NULL;
	}

	LRUBlock* ptBlock = ptCache->ptLRUCommittedBlock;

	// Point LRUCache to the next available free block.
	ptCache->ptLRUCommittedBlock = ptCache->ptLRUCommittedBlock->ptNext;
	if (ptCache->ptLRUCommittedBlock == NULL)
	{
		ptCache->ptMRUCommittedBlock = NULL;
	}
	else
	{
		ptCache->ptLRUCommittedBlock->ptPrevious = NULL;
	}

	// Remove committed block pointers from the popped block.
	ptBlock->ptNext = NULL;
	ptBlock->ptPrevious = NULL;

	return ptBlock;
}

/**
 * Decommits the specified block from the committed block list.
 *
 * 1.00: Inline
 * 1.07 Beta: D2CMPd.0x6FC853C0
 * 1.07: Inline
 * 1.10: Inline
 * 1.13C: D2CMP.0x6FE27320
 *
 * This implementation outputs effectively the same binary from 1.00, 1.07,
 * and 1.10 (via LRUCache_FreeBlock).
 */
static inline void __fastcall LRUCache_DecommitBlock(LRUCache* ptCache, LRUBlock* ptBlock)
{
	if (ptCache->ptLRUCommittedBlock == NULL)
	{
		return;
	}

	D2_ASSERT(ptBlock != NULL);

	if (ptBlock == ptCache->ptLRUCommittedBlock)
	{
		LRUCache_DecommitLRUBlock(ptCache);
		return;
	}

	if (ptBlock != ptCache->ptMRUCommittedBlock)
	{
		ptBlock->ptPrevious->ptNext = ptBlock->ptNext;
		ptBlock->ptNext->ptPrevious = ptBlock->ptPrevious;
	}
	else
	{
		ptCache->ptMRUCommittedBlock = ptCache->ptMRUCommittedBlock->ptPrevious;
		ptCache->ptMRUCommittedBlock->ptNext = NULL;
	}

	ptBlock->ptNext = NULL;
	ptBlock->ptPrevious = NULL;
}

/**
 * 1.00: D2CMP.0x1000C930
 * 1,07 Beta: D2CMPd.0x6FC84A70
 * 1.07: D2CMP.0x6FE19070
 * 1.10: D2CMP.0x6FDF8B30
 * 1.13C: D2CMP.0x6FE27270
 *
 * This implementation outputs effectively the same binary from 1.00, 1.07,
 * and 1.10.
 */
void __fastcall LRUCache_Initialize(LRUCache* ptCache, void* pMemPool, size_t dwAllocationLimit, DestroyBlockDataFunc ptfnDestroyBlockData, size_t dwCommittedBlocksLimit)
{
	ptCache->pMemPool = pMemPool;
	ptCache->dwAllocationLimit = dwAllocationLimit;
	ptCache->dwAllocated = 0;
	ptCache->ptfnDestroyBlockData = ptfnDestroyBlockData;
	ptCache->dwCommittedBlocksLimit = dwCommittedBlocksLimit;

	// Allocate memory for the blocks array.
	size_t dwLRUBlocksSize = ptCache->dwCommittedBlocks * sizeof(ptCache->ptBlocks[0]);
	ptCache->ptBlocks = (LRUBlock*) D2_CALLOC_POOL(ptCache->pMemPool, dwLRUBlocksSize);
	memset(ptCache->ptBlocks, 0, dwLRUBlocksSize);

	for (size_t i = 0; i < ptCache->dwCommittedBlocksLimit; ++i)
	{
		LRUBlock* ptCurrentBlock = &ptCache->ptBlocks[i];
		if (i < ptCache->dwCommittedBlocksLimit - 1)
		{
			ptCurrentBlock->ptNext = &ptCache->ptBlocks[i + 1];
		}

		if (i > 0)
		{
			ptCurrentBlock->ptPrevious = &ptCache->ptBlocks[i - 1];
		}

		ptCurrentBlock->ptData = NULL;
		ptCurrentBlock->dwSize = 0;
		ptCurrentBlock->ptSource = NULL;
	}

	ptCache->dwCommittedBlocks = 0;
	ptCache->ptLRUCommittedBlock = NULL;
	ptCache->ptMRUCommittedBlock = NULL;
	ptCache->ptFirstFreeBlock = ptCache->ptBlocks;
	ptCache->ptLastFreeBlock = &ptCache->ptBlocks[ptCache->dwCommittedBlocksLimit - 1];
	ptCache->nCacheMisses = 0;
	ptCache->nCacheAge = 0;
}

/**
 * 1.00: D2CMP.0x1000CA20
 * 1.07 Beta: D2CMPd.0x6FC84C50
 * 1.07: D2CMP.0x6FE19130
 * 1.10: D2CMP.0x6FDF8BF0
 * 1.13C: Inline
 *
 * This implementation outputs effectively the same binary from 1.00, 1.07,
 * and 1.10.
 */
void __fastcall LRUCache_Destroy(LRUCache* ptCache)
{
	while (ptCache->dwCommittedBlocks)
	{
		LRUCache_FreeBlock(ptCache);
	}

	if (ptCache->ptBlocks != NULL)
	{
		D2_FREE_POOL(ptCache->pMemPool, ptCache->ptBlocks);
		ptCache->ptBlocks = NULL;
	}
}

/**
 * 1.00: D2CMP.0x1000CC70
 * 1.07 Beta: D2CMPd.0x6FC84FD0
 * 1.07: D2CMP.0x6FE19300
 * 1.10: D2CMP.0x6FDF8DC0
 * 1.13C: D2CMP.0x6FE27400
 */
void __fastcall LRUCache_FreeBlock(LRUCache* ptCache, LRUBlock* ptBlock)
{
	if (ptCache->dwCommittedBlocks == 0)
	{
		return;
	}

	if (ptBlock == NULL)
	{
		ptBlock = LRUCache_DecommitLRUBlock(ptCache);
	}
	else
	{
		LRUCache_DecommitBlock(ptCache, ptBlock);
	}

	if (ptBlock != NULL)
	{
		ptCache->dwAllocated -= ptBlock->dwSize;
		if (ptBlock->ptSource != NULL)
		{
			ptCache->ptfnDestroyBlockData(ptBlock);
		}

		if (ptBlock->ptData != NULL)
		{
			D2_FREE_POOL(ptCache->pMemPool, ptBlock->ptData);
			ptBlock->ptData = NULL;
			ptBlock->dwSize = 0;
			ptBlock->ptSource = NULL;
		}

		LRUCache_QueueFreeBlock(ptCache, ptBlock);
	}

	--ptCache->dwCommittedBlocks;
}
