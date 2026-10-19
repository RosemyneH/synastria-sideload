#pragma once

#include <cstdint>
#include <cstdio>

void syn_features_set_log(FILE* log);
void syn_features_prepare(const char* root);
void syn_features_load(const char* root);
void* syn_features_item_record(int id);
void syn_features_before();
void syn_features_reset();
void syn_features_after();
bool syn_features_scripts_done();
int syn_features_class_count();
int syn_features_class_id(int index);
void syn_features_world();
void syn_features_harvest();
void syn_features_packet(int cmd, const uint8_t* rest, uint32_t size);
void syn_net_notify(uint32_t a, uint32_t b, const char* text);
int syn_features_cache_missing(const uint8_t* rest, uint32_t size, uint32_t* missing, int cap);
