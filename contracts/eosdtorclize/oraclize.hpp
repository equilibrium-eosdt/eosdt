#include "rates.hpp"


class oraclize : public rates {
public:
    oraclize(ds_account receiver, ds_account code, datastream<const char *> ds) : rates(receiver, code, ds) {
    }
private:
    typedef eosio::multi_index<"oraqueries"_n, oraqueries> oraqueries_type;
    typedef const oraqueries_type::const_iterator &oraqueries_iterator;

protected:

    bool is_query_running(const ds_symbol &symbol) {
        PRINT_STARTED("isqueryrunn"_n)
        oraqueries_type oraqueries(_self, _self.value);
        auto oraqueries_itr = oraqueries.find(symbol.raw());
        ds_assert(oraqueries_itr != oraqueries.end(), "symbol % does not exists in oraqueries.", symbol);

        auto query = oraqueries_itr->query;
        auto query_executed_at = oraqueries_itr->query_executed_at;
        auto query_checksum = oraqueries_itr->checksumm;

        auto elapsed = (time_get() - query_executed_at).to_seconds();

        auto settings = setting_get();

        auto result = query_checksum != checksum256() && elapsed <= settings.query_timeout;

        PRINT_FINISHED("isqueryrunn"_n)
        return result;
    }

    void refreshrates(const ds_symbol &token_symbol) {
        PRINT_STARTED("refreshrates"_n)


        auto curr_now = time_get();

        oraqueries_type oraqueries(_self, _self.value);
        auto oraqueries_itr = oraqueries.find(token_symbol.raw());
        ds_assert(oraqueries_itr != oraqueries.end(), "unknown symbol %.", token_symbol);

        auto query = oraqueries_itr->query;
        auto query_executed_at = oraqueries_itr->query_executed_at;
        auto query_checksum = oraqueries_itr->checksumm;

        ds_print("\r\nquery: %", query);
        ds_print("\r\nelapsed(%) = curr_now(%) - query_executed_at(%)",
                (curr_now - query_executed_at).to_seconds(), curr_now, query_executed_at);

        auto is_already_running = is_query_running(token_symbol);

        if (is_already_running) {
            ds_print("\r\nSuch query is already running");
        } else {
            auto query_checksumm = oraclize_query("URL", query, proofType_TLSNotary | proofStorage_IPFS);

            ds_assert(query_checksumm != checksum256(), "Bad query checksum. Try again later");

            oraqueries.modify(oraqueries_itr, _self, [&](auto &row) {
                row.checksumm = query_checksumm;
                row.query_executed_at = curr_now;
            });

            ds_print("\r\nquery_checksum: %", query_checksumm);
        }

        PRINT_FINISHED("refreshrates"_n)
    }

public:
    ACTION
    callback(const eosio::checksum256 &query_id, const std::vector<unsigned char> &result,
             const std::vector<unsigned char> &proof) {
        PRINT_STARTED("callback"_n)



        {

            if (!has_auth(provable_cbAddress())) {
                ds_print("\r\nnosuitable auth is found");
                PRINT_FINISHED("callback"_n);
                return;
            }
        }
        oraqueries_type oraqueries(_self, _self.value);
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
        rate_set(itr->asset_symbol, source_type::provablecb1a, i_to_price_type(itr->price_type), result_str);
        PRINT_FINISHED("callback"_n)
    }

    ACTION queryadd(const ds_symbol &symbol, const ds_string &query, const uint8_t price_type) {
        PRINT_STARTED("queryadd"_n)
        require_auth(_self);

        assert_price_type(price_type);

        auto updated_at = time_get();

        oraqueries_type oraqueries(_self, _self.value);

        auto oraqueries_itr = oraqueries.find(symbol.raw());

        auto set = [&](auto &row) {
            row.asset_symbol = symbol;
            row.query = query;
            row.price_type = price_type;

            row.query_updated_at = updated_at;
            row.query_executed_at = ds_time(0);
            row.checksumm = checksum256();
        };

        if (oraqueries_itr == oraqueries.end()) {
            ds_print("\r\nemplace query");
            oraqueries.emplace(_self, set);
        } else {
            ds_print("\r\nchange query");
            oraqueries.modify(oraqueries_itr, _self, set);
        }
        PRINT_FINISHED("queryadd"_n)
    }

    ACTION querydel(const ds_symbol &symbol) {
        PRINT_STARTED("querydel"_n)
        require_auth(_self);

        oraqueries_type oraqueries(_self, _self.value);
        auto oraqueries_itr = oraqueries.find(symbol.raw());
        ds_assert(oraqueries_itr != oraqueries.end(), "unknown symbol %.", symbol);

        oraqueries.erase(oraqueries_itr);

        PRINT_FINISHED("querydel"_n)
    }
};
