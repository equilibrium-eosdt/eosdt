
namespace eosdt {

    void eosdtnutoken::stableadd(ds_account account, uint8_t is_own, ds_string peg) {
        require_auth(_self);
        if (!is_account( account )) {
            check(false, "to account does not exist");
        }
        check(peg.size() > 0 && peg.size() <= 8, "invalid peg string");
        nutstables_table nutstables(_self,_self.value);
        auto stable_itr = nutstables.find(account.value);
        if (stable_itr == nutstables.end()) {
            nutstables.emplace(_self, [&](auto &row) {
                row.account = account;
                row.is_own = is_own;
                row.peg = peg;
            });
        } else {
            nutstables.modify(stable_itr, SAME_PAYER, [&](auto &row) {
                row.is_own = is_own;
                row.peg = peg;
            });
        }
    }

    void eosdtnutoken::stablerem(ds_account account) {
        require_auth(_self);
        nutstables_table nutstables(_self,_self.value);
        auto stable_itr = nutstables.find(account.value);
        if (stable_itr != nutstables.end()) {
            nutstables.erase(stable_itr);
        }
    }

}
