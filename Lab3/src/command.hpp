/// File: command.hpp
/// =================
/// Copyright 2020 Cloud-fantasy team
/// Contains command objects supported by the KV store.
#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>
#include "rpc/msgpack.hpp"

namespace simple_kv_store {

/// Tags.
typedef uint8_t command_type;
const command_type CMD_GET = 0;
const command_type CMD_SET = 1;
const command_type CMD_DEL = 2;
const command_type CMD_UNKNOWN = 3;
// enum class command_type : uint8_t {
//     GET,
//     SET,
//     DEL,
//     UNKNOWN
// };

/// Base command class.
class command {
public:
    /// Ctor.
    command(command_type type = CMD_UNKNOWN) : type(type) {}
    virtual ~command() = default;

    /// Tag
    command_type type = CMD_UNKNOWN;

    /// Returns the args of this command. 
    /// NOTE: all args are serialized as std::string.
    virtual std::vector<std::string> args() = 0;

    MSGPACK_DEFINE_ARRAY(type);
};

/// GET key
class get_command : public command {
public:
    /// Ctors.
    get_command();
    get_command(std::string &&key);
    get_command(const get_command &);
    get_command(get_command &&);
    virtual ~get_command() = default;

    /// Assignment.
    get_command& operator=(const get_command&);
    get_command& operator=(get_command&&);

    /// GETTER.
    std::string &key() { return key_; }
    const std::string &key() const { return key_; }

    virtual std::vector<std::string> args() override;

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE(command), key_)

private:
    /// Key into the store.
    std::string key_;
};

/// SET key value
class set_command : public command {
public:
    /// Ctors.
    set_command();
    set_command(std::string &&key, std::string &&value);
    set_command(const set_command&);
    set_command(set_command&&);
    virtual ~set_command() = default;

    /// Assignment.
    set_command& operator=(const set_command&);
    set_command& operator=(set_command&&);

    /// GETTER.
    std::string &key() { return key_; }
    const std::string &key() const { return key_; }
    std::string &value() { return value_; }
    const std::string &value() const { return value_; }

    virtual std::vector<std::string> args() override;

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE(command), key_, value_)

private:
    std::string key_;
    std::string value_;
};

/// DEL key1 key2 ...
class del_command : public command {
public:
    /// Ctors.
    del_command();
    del_command(std::vector<std::string> &&args);
    del_command(const del_command&);
    del_command(del_command&&);
    virtual ~del_command() = default;

    /// Assignment.
    del_command& operator=(const del_command&);
    del_command& operator=(del_command&&);

    void set_keys(std::vector<std::string> const &keys) { keys_ = keys; }

    virtual std::vector<std::string> args() override;

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE(command), keys_);

private:
    std::vector<std::string> keys_;
};

} // namespace simple_kv_store


#endif