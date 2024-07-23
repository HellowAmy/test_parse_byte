
#ifndef HANDLE_GLOABL_H
#define HANDLE_GLOABL_H

#include <iostream>
#include "Tvlog.h"

class handle_gloabl
{
public:
    struct ct_vote_index
    {
        char index;
        char index_wire;
    };
    struct ct_server_index
    {
        int type;
    };

public:
    ct_vote_index _vote_index;
    ct_server_index _server_type;

private:
    friend Tsingle_d<handle_gloabl>;

    handle_gloabl() {}
    ~handle_gloabl() {}
};

static handle_gloabl *_sp_global_ = Tsingle_d<handle_gloabl>::get();

#endif // HANDLE_GLOABL_H
