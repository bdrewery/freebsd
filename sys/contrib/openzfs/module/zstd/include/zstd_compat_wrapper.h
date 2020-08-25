/*
 * BSD 3-Clause New License (https://spdx.org/licenses/BSD-3-Clause.html)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 2020, Sebastian Gottschall
 */

/*
 * This wrapper fixes a problem, in case the ZFS filesystem driver, is compiled
 * staticly into the kernel.
 * This will cause a symbol collision with the older in-kernel zstd library.
 * The following macros will simply rename all local zstd symbols and references
 *
 * Note: if the zstd library for zfs is updated to a newer version, this macro
 * list usually needs to be updated.
 * this can be done with some hand crafting of the output of the following
 * script
 * nm zstd.o | awk '{print "#define "$3 " zfs_" $3}' > macrotable
 */

#define	BIT_initDStream zfs_BIT_initDStream
#define	BIT_mask zfs_BIT_mask
#define	BIT_reloadDStream zfs_BIT_reloadDStream
#define	ERR_getErrorString zfs_ERR_getErrorString
#define	FSE_NCountWriteBound zfs_FSE_NCountWriteBound
#define	FSE_buildCTable zfs_FSE_buildCTable
#define	FSE_buildCTable_raw zfs_FSE_buildCTable_raw
#define	FSE_buildCTable_rle zfs_FSE_buildCTable_rle
#define	FSE_buildCTable_wksp zfs_FSE_buildCTable_wksp
#define	FSE_buildDTable zfs_FSE_buildDTable
#define	FSE_buildDTable_raw zfs_FSE_buildDTable_raw
#define	FSE_buildDTable_rle zfs_FSE_buildDTable_rle
#define	FSE_compress zfs_FSE_compress
#define	FSE_compress2 zfs_FSE_compress2
#define	FSE_compressBound zfs_FSE_compressBound
#define	FSE_compress_usingCTable zfs_FSE_compress_usingCTable
#define	FSE_compress_usingCTable_generic zfs_FSE_compress_usingCTable_generic
#define	FSE_compress_wksp zfs_FSE_compress_wksp
#define	FSE_createCTable zfs_FSE_createCTable
#define	FSE_createDTable zfs_FSE_createDTable
#define	FSE_decompress zfs_FSE_decompress
#define	FSE_decompress_usingDTable zfs_FSE_decompress_usingDTable
#define	FSE_decompress_wksp zfs_FSE_decompress_wksp
#define	FSE_freeCTable zfs_FSE_freeCTable
#define	FSE_freeDTable zfs_FSE_freeDTable
#define	FSE_getErrorName zfs_FSE_getErrorName
#define	FSE_normalizeCount zfs_FSE_normalizeCount
#define	FSE_optimalTableLog zfs_FSE_optimalTableLog
#define	FSE_optimalTableLog_internal zfs_FSE_optimalTableLog_internal
#define	FSE_readNCount zfs_FSE_readNCount
#define	FSE_versionNumber zfs_FSE_versionNumber
#define	FSE_writeNCount zfs_FSE_writeNCount
#define	HIST_count zfs_HIST_count
#define	HIST_countFast zfs_HIST_countFast
#define	HIST_countFast_wksp zfs_HIST_countFast_wksp
#define	HIST_count_parallel_wksp zfs_HIST_count_parallel_wksp
#define	HIST_count_simple zfs_HIST_count_simple
#define	HIST_count_wksp zfs_HIST_count_wksp
#define	HUF_buildCTable zfs_HUF_buildCTable
#define	HUF_buildCTable_wksp zfs_HUF_buildCTable_wksp
#define	HUF_compress zfs_HUF_compress
#define	HUF_compress1X zfs_HUF_compress1X
#define	HUF_compress1X_repeat zfs_HUF_compress1X_repeat
#define	HUF_compress1X_usingCTable zfs_HUF_compress1X_usingCTable
#define	HUF_compress1X_wksp zfs_HUF_compress1X_wksp
#define	HUF_compress2 zfs_HUF_compress2
#define	HUF_compress4X_repeat zfs_HUF_compress4X_repeat
#define	HUF_compress4X_usingCTable zfs_HUF_compress4X_usingCTable
#define	HUF_compress4X_wksp zfs_HUF_compress4X_wksp
#define	HUF_compressBound zfs_HUF_compressBound
#define	HUF_compressWeights zfs_HUF_compressWeights
#define	HUF_decompress zfs_HUF_decompress
#define	HUF_decompress1X1 zfs_HUF_decompress1X1
#define	HUF_decompress1X1_DCtx zfs_HUF_decompress1X1_DCtx
#define	HUF_decompress1X1_DCtx_wksp zfs_HUF_decompress1X1_DCtx_wksp
#define	HUF_decompress1X1_DCtx_wksp_bmi2 zfs_HUF_decompress1X1_DCtx_wksp_bmi2
#define	HUF_decompress1X1_usingDTable zfs_HUF_decompress1X1_usingDTable
#define	HUF_decompress1X2 zfs_HUF_decompress1X2
#define	HUF_decompress1X2_DCtx zfs_HUF_decompress1X2_DCtx
#define	HUF_decompress1X2_DCtx_wksp zfs_HUF_decompress1X2_DCtx_wksp
#define	HUF_decompress1X2_usingDTable zfs_HUF_decompress1X2_usingDTable
#define	HUF_decompress1X_DCtx zfs_HUF_decompress1X_DCtx
#define	HUF_decompress1X_DCtx_wksp zfs_HUF_decompress1X_DCtx_wksp
#define	HUF_decompress1X_usingDTable zfs_HUF_decompress1X_usingDTable
#define	HUF_decompress1X_usingDTable_bmi2 zfs_HUF_decompress1X_usingDTable_bmi2
#define	HUF_decompress4X1 zfs_HUF_decompress4X1
#define	HUF_decompress4X1_DCtx zfs_HUF_decompress4X1_DCtx
#define	HUF_decompress4X1_DCtx_wksp zfs_HUF_decompress4X1_DCtx_wksp
#define	HUF_decompress4X1_usingDTable zfs_HUF_decompress4X1_usingDTable
#define	HUF_decompress4X2 zfs_HUF_decompress4X2
#define	HUF_decompress4X2_DCtx zfs_HUF_decompress4X2_DCtx
#define	HUF_decompress4X2_DCtx_wksp zfs_HUF_decompress4X2_DCtx_wksp
#define	HUF_decompress4X2_usingDTable zfs_HUF_decompress4X2_usingDTable
#define	HUF_decompress4X_DCtx zfs_HUF_decompress4X_DCtx
#define	HUF_decompress4X_hufOnly zfs_HUF_decompress4X_hufOnly
#define	HUF_decompress4X_hufOnly_wksp zfs_HUF_decompress4X_hufOnly_wksp
#define	HUF_decompress4X_hufOnly_wksp_bmi2 \
	zfs_HUF_decompress4X_hufOnly_wksp_bmi2
