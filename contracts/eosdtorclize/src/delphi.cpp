#include "eosdtorclize.hpp"

namespace eosdt {

    void eosdtorclize::delphirefres() {
        PRINT_STARTED("delphirefres"_n)

        struct datapoints {
            ds_ulong id;
            ds_account owner;
            ds_ulong value;
            ds_ulong median;
            eosio::time_point timestamp;

            ds_ulong primary_key() const { return id; }
            ds_ulong by_timestamp() const {return timestamp.elapsed.to_seconds();}
            ds_ulong by_value() const {return value;}
        };

        oraqueries_table oraqueries(_self, _self.value);
        for (auto itr = oraqueries.begin(); itr != oraqueries.end(); itr++) {
            if (itr->source_contract != "delphioracle"_n
                && itr->source_contract != "eostitantest"_n) {
                continue;
            }
            eosio::multi_index<"datapoints"_n, datapoints,
                    indexed_by<"value"_n, const_mem_fun<datapoints, uint64_t, &datapoints::by_value>>,
                    indexed_by<"timestamp"_n, const_mem_fun<datapoints, uint64_t, &datapoints::by_timestamp>>>
                                                                                  delphi_datapoint(itr->source_contract, name{itr->query}.value);
            auto t_idx = delphi_datapoint.get_index<"timestamp"_n>();
            auto itr_median = t_idx.crbegin();
            if (itr_median == t_idx.crend()) {
                ds_print("\r\ntable datapoints(%) for % is empty.", itr->source_contract, name{itr->query});
            } else {
                ds_asset median;
                if (itr->query == "eosusd" || itr->query == "eosnut" || itr->query == "nutusd" || itr->query == "kgramusd") {
                    median = ds_asset(itr_median->median * pow(10.0, itr->asset_symbol.precision() - 4), itr->asset_symbol);
                } else {
                    ds_assert(false, "New type: % of query.", itr->query);
                }
                ds_print("\r\n% median: % (%/%).", itr->source_contract, itr_median->median, median, itr->base);
                if(median.amount > 0) {
                    rate_set(source_type::delphioracle, i_to_price_type(itr->price_type), itr->base, median);
                }
            }
        }
        PRINT_FINISHED("delphirefres"_n)

    }
}
