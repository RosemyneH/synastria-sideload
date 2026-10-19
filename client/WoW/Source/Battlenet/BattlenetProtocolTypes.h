#ifndef WOW_SOURCE_BATTLENET_BATTLENETPROTOCOLTYPES_H
#define WOW_SOURCE_BATTLENET_BATTLENETPROTOCOLTYPES_H

#include <cstddef>
#include <cstdint>
#include "WoW/Source/Battlenet/BattlenetProtocol.h"
#include "WoW/Source/Battlenet/BSN/Types.h"

namespace Battlenet { namespace Attribute { namespace Access {
enum Enum {
    NO_ONE = 0,
    SELF = 1,
    SPECIAL = 2,
    EVERYONE = 3,
};
}}}

namespace Battlenet { namespace Attribute { namespace Arbitration {
enum Enum {
    NONE = 0,
    FCFS = 1,
};
}}}

namespace Battlenet { namespace Attribute { namespace Scope {
enum Enum {
    SLOT1 = 0,
    SLOT2 = 1,
    SLOT3 = 2,
    SLOT4 = 3,
    SLOT5 = 4,
    SLOT6 = 5,
    SLOT7 = 6,
    SLOT8 = 7,
    SLOT9 = 8,
    SLOT10 = 9,
    SLOT11 = 10,
    SLOT12 = 11,
    SLOT13 = 12,
    SLOT14 = 13,
    SLOT15 = 14,
    SLOT16 = 15,
    MAX_SLOT = 15,
    NUM_SLOTS = 16,
    NUM_UNUSABLE_SLOTS = 1,
    GAME = 20,
    SLOT = 21,
    NONE = 22,
};
}}}

namespace Battlenet { namespace Attribute { namespace Visibility {
enum Enum {
    NO_ONE = 0,
    SELF = 1,
    SPECIAL = 2,
    EVERYONE = 3,
};
}}}

namespace Battlenet { namespace Result { namespace Form {
enum Enum {
    SCORE_TEMPLATE = 0,
    SUMMARY_TEMPLATE = 1,
    HEADING = 2,
    VALUE = 3,
    EVENT = 4,
    BUILD_ORDER_TEMPLATE = 5,
    COMMENT = 6,
    COMMENT_TEMPLATE = 7,
    CURRENT_LADDER_TEMPLATE = 8,
    VALUE_PANE_4_TEMPLATE = 9,
    GRAPH_TEMPLATE = 10,
};
}}}

namespace Battlenet { namespace S2Game { namespace PlayerResultCode {
enum Enum {
    WIN = 0,
    LOSS = 1,
    TIE = 2,
    UNDECIDED = 3,
    BAILER = 4,
    DISAGREE = 5,
    WATCHER = 6,
};
}}}

namespace Battlenet { namespace S2Map { namespace LinkEntryType {
enum Enum {
    PRIMARY = 0,
    OPTIONAL = 1,
    DEPENDENCY = 2,
};
}}}

namespace Battlenet { namespace Account {
struct FullName {
    public:
    static const BSN::String<32> s_default_givenName;
    static const BSN::String<32> s_default_surname;

    static void SetDefault_givenName(BSN::String<32>& value);
    static void SetDefault_surname(BSN::String<32>& value);

    /* 0000 */ BSN::String<32> m_givenName;
    /* 008C */ BSN::String<32> m_surname;
};
}}

namespace Battlenet { namespace Attribute {
struct ArtId {
    public:
    static void SetDefault_visualFileIndex(uint8_t& value);
    static void SetDefault_top(uint16_t& value);
    static void SetDefault_left(uint16_t& value);
    static void SetDefault_height(uint16_t& value);
    static void SetDefault_width(uint16_t& value);

    /* 0000 */ uint8_t m_visualFileIndex;
    /* 0002 */ uint16_t m_top;
    /* 0004 */ uint16_t m_left;
    /* 0006 */ uint16_t m_height;
    /* 0008 */ uint16_t m_width;
};
}}

namespace Battlenet { namespace Attribute {
struct LocalizedString {
    public:
    static void SetDefault_colorHint(BSN::Optional<uint32_t>& value);
    static void SetDefault_fileIndex(uint8_t& value);
    static void SetDefault_id(uint16_t& value);

