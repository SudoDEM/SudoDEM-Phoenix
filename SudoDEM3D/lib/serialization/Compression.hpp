// 2026 © SudoDEM Project - Compression utilities using BZip2/ZLIB C API

#pragma once

#include <fstream>
#include <string>
#include <stdexcept>
#include <memory>

// BZip2 C API
extern "C" {
#include <bzlib.h>
}

// ZLIB C API
extern "C" {
#include <zlib.h>
}

namespace sudodem {

// Base compression/decompression interface
struct CompressionUtils {
    // Check if filename ends with extension
    static bool endsWith(const std::string& str, const std::string& suffix) {
        if (suffix.length() > str.length()) return false;
        return str.substr(str.length() - suffix.length()) == suffix;
    }

    // BZip2 compression
    static void compressBzip2(const std::string& inData, std::string& outData) {
        const char* src = inData.data();
        unsigned int srcLen = inData.size();
        
        // Estimate compressed size (worst case: 1.01 * srcLen + 600)
        unsigned int destLen = srcLen * 1.01 + 600;
        outData.resize(destLen);
        
        char* dest = &outData[0];
        int ret = BZ2_bzBuffToBuffCompress(dest, &destLen, const_cast<char*>(src), srcLen, 9, 0, 0);
        
        if (ret != BZ_OK) {
            throw std::runtime_error("BZip2 compression failed: " + std::to_string(ret));
        }
        
        outData.resize(destLen);
    }

    // BZip2 decompression
    static void decompressBzip2(const std::string& inData, std::string& outData) {
        const char* src = inData.data();
        unsigned int srcLen = inData.size();
        
        // Start with original size estimate, will grow if needed
        unsigned int destLen = srcLen * 10;
        int ret;
        
        // Retry with larger buffer if output buffer is too small
        do {
            outData.resize(destLen);
            char* dest = &outData[0];
            ret = BZ2_bzBuffToBuffDecompress(dest, &destLen, const_cast<char*>(src), srcLen, 0, 0);
            
            if (ret == BZ_OUTBUFF_FULL) {
                destLen *= 2;  // Double the buffer size and retry
            }
        } while (ret == BZ_OUTBUFF_FULL);
        
        if (ret != BZ_OK) {
            throw std::runtime_error("BZip2 decompression failed: " + std::to_string(ret));
        }
        
        outData.resize(destLen);
    }

    // GZIP compression
    static void compressGzip(const std::string& inData, std::string& outData) {
        const Bytef* src = reinterpret_cast<const Bytef*>(inData.data());
        uLong srcLen = inData.size();
        
        // Estimate compressed size
        uLong destLen = compressBound(srcLen);
        outData.resize(destLen);
        
        Bytef* dest = reinterpret_cast<Bytef*>(&outData[0]);
        int ret = compress2(dest, &destLen, src, srcLen, 9);
        
        if (ret != Z_OK) {
            throw std::runtime_error("GZIP compression failed: " + std::to_string(ret));
        }
        
        outData.resize(destLen);
    }

    // GZIP decompression
    static void decompressGzip(const std::string& inData, std::string& outData) {
        const Bytef* src = reinterpret_cast<const Bytef*>(inData.data());
        uLong srcLen = inData.size();
        
        // Start with larger size estimate
        uLong destLen = srcLen * 10;
        outData.resize(destLen);
        
        Bytef* dest = reinterpret_cast<Bytef*>(&outData[0]);
        int ret = uncompress(dest, &destLen, src, srcLen);
        
        if (ret != Z_OK) {
            throw std::runtime_error("GZIP decompression failed: " + std::to_string(ret));
        }
        
        outData.resize(destLen);
    }

    // Write data to file with optional compression
    static void writeToFile(const std::string& filename, const std::string& data) {
        if (endsWith(filename, ".bz2")) {
            std::string compressed;
            compressBzip2(data, compressed);
            std::ofstream ofs(filename, std::ios::binary);
            if (!ofs) throw std::runtime_error("Error opening file " + filename + " for writing");
            ofs.write(compressed.data(), compressed.size());
            ofs.close();
            if (!ofs) throw std::runtime_error("Error writing to file " + filename);
        } else if (endsWith(filename, ".gz")) {
            std::string compressed;
            compressGzip(data, compressed);
            std::ofstream ofs(filename, std::ios::binary);
            if (!ofs) throw std::runtime_error("Error opening file " + filename + " for writing");
            ofs.write(compressed.data(), compressed.size());
            ofs.close();
            if (!ofs) throw std::runtime_error("Error writing to file " + filename);
        } else {
            std::ofstream ofs(filename, std::ios::binary);
            if (!ofs) throw std::runtime_error("Error opening file " + filename + " for writing");
            ofs.write(data.data(), data.size());
            ofs.close();
            if (!ofs) throw std::runtime_error("Error writing to file " + filename);
        }
    }

    // Read data from file with optional decompression
    static std::string readFromFile(const std::string& filename) {
        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs) throw std::runtime_error("Error opening file " + filename + " for reading");
        
        std::string data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        
        if (endsWith(filename, ".bz2")) {
            std::string decompressed;
            decompressBzip2(data, decompressed);
            return decompressed;
        } else if (endsWith(filename, ".gz")) {
            std::string decompressed;
            decompressGzip(data, decompressed);
            return decompressed;
        }
        
        return data;
    }
};

} // namespace sudodem