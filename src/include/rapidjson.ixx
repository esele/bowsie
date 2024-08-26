// Tencent is pleased to support the open source community by making RapidJSON available.
//
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

module;

export module rapidjson;

#pragma warning(push)
#pragma warning(disable: 5244)

#define NDEBUG true

#include "rapidjson/rapidjson.h"

#include "rapidjson/allocators.h"
#include "rapidjson/cursorstreamwrapper.h"
#include "rapidjson/document.h"
#include "rapidjson/encodedstream.h"
#include "rapidjson/encodings.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/fwd.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/memorybuffer.h"
#include "rapidjson/memorystream.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/pointer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/reader.h"
#include "rapidjson/schema.h"
#include "rapidjson/stream.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/uri.h"
#include "rapidjson/writer.h"

#include "rapidjson/error/en.h"
#include "rapidjson/error/error.h"

#include "rapidjson/internal/biginteger.h"
#include "rapidjson/internal/clzll.h"
#include "rapidjson/internal/diyfp.h"
#include "rapidjson/internal/dtoa.h"
#include "rapidjson/internal/ieee754.h"
#include "rapidjson/internal/itoa.h"
#include "rapidjson/internal/meta.h"
#include "rapidjson/internal/pow10.h"
#include "rapidjson/internal/regex.h"
#include "rapidjson/internal/stack.h"
#include "rapidjson/internal/strfunc.h"
#include "rapidjson/internal/strtod.h"
#include "rapidjson/internal/swap.h"

#include "rapidjson/msinttypes/inttypes.h"
#include "rapidjson/msinttypes/stdint.h"

export using rapidjson::ASCII;
export using rapidjson::AutoUTF;
export using rapidjson::AutoUTFInputStream;
export using rapidjson::AutoUTFOutputStream;
export using rapidjson::BaseReaderHandler;
export using rapidjson::BasicIStreamWrapper;
export using rapidjson::BasicOStreamWrapper;
export using rapidjson::CreateValueByPointer;
// export using rapidjson::CrtAllocator;
export using rapidjson::CursorStreamWrapper;
export using rapidjson::Document;
export using rapidjson::EncodedInputStream;
export using rapidjson::EncodedOutputStream;
export using rapidjson::EraseValueByPointer;
export using rapidjson::FileReadStream;
export using rapidjson::FileWriteStream;
export using rapidjson::Free;
// export using rapidjson::GenericArray;
// export using rapidjson::GenericDocument;
// export using rapidjson::GenericInsituStringStream;
// export using rapidjson::GenericMember;
// export using rapidjson::GenericMemberIterator;
// export using rapidjson::GenericMemoryBuffer;
// export using rapidjson::GenericObject;
// export using rapidjson::GenericPointer;
// export using rapidjson::GenericReader;
// export using rapidjson::GenericSchemaDocument;
// export using rapidjson::GenericSchemaValidator;
// export using rapidjson::GenericStreamWrapper;
// export using rapidjson::GenericStringBuffer;
// export using rapidjson::GenericUri;
// export using rapidjson::GenericValue;
export using rapidjson::GetParseError_En;
export using rapidjson::GetParseErrorFunc;
export using rapidjson::GetPointerParseError_En;
export using rapidjson::GetPointerParseErrorFunc;
export using rapidjson::GetSchemaError_En;
export using rapidjson::GetSchemaErrorFunc;
export using rapidjson::GetValidateError_En;
export using rapidjson::GetValidateErrorFunc;
export using rapidjson::GetValueByPointer;
export using rapidjson::GetValueByPointerWithDefault;
export using rapidjson::IGenericRemoteSchemaDocumentProvider;
export using rapidjson::InsituStringStream;
export using rapidjson::IRemoteSchemaDocumentProvider;
// export using rapidjson::IStreamWrapper;
export using rapidjson::Malloc;
export using rapidjson::MemoryBuffer;
// export using rapidjson::MemoryPoolAllocator;
export using rapidjson::MemoryStream;
export using rapidjson::OStreamWrapper;
export using rapidjson::ParseResult;
export using rapidjson::Pointer;
export using rapidjson::PrettyWriter;
export using rapidjson::PutN;
export using rapidjson::PutReserve;
export using rapidjson::PutUnsafe;
export using rapidjson::Reader;
export using rapidjson::Realloc;
export using rapidjson::SchemaDocument;
export using rapidjson::SchemaValidatingReader;
export using rapidjson::SchemaValidator;
export using rapidjson::SetValueByPointer;
export using rapidjson::size_t;
export using rapidjson::SizeType;
export using rapidjson::SkipWhitespace;
export using rapidjson::Specification;
export using rapidjson::StdAllocator;
export using rapidjson::StreamTraits;
export using rapidjson::StringBuffer;
export using rapidjson::StringRef;
export using rapidjson::StringStream;
export using rapidjson::SwapValueByPointer;
export using rapidjson::Transcoder;
export using rapidjson::Type;
export using rapidjson::Uri;
export using rapidjson::UTF16;
export using rapidjson::UTF16BE;
export using rapidjson::UTF16LE;
export using rapidjson::UTF32;
export using rapidjson::UTF32BE;
export using rapidjson::UTF32LE;
export using rapidjson::UTF8;
export using rapidjson::Value;
export using rapidjson::WIStreamWrapper;
export using rapidjson::WOStreamWrapper;
export using rapidjson::Writer;

// export using rapidjson::internal::AddConst;
// export using rapidjson::internal::

#pragma warning(pop)