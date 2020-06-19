#pragma once


#ifndef ds_account
#define ds_account eosio::name
#endif

#ifndef SAME_PAYER
#define SAME_PAYER ds_account(0)
#endif

#ifndef ds_asset
#define ds_asset eosio::asset
#endif

#ifndef ds_symbol
#define ds_symbol eosio::symbol
#endif

#ifndef ds_string
#define ds_string std::string
#endif

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include "info.hpp"