#define	HUF_decompress4X_usingDTable zfs_HUF_decompress4X_usingDTable
#define	HUF_decompress4X_usingDTable_bmi2 zfs_HUF_decompress4X_usingDTable_bmi2
#define	HUF_estimateCompressedSize zfs_HUF_estimateCompressedSize
#define	HUF_fillDTableX2Level2 zfs_HUF_fillDTableX2Level2
#define	HUF_getErrorName zfs_HUF_getErrorName
#define	HUF_getNbBits zfs_HUF_getNbBits
#define	HUF_optimalTableLog zfs_HUF_optimalTableLog
#define	HUF_readCTable zfs_HUF_readCTable
#define	HUF_readDTableX1 zfs_HUF_readDTableX1
#define	HUF_readDTableX1_wksp zfs_HUF_readDTableX1_wksp
#define	HUF_readDTableX2 zfs_HUF_readDTableX2
#define	HUF_readDTableX2_wksp zfs_HUF_readDTableX2_wksp
#define	HUF_readStats zfs_HUF_readStats
#define	HUF_selectDecoder zfs_HUF_selectDecoder
#define	HUF_setMaxHeight zfs_HUF_setMaxHeight
#define	HUF_validateCTable zfs_HUF_validateCTable
#define	HUF_writeCTable zfs_HUF_writeCTable
#define	LL_base zfs_LL_base
#define	LL_bits zfs_LL_bits
#define	LL_defaultDTable zfs_LL_defaultDTable
#define	LL_defaultNorm zfs_LL_defaultNorm
#define	ML_base zfs_ML_base
#define	ML_bits zfs_ML_bits
#define	ML_defaultDTable zfs_ML_defaultDTable
#define	ML_defaultNorm zfs_ML_defaultNorm
#define	OF_base zfs_OF_base
#define	OF_bits zfs_OF_bits
#define	OF_defaultDTable zfs_OF_defaultDTable
#define	OF_defaultNorm zfs_OF_defaultNorm
#define	POOL_add zfs_POOL_add
#define	POOL_create zfs_POOL_create
#define	POOL_create_advanced zfs_POOL_create_advanced
#define	POOL_free zfs_POOL_free
#define	POOL_resize zfs_POOL_resize
#define	POOL_sizeof zfs_POOL_sizeof
#define	POOL_tryAdd zfs_POOL_tryAdd
#define	ZSTD_CCtxParams_getParameter zfs_ZSTD_CCtxParams_getParameter
#define	ZSTD_CCtxParams_init zfs_ZSTD_CCtxParams_init
#define	ZSTD_CCtxParams_init_advanced zfs_ZSTD_CCtxParams_init_advanced
#define	ZSTD_CCtxParams_reset zfs_ZSTD_CCtxParams_reset
#define	ZSTD_CCtxParams_setParameter zfs_ZSTD_CCtxParams_setParameter
#define	ZSTD_CCtx_getParameter zfs_ZSTD_CCtx_getParameter
#define	ZSTD_CCtx_loadDictionary zfs_ZSTD_CCtx_loadDictionary
#define	ZSTD_CCtx_loadDictionary_advanced zfs_ZSTD_CCtx_loadDictionary_advanced
#define	ZSTD_CCtx_loadDictionary_byReference \
	zfs_ZSTD_CCtx_loadDictionary_byReference