    /* 0000 */ BSN::Optional<uint32_t> m_colorHint;
    /* 0008 */ uint8_t m_fileIndex;
    /* 000A */ uint16_t m_id;
};
}}

namespace Battlenet { namespace Attribute {
struct LocaleTableEntry {
    public:
    static const BSN::FourCC s_default_locale;

    static void SetDefault_locale(BSN::FourCC& value);
    static void SetDefault_stringFileTable(BSN::Array<BSN::FixedBlob<40>, 10>& value);

    /* 0000 */ BSN::FourCC m_locale;
    /* 0004 */ BSN::Array<BSN::FixedBlob<40>, 10> m_stringFileTable;
};
}}

namespace Battlenet { namespace Attribute {
struct Visual {
    public:
    static void SetDefault_text(BSN::Optional<Battlenet::Attribute::LocalizedString>& value);
    static void SetDefault_tip(BSN::Optional<Battlenet::Attribute::LocalizedString>& value);
    static void SetDefault_art(BSN::Optional<Battlenet::Attribute::ArtId>& value);

    /* 0000 */ BSN::Optional<Battlenet::Attribute::LocalizedString> m_text;
    /* 0010 */ BSN::Optional<Battlenet::Attribute::LocalizedString> m_tip;
    /* 0020 */ BSN::Optional<Battlenet::Attribute::ArtId> m_art;
};
}}

namespace Battlenet { namespace Attribute {
struct Entry {
    public:
    static const BSN::FourCC s_default_code;

    static void SetDefault_code(BSN::FourCC& value);
    static void SetDefault_visual(Battlenet::Attribute::Visual& value);

    /* 0000 */ BSN::FourCC m_code;
    /* 0004 */ Battlenet::Attribute::Visual m_visual;
};
}}

namespace Battlenet { namespace Attribute {
struct DefinitionHandle {
    public:
    static void SetDefault_nameSpace(uint32_t& value);
    static void SetDefault_id(uint32_t& value);

    /* 0000 */ uint32_t m_nameSpace;
    /* 0004 */ uint32_t m_id;
};
}}

namespace Battlenet { namespace Attribute {
struct Address {
    public:
    static void SetDefault_scope(Battlenet::Attribute::Scope::Enum& value);
    static void SetDefault_definitionHandle(Battlenet::Attribute::DefinitionHandle& value);

    /* 0000 */ Battlenet::Attribute::Scope::Enum m_scope;
    /* 0004 */ Battlenet::Attribute::DefinitionHandle m_definitionHandle;
};
}}

namespace Battlenet { namespace Attribute {
struct Parent {
    public:
    static void SetDefault_address(Battlenet::Attribute::Address& value);
    static void SetDefault_set(BSN::Array<BSN::FourCC, 32>& value);

    /* 0000 */ Battlenet::Attribute::Address m_address;
    /* 000C */ BSN::Array<BSN::FourCC, 32> m_set;
};
}}

namespace Battlenet { namespace Attribute {
struct NearInstance {
    public:
    static void SetDefault_index(uint8_t& value);
    static void SetDefault_baked(bool& value);

    /* 0000 */ uint8_t m_index;
    /* 0001 */ bool m_baked;
};
}}

namespace Battlenet { namespace Attribute {
struct NearInstanceList : public BSN::Choice {
    public:
    /* 0004 */ uint8_t m_value[36];
};
}}

namespace Battlenet { namespace Attribute {
struct Instance {
    public:
    static void SetDefault_definitionHandle(Battlenet::Attribute::DefinitionHandle& value);
    static void SetDefault_data(Battlenet::Attribute::NearInstanceList& value);

    /* 0000 */ Battlenet::Attribute::DefinitionHandle m_definitionHandle;
    /* 0008 */ Battlenet::Attribute::NearInstanceList m_data;
};
}}

