#include "eosdtreserve.hpp"

namespace eosdt {

    void eosdtreserve::currentver() {
        ds_assert(false, GIT_VERSION);
    }

    void eosdtreserve::settingset(const ds_account &eosdtcntract_account, const ds_uint &unstake_period, const ds_asset &mindeposit,
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
}