#define	ZSTD_CCtx_refCDict zfs_ZSTD_CCtx_refCDict
#define	ZSTD_CCtx_refPrefix zfs_ZSTD_CCtx_refPrefix
#define	ZSTD_CCtx_refPrefix_advanced zfs_ZSTD_CCtx_refPrefix_advanced
#define	ZSTD_CCtx_reset zfs_ZSTD_CCtx_reset
#define	ZSTD_CCtx_setParameter zfs_ZSTD_CCtx_setParameter
#define	ZSTD_CCtx_setParametersUsingCCtxParams \
	zfs_ZSTD_CCtx_setParametersUsingCCtxParams
#define	ZSTD_CCtx_setPledgedSrcSize zfs_ZSTD_CCtx_setPledgedSrcSize
#define	ZSTD_CStreamInSize zfs_ZSTD_CStreamInSize
#define	ZSTD_CStreamOutSize zfs_ZSTD_CStreamOutSize
#define	ZSTD_DCtx_loadDictionary zfs_ZSTD_DCtx_loadDictionary
#define	ZSTD_DCtx_loadDictionary_advanced zfs_ZSTD_DCtx_loadDictionary_advanced
#define	ZSTD_DCtx_loadDictionary_byReference \
	zfs_ZSTD_DCtx_loadDictionary_byReference
#define	ZSTD_DCtx_refDDict zfs_ZSTD_DCtx_refDDict
#define	ZSTD_DCtx_refPrefix zfs_ZSTD_DCtx_refPrefix
#define	ZSTD_DCtx_refPrefix_advanced zfs_ZSTD_DCtx_refPrefix_advanced
#define	ZSTD_DCtx_reset zfs_ZSTD_DCtx_reset
#define	ZSTD_DCtx_setFormat zfs_ZSTD_DCtx_setFormat
#define	ZSTD_DCtx_setMaxWindowSize zfs_ZSTD_DCtx_setMaxWindowSize
#define	ZSTD_DCtx_setParameter zfs_ZSTD_DCtx_setParameter
#define	ZSTD_DDict_dictContent zfs_ZSTD_DDict_dictContent
#define	ZSTD_DDict_dictSize zfs_ZSTD_DDict_dictSize
#define	ZSTD_DStreamInSize zfs_ZSTD_DStreamInSize
#define	ZSTD_DStreamOutSize zfs_ZSTD_DStreamOutSize
#define	ZSTD_DUBT_findBestMatch zfs_ZSTD_DUBT_findBestMatch
#define	ZSTD_NCountCost zfs_ZSTD_NCountCost
#define	ZSTD_XXH64_digest zfs_ZSTD_XXH64_digest
#define	ZSTD_adjustCParams zfs_ZSTD_adjustCParams
#define	ZSTD_assignParamsToCCtxParams zfs_ZSTD_assignParamsToCCtxParams
#define	ZSTD_buildCTable zfs_ZSTD_buildCTable
#define	ZSTD_buildFSETable zfs_ZSTD_buildFSETable
#define	ZSTD_buildSeqStore zfs_ZSTD_buildSeqStore
#define	ZSTD_buildSeqTable zfs_ZSTD_buildSeqTable
#define	ZSTD_cParam_getBounds zfs_ZSTD_cParam_getBounds
#define	ZSTD_cParam_withinBounds zfs_ZSTD_cParam_withinBounds
#define	ZSTD_calloc zfs_ZSTD_calloc
#define	ZSTD_checkCParams zfs_ZSTD_checkCParams
#define	ZSTD_checkContinuity zfs_ZSTD_checkContinuity
#define	ZSTD_compress zfs_ZSTD_compress
#define	ZSTD_compress2 zfs_ZSTD_compress2
#define	ZSTD_compressBegin zfs_ZSTD_compressBegin
#define	ZSTD_compressBegin_advanced zfs_ZSTD_compressBegin_advanced
#define	ZSTD_compressBegin_advanced_internal \
	zfs_ZSTD_compressBegin_advanced_internal