namespace Battlenet { namespace Attribute {
struct Definition {
    public:
    static void SetDefault_handle(Battlenet::Attribute::DefinitionHandle& value);
    static void SetDefault_table(BSN::Array<Battlenet::Attribute::Entry, 32>& value);
    static void SetDefault_visual(Battlenet::Attribute::Visual& value);
    static void SetDefault_parents(BSN::Array<Battlenet::Attribute::Parent, 4>& value);
    static void SetDefault_arbitration(Battlenet::Attribute::Arbitration::Enum& value);
    static void SetDefault_visibility(Battlenet::Attribute::Visibility::Enum& value);
    static void SetDefault_access(Battlenet::Attribute::Access::Enum& value);
    static void SetDefault_flags(uint32_t& value);
    static void SetDefault_defaults(Battlenet::Attribute::NearInstanceList& value);
    static void SetDefault_sortOrder(uint8_t& value);

    /* 0000 */ Battlenet::Attribute::DefinitionHandle m_handle;
    /* 0008 */ BSN::Array<Battlenet::Attribute::Entry, 32> m_table;
    /* 060C */ Battlenet::Attribute::Visual m_visual;
    /* 0638 */ BSN::Array<Battlenet::Attribute::Parent, 4> m_parents;
    /* 087C */ Battlenet::Attribute::Arbitration::Enum m_arbitration;
    /* 0880 */ Battlenet::Attribute::Visibility::Enum m_visibility;
    /* 0884 */ Battlenet::Attribute::Access::Enum m_access;
    /* 0888 */ uint32_t m_flags;
    /* 088C */ Battlenet::Attribute::NearInstanceList m_defaults;
    /* 08B4 */ uint8_t m_sortOrder;
};
}}

namespace Battlenet { namespace Attribute {
struct DefinitionLock {
    public:
    static void SetDefault_handle(Battlenet::Attribute::DefinitionHandle& value);
    static void SetDefault_lockedScopes(BSN::Optional<BSN::FixedBitArray<16>>& value);

    /* 0000 */ Battlenet::Attribute::DefinitionHandle m_handle;
    /* 0008 */ BSN::Optional<BSN::FixedBitArray<16>> m_lockedScopes;
};
}}

namespace Battlenet { namespace Attribute {
struct WorkingSet {
    public:
    static void SetDefault_name(Battlenet::Attribute::LocalizedString& value);
    static void SetDefault_description(Battlenet::Attribute::LocalizedString& value);
    static void SetDefault_thumbnail(BSN::Optional<Battlenet::Attribute::ArtId>& value);
    static void SetDefault_bigMap(BSN::Optional<Battlenet::Attribute::ArtId>& value);
    static void SetDefault_maxPlayers(uint8_t& value);
    static void SetDefault_specialScope(Battlenet::Attribute::Scope::Enum& value);
    static void SetDefault_instances(BSN::Array<Battlenet::Attribute::Instance, 64>& value);
    static void SetDefault_visualFiles(BSN::Array<BSN::FixedBlob<40>, 16>& value);
    static void SetDefault_locales(BSN::Array<Battlenet::Attribute::LocaleTableEntry, 16>& value);

    /* 0000 */ Battlenet::Attribute::LocalizedString m_name;
    /* 000C */ Battlenet::Attribute::LocalizedString m_description;
    /* 0018 */ BSN::Optional<Battlenet::Attribute::ArtId> m_thumbnail;
    /* 0024 */ BSN::Optional<Battlenet::Attribute::ArtId> m_bigMap;
    /* 0030 */ uint8_t m_maxPlayers;
    /* 0034 */ Battlenet::Attribute::Scope::Enum m_specialScope;
    /* 0038 */ BSN::Array<Battlenet::Attribute::Instance, 64> m_instances;
    /* 0C3C */ BSN::Array<BSN::FixedBlob<40>, 16> m_visualFiles;
    /* 0EC0 */ BSN::Array<Battlenet::Attribute::LocaleTableEntry, 16> m_locales;
};
}}

namespace Battlenet { namespace Profile {
struct RecordAddress {
    public:
    static void SetDefault_label(uint32_t& value);
    static void SetDefault_id(uint64_t& value);

    /* 0000 */ uint32_t m_label;
    /* 0008 */ uint64_t m_id;
};
}}

namespace Battlenet { namespace Toon {
struct Handle {
    public:
    static const BSN::FourCC s_default_programId;

    static void SetDefault_region(uint8_t& value);
    static void SetDefault_programId(BSN::FourCC& value);
    static void SetDefault_realm(uint32_t& value);
    static void SetDefault_id(uint64_t& value);

