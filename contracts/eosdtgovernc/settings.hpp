#include "utils.hpp"
class settings : public eosio::contract {
private:
    typedef eosio::multi_index<"settings"_n, setting> settings_table;
protected:
    auto settings_get() {
        settings_table settings(_self, _self.value);
        auto itr = settings.find(0);
        ds_assert(itr != settings.end(), "% % on forum.", NEED_TO_SET_UP, SETTINGS);
        return *itr;
    }
    auto time_get() {
        auto time = ds_time(now());
        return time;
    }
public:
    settings(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
            eosio::contract(receiver, code, ds) {
    }
    ACTION settingset(const ds_account &eosdtcntract_account, const ds_account &liquidator_account,
                      const ds_account &oraclize_account,
                      const ds_account &nutoken_account, const ds_asset &min_proposal_weight,
                      const ds_uint &freeze_period,
                      const double &min_participation, const double &success_margin,
                      const ds_ulong &top_holders_amount) {
        PRINT_STARTED("timemove"_n)
        require_auth(_self);
        settings_table settings(_self, _self.value);
        ds_assert(min_proposal_weight.is_valid() && min_proposal_weight.symbol == USD_SYMBOL,
                  "wrong min_proposal_weight: % expected USD", min_proposal_weight);
        const auto set = [&](auto &row) {
            row.id = 0;
            row.time_shift = 0;
            row.eosdtcntract_account = eosdtcntract_account;
            row.liquidator_account = liquidator_account;
            row.oraclize_account = oraclize_account;
            row.nutoken_account = nutoken_account;
            row.min_proposal_weight = min_proposal_weight;
            row.freeze_period = freeze_period;
            row.min_participation = min_participation;
            row.success_margin = success_margin;
            row.top_holders_amount = top_holders_amount;
        };
        auto itr = settings.find(0);
        if (itr == settings.end()) {
            settings.emplace(_self, set);
        } else {
            ds_assert(eosdtcntract_account != itr->eosdtcntract_account
                      || liquidator_account != itr->liquidator_account
                      || oraclize_account != itr->oraclize_account
                      || nutoken_account != itr->nutoken_account
                      || min_proposal_weight != itr->min_proposal_weight
                      || freeze_period != itr->freeze_period
                      || min_participation != itr->min_participation
                      || success_margin != itr->success_margin
                      || top_holders_amount != itr->top_holders_amount, "% %.", SETTINGS, ALREADY_STORED);
            settings.modify(itr, ds_account(0), set);
        }
        PRINT_FINISHED("timemove"_n)
    }
};