#define	ZSTD_compressBegin_usingCDict zfs_ZSTD_compressBegin_usingCDict
#define	ZSTD_compressBegin_usingCDict_advanced \
	zfs_ZSTD_compressBegin_usingCDict_advanced
#define	ZSTD_compressBegin_usingDict zfs_ZSTD_compressBegin_usingDict
#define	ZSTD_compressBlock zfs_ZSTD_compressBlock
#define	ZSTD_compressBlock_btlazy2 zfs_ZSTD_compressBlock_btlazy2
#define	ZSTD_compressBlock_btlazy2_dictMatchState \
	zfs_ZSTD_compressBlock_btlazy2_dictMatchState
#define	ZSTD_compressBlock_btlazy2_extDict \
	zfs_ZSTD_compressBlock_btlazy2_extDict
#define	ZSTD_compressBlock_btopt zfs_ZSTD_compressBlock_btopt
#define	ZSTD_compressBlock_btopt_dictMatchState \
	zfs_ZSTD_compressBlock_btopt_dictMatchState
#define	ZSTD_compressBlock_btopt_extDict zfs_ZSTD_compressBlock_btopt_extDict
#define	ZSTD_compressBlock_btultra zfs_ZSTD_compressBlock_btultra
#define	ZSTD_compressBlock_btultra2 zfs_ZSTD_compressBlock_btultra2
#define	ZSTD_compressBlock_btultra_dictMatchState \
	zfs_ZSTD_compressBlock_btultra_dictMatchState
#define	ZSTD_compressBlock_btultra_extDict \
	zfs_ZSTD_compressBlock_btultra_extDict
#define	ZSTD_compressBlock_doubleFast zfs_ZSTD_compressBlock_doubleFast
#define	ZSTD_compressBlock_doubleFast_dictMatchState \
	zfs_ZSTD_compressBlock_doubleFast_dictMatchState
#define	ZSTD_compressBlock_doubleFast_extDict \
	zfs_ZSTD_compressBlock_doubleFast_extDict
#define	ZSTD_compressBlock_doubleFast_extDict_generic \
	zfs_ZSTD_compressBlock_doubleFast_extDict_generic
#define	ZSTD_compressBlock_fast zfs_ZSTD_compressBlock_fast
#define	ZSTD_compressBlock_fast_dictMatchState \
	zfs_ZSTD_compressBlock_fast_dictMatchState
#define	ZSTD_compressBlock_fast_extDict zfs_ZSTD_compressBlock_fast_extDict
#define	ZSTD_compressBlock_fast_extDict_generic \
	zfs_ZSTD_compressBlock_fast_extDict_generic
#define	ZSTD_compressBlock_greedy zfs_ZSTD_compressBlock_greedy
#define	ZSTD_compressBlock_greedy_dictMatchState \
	zfs_ZSTD_compressBlock_greedy_dictMatchState