    /* 0000 */ uint8_t m_region;
    /* 0004 */ BSN::FourCC m_programId;
    /* 0008 */ uint32_t m_realm;
    /* 0010 */ uint64_t m_id;
};
}}

namespace Battlenet { namespace S2Master {
struct InstanceHandle {
    public:
    static void SetDefault_instanceId(uint32_t& value);
    static void SetDefault_serverLabel(uint32_t& value);
    static void SetDefault_serverEpoch(uint32_t& value);

    /* 0000 */ uint32_t m_instanceId;
    /* 0004 */ uint32_t m_serverLabel;
    /* 0008 */ uint32_t m_serverEpoch;
};
}}

namespace Battlenet { namespace S2Map {
struct Handle {
    public:
    static void SetDefault_id(uint32_t& value);
    static void SetDefault_version(uint32_t& value);

    /* 0000 */ uint32_t m_id;
    /* 0004 */ uint32_t m_version;
};
}}

namespace Battlenet { namespace S2Map {
struct LinkEntry {
    public:
    static void SetDefault_handle(Battlenet::S2Map::Handle& value);
    static void SetDefault_type(Battlenet::S2Map::LinkEntryType::Enum& value);

    /* 0000 */ Battlenet::S2Map::Handle m_handle;
    /* 0008 */ Battlenet::S2Map::LinkEntryType::Enum m_type;
};
}}

namespace Battlenet { namespace S2Map {
struct Link {
    public:
    static const BSN::FourCC s_default_speed;

    static void SetDefault_variantIndex(uint8_t& value);
    static void SetDefault_speed(BSN::FourCC& value);
    static void SetDefault_entries(BSN::Array<Battlenet::S2Map::LinkEntry, 10>& value);

    /* 0000 */ uint8_t m_variantIndex;
    /* 0004 */ BSN::FourCC m_speed;
    /* 0008 */ BSN::Array<Battlenet::S2Map::LinkEntry, 10> m_entries;
};
}}

namespace Battlenet { namespace Result {
struct Container {
    public:
    static void SetDefault_handle(Battlenet::Attribute::DefinitionHandle& value);
    static void SetDefault_orderHint(uint32_t& value);

    /* 0000 */ Battlenet::Attribute::DefinitionHandle m_handle;
    /* 0008 */ uint32_t m_orderHint;
};
}}

namespace Battlenet { namespace Result {
struct Definition {
    public:
    static const BSN::String<31> s_default_name;

    static void SetDefault_name(BSN::String<31>& value);
    static void SetDefault_handle(Battlenet::Attribute::DefinitionHandle& value);
    static void SetDefault_visual(Battlenet::Attribute::Visual& value);
    static void SetDefault_containers(BSN::Array<Battlenet::Result::Container, 8>& value);
    static void SetDefault_form(Battlenet::Result::Form::Enum& value);
    static void SetDefault_flags(uint8_t& value);

    /* 0000 */ BSN::String<31> m_name;
    /* 0088 */ Battlenet::Attribute::DefinitionHandle m_handle;
    /* 0090 */ Battlenet::Attribute::Visual m_visual;
    /* 00BC */ BSN::Array<Battlenet::Result::Container, 8> m_containers;
    /* 0120 */ Battlenet::Result::Form::Enum m_form;
    /* 0124 */ uint8_t m_flags;
};
}}

namespace Battlenet { namespace S2Map {
struct MapSize {
    public:
    static void SetDefault_horizontal(uint16_t& value);
    static void SetDefault_vertical(uint16_t& value);

    /* 0000 */ uint16_t m_horizontal;
    /* 0002 */ uint16_t m_vertical;
};
}}

namespace Battlenet { namespace S2Map {
struct VariantId {
    public:
    static void SetDefault_category(uint8_t& value);
    static void SetDefault_mode(uint8_t& value);

    /* 0000 */ uint8_t m_category;
    /* 0001 */ uint8_t m_mode;
};
}}

