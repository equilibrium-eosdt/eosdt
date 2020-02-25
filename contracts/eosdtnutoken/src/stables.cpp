
namespace eosdt {

    ACTION eosdtnutoken::stableadd(ds_account account, uint8_t is_own, ds_string peg) {
        require_auth(_self);
        check( is_account( account ), "to account does not exist");
        nutstables_table nutstables(_self,_self.value);
        auto stable_itr = nutstables.find(account.value);
        if (stable_itr == nutstables.end()) {
            nutstables.emplace(_self, [&](auto &row) {
                row.account = account;
                row.is_own = is_own;
                row.peg = peg;
            });
        } else {
            nutstables.modify(stable_itr, ds_account(0), [&](auto &row) {
                row.is_own = is_own;
                row.peg = peg;
            });
        }
    }

    ACTION eosdtnutoken::stablerem(ds_account account) {
        require_auth(_self);
        nutstables_table nutstables(_self,_self.value);
        auto stable_itr = nutstables.find(account.value);
        if (stable_itr != nutstables.end()) {
            nutstables.erase(stable_itr);
        }
    }

}
