#include "eosdtorclize.hpp"

namespace eosdt {

    void eosdtorclize::delphirefresh_internal(const oraqueries & query) {
        delphi_datapoints delphi_datapoint(query.source_contract, name{query.query}.value);
        auto t_idx = delphi_datapoint.get_index<"timestamp"_n>();
        auto itr_median = t_idx.crbegin();
        if (itr_median == t_idx.crend()) {
            ds_print("\r\ntable datapoints(%) for % is empty.", query.source_contract, name{query.query});
            return;
        }
        if (query.query != "eosusd" &&
            query.query != "nutusd" &&
            query.query != "kgramusd" &&
            query.query != "btcusd") {
            ds_assert(false, "New type: % of query.", query.query);
        }
        auto median = ds_asset(itr_median->median * pow(10.0, query.asset_symbol.precision() - 4), query.asset_symbol);
        ds_print("\r\n% median: % (%/%).", query.source_contract, itr_median->median, median, query.base);
        if(median.amount > 0) {
            rate_set(source_type::delphioracle, i_to_price_type(query.price_type), query.base, median);
        }
    }

    void eosdtorclize::delphirefres() {
        PRINT_STARTED("delphirefres"_n)

        oraqueries_table oraqueries(_self, _self.value);
        for (auto query_itr = oraqueries.begin(); query_itr != oraqueries.end(); query_itr++) {
            if (query_itr->source_contract == "delphioracle"_n ||
                    query_itr->source_contract == "eostitantest"_n) {
                delphirefresh_internal(*query_itr);
            }
        }

        PRINT_FINISHED("delphirefres"_n)
    }
}