namespace Battlenet { namespace S2Map {
struct VariantInfo {
    public:
    static void SetDefault_variantId(Battlenet::S2Map::VariantId& value);
    static void SetDefault_categoryName(Battlenet::Attribute::LocalizedString& value);
    static void SetDefault_modeName(Battlenet::Attribute::LocalizedString& value);
    static void SetDefault_categoryDescription(BSN::Optional<Battlenet::Attribute::LocalizedString>& value);
    static void SetDefault_modeDescription(BSN::Optional<Battlenet::Attribute::LocalizedString>& value);
    static void SetDefault_visual(Battlenet::Attribute::Visual& value);
    static void SetDefault_instances(BSN::Array<Battlenet::Attribute::Instance, 64>& value);
    static void SetDefault_lockedAttributes(BSN::Array<Battlenet::Attribute::DefinitionLock, 64>& value);
    static void SetDefault_maxTeamSize(uint8_t& value);

    /* 0000 */ Battlenet::S2Map::VariantId m_variantId;
    /* 0004 */ Battlenet::Attribute::LocalizedString m_categoryName;
    /* 0010 */ Battlenet::Attribute::LocalizedString m_modeName;
    /* 001C */ BSN::Optional<Battlenet::Attribute::LocalizedString> m_categoryDescription;
    /* 002C */ BSN::Optional<Battlenet::Attribute::LocalizedString> m_modeDescription;
    /* 003C */ Battlenet::Attribute::Visual m_visual;
    /* 0068 */ BSN::Array<Battlenet::Attribute::Instance, 64> m_instances;
    /* 0C6C */ BSN::Array<Battlenet::Attribute::DefinitionLock, 64> m_lockedAttributes;
    /* 0F70 */ uint8_t m_maxTeamSize;
};
}}

namespace Battlenet { namespace S2Map {
struct BnetHeader {
    public:
    static const BSN::String<80> s_default_name;
    static const BSN::FixedBlob<40> s_default_archiveCacheHandle;

    static void SetDefault_handle(Battlenet::S2Map::Handle& value);
    static void SetDefault_name(BSN::String<80>& value);
    static void SetDefault_archiveCacheHandle(BSN::FixedBlob<40>& value);
    static void SetDefault_mapNamespace(uint32_t& value);
    static void SetDefault_workingSet(Battlenet::Attribute::WorkingSet& value);
    static void SetDefault_definitions(BSN::Array<Battlenet::Attribute::Definition, 64>& value);
    static void SetDefault_dependencies(BSN::Array<Battlenet::S2Map::Handle, 4>& value);
    static void SetDefault_resultDefinitions(BSN::Array<Battlenet::Result::Definition, 512>& value);
    static void SetDefault_locales(BSN::Array<Battlenet::Attribute::LocaleTableEntry, 16>& value);
    static void SetDefault_size(BSN::Optional<Battlenet::S2Map::MapSize>& value);
    static void SetDefault_tileset(BSN::Optional<Battlenet::Attribute::LocalizedString>& value);
    static void SetDefault_label(BSN::Optional<BSN::FourCC>& value);
    static void SetDefault_defaultVariantIndex(uint8_t& value);
    static void SetDefault_variants(BSN::Array<Battlenet::S2Map::VariantInfo, 32>& value);

    /* 0000 */ Battlenet::S2Map::Handle m_handle;
    /* 0008 */ BSN::String<80> m_name;
    /* 0154 */ BSN::FixedBlob<40> m_archiveCacheHandle;
    /* 017C */ uint32_t m_mapNamespace;
    /* 0180 */ Battlenet::Attribute::WorkingSet m_workingSet;
    /* 29C4 */ BSN::Array<Battlenet::Attribute::Definition, 64> m_definitions;
    /* 257C8 */ BSN::Array<Battlenet::S2Map::Handle, 4> m_dependencies;
    /* 257EC */ BSN::Array<Battlenet::Result::Definition, 512> m_resultDefinitions;
    /* 4A7F0 */ BSN::Array<Battlenet::Attribute::LocaleTableEntry, 16> m_locales;
    /* 4C174 */ BSN::Optional<Battlenet::S2Map::MapSize> m_size;
    /* 4C17C */ BSN::Optional<Battlenet::Attribute::LocalizedString> m_tileset;
    /* 4C18C */ BSN::Optional<BSN::FourCC> m_label;
    /* 4C194 */ uint8_t m_defaultVariantIndex;
    /* 4C198 */ BSN::Array<Battlenet::S2Map::VariantInfo, 32> m_variants;
};
}}

