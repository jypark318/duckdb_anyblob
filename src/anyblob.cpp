#define DUCKDB_EXTENSION_MAIN

#include "duckdb.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"

#include "cloud/provider.hpp"
#include "network/TaskedSendReceiver.hpp"

namespace duckdb {

using namespace anyblob;
using namespace anyblob::cloud;
using namespace anyblob::network;

static void AnyBlobReadFunction(DataChunk &args, ExpressionState &state, Vector &result) {
    auto &input = args.data[0];
    auto count = args.size();

    for (idx_t i = 0; i < count; i++) {
        auto path = input.GetValue(i).ToString();

        try {
            // 1. 비동기 전송 객체 초기화
            auto send_receiver = std::make_shared<TaskedSendReceiver>();
            auto handle = send_receiver->createHandle();

            // 2. Provider 생성 (AWS 자동 판별됨)
            auto provider = Provider::makeProvider(path, true /*https*/, "", "", handle.get());

            // 3. GET 요청 생성 (0~4096 byte 범위)
            auto request = provider->getRequest(path, {0, 4096});

            // 4. 요청 전송 (비동기 내부 스레드 처리, 결과는 동기 수신)
            auto response = handle->send(request.get(), provider->getAddress(), provider->getPort());

            // 5. 응답 바디를 문자열로 변환 후 DuckDB BLOB에 저장
            std::string data(reinterpret_cast<const char *>(response->body.data()), response->body.size());
            result.SetValue(i, Value::BLOB(data));
        } catch (std::exception &e) {
            throw InvalidInputException("AnyBlob read failed: %s", e.what());
        }
    }
}

// 확장 로딩 시 함수 등록
void AnyBlobExtension::Load(DuckDB &db) {
    ExtensionUtil::RegisterFunction(db, ScalarFunction(
        "anyblob_read",
        {LogicalType::VARCHAR},
        LogicalType::BLOB,
        AnyBlobReadFunction
    ));
}

} // namespace duckdb
#define DUCKDB_EXTENSION_MAIN

#include "duckdb.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"

#include "cloud/BlobReader.hpp"  // AnyBlob의 실제 헤더에 따라 조정

namespace duckdb {

static void AnyBlobReadFunction(DataChunk &args, ExpressionState &state, Vector &result) {
    auto &input = args.data[0];
    auto count = args.size();

    for (idx_t i = 0; i < count; i++) {
        auto path = input.GetValue(i).ToString();

        try {
            anyblob::BlobReader reader(path);  // 정확한 클래스 이름에 맞게 수정
            std::string data = reader.ReadAll();  // 실제 API 확인 필요

            result.SetValue(i, Value::BLOB(data));
        } catch (std::exception &e) {
            throw InvalidInputException("AnyBlob read failed: %s", e.what());
        }
    }
}

void AnyBlobExtension::Load(DuckDB &db) {
    ExtensionUtil::RegisterFunction(db, ScalarFunction(
        "anyblob_read",
        {LogicalType::VARCHAR},
        LogicalType::BLOB,
        AnyBlobReadFunction
    ));
}

} // namespace duckdb

