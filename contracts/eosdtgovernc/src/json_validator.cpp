#include "eosdtgovernc.hpp"

namespace eosdt {

    auto eosdtgovernc::get_json_parser(const char *json, const std::map<ds_account,ds_account> &cntract_types) const {
        json_parser::k_v_map key_values;
        auto ctr = EOSDTCNTRACT.to_string();
        key_values[(ctr+".governance_fee").c_str()]=json_parser::VALUE_DOUBLE;
        key_values[(ctr+".stability_fee").c_str()]=json_parser::VALUE_DOUBLE;
        key_values[(ctr+".critical_ltv").c_str()]=json_parser::VALUE_DOUBLE;
        key_values[(ctr+".liquidation_penalty").c_str()]=json_parser::VALUE_DOUBLE;
        key_values[(ctr+".liquidator_discount").c_str()]=json_parser::VALUE_DOUBLE;
        key_values[(ctr+".nut_auct_ratio").c_str()]=json_parser::VALUE_DOUBLE;
        key_values[(ctr+".nut_discount").c_str()]=json_parser::VALUE_DOUBLE;
        key_values[(ctr+".profit_factor").c_str()]=json_parser::VALUE_DOUBLE;
        key_values[(ctr+".vote_period").c_str()]=json_parser::VALUE_INT;
        key_values[(ctr+".stake_period").c_str()]=json_parser::VALUE_INT;
        key_values[(ctr+".reserve_ratio").c_str()]=json_parser::VALUE_DOUBLE;
        key_values[(ctr+".staking_weight").c_str()]=json_parser::VALUE_DOUBLE;
        key_values[(ctr+".referral_min_stake").c_str()]=json_parser::VALUE_DOUBLE;
        key_values[(ctr+".referral_ratio").c_str()]=json_parser::VALUE_DOUBLE;

        ctr = EOSDTLIQDATR.to_string();
        key_values[(ctr+".position_account").c_str()]=json_parser::VALUE_NAME;

        ctr = EOSDTORCLIZE.to_string();
        key_values[(ctr+".utility_listing_date").c_str()]=json_parser::VALUE_DATETIME;
        key_values[(ctr+".rate_timeout").c_str()]=json_parser::VALUE_INT;
        key_values[(ctr+".query_timeout").c_str()]=json_parser::VALUE_INT;
        key_values[(ctr+".provablecb1a_interval").c_str()]=json_parser::VALUE_INT;
        key_values[(ctr+".delphioracle_interval").c_str()]=json_parser::VALUE_INT;
        key_values[(ctr+".equilibriumdsp_interval").c_str()]=json_parser::VALUE_INT;

        ctr = EOSDTGOVERNC.to_string();
        key_values[(ctr+".position_account").c_str()]=json_parser::VALUE_NAME;
        key_values[(ctr+".min_proposal_weight").c_str()]=json_parser::VALUE_ASSET;
        key_values[(ctr+".freeze_period").c_str()]=json_parser::VALUE_INT;
        key_values[(ctr+".min_participation").c_str()]=json_parser::VALUE_DOUBLE;
        key_values[(ctr+".success_margin").c_str()]=json_parser::VALUE_DOUBLE;
        key_values[(ctr+".top_holders_amount").c_str()]=json_parser::VALUE_INT;
        key_values[(ctr+".max_bp_count").c_str()]=json_parser::VALUE_INT;
        key_values[(ctr+".max_bp_votes").c_str()]=json_parser::VALUE_INT;
        key_values[(ctr+".min_vote_stake").c_str()]=json_parser::VALUE_ASSET;
        key_values[(ctr+".unstake_period").c_str()]=json_parser::VALUE_INT;
        key_values[(ctr+".reward_weight").c_str()]=json_parser::VALUE_DOUBLE;
        key_values[(ctr+".min_reward").c_str()]=json_parser::VALUE_ASSET;
        
        key_values["reserved"]=json_parser::VALUE_STRING;
        return json_parser(json, key_values, cntract_types);
    }

    bool eosdtgovernc::validate_proposal_json_main(const char *json, const std::map<ds_account,ds_account> &cntract_types) {
        auto parser = get_json_parser(json, cntract_types);
        auto s = parser.validate();
        ds_assert(s == json_parser::STATUS_END, "\r\nproposal_json for type main is invalid, code %", s);
        return true;
    }
}
