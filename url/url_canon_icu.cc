// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ICU integration functions.

#include <stdlib.h>
#include <string.h>

#include "base/logging.h"
//#include "third_party/icu/source/common/unicode/utf8.h"
//#include "third_party/icu/source/common/unicode/ucnv_cb.h"
//#include "third_party/icu/source/common/unicode/uidna.h"
#include "base/third_party/icu/icu_utf.h"
#include "url/url_canon_icu.h"
#include "url/url_canon_internal.h"  // for _itoa_s

// todo(hege)
#include <windows.h>
typedef int ( WINAPI *Win7_IdnToAscii_Fn)(
	__in                        DWORD    dwFlags,
	__in_ecount(cchUnicodeChar) LPCWSTR  lpUnicodeCharStr,
	__in                        int      cchUnicodeChar,
	__out_ecount(cchASCIIChar)  LPWSTR   lpASCIICharStr,
	__in                        int      cchASCIIChar);
#define IDN_ALLOW_UNASSIGNED      0x01  // Allow unassigned "query" behavior per RFC 3454

static Win7_IdnToAscii_Fn Win7_IdnToAscii = (Win7_IdnToAscii_Fn)
  GetProcAddress(GetModuleHandle(L"kernel32.dll"),"IdnToAscii");

namespace url_canon {
/*
namespace {

// Called when converting a character that can not be represented, this will
// append an escaped version of the numerical character reference for that code
// point. It is of the form "&#1234;" and we will escape the non-digits to
// "%26%231234%3B". Why? This is what Netscape did back in the olden days.
void appendURLEscapedChar(const void* context,
                          UConverterFromUnicodeArgs* from_args,
                          const UChar* code_units,
                          int32_t length,
                          UChar32 code_point,
                          UConverterCallbackReason reason,
                          UErrorCode* err) {
  if (reason == UCNV_UNASSIGNED) {
    *err = U_ZERO_ERROR;

    const static int prefix_len = 6;
    const static char prefix[prefix_len + 1] = "%26%23";  // "&#" percent-escaped
    ucnv_cbFromUWriteBytes(from_args, prefix, prefix_len, 0, err);

    DCHECK(code_point < 0x110000);
    char number[8];  // Max Unicode code point is 7 digits.
    _itoa_s(code_point, number, 10);
    int number_len = static_cast<int>(strlen(number));
    ucnv_cbFromUWriteBytes(from_args, number, number_len, 0, err);

    const static int postfix_len = 3;
    const static char postfix[postfix_len + 1] = "%3B";   // ";" percent-escaped
    ucnv_cbFromUWriteBytes(from_args, postfix, postfix_len, 0, err);
  }
}

// A class for scoping the installation of the invalid character callback.
class AppendHandlerInstaller {
 public:
  // The owner of this object must ensure that the converter is alive for the
  // duration of this object's lifetime.
  AppendHandlerInstaller(UConverter* converter) : converter_(converter) {
    UErrorCode err = U_ZERO_ERROR;
    ucnv_setFromUCallBack(converter_, appendURLEscapedChar, 0,
                          &old_callback_, &old_context_, &err);
  }

  ~AppendHandlerInstaller() {
    UErrorCode err = U_ZERO_ERROR;
    ucnv_setFromUCallBack(converter_, old_callback_, old_context_, 0, 0, &err);
  }

 private:
  UConverter* converter_;

  UConverterFromUCallback old_callback_;
  const void* old_context_;
};

}  // namespace

ICUCharsetConverter::ICUCharsetConverter(UConverter* converter)
    : converter_(converter) {
}

ICUCharsetConverter::~ICUCharsetConverter() {
}

void ICUCharsetConverter::ConvertFromUTF16(const base::char16* input,
                                           int input_len,
                                           CanonOutput* output) {
  // Install our error handler. It will be called for character that can not
  // be represented in the destination character set.
  AppendHandlerInstaller handler(converter_);

  int begin_offset = output->length();
  int dest_capacity = output->capacity() - begin_offset;
  output->set_length(output->length());

  do {
    UErrorCode err = U_ZERO_ERROR;
    char* dest = &output->data()[begin_offset];
    int required_capacity = ucnv_fromUChars(converter_, dest, dest_capacity,
                                            input, input_len, &err);
    if (err != U_BUFFER_OVERFLOW_ERROR) {
      output->set_length(begin_offset + required_capacity);
      return;
    }

    // Output didn't fit, expand
    dest_capacity = required_capacity;
    output->Resize(begin_offset + dest_capacity);
  } while (true);
}
*/
// Converts the Unicode input representing a hostname to ASCII using IDN rules.
// The output must be ASCII, but is represented as wide characters.
//
// On success, the output will be filled with the ASCII host name and it will
// return true. Unlike most other canonicalization functions, this assumes that
// the output is empty. The beginning of the host will be at offset 0, and
// the length of the output will be set to the length of the new host name.
//
// On error, this will return false. The output in this case is undefined.

// todo(hege)
// IDNToASCII 
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd318149(v=vs.85).aspx
//
bool IDNToASCII(const base::char16* src, int src_len, CanonOutputW* output) {
  DCHECK(output->length() == 0);  // Output buffer is assumed empty.

	if(!Win7_IdnToAscii)
		return false;

	while (true) {
    // Use ALLOW_UNASSIGNED to be more tolerant of hostnames that violate
    // the spec (which do exist). This does not present any risk and is a
    // little more future proof.
    //UErrorCode err = U_ZERO_ERROR;
    //int num_converted = uidna_IDNToASCII(src, src_len, output->data(),
    //                                     output->capacity(),
    //                                     UIDNA_ALLOW_UNASSIGNED, NULL, &err);
		int ret = Win7_IdnToAscii(IDN_ALLOW_UNASSIGNED,src,src_len,NULL,0);
    if (!ret) 
			return false;
    output->Resize(ret);
		ret = Win7_IdnToAscii(IDN_ALLOW_UNASSIGNED,src,src_len,output->data(),
			output->capacity());
		if (!ret)
			return false;
		return true;
  }
}

bool ReadUTFChar(const char* str, int* begin, int length,
                 unsigned* code_point_out) {
  int code_point;  // Avoids warning when U8_NEXT writes -1 to it.
  CBU8_NEXT(str, *begin, length, code_point);
  *code_point_out = static_cast<unsigned>(code_point);

  // The ICU macro above moves to the next char, we want to point to the last
  // char consumed.
  (*begin)--;

  // Validate the decoded value.
  if (CBU_IS_UNICODE_CHAR(code_point))
    return true;
  *code_point_out = kUnicodeReplacementCharacter;
  return false;
}

bool ReadUTFChar(const base::char16* str, int* begin, int length,
                 unsigned* code_point) {
  if (CBU16_IS_SURROGATE(str[*begin])) {
    if (!CBU16_IS_SURROGATE_LEAD(str[*begin]) || *begin + 1 >= length ||
        !CBU16_IS_TRAIL(str[*begin + 1])) {
      // Invalid surrogate pair.
      *code_point = kUnicodeReplacementCharacter;
      return false;
    } else {
      // Valid surrogate pair.
      *code_point = CBU16_GET_SUPPLEMENTARY(str[*begin], str[*begin + 1]);
      (*begin)++;
    }
  } else {
    // Not a surrogate, just one 16-bit word.
    *code_point = str[*begin];
  }

  if (CBU_IS_UNICODE_CHAR(*code_point))
    return true;

  // Invalid code point.
  *code_point = kUnicodeReplacementCharacter;
  return false;
}

}  // namespace url_canon
