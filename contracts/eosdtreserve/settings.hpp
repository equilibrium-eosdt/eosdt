#include "utils.hpp"

using eosio::current_time_point;

class settings : public eosio::contract {


private:
    typedef eosio::multi_index<"ressettings"_n, ressettings> ressettings_table;

protected:

    ds_time time_get() {

        auto time = ds_time(current_time_point().sec_since_epoch());
        return time;

    }

    auto settings_get() {
        ressettings_table ressettings(_self, _self.value);
        auto itr = ressettings.find(0);
        ds_assert(itr != ressettings.end(), "% %.", SETTINGS, DOES_NOT_EXIST);
        return *itr;
    }

public:

    settings(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) : contract(receiver, code, ds) {
    }


    void settingset(const ds_account &eosdtcntract_account, const ds_uint &unstake_period, const ds_asset &mindeposit,
            const ds_uint &maxdepositors) {
        PRINT_STARTED("settingset"_n)
        require_auth(_self);

        ds_assert(is_account(eosdtcntract_account), "% '%' % %.", "eosdtcntract_account", eosdtcntract_account,
                  DOES_NOT_EXIST, AS_AN_ACCOUNT);

        ressettings_table ressettings(_self, _self.value);
        auto itr = ressettings.find(0);
        const auto set = [&](auto &row) {
            row.setting_id = 0;
            row.eosdtcntract_account = eosdtcntract_account;
            row.unstake_period = unstake_period;
            row.mindeposit = mindeposit;
            row.maxdepositors = maxdepositors;
        };
        if (itr == ressettings.end()) {
            ressettings.emplace(_self, set);
        } else {
            ressettings.modify(itr, ds_account(0), set);
        }
        PRINT_FINISHED("settingset"_n)
    }
};
