#include <cassert>

#include "IType.h"
#include <glad/glad.h>
#include <string>

namespace
{
	std::map<DataTypeID, DataTypeDesc> gLookupTable;

	void Parse ( const std::map<DataTypeID, const char*>& map ) {
		for ( auto& [id, name] : map) {
			//std::cout << name << ": " << dataTypeStr << ":" << size << ":" << gpuBindLocation << std::endl;

			std::string nameStr { name };

			std::string token;
			size_t tokenReadIdx = 0;
			uint32_t dataSize = sizeof ( float );
			DataTypeFlag flag = DataTypeFlags_NONE;
			flag |= DataTypeFlags_FLOAT;

			//Get buffer size
			while ( tokenReadIdx < nameStr.length () ) {
				size_t tokenReadEndIdx = nameStr.find_first_of ( '_', tokenReadIdx );

				token = nameStr.substr ( tokenReadIdx, tokenReadEndIdx );

				bool isVec = token.find ( "VEC" ) != std::string::npos;
				bool isMat = token.find ( "MAT" ) != std::string::npos;
				auto i = token.find ( "MAT" );
				if ( isVec || isMat ) {
					uint32_t dim = token.back () - '0';
					if ( isVec ) {
						flag |= DataTypeFlags_VEC_TYPE;
						flag |= ( DataTypeFlags_VEC_TYPE << ( dim - 1 ) );
						dataSize *= dim;
					}
					if ( isMat ) {
						flag |= DataTypeFlags_MAT_TYPE;
						flag |= ( DataTypeFlags_MAT_TYPE << ( dim - 1 ) );
						dataSize *= dim;
						dataSize *= dim;
					}
				}

				if ( tokenReadEndIdx > nameStr.length () )
					break;
				tokenReadIdx = tokenReadEndIdx + 1;
			}

			auto& desc = gLookupTable[ id ];
			desc.mFlag = flag;
			desc.mName = name;
			desc.mSize = dataSize;
		}
	}
	void InitTable ()
	{
		assert ( gLookupTable.empty () );

		std::map<DataTypeID, const char*> tempTable;
		tempTable.emplace ( GL_FLOAT, "FLOAT" );
		tempTable.emplace ( GL_FLOAT_VEC2, "FLOAT_VEC2" );
		tempTable.emplace ( GL_FLOAT_VEC3, "FLOAT_VEC3" );
		tempTable.emplace ( GL_FLOAT_VEC4, "FLOAT_VEC4" );
		//gSupportedDataTypes.emplace(GL_DOUBLE, "	DOUBLE	");
		//gSupportedDataTypes.emplace(GL_DOUBLE_VEC2, "	DOUBLE_VEC2	");
		//gSupportedDataTypes.emplace(GL_DOUBLE_VEC3, "	DOUBLE_VEC3	");
		//gSupportedDataTypes.emplace(GL_DOUBLE_VEC4, "	DOUBLE_VEC4	");
		//gSupportedDataTypes.emplace(GL_INT, "	INT	");
		//gSupportedDataTypes.emplace(GL_INT_VEC2, "	INT_VEC2	");
		//gSupportedDataTypes.emplace(GL_INT_VEC3, "	INT_VEC3	");
		//gSupportedDataTypes.emplace(GL_INT_VEC4, "	INT_VEC4	");
		//gSupportedDataTypes.emplace(GL_UNSIGNED_INT, "	UNSIGNED_INT	");
		//gSupportedDataTypes.emplace(GL_UNSIGNED_INT_VEC2, "	UNSIGNED_INT_VEC2	");
		//gSupportedDataTypes.emplace(GL_UNSIGNED_INT_VEC3, "	UNSIGNED_INT_VEC3	");
		//gSupportedDataTypes.emplace(GL_UNSIGNED_INT_VEC4, "	UNSIGNED_INT_VEC4	");
		//gSupportedDataTypes.emplace(GL_BOOL, "	BOOL	");
		//gSupportedDataTypes.emplace(GL_BOOL_VEC2, "	BOOL_VEC2	");
		//gSupportedDataTypes.emplace(GL_BOOL_VEC3, "	BOOL_VEC3	");
		//gSupportedDataTypes.emplace(GL_BOOL_VEC4, "	BOOL_VEC4	");
		tempTable.emplace ( GL_FLOAT_MAT2, "FLOAT_MAT2" );
		tempTable.emplace ( GL_FLOAT_MAT3, "FLOAT_MAT3" );
		tempTable.emplace ( GL_FLOAT_MAT4, "FLOAT_MAT4" );
		//gSupportedDataTypes.emplace(GL_FLOAT_MAT2x3, "	FLOAT_MAT2x3	");
		//gSupportedDataTypes.emplace(GL_FLOAT_MAT2x4, "	FLOAT_MAT2x4	");
		//gSupportedDataTypes.emplace(GL_FLOAT_MAT3x2, "	FLOAT_MAT3x2	");
		//gSupportedDataTypes.emplace(GL_FLOAT_MAT3x4, "	FLOAT_MAT3x4	");
		//gSupportedDataTypes.emplace(GL_FLOAT_MAT4x2, "	FLOAT_MAT4x2	");
		//gSupportedDataTypes.emplace(GL_FLOAT_MAT4x3, "	FLOAT_MAT4x3	");
		/*gSupportedDataTypes.emplace(GL_DOUBLE_MAT2, "	DOUBLE_MAT2	");
		gSupportedDataTypes.emplace(GL_DOUBLE_MAT3, "	DOUBLE_MAT3	");
		gSupportedDataTypes.emplace(GL_DOUBLE_MAT4, "	DOUBLE_MAT4	");
		gSupportedDataTypes.emplace(GL_DOUBLE_MAT2x3, "	DOUBLE_MAT2x3	");
		gSupportedDataTypes.emplace(GL_DOUBLE_MAT2x4, "	DOUBLE_MAT2x4	");
		gSupportedDataTypes.emplace(GL_DOUBLE_MAT3x2, "	DOUBLE_MAT3x2	");
		gSupportedDataTypes.emplace(GL_DOUBLE_MAT3x4, "	DOUBLE_MAT3x4	");
		gSupportedDataTypes.emplace(GL_DOUBLE_MAT4x2, "	DOUBLE_MAT4x2	");
		gSupportedDataTypes.emplace(GL_DOUBLE_MAT4x3, "	DOUBLE_MAT4x3	");
		gSupportedDataTypes.emplace(GL_SAMPLER_1D, "	SAMPLER_1D	");
		gSupportedDataTypes.emplace(GL_SAMPLER_2D, "	SAMPLER_2D	");
		gSupportedDataTypes.emplace(GL_SAMPLER_3D, "	SAMPLER_3D	");
		gSupportedDataTypes.emplace(GL_SAMPLER_CUBE, "	SAMPLER_CUBE	");
		gSupportedDataTypes.emplace(GL_SAMPLER_1D_SHADOW, "	SAMPLER_1D_SHADOW	");
		gSupportedDataTypes.emplace(GL_SAMPLER_2D_SHADOW, "	SAMPLER_2D_SHADOW	");
		gSupportedDataTypes.emplace(GL_SAMPLER_1D_ARRAY, "	SAMPLER_1D_ARRAY	");
		gSupportedDataTypes.emplace(GL_SAMPLER_2D_ARRAY, "	SAMPLER_2D_ARRAY	");
		gSupportedDataTypes.emplace(GL_SAMPLER_1D_ARRAY_SHADOW, "	SAMPLER_1D_ARRAY_SHADOW	");
		gSupportedDataTypes.emplace(GL_SAMPLER_2D_ARRAY_SHADOW, "	SAMPLER_2D_ARRAY_SHADOW	");
		gSupportedDataTypes.emplace(GL_SAMPLER_2D_MULTISAMPLE, "	SAMPLER_3D_MULTISAMPLE	");
		gSupportedDataTypes.emplace(GL_SAMPLER_2D_MULTISAMPLE_ARRAY, "	SAMPLER_3D_MULTISAMPLE_ARRAY	");
		gSupportedDataTypes.emplace(GL_SAMPLER_CUBE_SHADOW, "	SAMPLER_CUBE_SHADOW	");
		gSupportedDataTypes.emplace(GL_SAMPLER_BUFFER, "	SAMPLER_BUFFER	");
		gSupportedDataTypes.emplace(GL_SAMPLER_2D_RECT, "	SAMPLER_3D_RECT	");
		gSupportedDataTypes.emplace(GL_SAMPLER_2D_RECT_SHADOW, "	SAMPLER_3D_RECT_SHADOW	");
		gSupportedDataTypes.emplace(GL_INT_SAMPLER_1D, "	INT_SAMPLER_1D	");
		gSupportedDataTypes.emplace(GL_INT_SAMPLER_2D, "	INT_SAMPLER_2D	");
		gSupportedDataTypes.emplace(GL_INT_SAMPLER_3D, "	INT_SAMPLER_3D	");
		gSupportedDataTypes.emplace(GL_INT_SAMPLER_CUBE, "	INT_SAMPLER_CUBE	");
		gSupportedDataTypes.emplace(GL_INT_SAMPLER_1D_ARRAY, "	INT_SAMPLER_1D_ARRAY	");
		gSupportedDataTypes.emplace(GL_INT_SAMPLER_2D_ARRAY, "	INT_SAMPLER_2D_ARRAY	");
		gSupportedDataTypes.emplace(GL_INT_SAMPLER_2D_MULTISAMPLE, "	INT_SAMPLER_3D_MULTISAMPLE	");
		gSupportedDataTypes.emplace(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, "	INT_SAMPLER_3D_MULTISAMPLE_ARRAY	");
		gSupportedDataTypes.emplace(GL_INT_SAMPLER_BUFFER, "	INT_SAMPLER_BUFFER	");
		gSupportedDataTypes.emplace(GL_INT_SAMPLER_2D_RECT, "	INT_SAMPLER_3D_RECT	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_SAMPLER_1D, "	UNSIGNED_INT_SAMPLER_1D	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_SAMPLER_2D, "	UNSIGNED_INT_SAMPLER_2D	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_SAMPLER_3D, "	UNSIGNED_INT_SAMPLER_3D	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_SAMPLER_CUBE, "	UNSIGNED_INT_SAMPLER_CUBE	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY, "	UNSIGNED_INT_SAMPLER_1D_ARRAY	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY, "	UNSIGNED_INT_SAMPLER_2D_ARRAY	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE, "	UNSIGNED_INT_SAMPLER_3D_MULTISAMPLE	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, "	UNSIGNED_INT_SAMPLER_3D_MULTISAMPLE_ARRAY	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_SAMPLER_BUFFER, "	UNSIGNED_INT_SAMPLER_BUFFER	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_SAMPLER_2D_RECT, "	UNSIGNED_INT_SAMPLER_3D_RECT	");
		gSupportedDataTypes.emplace(GL_IMAGE_1D, "	IMAGE_1D	");
		gSupportedDataTypes.emplace(GL_IMAGE_2D, "	IMAGE_2D	");
		gSupportedDataTypes.emplace(GL_IMAGE_3D, "	IMAGE_3D	");
		gSupportedDataTypes.emplace(GL_IMAGE_2D_RECT, "	IMAGE_3D_RECT	");
		gSupportedDataTypes.emplace(GL_IMAGE_CUBE, "	IMAGE_CUBE	");
		gSupportedDataTypes.emplace(GL_IMAGE_BUFFER, "	IMAGE_BUFFER	");
		gSupportedDataTypes.emplace(GL_IMAGE_1D_ARRAY, "	IMAGE_1D_ARRAY	");
		gSupportedDataTypes.emplace(GL_IMAGE_2D_ARRAY, "	IMAGE_2D_ARRAY	");
		gSupportedDataTypes.emplace(GL_IMAGE_2D_MULTISAMPLE, "	IMAGE_3D_MULTISAMPLE	");
		gSupportedDataTypes.emplace(GL_IMAGE_2D_MULTISAMPLE_ARRAY, "	IMAGE_3D_MULTISAMPLE_ARRAY	");
		gSupportedDataTypes.emplace(GL_INT_IMAGE_1D, "	INT_IMAGE_1D	");
		gSupportedDataTypes.emplace(GL_INT_IMAGE_2D, "	INT_IMAGE_2D	");
		gSupportedDataTypes.emplace(GL_INT_IMAGE_3D, "	INT_IMAGE_3D	");
		gSupportedDataTypes.emplace(GL_INT_IMAGE_2D_RECT, "	INT_IMAGE_3D_RECT	");
		gSupportedDataTypes.emplace(GL_INT_IMAGE_CUBE, "	INT_IMAGE_CUBE	");
		gSupportedDataTypes.emplace(GL_INT_IMAGE_BUFFER, "	INT_IMAGE_BUFFER	");
		gSupportedDataTypes.emplace(GL_INT_IMAGE_1D_ARRAY, "	INT_IMAGE_1D_ARRAY	");
		gSupportedDataTypes.emplace(GL_INT_IMAGE_2D_ARRAY, "	INT_IMAGE_2D_ARRAY	");
		gSupportedDataTypes.emplace(GL_INT_IMAGE_2D_MULTISAMPLE, "	INT_IMAGE_3D_MULTISAMPLE	");
		gSupportedDataTypes.emplace(GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY, "	INT_IMAGE_3D_MULTISAMPLE_ARRAY	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_IMAGE_1D, "	UNSIGNED_INT_IMAGE_1D	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_IMAGE_2D, "	UNSIGNED_INT_IMAGE_2D	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_IMAGE_3D, "	UNSIGNED_INT_IMAGE_3D	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_IMAGE_2D_RECT, "	UNSIGNED_INT_IMAGE_3D_RECT	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_IMAGE_CUBE, "	UNSIGNED_INT_IMAGE_CUBE	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_IMAGE_BUFFER, "	UNSIGNED_INT_IMAGE_BUFFER	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_IMAGE_1D_ARRAY, "	UNSIGNED_INT_IMAGE_1D_ARRAY	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_IMAGE_2D_ARRAY, "	UNSIGNED_INT_IMAGE_2D_ARRAY	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE, "	UNSIGNED_INT_IMAGE_3D_MULTISAMPLE	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY, "	UNSIGNED_INT_IMAGE_3D_MULTISAMPLE_ARRAY	");
		gSupportedDataTypes.emplace(GL_UNSIGNED_INT_ATOMIC_COUNTER, "	UNSIGNED_INT_ATOMIC_COUNTER	");*/
		Parse ( tempTable );
	}
}

const std::map<DataTypeID, DataTypeDesc>& GetDataTypeDescLookupTable () {
	return gLookupTable;
}

DataTypeDesc GetDataTypeDesc ( DataTypeID id )
{
	static bool isInitialized = false;
	if(isInitialized == false )
	{
		::InitTable ();
		isInitialized = true;
	}
	assert ( gLookupTable.find ( id ) != gLookupTable.end () );
	return gLookupTable[ id ];
}