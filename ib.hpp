#pragma once

#include <infiniband/verbs.h>

struct IBContext {
    ibv_context *context;
    ibv_comp_channel *channel;
    ibv_pd *pd;
    ibv_mr *mr;
    ibv_cq *cq;
    char *buffer;
};
static IBContext ctx;

bool setup_ib();