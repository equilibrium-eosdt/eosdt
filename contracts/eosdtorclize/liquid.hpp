#include "defer.hpp"


class liquid : public defer {
public:
    liquid(ds_account receiver, ds_account code, datastream<const char *> ds) : defer(receiver, code, ds) {
    }
};
