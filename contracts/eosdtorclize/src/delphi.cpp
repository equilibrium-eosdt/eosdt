#include "eosdtorclize.hpp"

namespace eosdt {

    void eosdtorclize::delphirefres_internal(const ds_account source) {

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

        {
            eosio::multi_index<"datapoints"_n, datapoints,
                    indexed_by<"value"_n, const_mem_fun<datapoints, uint64_t, &datapoints::by_value>>,
                    indexed_by<"timestamp"_n, const_mem_fun<datapoints, uint64_t, &datapoints::by_timestamp>>>
                                                                                  delphi_datapoint(source, "eosusd"_n.value);
            auto t_idx = delphi_datapoint.get_index<"timestamp"_n>();
            auto itr = t_idx.crbegin();
            if (itr == t_idx.crend()) {
                ds_print("\r\ntable datapoints(%) for eosusd is empty.", source);
            } else {
                auto median = ds_asset(itr->median*pow(10.0, USD_SYMBOL_DECIMAL-4),USD_SYMBOL);
                ds_print("\r\n% median: % (%).", source, itr->median, median);
                if(median.amount>0) {
                    rate_set(source_type::delphioracle, price_type::EOS_TO_SYMBOL, median);
                }
            }
        }
        {
            eosio::multi_index<"datapoints"_n, datapoints,
                    indexed_by<"value"_n, const_mem_fun<datapoints, uint64_t, &datapoints::by_value>>,
                    indexed_by<"timestamp"_n, const_mem_fun<datapoints, uint64_t, &datapoints::by_timestamp>>>
                                                                                  delphi_datapoint(source, "eosnut"_n.value);
            auto t_idx = delphi_datapoint.get_index<"timestamp"_n>();
            auto itr = t_idx.crbegin();
            if (itr == t_idx.crend()) {
                ds_print("\r\ntable datapoints(%) for eosnut is empty.", source);
            } else {
                auto median = ds_asset(itr->median*pow(10.0, UTILITY_SYMBOL_DECIMAL-4),UTILITY_SYMBOL);
                ds_print("\r\n% median: % (%).", source, itr->median, median);
                if(median.amount>0) {
                    rate_set(source_type::delphioracle, price_type::SYMBOL_TO_EOS, median);
                }
            }
        }

    }

    void eosdtorclize::delphirefres() {
        PRINT_STARTED("delphirefres"_n)

        delphirefres_internal("delphioracle"_n);


        PRINT_FINISHED("delphirefres"_n)

    }
}
