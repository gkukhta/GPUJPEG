/*
 * Copyright (c) 2011-2019, CESNET z.s.p.o
 * Copyright (c) 2011, Silicon Genome, LLC.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
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
/**
 * @file
 * This file contains common definitions and declarations that doesn't need to
 * be exposed to public API.
 */
 
#ifndef GPUJPEG_COMMON_INTERNAL_H
#define GPUJPEG_COMMON_INTERNAL_H

#include <cuda_runtime.h>
#include <math.h> // NAN
#include "gpujpeg_util.h"
#include "libgpujpeg/gpujpeg_common.h"
#include "libgpujpeg/gpujpeg_type.h"

struct gpujpeg_timer {
    cudaEvent_t start;
    cudaEvent_t stop;
};

#define GPUJPEG_CUSTOM_TIMER_CREATE(name) \
    do { \
        GPUJPEG_CHECK(cudaEventCreate(&(name).start), ); \
        GPUJPEG_CHECK(cudaEventCreate(&(name).stop), ); \
    } while (0)

#define GPUJPEG_CUSTOM_TIMER_DESTROY(name) \
    do { \
        GPUJPEG_CHECK(cudaEventDestroy((name).start), ); \
        GPUJPEG_CHECK(cudaEventDestroy((name).stop), ); \
    } while (0)

/**
 * Start timer
 *
 * @param name
 * @todo stream
 */
#define GPUJPEG_CUSTOM_TIMER_START(name, stream) \
    GPUJPEG_CHECK(cudaEventRecord((name).start, stream), )

/**
 * Stop timer
 *
 * @param name
 */
#define GPUJPEG_CUSTOM_TIMER_STOP(name, stream) \
    GPUJPEG_CHECK(cudaEventRecord((name).stop, stream), )

/**
 * Get duration for timer
 *
 * @param name
 */
#define GPUJPEG_CUSTOM_TIMER_DURATION(name) \
    gpujpeg_custom_timer_get_duration((name).start, (name).stop)

/**
 * Default timer implementation
 */
#define GPUJPEG_TIMER_INIT() \
    struct gpujpeg_timer def; \
    GPUJPEG_CUSTOM_TIMER_CREATE(def)
#define GPUJPEG_TIMER_START() GPUJPEG_CUSTOM_TIMER_START(def, 0)
#define GPUJPEG_TIMER_STOP() GPUJPEG_CUSTOM_TIMER_STOP(def, 0)
#define GPUJPEG_TIMER_DURATION() GPUJPEG_CUSTOM_TIMER_DURATION(def)
#define GPUJPEG_TIMER_DEINIT() GPUJPEG_CUSTOM_TIMER_DESTROY(def)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * JPEG segment structure. Segment is data in scan generated by huffman coder
 * for N consecutive MCUs, where N is restart interval (e.g. data for MCUs between
 * restart markers)
 */
struct gpujpeg_segment
{
    /// Scan index (in which segment belongs)
    int scan_index;
    /// Segment index in the scan (position of segment in scan starting at 0)
    int scan_segment_index;
    /// MCU count in segment
    int mcu_count;

    /// Data compressed index (output/input data from/to segment for encoder/decoder)
    int data_compressed_index;
    /// Date temp index (temporary data of segment in CC 2.0 encoder)
    int data_temp_index;
    /// Data compressed size (output/input data from/to segment for encoder/decoder)
    int data_compressed_size;

    /// Offset of first block index
    int block_index_list_begin;
    /// Number of blocks of the segment
    int block_count;
};

/**
 * JPEG color component structure
 */
struct gpujpeg_component
{
    /// Component type (luminance or chrominance)
    enum gpujpeg_component_type type;

    /// Component sampling factor (horizontal and vertical)
    struct gpujpeg_component_sampling_factor sampling_factor;

    /// Real component width
    int width;
    /// Real component height
    int height;

    /// Allocated data width for component (rounded to 8 for 8x8 blocks)
    int data_width;
    /// Allocated data height for component (rounded to 8 for 8x8 blocks)
    int data_height;
    /// Allocated data size for component
    int data_size;

    /// MCU size for component (minimun coded unit size)
    int mcu_size;
    /// MCU size in component x-axis
    int mcu_size_x;
    /// MCU size in component y-axis
    int mcu_size_y;

