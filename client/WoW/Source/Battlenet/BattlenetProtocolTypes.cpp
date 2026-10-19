#include "WoW/Source/Battlenet/BattlenetProtocolTypes.h"

// STATIC: 0x00D45168
const BSN::String<32> Battlenet::Account::FullName::s_default_givenName("");

// STATIC: 0x00D451F8
const BSN::String<32> Battlenet::Account::FullName::s_default_surname("");

// STATIC: 0x00D45284
const BSN::FourCC Battlenet::Attribute::LocaleTableEntry::s_default_locale = BSN::FourCC();

// STATIC: 0x00D45288
const BSN::FourCC Battlenet::Attribute::Entry::s_default_code = BSN::FourCC();

// STATIC: 0x00D45290
const BSN::FourCC Battlenet::Toon::FullName::s_default_programId = BSN::FourCC();

// STATIC: 0x00D45294
const BSN::FixedBlob<40> Battlenet::Toon::UnlockDefinitionFile::s_default_unlockDefinitionFileCacheHandle = BSN::FixedBlob<40>();

// STATIC: 0x00D452BC
const BSN::FourCC Battlenet::Toon::Handle::s_default_programId = BSN::FourCC();

// STATIC: 0x00D452C0
const BSN::String<25> Battlenet::Toon::FullName::s_default_name("   ");

// STATIC: 0x00D45330
const BSN::String<31> Battlenet::Result::Definition::s_default_name("");

// STATIC: 0x00D453B8
const BSN::FourCC Battlenet::S2Map::Link::s_default_speed("   2");

// STATIC: 0x00D453BC
const BSN::FixedBlob<40> Battlenet::S2Map::InfoBase::s_default_headerCacheHandle = BSN::FixedBlob<40>();

// STATIC: 0x00D453E8
const BSN::String<80> Battlenet::S2Map::InfoBase::s_default_name("");

// STATIC: 0x00D45538
const BSN::String<80> Battlenet::S2Map::BnetHeader::s_default_name("");

// STATIC: 0x00D456F8
const BSN::FixedBlob<40> Battlenet::S2Map::BnetHeader::s_default_archiveCacheHandle = BSN::FixedBlob<40>();

// STATIC: 0x00D45720
const BSN::FourCC Battlenet::Event::OfflineDataStore::StoredData::s_default_program = BSN::FourCC();

// STATIC: 0x00D45724
const BSN::FourCC Battlenet::Event::OfflineDataStore::TagIdx::s_default_tag = BSN::FourCC();

// STATIC: 0x00D45728
const BSN::AsciiString<32> Battlenet::Event::OfflineDataStore::StoredData::s_default_gameAccountName(" ");

// STATIC: 0x00D45760
const BSN::FixedBlob<4> Battlenet::S2Game::Summary::Data::s_default_seed = BSN::FixedBlob<4>();

// OFFSET: 0x008BE1B0
void Battlenet::Account::FullName::SetDefault_givenName(BSN::String<32>& value) {
    value = Battlenet::Account::FullName::s_default_givenName;
}

// OFFSET: 0x008BE1D0
void Battlenet::Account::FullName::SetDefault_surname(BSN::String<32>& value) {
    value = Battlenet::Account::FullName::s_default_surname;
}

// OFFSET: 0x008BE760
void Battlenet::Attribute::ArtId::SetDefault_visualFileIndex(uint8_t& value) {
    value = 0;
}

// OFFSET: 0x008BE390
void Battlenet::Attribute::ArtId::SetDefault_top(uint16_t& value) {
    value = 0;
}

// OFFSET: 0x008BE390
void Battlenet::Attribute::ArtId::SetDefault_left(uint16_t& value) {
    value = 0;
}

// OFFSET: 0x008BE390
void Battlenet::Attribute::ArtId::SetDefault_height(uint16_t& value) {
    value = 0;
}

// OFFSET: 0x008BE390
void Battlenet::Attribute::ArtId::SetDefault_width(uint16_t& value) {
    value = 0;
}

