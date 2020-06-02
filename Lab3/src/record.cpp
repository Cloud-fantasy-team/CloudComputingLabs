#include <iostream>
#include "errors.hpp"
#include "record.hpp"

namespace cdb {
/*
record.
*/
std::vector<char> record::to_binary() const
{
    std::vector<char> binary;

    /// NOTE: we're using a really ad-hoc format.
    encode_uint8_t(binary, status);
    encode_uint32_t(binary, id);
    encode_uint32_t(binary, next_id);
    return binary;
}

record record::parse(const std::vector<char> &binary)
{
    std::size_t idx = 0;

    std::uint8_t status = decode_uint8_t(binary, idx);
    std::uint32_t id = decode_uint32_t(binary, idx);
    std::uint32_t next_id = decode_uint32_t(binary, idx);

    return {status, id, next_id};
}

/// Helpers.
/// TODO: remove these once we have encode_bytes(size, void*);

void record::encode_uint32_t(std::vector<char> &binary, std::uint32_t val)
{
    /// Little-endian
    binary.push_back(val & 0xFF);
    binary.push_back((val >> 8) & 0xFF);
    binary.push_back((val >> 16) & 0xFF);
    binary.push_back((val >> 24) & 0xFF);
}

void record::encode_uint8_t(std::vector<char> &binary, std::uint8_t val)
{
    /// Little-endian
    binary.push_back(val & 0xFF);
}

std::uint8_t record::decode_uint8_t(const std::vector<char> &binary, std::size_t &idx)
{
    if (idx > binary.size())
        __RECORD_THROW("not enough bytes");

    std::uint8_t ret = 0;
    ret |= binary[idx];
    idx += 1;

    return ret;
}

std::uint32_t record::decode_uint32_t(const std::vector<char> &binary, std::size_t &idx)
{
    if (idx + 4 > binary.size())
        __RECORD_THROW("not enough bytes");

    std::uint32_t ret = 0;
    ret |= binary[idx];
    ret |= (binary[idx + 1] << 8);
    ret |= (binary[idx + 2] << 16);
    ret |= (binary[idx + 3] << 24);
    idx += 4;

    return ret;
}

/*
record_manager
*/

record_manager::record_manager(std::string const &file_name)
    : file_(file_name, std::fstream::app | std::fstream::binary | std::fstream::in | std::fstream::out)
{
    if (!file_.is_open())
        __RECORD_THROW("cannot open log '" + file_name + "'");

    /// Always append.
    // file_.seekp(std::ios::end);
    file_.seekg(std::ios::beg);
    init_records();
}

void record_manager::log(const record &r)
{
    auto binary = r.to_binary();

    try 
    {
        /// Persist to disk.
        file_.write(binary.data(), binary.size());
        file_.flush();
        std::cout << "persist record " << (int)r.status << " " << r.id << " " << r.next_id << std::endl;
        
        /// If an update is done, no need to keep it in memory.
        if (r.status == RECORD_COMMIT_DONE || r.status == RECORD_ABORT_DONE)
            records_.erase(r.id);
        else
            records_[r.id] = r;
    }
    catch (std::exception &e)
    {
        __RECORD_THROW("failed logging record");
    }
}

void record_manager::init_records()
{
    /// Read all records in memory at once.
    std::vector<char> data(std::istreambuf_iterator<char>(file_), {});
    std::uint32_t start = 0;

    while (start < data.size())
    {
        auto r = record::parse(std::vector<char>{ data.begin() + start, /* Begin */
                                                data.begin() + start + record::record_size} /* End */);
        next_id_ = r.next_id;
        /// Ignore resolved record.
        if (r.status == RECORD_ABORT_DONE || r.status == RECORD_COMMIT_DONE)
            records_.erase(r.id);
        else
            records_[r.id] = r;
        start += record::record_size;
    }

    if (records_.empty())
        next_id_ = 0;
}

} // namespace cdb
