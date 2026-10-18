#pragma once

#include <cstdint>
#include <cstdio>

void syn_features_set_log(FILE* log);
void syn_features_load(const char* root);
void syn_features_before();
void syn_features_after();
void syn_features_world();
void syn_features_harvest();
void syn_features_packet(int cmd, const uint8_t* rest, uint32_t size);
