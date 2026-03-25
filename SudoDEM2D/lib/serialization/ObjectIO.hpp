// 2010 ©  Václav Šmilauer <eudoxos@arcig.cz>
// 2026 © SudoDEM Project - Migrated to Cereal serialization library

#pragma once

#include <locale>
#include <sstream>
#include <fstream>
#include <string>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/array.hpp>
#include <sudodem/lib/serialization/Compression.hpp>

namespace sudodem{
/* Utility template functions for (de)serializing objects using Cereal from/to streams or files.
*/
struct ObjectIO{
	// tell whether given filename looks like XML
	static bool isXmlFilename(const std::string f){
		return CompressionUtils::endsWith(f,".xml") || CompressionUtils::endsWith(f,".xml.bz2") || CompressionUtils::endsWith(f,".xml.gz");
	}
	// save to given stream and archive format
	template<class T, class oarchive>
	static void save(std::ostream& ofs, const std::string& objectTag, T& object){
		oarchive oa(ofs);
		oa(cereal::make_nvp(objectTag.c_str(), object));
		ofs.flush();
	}
	// load from given stream and archive format
	template<class T, class iarchive>
	static void load(std::istream& ifs, const std::string& objectTag, T& object){
		iarchive ia(ifs);
		ia(cereal::make_nvp(objectTag.c_str(), object));
	}
	// save to given file, guessing compression and XML/binary from extension
	template<class T>
	static void save(const std::string fileName, const std::string& objectTag, T& object){
		std::ostringstream oss;
		if(isXmlFilename(fileName)) save<T,cereal::XMLOutputArchive>(oss,objectTag,object);
		else save<T,cereal::BinaryOutputArchive>(oss,objectTag,object);
		std::string data = oss.str();
		CompressionUtils::writeToFile(fileName, data);
	}
	// load from given file, guessing compression and XML/binary from extension
	template<class T>
	static void load(const std::string fileName, const std::string& objectTag, T& object){
		std::string data = CompressionUtils::readFromFile(fileName);
		std::istringstream iss(data);
		if(isXmlFilename(fileName)) load<T,cereal::XMLInputArchive>(iss,objectTag,object);
		else load<T,cereal::BinaryInputArchive>(iss,objectTag,object);
	}
};

}