    /// MCU maximum compressed size for component
    int mcu_compressed_size;

    /// MCU count for component (for interleaved mode the same value as [en|de]coder->mcu_count)
    int mcu_count;
    /// MCU count in component x-axis
    int mcu_count_x;
    /// MCU count in component y-axis
    int mcu_count_y;
    /// Segment count in component
    int segment_count;
    /// MCU count per segment in component (the last segment can contain less MCUs, but all other must contain this count)
    int segment_mcu_count;

    /// Preprocessor data in device memory (output/input for encoder/decoder)
    uint8_t* d_data;

    /// DCT and quantizer data in host memory (output/input for encoder/decoder)
    int16_t* data_quantized;
    /// DCT and quantizer data in device memory (output/input for encoder/decoder)
    int16_t* d_data_quantized;
    /// Index of DCT and quantizer data in device and host buffers
    unsigned int data_quantized_index;

    int dc_huff_idx; ///< Huffman DC table index
    int ac_huff_idx; ///< Huffman AC table index
};

/**
 * Print component data
 *
 * @param component
 * @param d_data
 */
void
gpujpeg_component_print8(struct gpujpeg_component* component, uint8_t* d_data);

/**
 * Print component data
 *
 * @param component
 * @param d_data
 */
void
gpujpeg_component_print16(struct gpujpeg_component* component, int16_t* d_data);

/**
 * JPEG coder structure
 */
struct gpujpeg_coder
{
    /// Parameters (quality, restart_interval, etc.)
    struct gpujpeg_parameters param;

    /// Parameters for image data (width, height, comp_count, etc.)
    struct gpujpeg_image_parameters param_image;

    /// Color components
    struct gpujpeg_component* component;
    /// Color components in device memory
    struct gpujpeg_component* d_component;
    /// Number of allocated components
    int component_allocated_size;

    /// Segments for all components
    struct gpujpeg_segment* segment;
    /// Segments in device memory for all components
    struct gpujpeg_segment* d_segment;
    /// Number of allocated segments
    int segment_allocated_size;

    /// Preprocessor data (kernel function pointer)
    void* preprocessor;

    /// Maximum sampling factor from components
    struct gpujpeg_component_sampling_factor sampling_factor;
    /// MCU size (for all components)
    int mcu_size;
    /// MCU compressed size (for all components)
    int mcu_compressed_size;
    /// MCU count (for all components)
    int mcu_count;
    /// Segment total count for all components
    int segment_count;
    /// MCU count per segment (the last segment can contain less MCUs, but all other must contain this count)
    int segment_mcu_count;

    /// Image data width
    int data_width;
    /// Image data height
    int data_height;
    /// Number of raw image bytes
    int data_raw_size;
    /// Number of coefficient count for all components
    int data_size;
    /// Number of compressed bytes
    int data_compressed_size;

    /// Number od 8x8 blocks in all components
    int block_count;

    /// List of block indices in host memory (lower 7 bits are index of component,
    /// 8th bit is 0 for luminance block or 1 for chroma block and bits from 9
    /// above are base index of the block in quantized buffer data)
    uint64_t* block_list;
    /// List of block indices in device memory (same format as host-memory block list)
    uint64_t* d_block_list;
    /// Number of allocated components
    int block_allocated_size;

    /// Raw image data in host memory (loaded from file for encoder, saved to file for decoder)
    uint8_t* data_raw;
    /// Raw image data in device memory (loaded from file for encoder, saved to file for decoder)
    uint8_t* d_data_raw;
    /// Memory allocated by gpujpeg
    uint8_t* d_data_raw_allocated;
    /// Allocated data size
    size_t data_raw_allocated_size;

    /// Preprocessor data in device memory (output/input for encoder/decoder)
    uint8_t* d_data;
    /// DCT and quantizer data in host memory (output/input for encoder/decoder)
    int16_t* data_quantized;
    /// DCT and quantizer data in device memory (output/input for encoder/decoder)
    int16_t* d_data_quantized;
    /// Allocated size
    size_t data_allocated_size;

    /// Huffman coder data in host memory (output/input for encoder/decoder)
    uint8_t* data_compressed;
    /// Huffman coder data in device memory (output/input for encoder/decoder)
    uint8_t* d_data_compressed;
    /// Huffman coder temporary data (in device memory only)
    uint8_t* d_temp_huffman;
    /// Allocated size
    size_t data_compressed_allocated_size;