// OFFSET: 0x008BE760
void Battlenet::Attribute::LocalizedString::SetDefault_colorHint(BSN::Optional<uint32_t>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE760
void Battlenet::Attribute::LocalizedString::SetDefault_fileIndex(uint8_t& value) {
    value = 0;
}

// OFFSET: 0x008BE390
void Battlenet::Attribute::LocalizedString::SetDefault_id(uint16_t& value) {
    value = 0;
}

// OFFSET: 0x008BE1F0
void Battlenet::Attribute::LocaleTableEntry::SetDefault_locale(BSN::FourCC& value) {
    value = Battlenet::Attribute::LocaleTableEntry::s_default_locale;
}

// OFFSET: 0x008BE3F0
void Battlenet::Attribute::LocaleTableEntry::SetDefault_stringFileTable(BSN::Array<BSN::FixedBlob<40>, 10>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE760
void Battlenet::Attribute::Visual::SetDefault_text(BSN::Optional<Battlenet::Attribute::LocalizedString>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE760
void Battlenet::Attribute::Visual::SetDefault_tip(BSN::Optional<Battlenet::Attribute::LocalizedString>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE760
void Battlenet::Attribute::Visual::SetDefault_art(BSN::Optional<Battlenet::Attribute::ArtId>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE200
void Battlenet::Attribute::Entry::SetDefault_code(BSN::FourCC& value) {
    value = Battlenet::Attribute::Entry::s_default_code;
}

// OFFSET: 0x008BE4C0
void Battlenet::Attribute::Entry::SetDefault_visual(Battlenet::Attribute::Visual& value) {
    Battlenet::Attribute::Visual::SetDefault_text(value.m_text);
    Battlenet::Attribute::Visual::SetDefault_tip(value.m_tip);
    Battlenet::Attribute::Visual::SetDefault_art(value.m_art);
}

// OFFSET: 0x008BE3F0
void Battlenet::Attribute::DefinitionHandle::SetDefault_nameSpace(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::Attribute::DefinitionHandle::SetDefault_id(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE210
void Battlenet::Attribute::Address::SetDefault_scope(Battlenet::Attribute::Scope::Enum& value) {
    value = Battlenet::Attribute::Scope::NONE;
}

// OFFSET: 0x008BE300
void Battlenet::Attribute::Address::SetDefault_definitionHandle(Battlenet::Attribute::DefinitionHandle& value) {
    Battlenet::Attribute::DefinitionHandle::SetDefault_nameSpace(value.m_nameSpace);
    Battlenet::Attribute::DefinitionHandle::SetDefault_id(value.m_id);
}

// OFFSET: 0x008BE220
void Battlenet::Attribute::Parent::SetDefault_address(Battlenet::Attribute::Address& value) {
    Battlenet::Attribute::Address::SetDefault_scope(value.m_scope);
    Battlenet::Attribute::Address::SetDefault_definitionHandle(value.m_definitionHandle);
}

// OFFSET: 0x008BE3F0
void Battlenet::Attribute::Parent::SetDefault_set(BSN::Array<BSN::FourCC, 32>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE760
void Battlenet::Attribute::NearInstance::SetDefault_index(uint8_t& value) {
    value = 0;
}

// OFFSET: 0x008BE760
void Battlenet::Attribute::NearInstance::SetDefault_baked(bool& value) {
    value = false;
}

// OFFSET: 0x008BE300
void Battlenet::Attribute::Instance::SetDefault_definitionHandle(Battlenet::Attribute::DefinitionHandle& value) {
    Battlenet::Attribute::DefinitionHandle::SetDefault_nameSpace(value.m_nameSpace);
    Battlenet::Attribute::DefinitionHandle::SetDefault_id(value.m_id);
}

// OFFSET: 0x008BFC80
void Battlenet::Attribute::Instance::SetDefault_data(Battlenet::Attribute::NearInstanceList& value) {
    value.m_tag = 0;
    Battlenet::Attribute::NearInstance& element = *reinterpret_cast<Battlenet::Attribute::NearInstance*>(value.m_value);
    Battlenet::Attribute::NearInstance::SetDefault_index(element.m_index);
    Battlenet::Attribute::NearInstance::SetDefault_baked(element.m_baked);
}

// OFFSET: 0x008BE300
void Battlenet::Attribute::Definition::SetDefault_handle(Battlenet::Attribute::DefinitionHandle& value) {
    Battlenet::Attribute::DefinitionHandle::SetDefault_nameSpace(value.m_nameSpace);
    Battlenet::Attribute::DefinitionHandle::SetDefault_id(value.m_id);
}

// OFFSET: 0x008BE3F0
void Battlenet::Attribute::Definition::SetDefault_table(BSN::Array<Battlenet::Attribute::Entry, 32>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE4C0
void Battlenet::Attribute::Definition::SetDefault_visual(Battlenet::Attribute::Visual& value) {
    Battlenet::Attribute::Visual::SetDefault_text(value.m_text);
    Battlenet::Attribute::Visual::SetDefault_tip(value.m_tip);
    Battlenet::Attribute::Visual::SetDefault_art(value.m_art);
}

// OFFSET: 0x008BE3F0
void Battlenet::Attribute::Definition::SetDefault_parents(BSN::Array<Battlenet::Attribute::Parent, 4>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::Attribute::Definition::SetDefault_arbitration(Battlenet::Attribute::Arbitration::Enum& value) {
    value = Battlenet::Attribute::Arbitration::NONE;
}

// OFFSET: 0x008BE3F0
void Battlenet::Attribute::Definition::SetDefault_visibility(Battlenet::Attribute::Visibility::Enum& value) {
    value = Battlenet::Attribute::Visibility::NO_ONE;
}

// OFFSET: 0x008BE3F0
void Battlenet::Attribute::Definition::SetDefault_access(Battlenet::Attribute::Access::Enum& value) {
    value = Battlenet::Attribute::Access::NO_ONE;
}

// OFFSET: 0x008BE240
void Battlenet::Attribute::Definition::SetDefault_flags(uint32_t& value) {
    value = 2;
}

// OFFSET: 0x008BFC80
void Battlenet::Attribute::Definition::SetDefault_defaults(Battlenet::Attribute::NearInstanceList& value) {
    value.m_tag = 0;
    Battlenet::Attribute::NearInstance& element = *reinterpret_cast<Battlenet::Attribute::NearInstance*>(value.m_value);
    Battlenet::Attribute::NearInstance::SetDefault_index(element.m_index);
    Battlenet::Attribute::NearInstance::SetDefault_baked(element.m_baked);
}

// OFFSET: 0x008BE760
void Battlenet::Attribute::Definition::SetDefault_sortOrder(uint8_t& value) {
    value = 0;
}

// OFFSET: 0x008BE300
void Battlenet::Attribute::DefinitionLock::SetDefault_handle(Battlenet::Attribute::DefinitionHandle& value) {
    Battlenet::Attribute::DefinitionHandle::SetDefault_nameSpace(value.m_nameSpace);
    Battlenet::Attribute::DefinitionHandle::SetDefault_id(value.m_id);
}

// OFFSET: 0x008BE760
void Battlenet::Attribute::DefinitionLock::SetDefault_lockedScopes(BSN::Optional<BSN::FixedBitArray<16>>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE5C0
void Battlenet::Attribute::WorkingSet::SetDefault_name(Battlenet::Attribute::LocalizedString& value) {
    Battlenet::Attribute::LocalizedString::SetDefault_colorHint(value.m_colorHint);
    Battlenet::Attribute::LocalizedString::SetDefault_fileIndex(value.m_fileIndex);
    Battlenet::Attribute::LocalizedString::SetDefault_id(value.m_id);
}

// OFFSET: 0x008BE5C0
void Battlenet::Attribute::WorkingSet::SetDefault_description(Battlenet::Attribute::LocalizedString& value) {
    Battlenet::Attribute::LocalizedString::SetDefault_colorHint(value.m_colorHint);
    Battlenet::Attribute::LocalizedString::SetDefault_fileIndex(value.m_fileIndex);
    Battlenet::Attribute::LocalizedString::SetDefault_id(value.m_id);
}

// OFFSET: 0x008BE760
void Battlenet::Attribute::WorkingSet::SetDefault_thumbnail(BSN::Optional<Battlenet::Attribute::ArtId>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE760
void Battlenet::Attribute::WorkingSet::SetDefault_bigMap(BSN::Optional<Battlenet::Attribute::ArtId>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE250
void Battlenet::Attribute::WorkingSet::SetDefault_maxPlayers(uint8_t& value) {
    value = 1;
}

// OFFSET: 0x008BE210
void Battlenet::Attribute::WorkingSet::SetDefault_specialScope(Battlenet::Attribute::Scope::Enum& value) {
    value = Battlenet::Attribute::Scope::NONE;
}

// OFFSET: 0x008BE3F0
void Battlenet::Attribute::WorkingSet::SetDefault_instances(BSN::Array<Battlenet::Attribute::Instance, 64>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::Attribute::WorkingSet::SetDefault_visualFiles(BSN::Array<BSN::FixedBlob<40>, 16>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::Attribute::WorkingSet::SetDefault_locales(BSN::Array<Battlenet::Attribute::LocaleTableEntry, 16>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE760
void Battlenet::Toon::FullName::SetDefault_region(uint8_t& value) {
    value = 0;
}

// OFFSET: 0x008BE270
void Battlenet::Toon::FullName::SetDefault_programId(BSN::FourCC& value) {
    value = Battlenet::Toon::FullName::s_default_programId;
}

// OFFSET: 0x008BE3F0
void Battlenet::Toon::FullName::SetDefault_realm(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE280
void Battlenet::Toon::FullName::SetDefault_name(BSN::String<25>& value) {
    value = Battlenet::Toon::FullName::s_default_name;
}

// OFFSET: 0x008BE3F0
void Battlenet::Profile::RecordAddress::SetDefault_label(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE300
void Battlenet::Profile::RecordAddress::SetDefault_id(uint64_t& value) {
    value = 0;
}

// OFFSET: 0x008BE760
void Battlenet::Toon::Handle::SetDefault_region(uint8_t& value) {
    value = 0;
}

// OFFSET: 0x008BE260
void Battlenet::Toon::Handle::SetDefault_programId(BSN::FourCC& value) {
    value = Battlenet::Toon::Handle::s_default_programId;
}

// OFFSET: 0x008BE3F0
void Battlenet::Toon::Handle::SetDefault_realm(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE300
void Battlenet::Toon::Handle::SetDefault_id(uint64_t& value) {
    value = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Master::InstanceHandle::SetDefault_instanceId(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Master::InstanceHandle::SetDefault_serverLabel(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Master::InstanceHandle::SetDefault_serverEpoch(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Map::Handle::SetDefault_id(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Map::Handle::SetDefault_version(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE300
void Battlenet::S2Map::LinkEntry::SetDefault_handle(Battlenet::S2Map::Handle& value) {
    Battlenet::S2Map::Handle::SetDefault_id(value.m_id);
    Battlenet::S2Map::Handle::SetDefault_version(value.m_version);
}

// VERIFY: cb23fd42 auto
// OFFSET: 0x00955F90
void Battlenet::S2Map::LinkEntry::SetDefault_type(Battlenet::S2Map::LinkEntryType::Enum& value) {
    value = Battlenet::S2Map::LinkEntryType::OPTIONAL;
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::Link::SetDefault_variantIndex(uint8_t& value) {
    value = 0;
}

// OFFSET: 0x008BE2F0
void Battlenet::S2Map::Link::SetDefault_speed(BSN::FourCC& value) {
    value = Battlenet::S2Map::Link::s_default_speed;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Map::Link::SetDefault_entries(BSN::Array<Battlenet::S2Map::LinkEntry, 10>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE300
void Battlenet::Result::Container::SetDefault_handle(Battlenet::Attribute::DefinitionHandle& value) {
    Battlenet::Attribute::DefinitionHandle::SetDefault_nameSpace(value.m_nameSpace);
    Battlenet::Attribute::DefinitionHandle::SetDefault_id(value.m_id);
}

// OFFSET: 0x008BE3F0
void Battlenet::Result::Container::SetDefault_orderHint(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE2A0
void Battlenet::Result::Definition::SetDefault_name(BSN::String<31>& value) {
    value = Battlenet::Result::Definition::s_default_name;
}

// OFFSET: 0x008BE300
void Battlenet::Result::Definition::SetDefault_handle(Battlenet::Attribute::DefinitionHandle& value) {
    Battlenet::Attribute::DefinitionHandle::SetDefault_nameSpace(value.m_nameSpace);
    Battlenet::Attribute::DefinitionHandle::SetDefault_id(value.m_id);
}

// OFFSET: 0x008BE4C0
void Battlenet::Result::Definition::SetDefault_visual(Battlenet::Attribute::Visual& value) {
    Battlenet::Attribute::Visual::SetDefault_text(value.m_text);
    Battlenet::Attribute::Visual::SetDefault_tip(value.m_tip);
    Battlenet::Attribute::Visual::SetDefault_art(value.m_art);
}

// OFFSET: 0x008BE3F0
void Battlenet::Result::Definition::SetDefault_containers(BSN::Array<Battlenet::Result::Container, 8>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE2C0
void Battlenet::Result::Definition::SetDefault_form(Battlenet::Result::Form::Enum& value) {
    value = Battlenet::Result::Form::COMMENT;
}

// OFFSET: 0x008BE250
void Battlenet::Result::Definition::SetDefault_flags(uint8_t& value) {
    value = 1;
}

// OFFSET: 0x008BE390
void Battlenet::S2Map::MapSize::SetDefault_horizontal(uint16_t& value) {
    value = 0;
}

// OFFSET: 0x008BE390
void Battlenet::S2Map::MapSize::SetDefault_vertical(uint16_t& value) {
    value = 0;
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::VariantId::SetDefault_category(uint8_t& value) {
    value = 0;
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::VariantId::SetDefault_mode(uint8_t& value) {
    value = 0;
}

// OFFSET: 0x008BE3A0
void Battlenet::S2Map::VariantInfo::SetDefault_variantId(Battlenet::S2Map::VariantId& value) {
    Battlenet::S2Map::VariantId::SetDefault_category(value.m_category);
    Battlenet::S2Map::VariantId::SetDefault_mode(value.m_mode);
}

// OFFSET: 0x008BE5C0
void Battlenet::S2Map::VariantInfo::SetDefault_categoryName(Battlenet::Attribute::LocalizedString& value) {
    Battlenet::Attribute::LocalizedString::SetDefault_colorHint(value.m_colorHint);
    Battlenet::Attribute::LocalizedString::SetDefault_fileIndex(value.m_fileIndex);
    Battlenet::Attribute::LocalizedString::SetDefault_id(value.m_id);
}

// OFFSET: 0x008BE5C0
void Battlenet::S2Map::VariantInfo::SetDefault_modeName(Battlenet::Attribute::LocalizedString& value) {
    Battlenet::Attribute::LocalizedString::SetDefault_colorHint(value.m_colorHint);
    Battlenet::Attribute::LocalizedString::SetDefault_fileIndex(value.m_fileIndex);
    Battlenet::Attribute::LocalizedString::SetDefault_id(value.m_id);
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::VariantInfo::SetDefault_categoryDescription(BSN::Optional<Battlenet::Attribute::LocalizedString>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::VariantInfo::SetDefault_modeDescription(BSN::Optional<Battlenet::Attribute::LocalizedString>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE4C0
void Battlenet::S2Map::VariantInfo::SetDefault_visual(Battlenet::Attribute::Visual& value) {
    Battlenet::Attribute::Visual::SetDefault_text(value.m_text);
    Battlenet::Attribute::Visual::SetDefault_tip(value.m_tip);
    Battlenet::Attribute::Visual::SetDefault_art(value.m_art);
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Map::VariantInfo::SetDefault_instances(BSN::Array<Battlenet::Attribute::Instance, 64>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Map::VariantInfo::SetDefault_lockedAttributes(BSN::Array<Battlenet::Attribute::DefinitionLock, 64>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE250
void Battlenet::S2Map::VariantInfo::SetDefault_maxTeamSize(uint8_t& value) {
    value = 1;
}

// OFFSET: 0x008BE300
void Battlenet::S2Map::BnetHeader::SetDefault_handle(Battlenet::S2Map::Handle& value) {
    Battlenet::S2Map::Handle::SetDefault_id(value.m_id);
    Battlenet::S2Map::Handle::SetDefault_version(value.m_version);
}

// OFFSET: 0x008BE3B0
void Battlenet::S2Map::BnetHeader::SetDefault_name(BSN::String<80>& value) {
    value = Battlenet::S2Map::BnetHeader::s_default_name;
}

// OFFSET: 0x008BE5E0
void Battlenet::S2Map::BnetHeader::SetDefault_archiveCacheHandle(BSN::FixedBlob<40>& value) {
    value = Battlenet::S2Map::BnetHeader::s_default_archiveCacheHandle;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Map::BnetHeader::SetDefault_mapNamespace(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE720
void Battlenet::S2Map::BnetHeader::SetDefault_workingSet(Battlenet::Attribute::WorkingSet& value) {
    Battlenet::Attribute::WorkingSet::SetDefault_name(value.m_name);
    Battlenet::Attribute::WorkingSet::SetDefault_description(value.m_description);
    Battlenet::Attribute::WorkingSet::SetDefault_thumbnail(value.m_thumbnail);
    Battlenet::Attribute::WorkingSet::SetDefault_bigMap(value.m_bigMap);
    Battlenet::Attribute::WorkingSet::SetDefault_maxPlayers(value.m_maxPlayers);
    Battlenet::Attribute::WorkingSet::SetDefault_specialScope(value.m_specialScope);
    Battlenet::Attribute::WorkingSet::SetDefault_instances(value.m_instances);
    Battlenet::Attribute::WorkingSet::SetDefault_visualFiles(value.m_visualFiles);
    Battlenet::Attribute::WorkingSet::SetDefault_locales(value.m_locales);
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Map::BnetHeader::SetDefault_definitions(BSN::Array<Battlenet::Attribute::Definition, 64>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Map::BnetHeader::SetDefault_dependencies(BSN::Array<Battlenet::S2Map::Handle, 4>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Map::BnetHeader::SetDefault_resultDefinitions(BSN::Array<Battlenet::Result::Definition, 512>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Map::BnetHeader::SetDefault_locales(BSN::Array<Battlenet::Attribute::LocaleTableEntry, 16>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::BnetHeader::SetDefault_size(BSN::Optional<Battlenet::S2Map::MapSize>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::BnetHeader::SetDefault_tileset(BSN::Optional<Battlenet::Attribute::LocalizedString>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::BnetHeader::SetDefault_label(BSN::Optional<BSN::FourCC>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::BnetHeader::SetDefault_defaultVariantIndex(uint8_t& value) {
    value = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Map::BnetHeader::SetDefault_variants(BSN::Array<Battlenet::S2Map::VariantInfo, 32>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::GameHeader::SetDefault_dummy(bool& value) {
    value = false;
}

// OFFSET: 0x008BE6E0
void Battlenet::S2Map::Header::SetDefault_bnet(Battlenet::S2Map::BnetHeader& value) {
    Battlenet::S2Map::BnetHeader::SetDefault_handle(value.m_handle);
    Battlenet::S2Map::BnetHeader::SetDefault_name(value.m_name);
    Battlenet::S2Map::BnetHeader::SetDefault_archiveCacheHandle(value.m_archiveCacheHandle);
    Battlenet::S2Map::BnetHeader::SetDefault_mapNamespace(value.m_mapNamespace);
    Battlenet::S2Map::BnetHeader::SetDefault_workingSet(value.m_workingSet);
    Battlenet::S2Map::BnetHeader::SetDefault_definitions(value.m_definitions);
    Battlenet::S2Map::BnetHeader::SetDefault_dependencies(value.m_dependencies);
    Battlenet::S2Map::BnetHeader::SetDefault_resultDefinitions(value.m_resultDefinitions);
    Battlenet::S2Map::BnetHeader::SetDefault_locales(value.m_locales);
    Battlenet::S2Map::BnetHeader::SetDefault_size(value.m_size);
    Battlenet::S2Map::BnetHeader::SetDefault_tileset(value.m_tileset);
    Battlenet::S2Map::BnetHeader::SetDefault_label(value.m_label);
    Battlenet::S2Map::BnetHeader::SetDefault_defaultVariantIndex(value.m_defaultVariantIndex);
    Battlenet::S2Map::BnetHeader::SetDefault_variants(value.m_variants);
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::Header::SetDefault_game(Battlenet::S2Map::GameHeader& value) {
    Battlenet::S2Map::GameHeader::SetDefault_dummy(value.m_dummy);
}

// OFFSET: 0x008BE210
void Battlenet::Result::PlayerObject::SetDefault_scope(Battlenet::Attribute::Scope::Enum& value) {
    value = Battlenet::Attribute::Scope::NONE;
}

// OFFSET: 0x008BFCA0
void Battlenet::Result::PlayerObject::SetDefault_playerObjectType(Battlenet::Result::PlayerObjectType& value) {
    value.m_tag = 2;
    *reinterpret_cast<bool*>(value.m_value) = false;
}

// OFFSET: 0x008BE3D0
void Battlenet::S2Game::Summary::ParticipantRole::SetDefault_resultCode(Battlenet::S2Game::PlayerResultCode::Enum& value) {
    value = Battlenet::S2Game::PlayerResultCode::UNDECIDED;
}

// OFFSET: 0x008BE2D0
void Battlenet::Result::PlayerObjectType::Player::SetDefault_toonHandle(Battlenet::Toon::Handle& value) {
    Battlenet::Toon::Handle::SetDefault_region(value.m_region);
    Battlenet::Toon::Handle::SetDefault_programId(value.m_programId);
    Battlenet::Toon::Handle::SetDefault_realm(value.m_realm);
    Battlenet::Toon::Handle::SetDefault_id(value.m_id);
}

// OFFSET: 0x008BE760
void Battlenet::Result::PlayerObjectType::Player::SetDefault_toonProfileAddress(BSN::Optional<Battlenet::Profile::RecordAddress>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE300
void Battlenet::S2Map::InfoBase::SetDefault_handle(Battlenet::S2Map::Handle& value) {
    Battlenet::S2Map::Handle::SetDefault_id(value.m_id);
    Battlenet::S2Map::Handle::SetDefault_version(value.m_version);
}

// OFFSET: 0x008BE500
void Battlenet::S2Map::InfoBase::SetDefault_headerCacheHandle(BSN::FixedBlob<40>& value) {
    value = Battlenet::S2Map::InfoBase::s_default_headerCacheHandle;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Map::InfoBase::SetDefault_uploadTime(int32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::InfoBase::SetDefault_isLinked(bool& value) {
    value = false;
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::InfoBase::SetDefault_isLocked(bool& value) {
    value = false;
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::InfoBase::SetDefault_isPrivate(bool& value) {
    value = false;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Map::InfoBase::SetDefault_mapSize(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE320
void Battlenet::S2Map::InfoBase::SetDefault_name(BSN::String<80>& value) {
    value = Battlenet::S2Map::InfoBase::s_default_name;
}

// OFFSET: 0x008BE340
void Battlenet::S2Map::InfoBase::SetDefault_profileRecordAddress(Battlenet::Profile::RecordAddress& value) {
    Battlenet::Profile::RecordAddress::SetDefault_label(value.m_label);
    Battlenet::Profile::RecordAddress::SetDefault_id(value.m_id);
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::InfoBase::SetDefault_isMod(bool& value) {
    value = false;
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::InfoBase::SetDefault_label(BSN::Optional<BSN::FourCC>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE360
void Battlenet::S2Map::InfoBase::SetDefault_authorToonName(Battlenet::Toon::FullName& value) {
    Battlenet::Toon::FullName::SetDefault_region(value.m_region);
    Battlenet::Toon::FullName::SetDefault_programId(value.m_program);
    Battlenet::Toon::FullName::SetDefault_realm(value.m_realm);
    Battlenet::Toon::FullName::SetDefault_name(value.m_name);
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::InfoBase::SetDefault_isLatestVersion(bool& value) {
    value = false;
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::InfoBase::SetDefault_mainLocale(BSN::Optional<BSN::FourCC>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE520
void Battlenet::S2Map::Info::SetDefault_InfoBase(Battlenet::S2Map::InfoBase& value) {
    Battlenet::S2Map::InfoBase::SetDefault_handle(value.m_handle);
    Battlenet::S2Map::InfoBase::SetDefault_headerCacheHandle(value.m_headerCacheHandle);
    Battlenet::S2Map::InfoBase::SetDefault_uploadTime(value.m_uploadTime);
    Battlenet::S2Map::InfoBase::SetDefault_isLinked(value.m_isLinked);
    Battlenet::S2Map::InfoBase::SetDefault_isLocked(value.m_isLocked);
    Battlenet::S2Map::InfoBase::SetDefault_isPrivate(value.m_isPrivate);
    Battlenet::S2Map::InfoBase::SetDefault_mapSize(value.m_mapSize);
    Battlenet::S2Map::InfoBase::SetDefault_name(value.m_name);
    Battlenet::S2Map::InfoBase::SetDefault_profileRecordAddress(value.m_profileRecordAddress);
    Battlenet::S2Map::InfoBase::SetDefault_isMod(value.m_isMod);
    Battlenet::S2Map::InfoBase::SetDefault_label(value.m_label);
    Battlenet::S2Map::InfoBase::SetDefault_authorToonName(value.m_authorToonName);
    Battlenet::S2Map::InfoBase::SetDefault_isLatestVersion(value.m_isLatestVersion);
    Battlenet::S2Map::InfoBase::SetDefault_mainLocale(value.m_mainLocale);
}

// OFFSET: 0x008BE760
void Battlenet::S2Map::Info::SetDefault_authorRealName(BSN::Optional<Battlenet::Account::FullName>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE760
void Battlenet::Event::OfflineDataStore::Email::SetDefault_hash(BSN::Optional<uint64_t>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE3E0
void Battlenet::Event::OfflineDataStore::TagIdx::SetDefault_tag(BSN::FourCC& value) {
    value = Battlenet::Event::OfflineDataStore::TagIdx::s_default_tag;
}

// OFFSET: 0x008BE3F0
void Battlenet::Event::OfflineDataStore::TagIdx::SetDefault_idx(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE4E0
void Battlenet::Toon::UnlockDefinitionFile::SetDefault_unlockDefinitionFileCacheHandle(BSN::FixedBlob<40>& value) {
    value = Battlenet::Toon::UnlockDefinitionFile::s_default_unlockDefinitionFileCacheHandle;
}

// OFFSET: 0x008BE360
void Battlenet::Event::OfflineDataStore::StoredData::SetDefault_toon(Battlenet::Toon::FullName& value) {
    Battlenet::Toon::FullName::SetDefault_region(value.m_region);
    Battlenet::Toon::FullName::SetDefault_programId(value.m_program);
    Battlenet::Toon::FullName::SetDefault_realm(value.m_realm);
    Battlenet::Toon::FullName::SetDefault_name(value.m_name);
}

// OFFSET: 0x008BE400
void Battlenet::Event::OfflineDataStore::StoredData::SetDefault_fullName(Battlenet::Account::FullName& value) {
    Battlenet::Account::FullName::SetDefault_givenName(value.m_givenName);
    Battlenet::Account::FullName::SetDefault_surname(value.m_surname);
}

// OFFSET: 0x008BE760
void Battlenet::Event::OfflineDataStore::StoredData::SetDefault_accountRegion(uint8_t& value) {
    value = 0;
}

// OFFSET: 0x008BE760
void Battlenet::Event::OfflineDataStore::StoredData::SetDefault_gameAccountRegion(uint8_t& value) {
    value = 0;
}

// OFFSET: 0x008BE430
void Battlenet::Event::OfflineDataStore::StoredData::SetDefault_gameAccountName(BSN::AsciiString<32>& value) {
    value = Battlenet::Event::OfflineDataStore::StoredData::s_default_gameAccountName;
}

// OFFSET: 0x008BE300
void Battlenet::Event::OfflineDataStore::StoredData::SetDefault_gameAccountFlags(uint64_t& value) {
    value = 0;
}

// OFFSET: 0x008BE450
void Battlenet::Event::OfflineDataStore::StoredData::SetDefault_program(BSN::FourCC& value) {
    value = Battlenet::Event::OfflineDataStore::StoredData::s_default_program;
}

// OFFSET: 0x008BE760
void Battlenet::Event::OfflineDataStore::StoredData::SetDefault_tagidx(BSN::Optional<BSN::Array<Battlenet::Event::OfflineDataStore::TagIdx, 16>>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE760
void Battlenet::Event::OfflineDataStore::StoredData::SetDefault_unlockablesFiles(BSN::Optional<BSN::Array<Battlenet::Toon::UnlockDefinitionFile, 16>>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE3F0
void Battlenet::Event::OfflineDataStore::StoredData::SetDefault_lastOnline(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE2D0
void Battlenet::Event::OfflineDataStore::StoredData::SetDefault_toonHandle(Battlenet::Toon::Handle& value) {
    Battlenet::Toon::Handle::SetDefault_region(value.m_region);
    Battlenet::Toon::Handle::SetDefault_programId(value.m_programId);
    Battlenet::Toon::Handle::SetDefault_realm(value.m_realm);
    Battlenet::Toon::Handle::SetDefault_id(value.m_id);
}

// OFFSET: 0x008BE3F0
void Battlenet::Event::OfflineDataStore::StoredData::SetDefault_accountId(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE760
void Battlenet::Event::OfflineDataStore::OfflineData::SetDefault_info(BSN::Optional<BSN::Array<Battlenet::Event::OfflineDataStore::StoredData, 3>>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE3F0
void Battlenet::Result::Value::SetDefault_value(uint32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE760
void Battlenet::Result::Value::SetDefault_maxValue(BSN::Optional<uint32_t>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE3F0
void Battlenet::Result::Value::SetDefault_time(int32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE300
void Battlenet::Result::Instance::SetDefault_handle(Battlenet::Attribute::DefinitionHandle& value) {
    Battlenet::Attribute::DefinitionHandle::SetDefault_nameSpace(value.m_nameSpace);
    Battlenet::Attribute::DefinitionHandle::SetDefault_id(value.m_id);
}

// OFFSET: 0x008BE3F0
void Battlenet::Result::Instance::SetDefault_values(BSN::Array<BSN::Array<Battlenet::Result::Value, 32>, 15>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::Result::DefinitionListContainer::SetDefault_definitionList(BSN::Array<Battlenet::Result::Definition, 512>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::Result::InstanceListContainer::SetDefault_instanceList(BSN::Array<Battlenet::Result::Instance, 10>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::Result::PlayerListContainer::SetDefault_playerList(BSN::Array<Battlenet::Result::PlayerObject, 16>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BFCC0
void Battlenet::S2Game::Summary::PlayerData::SetDefault_playerObject(Battlenet::Result::PlayerObject& value) {
    Battlenet::Result::PlayerObject::SetDefault_scope(value.m_scope);
    Battlenet::Result::PlayerObject::SetDefault_playerObjectType(value.m_playerObjectType);
}

// OFFSET: 0x008BFCE0
void Battlenet::S2Game::Summary::PlayerData::SetDefault_role(Battlenet::S2Game::Summary::Role& value) {
    value.m_tag = 0;
    Battlenet::S2Game::Summary::ParticipantRole& element = *reinterpret_cast<Battlenet::S2Game::Summary::ParticipantRole*>(value.m_value);
    Battlenet::S2Game::Summary::ParticipantRole::SetDefault_resultCode(element.m_resultCode);
}

// OFFSET: 0x008BE760
void Battlenet::S2Game::Summary::PlayerData::SetDefault_actualRace(BSN::Optional<BSN::FourCC>& value) {
    value.m_exists = false;
}

// OFFSET: 0x008BE6F0
void Battlenet::S2Game::Summary::Data::SetDefault_mapLink(Battlenet::S2Map::Link& value) {
    Battlenet::S2Map::Link::SetDefault_variantIndex(value.m_variantIndex);
    Battlenet::S2Map::Link::SetDefault_speed(value.m_speed);
    Battlenet::S2Map::Link::SetDefault_entries(value.m_entries);
}

// OFFSET: 0x008BE760
void Battlenet::S2Game::Summary::Data::SetDefault_replayCacheHandle(BSN::Optional<BSN::FixedBlob<40>>& value) {
    value.m_exists = false;
}

// VERIFY: 0ff04e43 auto
// OFFSET: 0x00956350
void Battlenet::S2Game::Summary::Data::SetDefault_instanceHandle(Battlenet::S2Master::InstanceHandle& value) {
    Battlenet::S2Master::InstanceHandle::SetDefault_instanceId(value.m_instanceId);
    Battlenet::S2Master::InstanceHandle::SetDefault_serverLabel(value.m_serverLabel);
    Battlenet::S2Master::InstanceHandle::SetDefault_serverEpoch(value.m_serverEpoch);
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Game::Summary::Data::SetDefault_playerList(BSN::Array<Battlenet::S2Game::Summary::PlayerData, 16>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE710
void Battlenet::S2Game::Summary::Data::SetDefault_seed(BSN::FixedBlob<4>& value) {
    value = Battlenet::S2Game::Summary::Data::s_default_seed;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Game::Summary::Data::SetDefault_definitionList(BSN::Array<Battlenet::Attribute::Definition, 64>& value) {
    value.m_count = 0;
}

// OFFSET: 0x008BE720
void Battlenet::S2Game::Summary::Data::SetDefault_workingSet(Battlenet::Attribute::WorkingSet& value) {
    Battlenet::Attribute::WorkingSet::SetDefault_name(value.m_name);
    Battlenet::Attribute::WorkingSet::SetDefault_description(value.m_description);
    Battlenet::Attribute::WorkingSet::SetDefault_thumbnail(value.m_thumbnail);
    Battlenet::Attribute::WorkingSet::SetDefault_bigMap(value.m_bigMap);
    Battlenet::Attribute::WorkingSet::SetDefault_maxPlayers(value.m_maxPlayers);
    Battlenet::Attribute::WorkingSet::SetDefault_specialScope(value.m_specialScope);
    Battlenet::Attribute::WorkingSet::SetDefault_instances(value.m_instances);
    Battlenet::Attribute::WorkingSet::SetDefault_visualFiles(value.m_visualFiles);
    Battlenet::Attribute::WorkingSet::SetDefault_locales(value.m_locales);
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Game::Summary::Data::SetDefault_simulationTime(int32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE3F0
void Battlenet::S2Game::Summary::Data::SetDefault_endTime(int32_t& value) {
    value = 0;
}

// OFFSET: 0x008BE760
void Battlenet::S2Game::Summary::Data::SetDefault_resultsCacheHandle(BSN::Optional<BSN::FixedBlob<40>>& value) {
    value.m_exists = false;
}
