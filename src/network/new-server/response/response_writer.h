#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

#include "graph_models/common/datatypes/datetime.h"
#include "graph_models/object_id.h"
#include "network/new-server/protocol.h"
#include "network/new-server/response/response_buffer.h"
#include "query/executor/binding.h"
#include "query/var_id.h"


namespace NewServer {

/**
 * Response Writer is the class that exposes the methods for writing data to the client. Derived classes must implement
 * the ObjectId encoding for its specific data model.
 */
class ResponseWriter {
public:
    // Convert an object id to its string representation
    virtual std::string encode_object_id(const ObjectId& oid) const = 0;

    virtual uint64_t get_model_id() const = 0;

    virtual uint64_t get_catalog_version() const = 0;

    ResponseWriter(StreamingSession& session_) : response_buffer { session_ }, response_ostream { &response_buffer } { }

    virtual ~ResponseWriter() = default;

    // Force the buffer to be flushed to the stream
    void flush();

    // Primitive types encoding
    std::string encode_null() const;
    std::string encode_bool(bool value) const;
    std::string encode_uint8(uint8_t value) const;
    std::string encode_size(uint32_t value) const;
    std::string encode_float(float value) const;
    std::string encode_double(double value) const;
    std::string encode_uint32(uint32_t value) const;
    std::string encode_uint64(uint64_t value) const;
    std::string encode_int64(int64_t value) const;
    std::string encode_string(const std::string& value, Protocol::DataType data_type) const;
    std::string encode_path(uint64_t path_id) const;
    std::string encode_date(DateTime datetime) const;
    std::string encode_time(DateTime datetime) const;
    std::string encode_datetime(DateTime datetime) const;

    // Helpers writing responses
    void write_run_success(const std::vector<VarId>& projection_vars);
    void write_pull_success_has_next();
    void write_discard_success();
    void write_pull_success_final(uint64_t result_count,
                                  uint64_t parser_duration_ms,
                                  uint64_t optimizer_duration_ms,
                                  uint64_t execution_duration);
    void write_catalog_success();
    void write_record(const std::vector<VarId>& projection_vars, const Binding& binding);
    void write_error(const std::string& message);

    void write_object_id(const ObjectId& oid) {
        const auto encoded_oid = encode_object_id(oid);
        response_ostream.write(encoded_oid.data(), encoded_oid.size());
    }
    void write_null() {
        const auto enc = encode_null();
        response_ostream.write(enc.c_str(), enc.size());
    }
    void write_uint8(uint8_t value) {
        const auto enc = encode_uint8(value);
        response_ostream.write(enc.c_str(), enc.size());
    }
    void write_bool(bool value) {
        const auto enc = encode_bool(value);
        response_ostream.write(enc.c_str(), enc.size());
    }
    void write_uint32(uint32_t value) {
        const auto enc = encode_uint32(value);
        response_ostream.write(enc.c_str(), enc.size());
    }
    void write_float(float value) {
        const auto enc = encode_float(value);
        response_ostream.write(enc.c_str(), enc.size());
    }
    void write_double(double value) {
        const auto enc = encode_double(value);
        response_ostream.write(enc.c_str(), enc.size());
    }
    void write_uint64(uint64_t value) {
        const auto enc = encode_uint64(value);
        response_ostream.write(enc.c_str(), enc.size());
    }
    void write_int64(int64_t value) {
        const auto enc = encode_int64(value);
        response_ostream.write(enc.c_str(), enc.size());
    }
    void write_string(const std::string& value, Protocol::DataType data_type) {
        const auto enc = encode_string(value, data_type);
        response_ostream.write(enc.c_str(), enc.size());
    }
    void write_map_header(uint32_t size) {
        response_ostream.put(static_cast<char>(Protocol::DataType::MAP));
        write_size(size);
    }
    void write_list_header(uint32_t size) {
        response_ostream.put(static_cast<char>(Protocol::DataType::LIST));
        write_size(size);
    }

    // Seals the buffer by calling ResponseBuffer::seal()
    void seal();

private:
    ResponseBuffer response_buffer;

    std::ostream response_ostream;

    // Path helpers. write_path_edge receives a pointer to a number in order to track the path length.
    void write_path_node(std::ostream& os, ObjectId oid) const;
    void write_path_edge(std::ostream& os, ObjectId oid, bool reverse, uint_fast32_t* path_length) const;

    // Write the size prefix of a variable-size object
    void write_size(uint_fast32_t size);
};
} // namespace NewServer
