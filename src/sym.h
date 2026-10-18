#pragma once

#include <cstdint>

// ʕ •ᴥ•ʔ✿ Resolve a symbol in the running PIE by name. Address includes the load bias. ✿ ʕ •ᴥ•ʔ
bool syn_resolve(const char* name, void** out);
