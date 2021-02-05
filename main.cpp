#include <openssl/evp.h>
#include <fmt/format.h>
#include "ThreadPool.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <map>
#include <array>
#include <vector>
#include <memory>
#include <future>

static const uint32_t SIXTY_FOUR_KB = 65536u;

namespace fs = std::filesystem;

using std::shared_ptr;
using std::unique_ptr;
using fmt::format;

using DupeMap = std::map<std::vector<uint8_t>, std::vector<fs::path>>;

/**
 * \brief Generates an SHA3-256 hash from the binary data in the given file
 * \param path location of file to hash
 * \return vector containing computed hash
 */
std::vector<uint8_t> hashFile(const fs::path& path)
{
    auto hash = std::array<uint8_t, EVP_MAX_MD_SIZE>();

    auto file = std::ifstream(path, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for reading.");
    }

    // Set up generalized message digest system from OpenSSL
    EVP_MD_CTX *mdCtx = EVP_MD_CTX_new();
    const EVP_MD *md = EVP_sha3_256();
    EVP_DigestInit_ex(mdCtx, md, nullptr);

    // Read file data a chunk at a time.
    const auto& buffSize = SIXTY_FOUR_KB;
    auto buffer = std::array<uint8_t, buffSize>();
    int bytesRead;
    while ((bytesRead = file.readsome(reinterpret_cast<char*>(buffer.data()), buffSize)))
    {
        EVP_DigestUpdate(mdCtx, reinterpret_cast<unsigned char*>(buffer.data()), bytesRead);
    }

    // Finalize digest and clean up.
    uint32_t hashSize;
    EVP_DigestFinal_ex(mdCtx, reinterpret_cast<unsigned char*>(hash.data()),
                       reinterpret_cast<unsigned int *>(&hashSize));

    EVP_MD_CTX_free(mdCtx);

    file.close();

    std::vector<uint8_t> hashVec;
    hashVec.reserve(hashSize);

    std::move(hash.begin(), hash.begin()+hashSize, std::back_inserter(hashVec));

    return hashVec;
}

DupeMap scanDirectory(const fs::path& path, const std::shared_ptr<ThreadPool>& threadPool)
{
    auto dir = path;
    if (!fs::is_directory(dir))
    {
        dir = dir.parent_path();
    }

    auto dupes = std::map<std::vector<uint8_t>, std::vector<fs::path>>();
    for (const auto& item : fs::recursive_directory_iterator(dir))
    {
        if (!fs::is_regular_file(item))
        {
            continue;
        }

        std::clog << format("Checking file: {:s}\n", item.path().string());

        auto res = threadPool->enqueue(hashFile, path);

        res.wait();
        const auto hash = res.get();
        if (dupes.find(hash) == dupes.end())
        {
            // Hash is not in map
            std::vector<fs::path> paths;
            paths.push_back(item.path());
            dupes.insert(std::pair<std::vector<uint8_t>, std::vector<fs::path>>(hash, paths));
        }
        else
        {
            dupes.at(hash).push_back(item.path());
        }
    }

    return dupes;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "File path required.\n";
        return 1;
    }

    auto threadPool = std::make_shared<ThreadPool>();

    const auto startPath = fs::path(argv[1]);

    DupeMap dupes = 

    return 0;
}
