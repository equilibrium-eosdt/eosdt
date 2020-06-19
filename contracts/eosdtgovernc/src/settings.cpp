#include "eosdtgovernc.hpp"

namespace eosdt {

    void eosdtgovernc::currentver() {
        ds_assert(false, GIT_VERSION);
    }

    void eosdtgovernc::settingset(const ds_account &position_account, const ds_asset &min_proposal_weight,
                                  const ds_uint &freeze_period,
                                  const double &min_participation, const double &success_margin,
                                  const ds_uint &top_holders_amount, const ds_uint &max_bp_count,
                                  const ds_uint &max_bp_votes, const ds_asset &min_vote_stake,
                                  const ds_uint &unstake_period,
                                  const double reward_weight, const double stake_reward) {
        PRINT_STARTED("settingset"_n)
        require_auth(_self);

        struct setting_old {
            ds_ulong setting_id;
            ds_ulong primary_key() const { return setting_id; }
        };
        eosio::multi_index<"govsettings"_n, setting_old> settings_old(_self, _self.value);
        auto deleted = false;
        for (auto old = settings_old.begin(); old != settings_old.end(); old = settings_old.erase(old))
            deleted = true;
        ds_print("\r\ngovsettings deleted %", deleted);
        govsettings_table settings(_self, _self.value);
        ds_assert(min_proposal_weight.is_valid() && min_proposal_weight.symbol == USD_SYMBOL,
                  "wrong min_proposal_weight: % expected USD", min_proposal_weight);
        ds_assert(min_vote_stake.symbol == NUT_SYMBOL, "wrong min_vote_stake symbol: % expected:  %",
                  min_vote_stake, NUT_SYMBOL);
        ds_assert(reward_weight>=0, "wrong reward_weight value: % expected more or equal zero.", reward_weight);
        ds_assert(stake_reward >= 0.0 && stake_reward <= 1.0, "wrong stake_reward value: % expected in [0, 1].", stake_reward);
        ds_assert(is_account(position_account), "% '%' % %.", "position_account",
                  position_account, DOES_NOT_EXIST, AS_AN_ACCOUNT);

        const auto set = [&](auto &row) {
            row.setting_id = 0;
            row.position_account = position_account;
            row.min_proposal_weight = min_proposal_weight;
            row.freeze_period = freeze_period;
            row.min_participation = min_participation;
            row.success_margin = success_margin;
            row.top_holders_amount = top_holders_amount;
            row.max_bp_count = max_bp_count;
            row.max_bp_votes = max_bp_votes;
            row.min_vote_stake = min_vote_stake;
            row.unstake_period = unstake_period;
            row.reward_weight = reward_weight;
            row.stake_reward = stake_reward;
        };
        settings.emplace(_self, set);
        PRINT_FINISHED("settingset"_n)
    }

}