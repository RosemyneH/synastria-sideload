#include "WoW/Source/Battlenet/Tumor/TumorManager.h"
#include <cstring>
#include <new>
#include "Engine/Source/Storm/H/storm.h"
#include "WoW/Source/Battlenet/Tumor/CreepTendril.h"

// STATIC: 0x00DCEE58
BSN::FourCC Battlenet::Event::CREEP("Crep");

// STATIC: 0x00DCEE5C
BSN::FourCC Battlenet::Event::APP("App");

// STATIC: 0x00DCEE60
BSN::FourCC Battlenet::Event::DATAGRAM_CONNECTION_ACK("DCAC");

// STATIC: 0x00DCEE64
BSN::FourCC Battlenet::Event::S2_LOBBY("S2Lb");

// STATIC: 0x00DCEE68
BSN::FourCC Battlenet::Event::COMSAT_UPDATE("CMSU");

// STATIC: 0x00DCEE6C
BSN::FourCC Battlenet::Event::TO_UI("ToUI");

// STATIC: 0x00DCEE70
BSN::FourCC Battlenet::Event::COMSAT_STATE("CMST");

// STATIC: 0x00DCEE74
BSN::FourCC Battlenet::Event::FROM_UI("FmUI");

// STATIC: 0x00DCEE78
BSN::FourCC Battlenet::Event::DATAGRAM_CONNECTION_INIT("DCDI");

// VERIFY: 88129388 auto
// OFFSET: 0x009BFDB0
void Battlenet::TumorManagerRegisterFunction(void* param, const char* name, GeneralFunction::Function func, void* funcParam) {
    static_cast<TumorManager*>(param)->RegisterFunction(name, func, funcParam);
}

// VERIFY: def51cae auto
// OFFSET: 0x009BFDD0
Battlenet::Tumor* Battlenet::TumorManager::GetTumor() {
    return &this->m_tumor;
}

// VERIFY: 438ef535 manual
// OFFSET: 0x009BFDE0
void Battlenet::TumorManager::AddTendril(Tendril* tendril) {
    uint32_t i = 0;

    while (this->m_tendrils[i]) {
        if (++i >= 20) {
            DebugLog("No room for new tendril!");
            return;
        }
    }

    this->m_tendrils[i] = tendril;
    tendril->DoSubscriptions();
}

// VERIFY: afb39340 auto
// OFFSET: 0x009BFE30
void Battlenet::TumorManager::DestroyTendrils() {
    for (int32_t i = 0; i < 20; i++) {
        if (this->m_tendrils[i]) {
            this->m_tendrils[i]->Destroy();
            this->m_tendrils[i] = nullptr;
        }
    }
}

// VERIFY: 35a221a0 auto
// OFFSET: 0x009BFE60
void Battlenet::TumorManager::SendEvent(uint8_t eventType, const uint32_t& adapter, const BSN::EncodedObject* data) {
    this->m_eventAdapter->SendEvent(eventType, adapter, data);
}

// VERIFY: 8aa60d79 auto
// OFFSET: 0x009BFE80
void Battlenet::TumorManager::SendEvent(uint8_t eventType, const BSN::FourCC& eventId, const BSN::EncodedObject* data) {
    this->m_eventAdapter->SendEvent(eventType, eventId, data);
}

// VERIFY: 6a4672bf manual
// OFFSET: 0x009C0070
GeneralFunction::Arg Battlenet::TumorManagerSendEvent(void* param, int32_t argc, const GeneralFunction::Arg* args) {
    if (argc != 3) {
        DebugLog("Usage: SendEvent(eventId, dest, event)");
        return GeneralFunction::Arg();
    }

    const char* target = args[1].ToString();
    BSN::FourCC eventId;
    eventId.m_value = 0;

    for (uint8_t c = *target; *target; c = *target) {
        target++;
        eventId.m_value = c | (eventId.m_value << 8);
    }

    const BSN::EncodedObject* data = args[2].m_type == GeneralFunction::Arg::TYPE_OBJECT ? args[2].m_object : nullptr;
    static_cast<TumorManager*>(param)->SendEvent(static_cast<uint8_t>(args[0].ToUInt64()), eventId, data);
    return GeneralFunction::Arg();
}

