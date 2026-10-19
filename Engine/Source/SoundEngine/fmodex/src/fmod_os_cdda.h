#ifndef _FMOD_OS_CDDA
#define _FMOD_OS_CDDA

#include "fmod.h"
#include "fmod_linkedlist.h"


namespace FMOD
{

//#define FMOD_OS_CDDA_DEBUG
//#define FMOD_OS_CDDA_DUMPTOC

#ifdef FMOD_OS_CDDA_DEBUG
  #define FMOD_OS_CDDA_LOG_FILENAME "fmodcdda.log"
  #define FMOD_OS_CDDA_DEBUGPRINT   FMOD_OS_CDDA_DebugPrint
  #define FMOD_OS_CDDA_DEBUGINIT    FMOD_OS_CDDA_DebugInit()
  #ifndef FMOD_OS_CDDA_DUMPTOC
    #define FMOD_OS_CDDA_DUMPTOC
  #endif
  void FMOD_OS_CDDA_DebugPrint(char *format, ...);
  void FMOD_OS_CDDA_AddProfileData(int elapsedtime, unsigned int bytesread);
#else
  #define FMOD_OS_CDDA_DEBUGPRINT
  #define FMOD_OS_CDDA_DEBUGINIT
#endif


#define FMOD_CDDA_MAX_DEVICES       8         // Maximum number of devices we can handle
#define FMOD_CDDA_MAX_TRACKS        100       // Maximum number of tracks on a single device
#define FMOD_CDDA_SPINUP_TIME       1         // How many seconds to wait for a device to spin up to speed
#define FMOD_CDDA_SPINDOWN_TIME     5         // How many seconds before a device spins down after no activity
#define SIZEOF_CDDA_SECTOR          2352

typedef struct SRB SRB;
typedef unsigned long (*FUNC_SENDASPI32COMMAND)(SRB *srb);
typedef unsigned long (*FUNC_GETASPI32SUPPORTINFO)();

typedef struct
{
    unsigned int  value;
    char         *string;

} FMOD_CDDA_StringList;

typedef struct
{
    unsigned char SK;
    unsigned char ASC;
    unsigned char ASCQ;

} FMOD_CDDA_SenseKey;

/*
    Standard INQUIRY data
*/
typedef struct
{
    unsigned char peripheral_device_type     : 5;
    unsigned char peripheral_qualifier       : 3;

    unsigned char device_type_modifier       : 7;
    unsigned char removable_medium           : 1;

    unsigned char ansi_version               : 3;
    unsigned char ecma_version               : 3;
    unsigned char iso_version                : 2;

    unsigned char response_data_format       : 4;
    unsigned char res00                      : 2;
    unsigned char term_io_process            : 1;
    unsigned char async_event_notification   : 1;

    unsigned char additional_length;

    unsigned char res01;

    unsigned char res02;

    unsigned char soft_reset                 : 1;
    unsigned char cmd_queue                  : 1;
    unsigned char res03                      : 1;
    unsigned char linked                     : 1;
    unsigned char sync                       : 1;
    unsigned char wide_bus_16                : 1;
    unsigned char wide_bus_32                : 1;
    unsigned char relative_addressing        : 1;

    char          vendor_id[8];                         // Not null-terminated - padded with spaces
    char          product_id[16];
    char          product_revision[4];

} FMOD_CDDA_InquiryData;

/*
    CD capabilities and mechanism status page (mode page 0x2a)
*/
typedef struct
{
    unsigned char page_code                  : 6;
    unsigned char res00                      : 1;
    unsigned char ps                         : 1;

    unsigned char page_length;

    unsigned char cdr_read                   : 1;
    unsigned char cdrw_read                  : 1;
    unsigned char method2                    : 1;
    unsigned char dvdrom_read                : 1;
    unsigned char dvdr_read                  : 1;
    unsigned char dvdram_read                : 1;
    unsigned char res01                      : 2;

    unsigned char cdr_write                  : 1;
    unsigned char cdrw_write                 : 1;
    unsigned char test_write                 : 1;
    unsigned char res02                      : 1;
    unsigned char dvdr_write                 : 1;
    unsigned char dvdram_write               : 1;
    unsigned char res022                     : 2;

    unsigned char audio_play                 : 1;
    unsigned char composite                  : 1;
    unsigned char digital_port1              : 1;
    unsigned char digital_port2              : 1;
    unsigned char mode2_form1                : 1;
    unsigned char mode2_form2                : 1;
    unsigned char multi_session              : 1;
    unsigned char buf                        : 1;

    unsigned char cdda_support               : 1;
    unsigned char cdda_accurate              : 1;
    unsigned char rw_support                 : 1;
    unsigned char rw_deinterleaved_corrected : 1;
    unsigned char c2pointer_support          : 1;
    unsigned char isrc                       : 1;
    unsigned char upc                        : 1;
    unsigned char read_barcode               : 1;

    unsigned char lock                       : 1;
    unsigned char lock_state                 : 1;
    unsigned char prevent_jumper             : 1;
    unsigned char eject                      : 1;
    unsigned char res04                      : 1;
    unsigned char loading_mech_type          : 3;

    unsigned char seperate_vol_per_channel   : 1;
    unsigned char seperate_channel_mute      : 1;
    unsigned char disc_present_reporting     : 1;
    unsigned char sw_slot_selection          : 1;
    unsigned char side_change_capable        : 1;
    unsigned char rw_in_leadin               : 1;
    unsigned char res05                      : 2;

    unsigned char max_read_speed_hi;                    // Obsolete?
    unsigned char max_read_speed_lo;                    // Obsolete?

    unsigned char num_volume_levels_hi;
    unsigned char num_volume_levels_lo;

    unsigned char buffer_size_hi;
    unsigned char buffer_size_lo;

    unsigned char current_read_speed_hi;                // Obsolete?
    unsigned char current_read_speed_lo;                // Obsolete?

    unsigned char res06;

    unsigned char res07                      : 1;
    unsigned char bck                        : 1;
    unsigned char rck                        : 1;
    unsigned char lsbf                       : 1;
    unsigned char length                     : 2;
    unsigned char res08                      : 2;

    unsigned char max_write_speed_hi;                   // Obsolete?
    unsigned char max_write_speed_lo;                   // Obsolete?

    unsigned char current_write_speed_hi;               // Obsolete?
    unsigned char current_write_speed_lo;               // Obsolete?

    unsigned char copy_mgmt_revision_hi;
    unsigned char copy_mgmt_revision_lo;

    unsigned char res09;
    unsigned char res10;

} FMOD_CDDA_Caps;

typedef struct
{
    unsigned int      num_tracks;

    struct
    {
        unsigned char pre_emphasis           : 1;       // TRUE = audio w/pre-emphasis, FALSE = audio w/out pre-emphasis
        unsigned char digital_copy_permitted : 1;       // TRUE = copy permitted, FALSE = copy prohibited
        unsigned char data_track             : 1;       // TRUE = data track, FALSE = audio track
        unsigned char four_channel           : 1;       // TRUE = four channel audio, FALSE = two channel audio
        unsigned char adr                    : 4;       // ADR sub-channel Q field

    } flags[FMOD_CDDA_MAX_TRACKS];

    unsigned char     track_number[FMOD_CDDA_MAX_TRACKS];
    unsigned int      start_sector[FMOD_CDDA_MAX_TRACKS];
    unsigned int      num_sectors[FMOD_CDDA_MAX_TRACKS];

} FMOD_CDDA_TOC;

struct FMOD_CDDA_DEVICE
{
    char                   *name;                       // NULL-terminated string describing this device i.e. drive letter
    char                   *scsiaddr;                   // ASCII string representing the scsi addr of this device
    char                   *devicename;                 // String containing the vendor id and product id etc. of this device
    int                     letter;                     // Drive letter for this drive
    int                     target_id;                  // SCSI address
    int                     adapter_id;
    int                     lun_id;
    int                     device_open;                // TRUE if device has been opened
    int                     speed;                      // Desired speed e.g. 4x, 32x
    FMOD_CDDA_SenseKey      sense_key;
    FMOD_CDDA_TOC           toc;                        // Table of contents
    FMOD_CDTOC              usertoc;                    // Table of contents for user consumption
    FMOD_CDDA_InquiryData   inq_data;                   // Standard INQUIRY data
    FMOD_CDDA_Caps         *caps;                       // Mode page 0x2A capabilities info
    FUNC_SENDASPI32COMMAND  SendASPI32Command;
};


FMOD_RESULT FMOD_OS_CDDA_Init(bool force_aspi);
FMOD_RESULT FMOD_OS_CDDA_Shutdown();
bool        FMOD_OS_CDDA_IsDeviceName(char *name);
FMOD_RESULT FMOD_OS_CDDA_GetNumDevices(int *num);
FMOD_RESULT FMOD_OS_CDDA_GetDeviceName(int devicenum, char *name, int namelen, char *scsiaddr, int scsiaddrlen, char *devicename, int devicenamelen);
FMOD_RESULT FMOD_OS_CDDA_OpenDevice(char *name, FMOD_CDDA_DEVICE **device);
FMOD_RESULT FMOD_OS_CDDA_CloseDevice(FMOD_CDDA_DEVICE *device);

FMOD_RESULT FMOD_OS_CDDA_ReadSectors(FMOD_CDDA_DEVICE *device, char *buf, unsigned int start_sector, unsigned int num_sectors);
FMOD_RESULT FMOD_OS_CDDA_ReadToc(FMOD_CDDA_DEVICE *device, FMOD_CDDA_TOC *toc);
FMOD_RESULT FMOD_OS_CDDA_ReadTocRaw(FMOD_CDDA_DEVICE *device, FMOD_CDDA_TOC *toc);
FMOD_RESULT FMOD_OS_CDDA_SetSpeed(FMOD_CDDA_DEVICE *cdrom, int speed);
bool        FMOD_OS_CDDA_TestUnitReady(FMOD_CDDA_DEVICE *cdrom);


}

#endif


