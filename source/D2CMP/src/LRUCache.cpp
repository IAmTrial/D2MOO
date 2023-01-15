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
 * 1.00: D2CMP.0x1000C930
 * 1,07 Beta: D2CMPd.0x6FC84A70
 * 1.07: D2CMP.0x6FE19070
 * 1.10: D2CMP.0x6FDF8B30
 * 1.13C: D2CMP.0x6FE27270
 *
 * This implementation outputs effectively the same binary from 1.00, 1.07,
 * and 1.10.
 */
void __fastcall LRUCache_Initialize(LRUCache* pLRUCache, void* pMemPool, size_t dwAllocationLimit, DestroyBlockValueFunc pfnDestroyBlockValue, size_t dwCommittedBlocksLimit)
{
	pLRUCache->pMemPool = pMemPool;
	pLRUCache->dwAllocationLimit = dwAllocationLimit;
	pLRUCache->dwAllocated = 0;
	pLRUCache->pfnDestroyBlockValue = pfnDestroyBlockValue;
	pLRUCache->dwCommittedBlocksLimit = dwCommittedBlocksLimit;

	// Allocate memory for the blocks array.
	size_t dwLRUBlocksSize = pLRUCache->dwCommittedBlocks * sizeof(pLRUCache->pLRUBlocks[0]);
	pLRUCache->pLRUBlocks = (struct LRUBlock*)FOG_AllocPool(pLRUCache->pMemPool, dwLRUBlocksSize, __FILE__, __LINE__, 0);
	memset(pLRUCache->pLRUBlocks, 0, dwLRUBlocksSize);

	for (size_t i = 0; i < pLRUCache->dwCommittedBlocksLimit; ++i)
	{
		LRUBlock* pCurrentBlock = &pLRUCache->pLRUBlocks[i];
		if (i < pLRUCache->dwCommittedBlocksLimit - 1)
		{
			pCurrentBlock->pNext = &pLRUCache->pLRUBlocks[i + 1];
		}

		if (i > 0)
		{
			pCurrentBlock->pPrevious = &pLRUCache->pLRUBlocks[i - 1];
		}

		pCurrentBlock->pData = NULL;
		pCurrentBlock->dwSize = 0;
		pCurrentBlock->pSource = NULL;
	}

	pLRUCache->dwCommittedBlocks = 0;
	pLRUCache->pLRUCommittedBlock = NULL;
	pLRUCache->pMRUCommittedBlock = NULL;
	pLRUCache->pFirstFreeBlock = pLRUCache->pLRUBlocks;
	pLRUCache->pLastFreeBlock = &pLRUCache->pLRUBlocks[pLRUCache->dwCommittedBlocksLimit - 1];
	pLRUCache->nCacheMisses = 0;
	pLRUCache->nCacheAge = 0;
}
