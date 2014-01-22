/*
 * Centaurean Density
 * http://www.libssc.net
 *
 * Copyright (c) 2013, Guillaume Voirin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Centaurean nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * 21/01/14 21:32
 */

#include <secure/_string.h>
#include "density_api.h"
#include "globals.h"
#include "kernel_encode.h"

typedef struct {
    density_memory_location *fillBuffer;
    uint_fast32_t fillBytes;
} density_kernel_encode_warp_reader;

DENSITY_FORCE_INLINE density_kernel_encode_warp_reader *density_kernel_encode_warp_instanciate_reader(uint_fast32_t byteCount) {
    density_kernel_encode_warp_reader *reader = (density_kernel_encode_warp_reader *) malloc(sizeof(density_kernel_encode_warp_reader));
    //reader->memoryLocation = memoryLocation;
    density_memory_location *fillBuffer = (density_memory_location *) malloc(sizeof(density_memory_location));;
    fillBuffer->pointer = (density_byte *) malloc(byteCount * sizeof(density_byte));
    fillBuffer->available_bytes = byteCount;
    reader->fillBuffer = fillBuffer;
    reader->fillBytes = byteCount;
    return reader;
}

DENSITY_FORCE_INLINE void density_kernel_encode_warp_delete_reader(density_kernel_encode_warp_reader *reader) {
    free(reader->fillBuffer->pointer);
    free(reader);
}

DENSITY_FORCE_INLINE density_memory_location * density_kernel_encode_warp_read(density_kernel_encode_warp_reader *reader, density_memory_location *in, bool flush) {
    if (!reader->fillBuffer->available_bytes)
        reader->fillBuffer->available_bytes = reader->fillBytes;
    if (reader->fillBuffer->available_bytes < reader->fillBytes) {
        if (in->available_bytes < reader->fillBuffer->available_bytes) {
            memcpy(reader->fillBuffer + (reader->fillBytes - reader->fillBuffer->available_bytes), in->pointer, in->available_bytes);
            reader->fillBuffer->available_bytes -= in->available_bytes;
            in->pointer += in->available_bytes;
            in->available_bytes = 0;
            return NULL;
        } else {
            memcpy(reader->fillBuffer + (reader->fillBytes - reader->fillBuffer->available_bytes), in->pointer, reader->fillBuffer->available_bytes);
            reader->fillBuffer->available_bytes = 0;
            in->pointer += reader->fillBuffer->available_bytes;
            in->available_bytes -= reader->fillBuffer->available_bytes;
            return reader->fillBuffer;
        }
    } else if (in->available_bytes < reader->fillBytes) {
        memcpy(reader->fillBuffer + (reader->fillBytes - reader->fillBuffer->available_bytes), in->pointer, in->available_bytes);
        reader->fillBuffer->available_bytes -= in->available_bytes;
        in->pointer += in->available_bytes;
        in->available_bytes = 0;
        return NULL;
    } else {
        return in;
    }
}


