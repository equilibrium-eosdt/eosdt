#include "eosdtgovernc.hpp"

#include "settings.cpp"
#include "json_parser.hpp"
#include "proposals.cpp"
#include "voters.cpp"
#include "bpvotes.cpp"
#include "json_validator.cpp"
#include "votes.cpp"
#include "applicator.cpp"
#include "params.cpp"

namespace eosdt {
}/// namespace eosdt

EOSIO_DISPATCH_EX(eosdt::eosdtgovernc, (currentver)(settingset)
        (propose)(clnproposal)
        (transfer)(unstake)
        (vote)(unvote)
        (apply)(applybpproxy)
        (expire)(bpvotescount)(voterreborn)
        (bpregister)(bpsetparams)(bpunregister)
        (reinit)(calcnutbal)
)