    int cuda_cc_major; ///< CUDA Compute capability (major version)
    int cuda_cc_minor; ///< CUDA Compute capability (minor version)

    // Operation durations
    struct gpujpeg_timer duration_memory_to;
    struct gpujpeg_timer duration_memory_from;
    struct gpujpeg_timer duration_memory_map;
    struct gpujpeg_timer duration_memory_unmap;
    struct gpujpeg_timer duration_preprocessor;
    struct gpujpeg_timer duration_dct_quantization;
    struct gpujpeg_timer duration_huffman_coder;
    struct gpujpeg_timer duration_stream;
    struct gpujpeg_timer duration_in_gpu;

    size_t allocated_gpu_memory_size; ///< for gpujpeg_encoder_max_pixels() only (remove?)
};

/**
 * Initialize JPEG coder (allocate buffers and initialize structures)
 *
 * @param codec  Codec structure
 * @return 0 if succeeds, otherwise nonzero
 */
int
gpujpeg_coder_init(struct gpujpeg_coder* coder);

/**
 * Initialize JPEG coder (allocate buffers and initialize structures)
 *
 * @param codec        Codec structure
 * @param param
 * @param param_image
 * @param stream       CUDA stream
 * @return size of allocated device memory in bytes if succeeds, otherwise 0
 */
size_t
gpujpeg_coder_init_image(struct gpujpeg_coder * coder, const struct gpujpeg_parameters * param, const struct gpujpeg_image_parameters * param_image, cudaStream_t stream);

/**
 * Returns duration statistics for last coded image
 */
int
gpujpeg_coder_get_stats(struct gpujpeg_coder *coder, struct gpujpeg_duration_stats *stats);

/**
 * Deinitialize JPEG coder (free buffers)
 *
 * @param codec  Codec structure
 * @return 0 if succeeds, otherwise nonzero
 */
int
gpujpeg_coder_deinit(struct gpujpeg_coder* coder);

struct gpujpeg_component;

/**
 * Returns convenient name for subsampling (4:2:0 etc.). If it cannot be constructed
 * returns the format W1xH1:W2xH2:W3xH3.
 */
const char*
gpujpeg_subsampling_get_name(int comp_count, const struct gpujpeg_component *components);

/**
 * Returns gpujpeg_component[] which has sampling factors set
 * native to given pixel_format
 */
const struct gpujpeg_component *
gpujpeg_get_component_subsampling(enum gpujpeg_pixel_format pixel_format);

/**
 * Returns subsampling configuration of a planar pixel format in array of 8
 * ints - [W0 H0 W1 H1 ...] where each number represent given component
 * horizontal/vertical sampling factor as defined for JPEG, eg. (2 1 1 1 1 1)
 * for 4:2:2 (first comonent - Y - is sampled in horizontal dimension twice
 * compared to the remaining components.
 *
 * @returns array of 8 components representing the sampling factor of the pixel format
 */
const int *
gpujpeg_pixel_format_get_sampling_factor(enum gpujpeg_pixel_format pixel_format);

/** Returns number of bytes per pixel */
int
gpujpeg_pixel_format_get_unit_size(enum gpujpeg_pixel_format pixel_format);

/** Returns true if a pixel format is interleaved (packed, more than one component) */
int
gpujpeg_pixel_format_is_interleaved(enum gpujpeg_pixel_format pixel_format);

/**
 * @retval 0 parameters are different
 * @retval 1 parameters are the same
 */
int
gpujpeg_parameters_equals(const struct gpujpeg_parameters *p1 , const struct gpujpeg_parameters *p2);

/**
 * @retval 0 parameters are different
 * @retval 1 parameters are the same
 */
int
gpujpeg_image_parameters_equals(const struct gpujpeg_image_parameters *p1 , const struct gpujpeg_image_parameters *p2);

/**
 * returns difference between specified CUDA events
 *
 * @returns duration in ms, 0.0F in case of error
 */
float
gpujpeg_custom_timer_get_duration(cudaEvent_t start, cudaEvent_t stop);


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // GPUJPEG_COMMON_INTERNAL_H

/* vi: set expandtab sw=4: */
