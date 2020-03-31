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
            ds_print("\r\noraqueries: {checksum: %,asset_symbol: %}.",
                     itr->checksumm, itr->asset_symbol);
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

    bool eosdtorclize::is_query_running(const ds_symbol &symbol, const ds_account &source_contract, const ds_symbol &base) {
        PRINT_STARTED("isqueryrunn"_n)
        oraqueries_table oraqueries(_self, _self.value);
        auto index = oraqueries.template get_index<"assetsource"_n>();
        auto itr = index.find(compress_key(symbol.code().raw(), source_contract.value^base.code().raw()));
        ds_assert(itr != index.end(), "is_query_running symbol: %/% and contract: % does not exists in oraqueries.",
                symbol, base, source_contract);

        auto query = itr->query;
        auto query_executed_at = itr->query_executed_at;
        auto query_checksum = itr->checksumm;

        auto elapsed = (time_get() - query_executed_at).to_seconds();

        auto query_timeout = orasetting_get().query_timeout;

        auto result = query_checksum != checksum256() && elapsed <= query_timeout;

        PRINT_FINISHED("isqueryrunn"_n)
        return result;
    }

    void eosdtorclize::refreshrates(const ds_symbol &symbol, const ds_account &source_contract,
            const ds_symbol &base) {
        PRINT_STARTED("refreshrates"_n)
        auto curr_now = time_get();

        oraqueries_table oraqueries(_self, _self.value);
        auto index = oraqueries.template get_index<"assetsource"_n>();
        auto itr = index.find(compress_key(symbol.code().raw(), source_contract.value^base.code().raw()));
        ds_assert(itr != index.end(), "refreshrates symbol: %/% and contract: % does not exists in oraqueries.",
                  symbol, base, source_contract);

        auto query = itr->query;
        auto query_executed_at = itr->query_executed_at;
        auto query_checksum = itr->checksumm;

        ds_print("\r\nquery: %", query);
        ds_print("\r\nelapsed(%) = curr_now(%) - query_executed_at(%)",
                 (curr_now - query_executed_at).to_seconds(), curr_now, query_executed_at);

        auto is_already_running = is_query_running(symbol, source_contract, base);

        if (is_already_running) {
            ds_print("\r\nSuch query is already running");
        } else {
            auto query_checksumm = oraclize_query("URL", query, proofType_TLSNotary | proofStorage_IPFS);

            ds_assert(query_checksumm != checksum256(), "Bad query checksum. Try again later");

            index.modify(itr, _self, [&](auto &row) {
                row.checksumm = query_checksumm;
                row.query_executed_at = curr_now;
            });

            ds_print("\r\nquery_checksum: %", query_checksumm);
        }

        PRINT_FINISHED("refreshrates"_n)
    }


}