namespace Battlenet { namespace S2Map {
struct GameHeader {
    public:
    static void SetDefault_dummy(bool& value);

    /* 0000 */ bool m_dummy;
};
}}

namespace Battlenet { namespace S2Map {
struct Header {
    public:
    static void SetDefault_bnet(Battlenet::S2Map::BnetHeader& value);
    static void SetDefault_game(Battlenet::S2Map::GameHeader& value);

    /* 0000 */ Battlenet::S2Map::BnetHeader m_bnet;
    /* 6B01C */ Battlenet::S2Map::GameHeader m_game;
};
}}

namespace Battlenet { namespace Result {
struct PlayerObjectType : public BSN::Choice {
    public:
    struct Player {
        public:
        static void SetDefault_toonHandle(Battlenet::Toon::Handle& value);
        static void SetDefault_toonProfileAddress(BSN::Optional<Battlenet::Profile::RecordAddress>& value);

        /* 0000 */ Battlenet::Toon::Handle m_toonHandle;
        /* 0018 */ BSN::Optional<Battlenet::Profile::RecordAddress> m_toonProfileAddress;
    };

    /* 0004 */ uint8_t m_value[48];
};
}}

namespace Battlenet { namespace Result {
struct PlayerObject {
    public:
    static void SetDefault_scope(Battlenet::Attribute::Scope::Enum& value);
    static void SetDefault_playerObjectType(Battlenet::Result::PlayerObjectType& value);

    /* 0000 */ Battlenet::Attribute::Scope::Enum m_scope;
    /* 0004 */ Battlenet::Result::PlayerObjectType m_playerObjectType;
};
}}

namespace Battlenet { namespace S2Game { namespace Summary {
struct ParticipantRole {
    public:
    static void SetDefault_resultCode(Battlenet::S2Game::PlayerResultCode::Enum& value);

    /* 0000 */ Battlenet::S2Game::PlayerResultCode::Enum m_resultCode;
};
}}}

namespace Battlenet { namespace S2Map {
struct InfoBase {
    public:
    static const BSN::FixedBlob<40> s_default_headerCacheHandle;
    static const BSN::String<80> s_default_name;

    static void SetDefault_handle(Battlenet::S2Map::Handle& value);
    static void SetDefault_headerCacheHandle(BSN::FixedBlob<40>& value);
    static void SetDefault_uploadTime(int32_t& value);
    static void SetDefault_isLinked(bool& value);
    static void SetDefault_isLocked(bool& value);
    static void SetDefault_isPrivate(bool& value);
    static void SetDefault_mapSize(uint32_t& value);
    static void SetDefault_name(BSN::String<80>& value);
    static void SetDefault_profileRecordAddress(Battlenet::Profile::RecordAddress& value);
    static void SetDefault_isMod(bool& value);
    static void SetDefault_label(BSN::Optional<BSN::FourCC>& value);
    static void SetDefault_authorToonName(Battlenet::Toon::FullName& value);
    static void SetDefault_isLatestVersion(bool& value);
    static void SetDefault_mainLocale(BSN::Optional<BSN::FourCC>& value);

    /* 0000 */ Battlenet::S2Map::Handle m_handle;
    /* 0008 */ BSN::FixedBlob<40> m_headerCacheHandle;
    /* 0030 */ int32_t m_uploadTime;
    /* 0034 */ bool m_isLinked;
    /* 0035 */ bool m_isLocked;
    /* 0036 */ bool m_isPrivate;
    /* 0038 */ uint32_t m_mapSize;
    /* 003C */ BSN::String<80> m_name;
    /* 0188 */ Battlenet::Profile::RecordAddress m_profileRecordAddress;
    /* 0198 */ bool m_isMod;
    /* 019C */ BSN::Optional<BSN::FourCC> m_label;
    /* 01A4 */ Battlenet::Toon::FullName m_authorToonName;
    /* 0220 */ bool m_isLatestVersion;
    /* 0224 */ BSN::Optional<BSN::FourCC> m_mainLocale;
};
}}

namespace Battlenet { namespace S2Map {
struct Info : public Battlenet::S2Map::InfoBase {
    public:
    static void SetDefault_InfoBase(Battlenet::S2Map::InfoBase& value);
    static void SetDefault_authorRealName(BSN::Optional<Battlenet::Account::FullName>& value);

