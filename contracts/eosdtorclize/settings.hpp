#include "utils.hpp"
#include "eos_api.hpp"

class settings : public eosio::contract {


private:
    typedef eosio::multi_index<"orasettings"_n, oracle_settings> oracle_settings_table;

protected:
    auto setting_get() {
        oracle_settings_table settings(_self, _self.value);
        auto itr = settings.find(0);
        ds_assert(itr != settings.end(), "% %.", ORASETTINGS, DOES_NOT_EXIST);
        return *itr;
    }

public:

    settings(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) : contract(receiver, code, ds) {
    }


    void settingset(const ds_int &rate_timeout, const ds_int &query_timeout,
                    const ds_int &master_interval, const ds_int &slave_interval) {
        PRINT_STARTED("settingset"_n)
        require_auth(_self);
        oracle_settings_table settings(_self, _self.value);
        auto itr = settings.find(0);
        const auto set = [&](auto &row) {
            row.id = 0;
            row.rate_timeout = rate_timeout;
            row.query_timeout = query_timeout;
            row.master_interval = master_interval;
            row.slave_interval = slave_interval;
        };
        if (itr == settings.end()) {
            settings.emplace(_self, set);
        } else {
            settings.modify(itr, ds_account(0), set);
        }
        PRINT_FINISHED("settingset"_n)
    }

    void setlistdate(const ds_time &utility_listing_date) {
        PRINT_STARTED("setlistdate"_n)
        require_auth(_self);
        oracle_settings_table settings(_self, _self.value);
        auto itr = settings.find(0);
        ds_assert(itr != settings.end(), "% %.", ORASETTINGS, DOES_NOT_EXIST);
        ds_assert(itr->utility_listing_date.utc_seconds == 0 || time_point_sec(now()) < itr->utility_listing_date, "cant modify listing date");

        settings.modify(itr, ds_account(0), [&](auto &row) {
            row.utility_listing_date = utility_listing_date;
        });

        PRINT_FINISHED("setlistdate"_n)
    }
};
