#pragma once

#include "utils.hpp"

namespace eosdt {

    typedef eosio::multi_index<"accounts"_n, sysaccount> sysaccounts_table;
    typedef eosio::multi_index<"stat"_n, sysstat> sysstat_table;
    typedef eosio::multi_index<"producers"_n, sysproducer,
            eosio::indexed_by<"prototalvote"_n, eosio::const_mem_fun<sysproducer, double, &sysproducer::by_votes>  >
    > sysproducers_table;
    typedef eosio::multi_index<"global"_n, sysglobal> sysglobal_table;
    typedef eosio::multi_index<"global4"_n, sysglobal4> sysglobal4_table;
    typedef eosio::multi_index<"voters"_n, sysvoter> sysvoters_table;

    typedef eosio::multi_index<"rexpool"_n, rexpool> rexpool_table;
    typedef eosio::multi_index<"rexbal"_n, rexbalance> rexbalance_table;
    typedef eosio::multi_index<"rexqueue"_n, rexqueue,
            eosio::indexed_by<"bytime"_n, eosio::const_mem_fun < rexqueue, uint64_t, &rexqueue::by_time>>>
    rexqueue_table;
    typedef eosio::multi_index<"rexfund"_n, rexfund> rexfund_table;

    typedef eosio::multi_index<"orasettings"_n, orasettings> orasettings_table;
    typedef eosio::multi_index<"orarates"_n, orarate_old> orarates_old_table;
    typedef eosio::multi_index<"oraclerates"_n, orarate,
            eosio::indexed_by<"ratebase"_n, eosio::const_mem_fun < orarate, __uint128_t, &orarate::by_rate_base>>> orarates_table;

    typedef eosio::multi_index<"oraqueries"_n, oraqueries_old> oraqueries_old_table;
    typedef eosio::multi_index<"oraclqueries"_n, oraqueries,
    eosio::indexed_by<"assetsource"_n, eosio::const_mem_fun < oraqueries, __uint128_t, &oraqueries::by_asset_source>>> oraqueries_table;
    typedef eosio::multi_index<"subscribers"_n, orasubscribe> orasubscribers_table;

    typedef eosio::multi_index<"ctrsettings"_n, ctrsetting> ctrsettings_table;
    typedef eosio::multi_index<"ctrsettings"_n, ctrsetting_time> ctrsettings_time_table;
    typedef eosio::multi_index<"parameters"_n, ctrparameter> ctrparameters_table;
    typedef eosio::multi_index<"parameters"_n, posparameter> posparameters_table;
    typedef eosio::multi_index<"positions"_n, ctrposition,
            eosio::indexed_by<"maker"_n, eosio::const_mem_fun < ctrposition, uint64_t, &ctrposition::get_maker>>>
    ctrpositions_table;
    typedef eosio::multi_index<"positions"_n, posposition,
            eosio::indexed_by<"maker"_n, eosio::const_mem_fun < posposition, uint64_t, &posposition::get_maker>>>
    pospositions_table;
    typedef eosio::multi_index<"ctrltvratios"_n, ctrltvratio,
            eosio::indexed_by<"byltvratio"_n,
                    eosio::const_mem_fun < ctrltvratio, double, &ctrltvratio::by_ltv_ratio>>>
    ctrltvratio_table;

    typedef eosio::multi_index<"ctrreferrals"_n, ctrreferral,
            eosio::indexed_by<"byreferral"_n, eosio::const_mem_fun < ctrreferral, uint64_t, &ctrreferral::by_referral>>
            >
    ctrreferrals_table;

    typedef eosio::multi_index<"positionrefs"_n, ctrpositionref,
            eosio::indexed_by<"byreferralid"_n, eosio::const_mem_fun < ctrpositionref, uint64_t, &ctrpositionref::by_referral_id>>
    >
    ctrpositionrefs_table;

    typedef eosio::multi_index<"liqsettings"_n, liqsetting> liqsettings_table;
    typedef eosio::multi_index<"parameters"_n, liqparameter> liqparameters_table;

    typedef eosio::multi_index<"govsettings"_n, govsetting> govsettings_table;
    typedef eosio::multi_index<"proposals"_n, govproposal,
            eosio::indexed_by<"byproposer"_n, eosio::const_mem_fun < govproposal, uint64_t, &govproposal::by_proposer>>
    >
    govproposals_table;
    typedef eosio::multi_index<"govvoters"_n, govvoter> govvoters_table;
    typedef eosio::multi_index<"votes"_n, govvote,
            eosio::indexed_by<"byproposal"_n, eosio::const_mem_fun < govvote, __uint128_t, &govvote::by_proposal>>,
    eosio::indexed_by<"byvoter"_n, eosio::const_mem_fun<govvote, __uint128_t, &govvote::by_voter>>>
    govvotes_table;
    typedef eosio::multi_index<"bpvotes"_n, govbpvote,
            eosio::indexed_by<"byvotes"_n, eosio::const_mem_fun < govbpvote, uint64_t, &govbpvote::byvotes>>>
    govbpvotes_table;
    typedef eosio::multi_index<"govparams"_n, govparam> govparams_table;
    typedef eosio::multi_index<"govbpparams"_n, govbpparam> govbpparams_table;
    typedef eosio::multi_index<"votingstatus"_n, votingstatus_item> votingstatus_table;
    typedef eosio::multi_index<"govposcntrs"_n, govposcntr> govposcntrs_table;

    typedef eosio::multi_index<"ressettings"_n, ressetting> ressettings_table;
    typedef eosio::multi_index<"resparams"_n, resparam> resparams_table;
    typedef eosio::multi_index<"respositions"_n, respositions,
            eosio::indexed_by<"depositor"_n,
                    eosio::const_mem_fun < respositions, uint64_t, &respositions::get_depositor>>,
            eosio::indexed_by<"balance"_n, eosio::const_mem_fun<respositions, uint64_t, &respositions::get_balance>>
    >
    respositions_table;

    typedef eosio::multi_index<"excsettings"_n, excsetting> excsettings_table;

    typedef eosio::multi_index<"locsettings"_n, locsetting> locsettings_table;

    typedef eosio::multi_index<"tstsettings"_n, tstsetting> tstsettings_table;


    typedef eosio::multi_index<"defideavotes"_n, defiideavote> defiideavotes_table;
    typedef eosio::multi_index<"defideas"_n, defiidea,eosio::indexed_by<"byname"_n,
            eosio::const_mem_fun < defiidea, ds_checksum, &defiidea::by_name>>> defiideas_table;
    typedef eosio::multi_index<"defideavotrs"_n, defiideavoter> defiideavoters_table;
}