    /* 0230 */ BSN::Optional<Battlenet::Account::FullName> m_authorRealName;
};
}}

namespace Battlenet { namespace Event { namespace OfflineDataStore {
struct Email {
    public:
    static void SetDefault_hash(BSN::Optional<uint64_t>& value);

    /* 0000 */ BSN::Optional<uint64_t> m_hash;
};
}}}

namespace Battlenet { namespace Event { namespace OfflineDataStore {
struct TagIdx {
    public:
    static const BSN::FourCC s_default_tag;

    static void SetDefault_tag(BSN::FourCC& value);
    static void SetDefault_idx(uint32_t& value);

    /* 0000 */ BSN::FourCC m_tag;
    /* 0004 */ uint32_t m_idx;
};
}}}

namespace Battlenet { namespace Toon {
struct UnlockDefinitionFile {
    public:
    static const BSN::FixedBlob<40> s_default_unlockDefinitionFileCacheHandle;

    static void SetDefault_unlockDefinitionFileCacheHandle(BSN::FixedBlob<40>& value);

    /* 0000 */ BSN::FixedBlob<40> m_unlockDefinitionFileCacheHandle;
};
}}

namespace Battlenet { namespace Event { namespace OfflineDataStore {
struct StoredData {
    public:
    static const BSN::AsciiString<32> s_default_gameAccountName;
    static const BSN::FourCC s_default_program;

    static void SetDefault_toon(Battlenet::Toon::FullName& value);
    static void SetDefault_fullName(Battlenet::Account::FullName& value);
    static void SetDefault_accountRegion(uint8_t& value);
    static void SetDefault_gameAccountRegion(uint8_t& value);
    static void SetDefault_gameAccountName(BSN::AsciiString<32>& value);
    static void SetDefault_gameAccountFlags(uint64_t& value);
    static void SetDefault_program(BSN::FourCC& value);
    static void SetDefault_tagidx(BSN::Optional<BSN::Array<Battlenet::Event::OfflineDataStore::TagIdx, 16>>& value);
    static void SetDefault_unlockablesFiles(BSN::Optional<BSN::Array<Battlenet::Toon::UnlockDefinitionFile, 16>>& value);
    static void SetDefault_lastOnline(uint32_t& value);
    static void SetDefault_toonHandle(Battlenet::Toon::Handle& value);
    static void SetDefault_accountId(uint32_t& value);

    /* 0000 */ Battlenet::Toon::FullName m_toon;
    /* 007C */ Battlenet::Account::FullName m_fullName;
    /* 0194 */ uint8_t m_accountRegion;
    /* 0195 */ uint8_t m_gameAccountRegion;
    /* 0198 */ BSN::AsciiString<32> m_gameAccountName;
    /* 01C0 */ uint64_t m_gameAccountFlags;
    /* 01C8 */ BSN::FourCC m_program;
    /* 01CC */ BSN::Optional<BSN::Array<Battlenet::Event::OfflineDataStore::TagIdx, 16>> m_tagidx;
    /* 0254 */ BSN::Optional<BSN::Array<Battlenet::Toon::UnlockDefinitionFile, 16>> m_unlockablesFiles;
    /* 04DC */ uint32_t m_lastOnline;
    /* 04E0 */ Battlenet::Toon::Handle m_toonHandle;
    /* 04F8 */ uint32_t m_accountId;
};
}}}

namespace Battlenet { namespace Event { namespace OfflineDataStore {
struct OfflineData {
    public:
    static void SetDefault_info(BSN::Optional<BSN::Array<Battlenet::Event::OfflineDataStore::StoredData, 3>>& value);

    /* 0000 */ BSN::Optional<BSN::Array<Battlenet::Event::OfflineDataStore::StoredData, 3>> m_info;
};
}}}

namespace Battlenet { namespace Result {
struct Value {
    public:
    static void SetDefault_value(uint32_t& value);
    static void SetDefault_maxValue(BSN::Optional<uint32_t>& value);
    static void SetDefault_time(int32_t& value);

    /* 0000 */ uint32_t m_value;
    /* 0004 */ BSN::Optional<uint32_t> m_maxValue;
    /* 000C */ int32_t m_time;
};
}}

