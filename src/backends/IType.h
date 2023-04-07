#pragma once
#include <cstdint>
#include <map>
using DataTypeID = uint32_t;
using DataTypeFlag = uint32_t;
enum DataTypeFlags_
{
	DataTypeFlags_NONE = 0,
	DataTypeFlags_UNSIGNED = 1,
	DataTypeFlags_SIGNED = 2,
	DataTypeFlags_FLOAT = 4,
	DataTypeFlags_INT = 8,
	DataTypeFlags_VEC_TYPE = 16,
	DataTypeFlags_VEC2 = 32,
	DataTypeFlags_VEC3 = 64,
	DataTypeFlags_VEC4 = 128,
	DataTypeFlags_MAT_TYPE = 256,
	DataTypeFlags_MAT2 = 512,
	DataTypeFlags_MAT3 = 1024,
	DataTypeFlags_MAT4 = 2048,
};

struct DataTypeDesc
{
	const char* mName;
	uint32_t mSize;
	DataTypeFlag mFlag;
};

DataTypeDesc GetDataTypeDesc ( DataTypeID id );
const std::map<DataTypeID, DataTypeDesc>& GetDataTypeDescLookupTable ();