#define	ZSTD_compressBlock_greedy_extDict zfs_ZSTD_compressBlock_greedy_extDict
#define	ZSTD_compressBlock_internal zfs_ZSTD_compressBlock_internal
#define	ZSTD_compressBlock_lazy zfs_ZSTD_compressBlock_lazy
#define	ZSTD_compressBlock_lazy2 zfs_ZSTD_compressBlock_lazy2
#define	ZSTD_compressBlock_lazy2_dictMatchState \
	zfs_ZSTD_compressBlock_lazy2_dictMatchState
#define	ZSTD_compressBlock_lazy2_extDict zfs_ZSTD_compressBlock_lazy2_extDict
#define	ZSTD_compressBlock_lazy_dictMatchState \
	zfs_ZSTD_compressBlock_lazy_dictMatchState
#define	ZSTD_compressBlock_lazy_extDict zfs_ZSTD_compressBlock_lazy_extDict
#define	ZSTD_compressBound zfs_ZSTD_compressBound
#define	ZSTD_compressCCtx zfs_ZSTD_compressCCtx
#define	ZSTD_compressContinue zfs_ZSTD_compressContinue
#define	ZSTD_compressContinue_internal zfs_ZSTD_compressContinue_internal
#define	ZSTD_compressEnd zfs_ZSTD_compressEnd
#define	ZSTD_compressLiterals zfs_ZSTD_compressLiterals
#define	ZSTD_compressRleLiteralsBlock zfs_ZSTD_compressRleLiteralsBlock
#define	ZSTD_compressStream zfs_ZSTD_compressStream
#define	ZSTD_compressStream2 zfs_ZSTD_compressStream2
#define	ZSTD_compressStream2_simpleArgs zfs_ZSTD_compressStream2_simpleArgs
#define	ZSTD_compressSuperBlock zfs_ZSTD_compressSuperBlock
#define	ZSTD_compress_advanced zfs_ZSTD_compress_advanced
#define	ZSTD_compress_advanced_internal zfs_ZSTD_compress_advanced_internal
#define	ZSTD_compress_internal zfs_ZSTD_compress_internal
#define	ZSTD_compress_usingCDict zfs_ZSTD_compress_usingCDict
#define	ZSTD_compress_usingCDict_advanced zfs_ZSTD_compress_usingCDict_advanced
#define	ZSTD_compress_usingDict zfs_ZSTD_compress_usingDict
#define	ZSTD_copyCCtx zfs_ZSTD_copyCCtx
#define	ZSTD_copyDCtx zfs_ZSTD_copyDCtx
#define	ZSTD_copyDDictParameters zfs_ZSTD_copyDDictParameters
#define	ZSTD_count zfs_ZSTD_count
#define	ZSTD_count_2segments zfs_ZSTD_count_2segments
#define	ZSTD_createCCtx zfs_ZSTD_createCCtx
#define	ZSTD_createCCtxParams zfs_ZSTD_createCCtxParams
#define	ZSTD_createCCtx_advanced zfs_ZSTD_createCCtx_advanced
#define	ZSTD_createCDict zfs_ZSTD_createCDict
#define	ZSTD_createCDict_advanced zfs_ZSTD_createCDict_advanced
#define	ZSTD_createCDict_byReference zfs_ZSTD_createCDict_byReference
#define	ZSTD_createCStream zfs_ZSTD_createCStream
#define	ZSTD_createCStream_advanced zfs_ZSTD_createCStream_advanced
#define	ZSTD_createDCtx zfs_ZSTD_createDCtx
#define	ZSTD_createDCtx_advanced zfs_ZSTD_createDCtx_advanced
#define	ZSTD_createDDict zfs_ZSTD_createDDict
#define	ZSTD_createDDict_advanced zfs_ZSTD_createDDict_advanced
#define	ZSTD_createDDict_byReference zfs_ZSTD_createDDict_byReference
#define	ZSTD_createDStream zfs_ZSTD_createDStream
#define	ZSTD_createDStream_advanced zfs_ZSTD_createDStream_advanced
#define	ZSTD_crossEntropyCost zfs_ZSTD_crossEntropyCost
#define	ZSTD_cycleLog zfs_ZSTD_cycleLog
#define	ZSTD_dParam_getBounds zfs_ZSTD_dParam_getBounds
#define	ZSTD_decodeLiteralsBlock zfs_ZSTD_decodeLiteralsBlock
#define	ZSTD_decodeSeqHeaders zfs_ZSTD_decodeSeqHeaders
#define	ZSTD_decodingBufferSize_min zfs_ZSTD_decodingBufferSize_min
#define	ZSTD_decompress zfs_ZSTD_decompress
#define	ZSTD_decompressBegin zfs_ZSTD_decompressBegin
#define	ZSTD_decompressBegin_usingDDict zfs_ZSTD_decompressBegin_usingDDict
#define	ZSTD_decompressBegin_usingDict zfs_ZSTD_decompressBegin_usingDict
#define	ZSTD_decompressBlock zfs_ZSTD_decompressBlock
#define	ZSTD_decompressBlock_internal zfs_ZSTD_decompressBlock_internal
#define	ZSTD_decompressBound zfs_ZSTD_decompressBound
#define	ZSTD_decompressContinue zfs_ZSTD_decompressContinue
#define	ZSTD_decompressContinueStream zfs_ZSTD_decompressContinueStream
#define	ZSTD_decompressDCtx zfs_ZSTD_decompressDCtx
#define	ZSTD_decompressMultiFrame zfs_ZSTD_decompressMultiFrame
#define	ZSTD_decompressStream zfs_ZSTD_decompressStream
#define	ZSTD_decompressStream_simpleArgs zfs_ZSTD_decompressStream_simpleArgs
#define	ZSTD_decompress_usingDDict zfs_ZSTD_decompress_usingDDict
#define	ZSTD_decompress_usingDict zfs_ZSTD_decompress_usingDict
#define	ZSTD_defaultCParameters zfs_ZSTD_defaultCParameters
#define	ZSTD_did_fieldSize zfs_ZSTD_did_fieldSize
#define	ZSTD_encodeSequences zfs_ZSTD_encodeSequences
#define	ZSTD_encodeSequences_default zfs_ZSTD_encodeSequences_default
#define	ZSTD_endStream zfs_ZSTD_endStream
#define	ZSTD_estimateCCtxSize zfs_ZSTD_estimateCCtxSize
#define	ZSTD_estimateCCtxSize_usingCCtxParams \
	zfs_ZSTD_estimateCCtxSize_usingCCtxParams
