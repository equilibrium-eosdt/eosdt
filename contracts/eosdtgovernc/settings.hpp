#include "utils.hpp"
#include "info.hpp"

using eosio::current_time_point;

class settings : public eosio::contract {

private:
    typedef eosio::multi_index<"govsettings"_n, govsetting> govsettings_table;
    typedef eosio::multi_index<"ctrsettings"_n, ctrsetting> ctrsettings_table;

    auto now() {
        return current_time_point().sec_since_epoch();
    }

protected:

    auto settings_get() {
        govsettings_table settings(_self, _self.value);
        auto itr = settings.find(0);
        ds_assert(itr != settings.end(), "% %.", NEED_TO_SET_UP, SETTINGS);
        return *itr;
    }

    auto eosdt_ctract_setting_get() {
        auto govsettings = settings_get();
        auto eosd_ctract = govsettings.eosdtcntract_account;
        ctrsettings_table eosdt_ctract_settings(eosd_ctract, eosd_ctract.value);
        return *eosdt_ctract_settings.find(0);
    }

    auto time_get() {
        auto time = ds_time(current_time_point().sec_since_epoch());
        return time;

    }
public:
    settings(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
            eosio::contract(receiver, code, ds) {
    }

    ACTION currentver()
    {
        ds_assert(false,GIT_VERSION);
    }

    ACTION settingset(const ds_account &eosdtcntract_account, const ds_asset &min_proposal_weight,
                      const ds_uint &freeze_period,
                      const double &min_participation, const double &success_margin,
                      const ds_uint &top_holders_amount, const ds_uint &max_bp_count,
                      const ds_uint &max_bp_votes, const ds_asset &min_vote_stake, const ds_uint &unstake_period) {
        PRINT_STARTED("settingset"_n)
        require_auth(_self);

        struct setting_old {
            ds_ulong setting_id;

            ds_ulong primary_key() const { return setting_id; }
        };
        eosio::multi_index<"settings"_n, setting_old> settings_old(_self, _self.value);
        for (auto old = settings_old.begin(); old != settings_old.end(); old = settings_old.erase(old));

        govsettings_table settings(_self, _self.value);
        ds_assert(min_proposal_weight.is_valid() && min_proposal_weight.symbol == USD_SYMBOL,
                  "wrong min_proposal_weight: % expected USD", min_proposal_weight);
        ds_assert(min_vote_stake.symbol == UTILITY_SYMBOL, "wrong min_vote_stake symbol: % expected:  %", min_vote_stake, UTILITY_SYMBOL);
        const auto set = [&](auto &row) {
            row.setting_id = 0;
            row.eosdtcntract_account = eosdtcntract_account;
            row.min_proposal_weight = min_proposal_weight;
            row.freeze_period = freeze_period;
            row.min_participation = min_participation;
            row.success_margin = success_margin;
            row.top_holders_amount = top_holders_amount;
            row.max_bp_count = max_bp_count;
            row.max_bp_votes = max_bp_votes;
            row.min_vote_stake = min_vote_stake;
            row.unstake_period = unstake_period;
        };
        auto itr = settings.find(0);
        if (itr == settings.end()) {
            settings.emplace(_self, set);
        } else {
            settings.modify(itr, ds_account(0), set);
        }
        PRINT_FINISHED("settingset"_n)
    }






};
