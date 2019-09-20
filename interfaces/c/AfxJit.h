#ifndef ADVANCEDFX_JIT_H
#define ADVANCEDFX_JIT_H
#include "AfxTypes.h"
#include "AfxInterface.h"

#define ADVANCEDFX_JIT_CONTEXT_CODE_ISTRING_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xEFB58A59,0x536D,0x4466,0x9E7D,0x8F,0x4F,0x21,0xA2,0xB2,0x16)

#define ADVANCEDFX_JIT_CONTEXT_LOG_ERROR_ISETSTRING_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xFAFB5DE5,0x30B4,0x4D99,0x9861,0x0E,0xA6,0x6A,0xB9,0xB6,0x37)

#define ADVANCEDFX_JIT_CONTEXT_LOG_WARNING_ISETSTRING_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x4668D6CE,0x0422,0x4B59,0xBD9F,0x32,0xB1,0x33,0x63,0xF5,0xE4)

#define ADVANCEDFX_JIT_CONTEXT_LOG_MESSAGE_ISETSTRING_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0xD8BD0A7F,0x6F5D,0x4F82,0xB7B1,0x11,0x5A,0x1E,0xC6,0x4B,0xC7)

#define ADVANCEDFX_JIT_PYTHON_IJIT_UUID_FN(fn) ADVANCEDFX_UUID_APPLY_FN(fn,0x60B22765,0x9E18,0x410F,0x8401,0x9B,0x1E,0x3C,0xE8,0x74,0xC8)

struct AdvancedxIJit {
	/**
	 * @param outErrorString can be 0.
	 * @returns 0 upon error, assembly otherwise.
	 */
	void (*Execute)(struct AdvancedxIJit* This, AdvancedfxIFactory * context);
};

#endif