#define	ZSTD_estimateCCtxSize_usingCParams \
	zfs_ZSTD_estimateCCtxSize_usingCParams
#define	ZSTD_estimateCDictSize zfs_ZSTD_estimateCDictSize
#define	ZSTD_estimateCDictSize_advanced zfs_ZSTD_estimateCDictSize_advanced
#define	ZSTD_estimateCStreamSize zfs_ZSTD_estimateCStreamSize
#define	ZSTD_estimateCStreamSize_usingCCtxParams \
	zfs_ZSTD_estimateCStreamSize_usingCCtxParams
#define	ZSTD_estimateCStreamSize_usingCParams \
	zfs_ZSTD_estimateCStreamSize_usingCParams
#define	ZSTD_estimateDCtxSize zfs_ZSTD_estimateDCtxSize
#define	ZSTD_estimateDDictSize zfs_ZSTD_estimateDDictSize
#define	ZSTD_estimateDStreamSize zfs_ZSTD_estimateDStreamSize
#define	ZSTD_estimateDStreamSize_fromFrame \
	zfs_ZSTD_estimateDStreamSize_fromFrame
#define	ZSTD_fcs_fieldSize zfs_ZSTD_fcs_fieldSize
#define	ZSTD_fillDoubleHashTable zfs_ZSTD_fillDoubleHashTable
#define	ZSTD_fillHashTable zfs_ZSTD_fillHashTable
#define	ZSTD_findDecompressedSize zfs_ZSTD_findDecompressedSize
#define	ZSTD_findFrameCompressedSize zfs_ZSTD_findFrameCompressedSize
#define	ZSTD_findFrameSizeInfo zfs_ZSTD_findFrameSizeInfo
#define	ZSTD_flushStream zfs_ZSTD_flushStream
#define	ZSTD_frameHeaderSize zfs_ZSTD_frameHeaderSize
#define	ZSTD_free zfs_ZSTD_free
#define	ZSTD_freeCCtx zfs_ZSTD_freeCCtx
#define	ZSTD_freeCCtxParams zfs_ZSTD_freeCCtxParams
#define	ZSTD_freeCDict zfs_ZSTD_freeCDict
#define	ZSTD_freeCStream zfs_ZSTD_freeCStream
#define	ZSTD_freeDCtx zfs_ZSTD_freeDCtx
#define	ZSTD_freeDDict zfs_ZSTD_freeDDict
#define	ZSTD_freeDStream zfs_ZSTD_freeDStream
#define	ZSTD_fseBitCost zfs_ZSTD_fseBitCost
#define	ZSTD_getBlockSize zfs_ZSTD_getBlockSize
#define	ZSTD_getCParams zfs_ZSTD_getCParams
#define	ZSTD_getCParamsFromCCtxParams zfs_ZSTD_getCParamsFromCCtxParams
#define	ZSTD_getCParamsFromCDict zfs_ZSTD_getCParamsFromCDict
#define	ZSTD_getCParams_internal zfs_ZSTD_getCParams_internal
#define	ZSTD_getDDict zfs_ZSTD_getDDict
#define	ZSTD_getDecompressedSize zfs_ZSTD_getDecompressedSize
#define	ZSTD_getDictID_fromDDict zfs_ZSTD_getDictID_fromDDict
#define	ZSTD_getDictID_fromDict zfs_ZSTD_getDictID_fromDict
#define	ZSTD_getDictID_fromFrame zfs_ZSTD_getDictID_fromFrame
#define	ZSTD_getErrorCode zfs_ZSTD_getErrorCode
#define	ZSTD_getErrorName zfs_ZSTD_getErrorName
#define	ZSTD_getErrorString zfs_ZSTD_getErrorString
#define	ZSTD_getFrameContentSize zfs_ZSTD_getFrameContentSize
#define	ZSTD_getFrameHeader zfs_ZSTD_getFrameHeader
#define	ZSTD_getFrameHeader_advanced zfs_ZSTD_getFrameHeader_advanced
#define	ZSTD_getFrameProgression zfs_ZSTD_getFrameProgression
#define	ZSTD_getParams zfs_ZSTD_getParams
#define	ZSTD_getSeqStore zfs_ZSTD_getSeqStore
#define	ZSTD_getSequences zfs_ZSTD_getSequences
#define	ZSTD_getcBlockSize zfs_ZSTD_getcBlockSize
#define	ZSTD_hashPtr zfs_ZSTD_hashPtr
#define	ZSTD_initCDict_internal zfs_ZSTD_initCDict_internal
#define	ZSTD_initCStream zfs_ZSTD_initCStream
#define	ZSTD_initCStream_advanced zfs_ZSTD_initCStream_advanced
#define	ZSTD_initCStream_internal zfs_ZSTD_initCStream_internal
#define	ZSTD_initCStream_srcSize zfs_ZSTD_initCStream_srcSize
#define	ZSTD_initCStream_usingCDict zfs_ZSTD_initCStream_usingCDict
#define	ZSTD_initCStream_usingCDict_advanced \
	zfs_ZSTD_initCStream_usingCDict_advanced
