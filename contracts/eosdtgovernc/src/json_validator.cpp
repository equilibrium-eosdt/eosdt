#include "eosdtgovernc.hpp"

namespace eosdt {

    auto eosdtgovernc::get_json_parser(const char *json) const {
        json_parser::k_v_map key_values;
        key_values.insert(json_parser::k_v_pair("eosdtcntract.governance_fee", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.stability_fee", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.critical_ltv", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.liquidation_penalty", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.liquidator_discount", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.nut_auct_ratio", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.nut_discount", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.profit_factor", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.vote_period", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.stake_period", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.reserve_ratio", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.staking_weight", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.referral_min_stake", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.referral_ratio", json_parser::VALUE_DOUBLE));


        key_values.insert(json_parser::k_v_pair("eosdliqdatr.eosdtcntract_account", json_parser::VALUE_NAME));

        key_values.insert(json_parser::k_v_pair("eosdtorclize.utility_listing_date", json_parser::VALUE_DATETIME));
        key_values.insert(json_parser::k_v_pair("eosdtorclize.rate_timeout", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtorclize.query_timeout", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtorclize.provablecb1a_interval", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtorclize.delphioracle_interval", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtorclize.equilibriumdsp_interval", json_parser::VALUE_INT));

        key_values.insert(json_parser::k_v_pair("eosdtgovernc.eosdtcntract_account", json_parser::VALUE_NAME));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.min_proposal_weight", json_parser::VALUE_ASSET));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.freeze_period", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.min_participation", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.success_margin", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.top_holders_amount", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.max_bp_count", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.max_bp_votes", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.min_vote_stake", json_parser::VALUE_ASSET));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.unstake_period", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.reward_weight", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.min_reward", json_parser::VALUE_ASSET));

        key_values.insert(json_parser::k_v_pair("reserved", json_parser::VALUE_STRING));
        return json_parser(json, key_values);
    }

    bool eosdtgovernc::validate_proposal_json_main(const char *json) {
        auto parser = get_json_parser(json);
        auto s = parser.validate();
        ds_assert(s == json_parser::STATUS_END, "\r\nproposal_json for type main is invalid, code %", s);
        return true;
    }
}
