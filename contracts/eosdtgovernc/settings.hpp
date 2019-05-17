#include "utils.hpp"


class settings : public eosio::contract {

private:
    typedef eosio::multi_index<"govsettings"_n, govsetting> govsettings_table;


protected:

    auto settings_get() {
        govsettings_table settings(_self, _self.value);
        auto itr = settings.find(0);
        ds_assert(itr != settings.end(), "% %.", NEED_TO_SET_UP, SETTINGS);
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
                      const ds_ulong &top_holders_amount, const std::optional<double> &min_threshold) {
        PRINT_STARTED("timemove"_n)
        require_auth(_self);

        struct setting_old {
            ds_ulong id;

            ds_ulong primary_key() const { return id; }
        };
        eosio::multi_index<"settings"_n, setting_old> settings_old(_self, _self.value);
        for (auto old = settings_old.begin(); old != settings_old.end(); old = settings_old.erase(old));

        govsettings_table settings(_self, _self.value);
        ds_assert(min_proposal_weight.is_valid() && min_proposal_weight.symbol == USD_SYMBOL,
                  "wrong min_proposal_weight: % expected USD", min_proposal_weight);
        const auto set = [&](auto &row) {
            row.setting_id = 0;
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
            if (min_threshold) {
                row.min_threshold = *min_threshold;
            }
            else {
                row.min_threshold = 0.05;
            }

        };
        auto itr = settings.find(0);
        if (itr == settings.end()) {
            settings.emplace(_self, set);
        } else {
            settings.modify(itr, ds_account(0), set);
        }
        PRINT_FINISHED("timemove"_n)
    }
};