#define	ZSTD_initCStream_usingDict zfs_ZSTD_initCStream_usingDict
#define	ZSTD_initDDict_internal zfs_ZSTD_initDDict_internal
#define	ZSTD_initDStream zfs_ZSTD_initDStream
#define	ZSTD_initDStream_usingDDict zfs_ZSTD_initDStream_usingDDict
#define	ZSTD_initDStream_usingDict zfs_ZSTD_initDStream_usingDict
#define	ZSTD_initFseState zfs_ZSTD_initFseState
#define	ZSTD_initStaticCCtx zfs_ZSTD_initStaticCCtx
#define	ZSTD_initStaticCDict zfs_ZSTD_initStaticCDict
#define	ZSTD_initStaticCStream zfs_ZSTD_initStaticCStream
#define	ZSTD_initStaticDCtx zfs_ZSTD_initStaticDCtx
#define	ZSTD_initStaticDDict zfs_ZSTD_initStaticDDict
#define	ZSTD_initStaticDStream zfs_ZSTD_initStaticDStream
#define	ZSTD_initStats_ultra zfs_ZSTD_initStats_ultra
#define	ZSTD_insertAndFindFirstIndex zfs_ZSTD_insertAndFindFirstIndex
#define	ZSTD_insertAndFindFirstIndexHash3 zfs_ZSTD_insertAndFindFirstIndexHash3
#define	ZSTD_insertAndFindFirstIndex_internal \
	zfs_ZSTD_insertAndFindFirstIndex_internal
