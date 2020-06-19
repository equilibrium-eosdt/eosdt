#include "eosdtorclize.hpp"
#include "../oraclize/eos_api.hpp"

namespace eosdt {
    void eosdtorclize::queryadd(const ds_symbol &symbol, const ds_account &source_contract, const ds_string &query,
                                const uint8_t price_type, const ds_symbol &base, const double filter) {
        PRINT_STARTED("queryadd"_n)
        require_auth(_self);

        assert_price_type(price_type);
        ds_print("\r\nqueryadd get time.");
        auto updated_at = time_get();
        oraqueries_table oraqueries(_self, _self.value);
        auto queries_empty = oraqueries.begin()==oraqueries.end();
        ds_print("\r\nqueryadd queries_empty=%", queries_empty);
        auto comp_key = compress_key(symbol.code().raw(), source_contract.value^base.code().raw());
        auto modified = false;
        if (!queries_empty) {
            ds_print("\r\nqueryadd get index.");
            auto index = oraqueries.template get_index<"assetsource"_n>();
            ds_assert(index.begin() != index.end(), "need to remove old struct for oraqueries.");
            ds_print("\r\nqueryadd find comp_key:%", comp_key);
            auto itr = index.find(comp_key);
            ds_print("\r\nqueryadd found");
            if (itr != index.end()) {
                ds_print("\r\nmodify query: %", query);
                index.modify(itr, _self, [&](auto &row) {
                    row.asset_symbol = symbol;
                    row.query = query;
                    row.price_type = price_type;
                    row.filter = filter;

                    row.query_updated_at = updated_at;
                    row.query_executed_at = ds_time(0);
                    row.checksumm = checksum256();
                });
                modified = true;
            }
            else {
                ds_print("\r\nquery not found: %", query);
            }
        }
        if (!modified) {
            auto id = oraqueries.available_primary_key();
            ds_print("\r\nemplace query: %, price_type: %, pair: %/%, id:%, comp_key:%",
                    query, price_type, symbol, base, id, comp_key);
            oraqueries.emplace(_self, [&](auto &row) {
                row.id = id;
                row.source_contract = source_contract;
                row.asset_symbol = symbol;
                row.query = query;
                row.price_type = price_type;
                row.base = base;
                row.filter = filter;

                row.query_updated_at = updated_at;
                row.query_executed_at = ds_time(0);
                row.checksumm = checksum256();
            });
        }
        PRINT_FINISHED("queryadd"_n)
    }

    void eosdtorclize::querydel(const ds_symbol &symbol, const ds_account &source_contract, const ds_symbol &base) {
        PRINT_STARTED("querydel"_n)
        require_auth(_self);

        oraqueries_table oraqueries(_self, _self.value);
        auto index = oraqueries.template get_index<"assetsource"_n>();
        ds_assert(oraqueries.begin()==oraqueries.end() || index.begin()!=index.end(),
                  "need to remove old struct for oraqueries.");
        auto itr = index.find(compress_key(symbol.code().raw(), source_contract.value^base.code().raw()));
        ds_assert(itr != index.end(), "unknown pair symbol: %/% and source: %.",
                  symbol, base, source_contract);

        index.erase(itr);

        PRINT_FINISHED("querydel"_n)
    }

}