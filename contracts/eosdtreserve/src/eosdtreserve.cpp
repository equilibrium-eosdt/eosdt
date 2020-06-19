#include "eosdtreserve.hpp"

#include "settings.cpp"
#include "params.cpp"
#include "positions.cpp"

namespace eosdt {
}/// namespace eosdt



EOSIO_DISPATCH_EX(eosdt::eosdtreserve,(currentver)(settingset)(paraminit)
(transfer)(withdraw)(sendeos)
)