// VERIFY: af469e54 auto
// OFFSET: 0x009C0110
Battlenet::TumorManager::TumorManager(Application& application) : m_tumor(application) {
    this->m_eventAdapter = nullptr;
    this->m_tumor.AllocTendrilId(this->m_tendrilId);
    this->m_eventAdapter = this->m_tumor.AllocEventAdapter(Battlenet::Event::APP, application, this->m_tendrilId);
    memset(this->m_tendrils, 0, sizeof(this->m_tendrils));
}

// VERIFY: 17a2f63a manual
// OFFSET: 0x009C0180
Battlenet::TumorManager::~TumorManager() {
}

// VERIFY: e72e8e42 manual
// OFFSET: 0x009C0A10
Battlenet::TumorManager* Battlenet::TumorManager::Create(Application& application, const Authentication::Settings& settings) {
    void* memory = Battlenet::g_allocator->Alloc(sizeof(TumorManagerImplementation));
    TumorManagerImplementation* manager = memory ? new (memory) TumorManagerImplementation(application) : nullptr;

    CreepTendril::Instance* creep = CreepTendril::Instance::Create(settings, *manager);

    if (!creep) {
        Battlenet::Delete(manager);
        return nullptr;
    }

    manager->AddTendril(creep);
    manager->m_eventAdapter->Subscribe(Battlenet::Event::APP, manager->m_tendrilId);
    manager->m_eventAdapter->Subscribe(Battlenet::Event::TO_UI, manager->m_tendrilId);
    manager->RegisterFunction("SendEvent", Battlenet::TumorManagerSendEvent, manager);
    creep->EnumFunctions(Battlenet::TumorManagerRegisterFunction, manager);
    return manager;
}

// INLINED
Battlenet::TumorManagerImplementation::TumorManagerImplementation(Application& application) : TumorManager(application) {
}

// VERIFY: 49eb6e79 manual
// OFFSET: 0x009C0540
void Battlenet::TumorManagerImplementation::EnumFunctions(EnumFunctionsCallback callback, void* param) {
    MapKey key;
    FunctionEntry entry = { nullptr, nullptr };
    uint32_t iterator = 0;

    while (this->m_functions.Next(iterator, key, entry)) {
        callback(param, key.m_value.GetString(), entry.m_func, entry.m_param);
    }
}

// VERIFY: 6d614d62 manual
// OFFSET: 0x009C05E0
GeneralFunction::Arg Battlenet::TumorManagerImplementation::CallFunction(const char* name, int32_t argc, GeneralFunction::Arg* args) {
    GeneralFunction::Function func = nullptr;
    void* param = nullptr;
    uint32_t index;
    bool found;

    {
        MapKey key((String(name)));

        if (this->m_functions.Find(key, index)) {
            func = this->m_functions.m_entries[index].m_value.m_func;
            param = this->m_functions.m_entries[index].m_value.m_param;
            found = true;
        } else {
            found = false;
        }
    }

    if (!found) {
        return GeneralFunction::Arg();
    }

    return func(param, argc, args);
}

// VERIFY: 9f1a0722 manual
// OFFSET: 0x009C09C0
Battlenet::TumorManagerImplementation::~TumorManagerImplementation() {
    this->DestroyTendrils();
}

// VERIFY: 9227520d manual
// OFFSET: 0x009C0AF0
void Battlenet::TumorManagerImplementation::RegisterFunction(const char* name, GeneralFunction::Function func, void* param) {
    MapKey key((String(name)));
    FunctionEntry& entry = this->m_functions[key];
    entry.m_func = func;
    entry.m_param = param;
}

// VERIFY: 08db47a2 manual
// OFFSET: 0x009C0B70
void Battlenet::TumorManagerImplementation::UnregisterFunction(const char* name, GeneralFunction::Function func, void* param) {
    GeneralFunction::Function registeredFunc = nullptr;
    void* registeredParam = nullptr;
    uint32_t index;
    bool found;

    {
        MapKey key((String(name)));

        if (this->m_functions.Find(key, index)) {
            registeredFunc = this->m_functions.m_entries[index].m_value.m_func;
            registeredParam = this->m_functions.m_entries[index].m_value.m_param;
            found = true;
        } else {
            found = false;
        }
    }

    if (found && registeredFunc == func && registeredParam == param) {
        MapKey key(name);
        FunctionEntry removed;
        this->m_functions.Remove(key, removed);
    }
}