namespace Battlenet { namespace Result {
struct Instance {
    public:
    static void SetDefault_handle(Battlenet::Attribute::DefinitionHandle& value);
    static void SetDefault_values(BSN::Array<BSN::Array<Battlenet::Result::Value, 32>, 15>& value);

    /* 0000 */ Battlenet::Attribute::DefinitionHandle m_handle;
    /* 0008 */ BSN::Array<BSN::Array<Battlenet::Result::Value, 32>, 15> m_values;
};
}}

namespace Battlenet { namespace Result {
struct DefinitionListContainer {
    public:
    static void SetDefault_definitionList(BSN::Array<Battlenet::Result::Definition, 512>& value);

    /* 0000 */ BSN::Array<Battlenet::Result::Definition, 512> m_definitionList;
};
}}

namespace Battlenet { namespace Result {
struct InstanceListContainer {
    public:
    static void SetDefault_instanceList(BSN::Array<Battlenet::Result::Instance, 10>& value);

    /* 0000 */ BSN::Array<Battlenet::Result::Instance, 10> m_instanceList;
};
}}

namespace Battlenet { namespace Result {
struct PlayerListContainer {
    public:
    static void SetDefault_playerList(BSN::Array<Battlenet::Result::PlayerObject, 16>& value);

    /* 0000 */ BSN::Array<Battlenet::Result::PlayerObject, 16> m_playerList;
};
}}

namespace Battlenet { namespace S2Game { namespace Summary {
struct Role : public BSN::Choice {
    public:
    /* 0004 */ uint8_t m_value[4];
};
}}}

namespace Battlenet { namespace S2Game { namespace Summary {
struct PlayerData {
    public:
    static void SetDefault_playerObject(Battlenet::Result::PlayerObject& value);
    static void SetDefault_role(Battlenet::S2Game::Summary::Role& value);
    static void SetDefault_actualRace(BSN::Optional<BSN::FourCC>& value);

    /* 0000 */ Battlenet::Result::PlayerObject m_playerObject;
    /* 0038 */ Battlenet::S2Game::Summary::Role m_role;
    /* 0040 */ BSN::Optional<BSN::FourCC> m_actualRace;
};
}}}

namespace Battlenet { namespace S2Game { namespace Summary {
struct Data {
    public:
    static const BSN::FixedBlob<4> s_default_seed;

    static void SetDefault_mapLink(Battlenet::S2Map::Link& value);
    static void SetDefault_replayCacheHandle(BSN::Optional<BSN::FixedBlob<40>>& value);
    static void SetDefault_instanceHandle(Battlenet::S2Master::InstanceHandle& value);
    static void SetDefault_playerList(BSN::Array<Battlenet::S2Game::Summary::PlayerData, 16>& value);
    static void SetDefault_seed(BSN::FixedBlob<4>& value);
    static void SetDefault_definitionList(BSN::Array<Battlenet::Attribute::Definition, 64>& value);
    static void SetDefault_workingSet(Battlenet::Attribute::WorkingSet& value);
    static void SetDefault_simulationTime(int32_t& value);
    static void SetDefault_endTime(int32_t& value);
    static void SetDefault_resultsCacheHandle(BSN::Optional<BSN::FixedBlob<40>>& value);

    /* 0000 */ Battlenet::S2Map::Link m_mapLink;
    /* 0084 */ BSN::Optional<BSN::FixedBlob<40>> m_replayCacheHandle;
    /* 00B0 */ Battlenet::S2Master::InstanceHandle m_instanceHandle;
    /* 00BC */ BSN::Array<Battlenet::S2Game::Summary::PlayerData, 16> m_playerList;
    /* 0540 */ BSN::FixedBlob<4> m_seed;
    /* 0544 */ BSN::Array<Battlenet::Attribute::Definition, 64> m_definitionList;
    /* 23348 */ Battlenet::Attribute::WorkingSet m_workingSet;
    /* 25B8C */ int32_t m_simulationTime;
    /* 25B90 */ int32_t m_endTime;
    /* 25B94 */ BSN::Optional<BSN::FixedBlob<40>> m_resultsCacheHandle;
};
}}}

#endif
