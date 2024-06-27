#pragma once

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <ostream>

#include "query/exceptions.h"
#include "graph_models/object_id.h"

class Inliner {
public:
    static uint64_t inline_int(int64_t int_value) {
        if (int_value < 0) {
            int_value *= -1;

            // check if it needs more than 7 bytes
            if ( (int_value & 0xFF00'0000'0000'0000UL) == 0) {
                int_value = (~int_value) & 0x00FF'FFFF'FFFF'FFFFUL;
                return ObjectId::MASK_NEGATIVE_INT | int_value;
            } else {
                // VALUE_EXTERNAL_INT_MASK
                throw NotSupportedException("BIG INTEGERS NOT SUPPORTED YET");
                return 0;
            }
        } else {
            // check if it needs more than 7 bytes
            if ( (int_value & 0xFF00'0000'0000'0000UL) == 0) {
                return ObjectId::MASK_POSITIVE_INT | int_value;
            } else {
                // VALUE_EXTERNAL_INT_MASK
                throw NotSupportedException("BIG INTEGERS NOT SUPPORTED YET");
                return 0;
            }
        }
    }

    // assumes null-terminated string
    static uint64_t inline_string(const char* str) {
        uint64_t res = 0;
        int shift_size = 8*6;
        for (const char* i = str; *i != '\0'; i++) {
            // MUST convert to uint8_t and then to uint64_t.
            // Shift with shift_size >=32 is undefined behaviour.
            uint8_t byte = *i;
            uint64_t byte64 = static_cast<uint64_t>(byte);
            res |= byte64 << shift_size;
            shift_size -= 8;
        }
        return res;
    }

    // Used 5 bytes on RDF literals with datatype or language (this attributes uses 2 additional bytes)
    static uint64_t inline_string5(const char* str) {
        uint64_t res = 0;
        int shift_size = 8*4;
        for (const char* i = str; *i != '\0'; i++) {
            // MUST convert to uint8_t and then to uint64_t.
            // Shift with shift_size >=32 is undefined behaviour.
            uint8_t byte = *i;
            uint64_t byte64 = static_cast<uint64_t>(byte);
            res |= byte64 << shift_size;
            shift_size -= 8;
        }
        return res;
    }

    static uint64_t inline_iri(const char* str) {
        uint64_t res = 0;
        int shift_size = 8*5;
        for (const char* i = str; *i != '\0'; i++) {
            // MUST convert to uint8_t and then to uint64_t.
            // Shift with shift_size >=32 is undefined behaviour.
            uint8_t byte = *i;
            uint64_t byte64 = static_cast<uint64_t>(byte);
            res |= byte64 << shift_size;
            shift_size -= 8;
        }
        return res;
    }

    static uint64_t inline_float(float f) {
        unsigned char bytes[sizeof(float)];
        std::memcpy(bytes, &f, sizeof(float));

        uint64_t res = 0;
        int shift_size = 0;
        for (std::size_t i = 0; i < sizeof(bytes); ++i) {
            uint64_t byte = bytes[i];
            res |= byte << shift_size;
            shift_size += 8;
        }
        return res | ObjectId::MASK_FLOAT;
    }

    //
    template <uint_fast32_t N>
    static uint64_t decode(uint64_t val) {
        static_assert(N > 0 && N <= ObjectId::MAX_INLINED_BYTES);
        uint64_t res = 0; // Ensure null-termination.
        uint8_t* c = reinterpret_cast<uint8_t*>(&res);
        int shift_size = (N - 1) * 8;
        for (uint_fast32_t i = 0; i < N; i++) {
            uint8_t byte = (val >> shift_size) & 0xFF;
            c[i]         = byte;
            shift_size  -= 8;
        }
        return res;
    }

    template <uint_fast32_t N>
    static void print_string_inlined(std::ostream& os, uint64_t unmasked_id) {
        char buff[N];
        int suffix_shift_size = 8 * (N - 1);

        uint_fast32_t str_len = N;
        for (uint_fast32_t i = 0; i < N; i++) {
            uint8_t byte = (unmasked_id >> suffix_shift_size) & 0xFF;
            char c = byte;
            if (c == '\0') {
                str_len = i;
                break;
            }
            buff[i] = c;
            suffix_shift_size -= 8;
        }

        os.write(buff, str_len);
    }
};
