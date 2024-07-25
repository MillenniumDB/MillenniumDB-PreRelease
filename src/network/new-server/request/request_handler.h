#pragma once

#include <cstdint>
#include <memory>

#include "network/new-server/request/request_reader.h"
#include "query/executor/query_executor/streaming_query_executor.h"
#include "query/parser/op/op.h"

namespace NewServer {

class StreamingSession;

class RequestHandler {
public:
    StreamingSession& session;

    std::unique_ptr<ResponseWriter> response_writer;

    RequestHandler(StreamingSession& session, std::unique_ptr<ResponseWriter> response_writer) :
        session         (session),
        response_writer (std::move(response_writer)) { }

    virtual ~RequestHandler() = default;

    void handle(const uint8_t* request_bytes, std::size_t request_size);

private:
    std::chrono::system_clock::time_point execution_start;

    uint64_t parser_duration_ms;
    uint64_t optimizer_duration_ms;
    uint64_t execution_duration_ms;

    RequestReader request_reader;

    std::unique_ptr<StreamingQueryExecutor> current_physical_plan;

    virtual std::unique_ptr<Op> create_logical_plan(const std::string& query) = 0;

    virtual std::unique_ptr<StreamingQueryExecutor> create_readonly_physical_plan(Op& logical_plan) = 0;

    // Build the logical and physical plan. On success store the result in current_physical_plan and transition to
    // STREAMING state.
    void handle_run(const std::string& query);

    // Pull num_records from current_physical_plan. On success if no more records are available, transition to READY
    // state, otherwise keep the STREAMING state.
    void handle_pull(uint32_t num_records);

    // Discard the current query execution. On success transition to READY state.
    void handle_discard();

    // Send a catalog response to the client
    void handle_catalog();
};
} // namespace NewServer
