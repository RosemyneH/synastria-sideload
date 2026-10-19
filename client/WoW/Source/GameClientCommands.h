#ifndef WOW_SOURCE_GAMECLIENTCOMMANDS_H
#define WOW_SOURCE_GAMECLIENTCOMMANDS_H

#include <cstddef>
#include <cstdint>

class C3Vector;
class CDataStore;
class ClientConnection;

template <class T>
class StaticSingleton {
    public:
    static T* m_instance;

    static T* Instance() {
        if (!StaticSingleton<T>::m_instance) {
            StaticSingleton<T>::m_instance = new T;
        }

        return StaticSingleton<T>::m_instance;
    }
};

class GameClientCommands {
    public:
    virtual void Install();
    virtual void Uninstall();
};

class CommandManager {
    public:
    /* 0000 */ GameClientCommands* m_commands[8];
    /* 0020 */ uint32_t m_count;
    /* 0024 */ ClientConnection* m_connection;

    CommandManager() {
        this->m_count = 0;
    }

    ~CommandManager();
    void InternalSend(CDataStore* msg);
};

CDataStore& operator<<(CDataStore& msg, const C3Vector& vec);
int32_t CCommand_CharmTeleport(const char* command, const char* arguments);
int32_t CCommand_SetRawPos(const char* command, const char* arguments);
int32_t CCommand_Teleport(const char* command, const char* arguments);
int32_t CCommand_ToggleCloak(const char* command, const char* arguments);
int32_t CCommand_ToggleHelm(const char* command, const char* arguments);
int32_t CCommand_TogglePlayer(const char* command, const char* arguments);
int32_t CCommand_WorldTeleport(const char* command, const char* arguments);

#endif