#define	ZSTD_insertBlock zfs_ZSTD_insertBlock
#define	ZSTD_invalidateRepCodes zfs_ZSTD_invalidateRepCodes
#define	ZSTD_isFrame zfs_ZSTD_isFrame
#define	ZSTD_ldm_adjustParameters zfs_ZSTD_ldm_adjustParameters
#define	ZSTD_ldm_blockCompress zfs_ZSTD_ldm_blockCompress
#define	ZSTD_ldm_fillHashTable zfs_ZSTD_ldm_fillHashTable
#define	ZSTD_ldm_generateSequences zfs_ZSTD_ldm_generateSequences
#define	ZSTD_ldm_getMaxNbSeq zfs_ZSTD_ldm_getMaxNbSeq
#define	ZSTD_ldm_getTableSize zfs_ZSTD_ldm_getTableSize
#define	ZSTD_ldm_skipSequences zfs_ZSTD_ldm_skipSequences
#define	ZSTD_loadCEntropy zfs_ZSTD_loadCEntropy
#define	ZSTD_loadDEntropy zfs_ZSTD_loadDEntropy
#define	ZSTD_loadDictionaryContent zfs_ZSTD_loadDictionaryContent
#define	ZSTD_makeCCtxParamsFromCParams zfs_ZSTD_makeCCtxParamsFromCParams
#define	ZSTD_malloc zfs_ZSTD_malloc
#define	ZSTD_maxCLevel zfs_ZSTD_maxCLevel
#define	ZSTD_minCLevel zfs_ZSTD_minCLevel
#define	ZSTD_nextInputType zfs_ZSTD_nextInputType
#define	ZSTD_nextSrcSizeToDecompress zfs_ZSTD_nextSrcSizeToDecompress
#define	ZSTD_noCompressLiterals zfs_ZSTD_noCompressLiterals
#define	ZSTD_referenceExternalSequences zfs_ZSTD_referenceExternalSequences
#define	ZSTD_rescaleFreqs zfs_ZSTD_rescaleFreqs
#define	ZSTD_resetCCtx_internal zfs_ZSTD_resetCCtx_internal
#define	ZSTD_resetCCtx_usingCDict zfs_ZSTD_resetCCtx_usingCDict
#define	ZSTD_resetCStream zfs_ZSTD_resetCStream
#define	ZSTD_resetDStream zfs_ZSTD_resetDStream
#define	ZSTD_resetSeqStore zfs_ZSTD_resetSeqStore
#define	ZSTD_reset_compressedBlockState zfs_ZSTD_reset_compressedBlockState
#define	ZSTD_safecopy zfs_ZSTD_safecopy
#define	ZSTD_selectBlockCompressor zfs_ZSTD_selectBlockCompressor
#define	ZSTD_selectEncodingType zfs_ZSTD_selectEncodingType
#define	ZSTD_seqToCodes zfs_ZSTD_seqToCodes
#define	ZSTD_sizeof_CCtx zfs_ZSTD_sizeof_CCtx
#define	ZSTD_sizeof_CDict zfs_ZSTD_sizeof_CDict
#define	ZSTD_sizeof_CStream zfs_ZSTD_sizeof_CStream
#define	ZSTD_sizeof_DCtx zfs_ZSTD_sizeof_DCtx
#define	ZSTD_sizeof_DDict zfs_ZSTD_sizeof_DDict
#define	ZSTD_sizeof_DStream zfs_ZSTD_sizeof_DStream
#define	ZSTD_toFlushNow zfs_ZSTD_toFlushNow
#define	ZSTD_updateRep zfs_ZSTD_updateRep
#define	ZSTD_updateStats zfs_ZSTD_updateStats
#define	ZSTD_updateTree zfs_ZSTD_updateTree
#define	ZSTD_versionNumber zfs_ZSTD_versionNumber
#define	ZSTD_versionString zfs_ZSTD_versionString
#define	ZSTD_writeFrameHeader zfs_ZSTD_writeFrameHeader
#define	ZSTD_writeLastEmptyBlock zfs_ZSTD_writeLastEmptyBlock
#define	algoTime zfs_algoTime
#define	attachDictSizeCutoffs zfs_attachDictSizeCutoffs
#define	g_ctx zfs_g_ctx
#define	g_debuglevel zfs_g_debuglevel
#define	kInverseProbabilityLog256 zfs_kInverseProbabilityLog256
#define	repStartValue zfs_repStartValue
#define	FSE_isError zfs_FSE_isError
#define	HUF_isError zfs_HUF_isError
