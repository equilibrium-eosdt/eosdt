#include "eosdtorclize.hpp"
#include "../oraclize/eos_api.hpp"

namespace eosdt {

    void eosdtorclize::callback(const ds_checksum &query_id, const std::vector<unsigned char> &result,
                                const std::vector<unsigned char> &proof) {
        PRINT_STARTED("callback"_n)
        {
            if (!has_auth(provable_cbAddress())) {
                ds_print("\r\nnosuitable auth is found");
                PRINT_FINISHED("callback"_n);
                return;
            }
        }
        oraqueries_table oraqueries(_self, _self.value);
        ds_print("\r\nquery_id: %", query_id);

        ds_assert(query_id != checksum256(), "query_id cannot be empty");

        auto itr = oraqueries.begin();
        for (; itr != oraqueries.end() && !equal(itr->checksumm, query_id); itr++) {
            ds_print("\r\noraqueries: {checksum: %,asset_symbol: %, base: %}.",
                     itr->checksumm, itr->asset_symbol, itr->base);
        }

        if (itr == oraqueries.end()) {
            ds_print("\r\nquery_id % is not found", query_id);
            PRINT_FINISHED("callback"_n);
            return;
        }

        oraqueries.modify(itr, _self, [&](auto &row) {
            row.checksumm = checksum256();
        });

        auto result_str = vector_to_string(result);
        rate_set(itr->asset_symbol, source_type::provablecb1a, i_to_price_type(itr->price_type), itr->base, result_str);
        PRINT_FINISHED("callback"_n)
    }

    bool eosdtorclize::is_query_running(const oraqueries & query, const ds_time & curr_now) {
        PRINT_STARTED("isqueryrunn"_n)

        auto elapsed = (curr_now - query.query_executed_at).to_seconds();
        auto query_timeout = orasetting_get().query_timeout;
        auto result = query.checksumm != checksum256() && elapsed <= query_timeout;
        ds_print("\r\nquery: %, elapsed: %, is_running: %", query.query, elapsed, result);

        PRINT_FINISHED("isqueryrunn"_n)
        return result;
    }

    void eosdtorclize::provablerefresh_internal(const oraqueries & query) {
        PRINT_STARTED("provablerefr"_n)

        auto curr_now = time_get();
        if (is_query_running(query, curr_now)) {
            return;
        }

        auto query_checksumm = oraclize_query("URL", query.query, proofType_TLSNotary | proofStorage_IPFS);
        ds_assert(query_checksumm != checksum256(), "Bad query checksum. Try again later");

        oraqueries_table oraqueries(_self, _self.value);
        auto index = oraqueries.template get_index<"assetsource"_n>();
        auto itr = index.find(compress_key(query.asset_symbol.code().raw(),
                query.source_contract.value ^ query.base.code().raw()));
        ds_assert(itr != index.end(), "refreshrates symbol: %/% and contract: % does not exist in oraqueries.",
                  query.asset_symbol, query.base, query.source_contract);
        index.modify(itr, _self, [&](auto &row) {
            row.checksumm = query_checksumm;
            row.query_executed_at = curr_now;
        });

        ds_print("\r\nnew query_checksum: %", query_checksumm);
        PRINT_FINISHED("provablerefr"_n)
    }
}
