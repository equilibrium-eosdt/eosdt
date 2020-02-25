#pragma once
#define EOSDTCURRENT "eosdtgovernc"_n

#include "info.hpp"
#include "../../eosdt/eosdt.hpp"


namespace eosdt {

    class [[eosio::contract("eosdtgovernc")]] eosdtgovernc : public eosdt::contract {

    public:
        using eosdt::contract::contract;

        void currentver();

        void settingset(const ds_account &eosdtcntract_account, const ds_asset &min_proposal_weight,
                        const ds_uint &freeze_period,
                        const double &min_participation, const double &success_margin,
                        const ds_uint &top_holders_amount, const ds_uint &max_bp_count,
                        const ds_uint &max_bp_votes, const ds_asset &min_vote_stake, const ds_uint &unstake_period,
                        const double reward_weight, const ds_asset min_reward);

        void propose(const ds_account &proposer, const ds_account &proposal_name,
                     const ds_string &title, const ds_string &proposal_json,
                     const ds_time &expires_at, uint8_t proposal_type
        );

        void clnproposal(const ds_account &proposal_name, const ds_ulong &max_count);

        void transfer(const ds_account &from, const ds_account &to, ds_asset &quantity, const ds_string &memo);

        void unstake(const ds_account &voter, ds_asset &quantity);

        void vote(const ds_account &voter, const ds_account &proposal_name, uint8_t vote, const ds_string &vote_json);

        void unvote(const ds_account &voter, const ds_account &proposal_name);

        void voterreborn(const ds_account &voter);

        void apply(const ds_account &proposal_name);

        void applybpproxy();

        void bpvotescount();

        void expire(const ds_account &proposal_name);

        void bpregister(const ds_account &bp_name, const ds_asset &reward_amount);

        void bpsetparams(const ds_account &bp_name, const ds_asset &reward_amount);

        void bpunregister(const ds_account &bp_name);

        void reinit();

        void calcnutbal();
    private:
        ds_asset voting_amount_get(const ds_account &owner, const ds_symbol &sym) const;

        auto get_json_parser(const char *json) const;

        bool validate_proposal_json_main(const char *json);

        auto proposal_get(const ds_account &proposal_name);

        void proposal_del(const ds_account &proposal_name);

        void voting_amount_add(const ds_account &voter, const ds_asset &value, const ds_account &ram_payer);

        bool voting_amount_sub(const ds_account &voter, const ds_asset &value);

        auto get_json_parser_for_bpvotes(const char *json);

        void validate_bpvote_json(const ds_string &vote_json);

        void activate_bpvote_internal(const ds_account &voter);

        void deactivate_bpvote_internal(const ds_account &voter);

        void apply_ctrsetting(const ds_string &json);

        void apply_liqsetting(const ds_string &json);

        void apply_orasetting(const ds_string &json);

        void apply_govsetting(const ds_string &json);

        void vote_count(const ds_account &proposal_name,
                        ds_int &count_all, ds_int &count_yes, ds_int &count_no,
                        ds_asset &quantity_all, ds_asset &quantity_yes, ds_asset &quantity_no,
                        std::map <std::string, ds_asset> &vote_producers);

        void bpvoteerase(const ds_account &voter);

        void bpdeposit(const ds_string &bp_name, const ds_asset &quantity);

        void dobilling();

        void updatebilling(const std::set <ds_account> &top_producers);

        void updvotingbal(const ds_asset &quantity);

        ds_asset getvotingbal();

        ds_checksum calc_accounts_hash(std::vector <ds_account> &accounts);

        void save_vote_hash(eosio::checksum256 hash);

    };
} /// namespace eosdt

