#include "eosdtgovernc.hpp"

namespace eosdt {
    void eosdtgovernc::addposcntr(const ds_account &position_account) {
        PRINT_STARTED("addposcntr"_n)
        require_auth(_self);
        ds_assert(is_account(position_account), "% '%' % %.", "position_account",
                  position_account, DOES_NOT_EXIST, AS_AN_ACCOUNT);
        govposcntrs_table govposcntrs(_self, _self.value);
        ds_assert(govposcntrs.find(position_account.value) == govposcntrs.end(),
                  "%: '%' %.", "position_account", position_account, ALREADY_STORED);
        govposcntrs.emplace(_self, [&](auto &row) {
            row.position_account = position_account;
        });
        PRINT_FINISHED("addposcntr"_n)
    }

    void eosdtgovernc::remposcntr(const ds_account &position_account) {
        PRINT_STARTED("remposcntr"_n)
        require_auth(_self);
        govposcntrs_table govposcntrs(_self, _self.value);
        auto itr = govposcntrs.find(position_account.value);
        ds_assert(itr != govposcntrs.end(), "%: '%' %.",
                  "position_account", position_account, DOES_NOT_EXIST);
        govposcntrs.erase(itr);
        PRINT_FINISHED("remposcntr"_n)
    }

    auto eosdtgovernc::getposcntr(const ds_account &position_account) {
        PRINT_STARTED("getposcntr"_n)
        std::map <ds_account, ds_account> map;
        govposcntrs_table govposcntrs(_self, _self.value);
        for (auto itr = govposcntrs.begin(); itr != govposcntrs.end(); itr++)
        {
            auto setting = ctrsetting_get(itr->position_account);
            map[itr->position_account] = EOSDTCNTRACT;
            map[setting.liquidator_account] = EOSDTLIQDATR;
            map[setting.oraclize_account] = EOSDTORCLIZE;
            map[setting.governc_account] = EOSDTGOVERNC;
        };
        auto settings = ctrsetting_get(position_account);
        map[position_account] = EOSDTCNTRACT;
        map[settings.liquidator_account] = EOSDTLIQDATR;
        map[settings.oraclize_account] = EOSDTORCLIZE;
        map[EOSDTRESERVE] = EOSDTRESERVE;
        map[settings.governc_account] = EOSDTGOVERNC;
        PRINT_FINISHED("getposcntr"_n)
        return map;
    }

}
