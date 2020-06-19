#include "eosdtorclize.hpp"

namespace eosdt {

    void eosdtorclize::currentver() {
        ds_assert(false, GIT_VERSION);
    }

    void eosdtorclize::getcurrent() {
        ds_print("sec=%\tver=%", time_get(), GIT_VERSION);
    }

    void eosdtorclize::settingset(const ds_int &rate_timeout, const ds_int &query_timeout,
                                  const ds_int &provablecb1a_interval,
                                  const ds_int &delphioracle_interval, const ds_int &equilibriumdsp_interval,
                                  const ds_int &validity_timeout ) {
        PRINT_STARTED("settingset"_n)
        require_auth(_self);
        struct orasetting_del {
            ds_ulong id;

            ds_ulong primary_key() const { return id; }
        };
        eosio::multi_index<"orasettings"_n, orasetting_del> orasettings_del(_self, _self.value);
        for (auto itr = orasettings_del.begin(); itr != orasettings_del.end(); itr = orasettings_del.erase(itr));


        orasettings_table orasettings(_self, _self.value);
        auto itr = orasettings.find(0);
        const auto set = [&](auto &row) {
            row.id = 0;
            row.rate_timeout = rate_timeout;
            row.query_timeout = query_timeout;
            row.provablecb1a_interval = provablecb1a_interval;
            row.delphioracle_interval = delphioracle_interval;
            row.equilibriumdsp_interval = equilibriumdsp_interval;
            row.validity_timeout = validity_timeout;
        };
        if (itr == orasettings.end()) {
            orasettings.emplace(_self, set);
        } else {
            orasettings.modify(itr, ds_account(0), set);
        }
        PRINT_FINISHED("settingset"_n)
    }

}