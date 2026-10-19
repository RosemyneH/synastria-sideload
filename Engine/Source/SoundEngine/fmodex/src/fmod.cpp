/*$ preserve start $*/

#include "fmod_settings.h"

#include "fmod.h"
#include "fmod.hpp"
#include "fmod_channeli.h"
#include "fmod_debug.h"
#include "fmod_dspi.h"
#include "fmod_memory.h"
#include "fmod_linkedlist.h"
#include "fmod_listener.h"
#include "fmod_pluginfactory.h"
#include "fmod_soundi.h"
#include "fmod_systemi.h"
#include "fmod_cmdlog.h"


#if 0 //def DEBUG

#include <assert.h>

#if !defined(__MWERKS__) && !defined(__SN__)
void * operator new(unsigned int size) 
{ 
    assert(!"ERROR : tried to alloc a pointer using global new");
    return 0;
}

void operator delete(void *ptr)
{ 
    assert(!"ERROR : tried to delete a pointer using global delete\n");
}
#endif

#endif


/*
[API]
[
	[DESCRIPTION]
	Specifies a method for FMOD to allocate memory, either through callbacks or its own internal memory management. You can also supply a pool of memory for FMOD to work with and it will do so with no extra calls to malloc or free.

	[PARAMETERS]
    'poolmem'       If you want a fixed block of memory for FMOD to use, pass it in here.  Specify the length in poollen.  Specifying NULL doesn't use internal management and it relies on callbacks.
    'poollen'       Length in bytes of the pool of memory for FMOD to use specified in.  Specifying 0 turns off internal memory management and relies purely on callbacks.  Length must be a multiple of 512.
	'useralloc'     Only supported if pool is NULL.  Otherwise it overrides the FMOD internal calls to alloc.  Compatible with ansi malloc().
	'userrealloc'   Only supported if pool is NULL.  Otherwise it overrides the FMOD internal calls to realloc.  Compatible with ansi realloc().
    'userfree'      Only supported if pool is NULL.  Otherwise it overrides the FMOD internal calls to free. Compatible with ansi free().
    'memtypeflags'  FMOD_MEMORY_TYPE flags you wish to receive through your memory callbacks. See FMOD_MEMORY_TYPE.

 	[RETURN_VALUE]

	[REMARKS]
    This function is useful for systems that want FMOD to use their own memory management or for fixed memory devices such as Xbox, Xbox360, PS2 and GameCube that don't want any allocations occurring out of their control causing fragmentation or unpredictable overflows in a tight memory space.
    <p>
    FMOD only does allocation when creating streams, music or samples and the System::init stage. It never allocates or deallocates memory during the course of runtime processing. To find out the required fixed size the user can call Memory_Initialize with an overly large pool size (or no pool) and find out the maximum RAM usage at any one time with Memory_GetStats.
    <p>
    FMOD behaves differently based on what you pass into this function in 3 different combinations. For example :
    <p>
    <PRE>
    FMOD::Memory_Initialize(NULL, 0,   NULL,    NULL,      NULL);   // Falls back purely to ansi C malloc, realloc and free.
    FMOD::Memory_Initialize(NULL, 0,   myalloc, myrealloc, myfree); // Calls user supplied callbacks every time FMOD does a memory allocation or deallocation.
    FMOD::Memory_Initialize(ptr,  len, NULL,    NULL,      NULL);   // Uses "ptr" and manages memory internally.  NO extra mallocs or frees are performed from this point.
    </PRE>
    <p>
    Callbacks and memory pools cannot be combined. If a memory pool is provided by the user, FMOD accesses that pool using its own memory management scheme. FMOD's internal memory management scheme is extremely efficient and also faster than the standard C malloc and free.
    <p>
    When running on Xbox 1, you MUST specify a pointer and length. The memory provided must be enough to store all sample data.
    <p>
    <b>NOTE!</b> Your memory callbacks must be threadsafe otherwise unexpected behaviour may occur. FMOD calls memory allocation functions from other threads (such as the asynchronous loading thread used when you specify FMOD_NONBLOCKING) and sometimes from the mixer thread.
    <p>
    <b>NOTE!</b> If you specify a fixed size pool that is too small, FMOD will return FMOD_ERR_MEMORY when the limit of the fixed size pool is exceeded. At this point, it's possible that FMOD may become unstable. To maintain stability, do not allow FMOD to run out of memory.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]      
    FMOD_MEMORY_ALLOCCALLBACK
    FMOD_MEMORY_REALLOCCALLBACK
    FMOD_MEMORY_FREECALLBACK
    Memory_GetStats
    System::close
]
*/
// VERIFY: ded72527 auto
// OFFSET: 0x008D0550
FMOD_RESULT F_API FMOD_Memory_Initialize(void *poolmem, int poollen, FMOD_MEMORY_ALLOCCALLBACK useralloc, FMOD_MEMORY_REALLOCCALLBACK userrealloc, FMOD_MEMORY_FREECALLBACK userfree)
{
    if (!FMOD::gGlobal->gSystemHead->isEmpty())
    {
        return FMOD_ERR_INITIALIZED;
    }

    if (poollen % FMOD_MEMORY_DEFAULTBLOCKSIZE)
    {
        FLOGC((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "FMOD_Memory_Initialize", "Please pass a pool size aligned to a %d byte boundary\n", FMOD_MEMORY_DEFAULTBLOCKSIZE));
        return FMOD_ERR_INVALID_PARAM;
    }

    #ifdef PLATFORM_XENON
    // The CPU's virtual addresses for 'physical memory' allocations are divided
    // into three sections:
    //
    // 0xA0000000 - 0xBFFFFFF 64 KB Pages
    // 0xC0000000 - 0xDFFFFFF 16 MB Pages (Address range also used for cached-read-only pages, see below)
    // 0xE0000000 - 0xFFFFFFF  4 KB Pages
    if (poolmem && (unsigned int)poolmem < 0xA0000000)
    {
        FLOGC((FMOD_DEBUG_LEVEL_ERROR, __FILE__, __LINE__, "FMOD_Memory_Initialize", "Memory must be physical not virtual.\n"));
        return FMOD_ERR_INVALID_ADDRESS;
    }
    #endif

    if (poollen && poolmem)
    {
        FMOD_RESULT result;

        if (useralloc || userrealloc || userfree)
        {
            return FMOD_ERR_INVALID_PARAM;
        }
        if (poollen < FMOD_MEMORY_DEFAULTBLOCKSIZE)
        {
            return FMOD_ERR_INVALID_PARAM;
        }

        result = FMOD::gGlobal->gSystemPool->init(poolmem, poollen, FMOD_MEMORY_DEFAULTBLOCKSIZE);
        if (result == FMOD_OK)
        {
            FMOD::gGlobal->gSystemPool->setCallbacks(0,0,0);
        }

        return result;
    }
    else
    {
        if (poolmem || poollen)
        {
            return FMOD_ERR_INVALID_PARAM;
        }

        if (useralloc && userrealloc && userfree)
        {
            FMOD::gGlobal->gSystemPool->setCallbacks(useralloc, userrealloc, userfree);
        }
        else if (!useralloc && !userrealloc && !userfree)
        {
            FMOD::gGlobal->gSystemPool->setCallbacks(FMOD::Memory_DefaultMalloc, FMOD::Memory_DefaultRealloc, FMOD::Memory_DefaultFree);
        }
        else
        {
            return FMOD_ERR_INVALID_PARAM;
        }
    }

    return FMOD_OK;
}


/*
[API]
[
	[DESCRIPTION]
    Returns information on the memory usage of FMOD.  This is useful for determining a fixed memory size to make FMOD work within for fixed memory machines such as consoles.

	[PARAMETERS]
    'currentalloced'    Address of a variable that receives the currently allocated memory at time of call.  Optional.  Specify 0 or NULL to ignore. 
    'maxalloced'        Address of a variable that receives the maximum allocated memory since System::init or Memory_Initialize.  Optional.  Specify 0 or NULL to ignore. 
    'blocking'          Boolean indicating whether to favour speed or accuracy. Specifying true for this parameter will flush the DSP network to make sure all queued allocations happen immediately, which can be costly.

 	[RETURN_VALUE]
 
	[REMARKS]
    Note that if using FMOD::Memory_Initialize, the memory usage will be slightly higher than without it, as FMOD has to have a small amount of memory overhead to manage the available memory.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::init
    Memory_Initialize
]
*/
FMOD_RESULT F_API FMOD_Memory_GetStats(int *currentalloced, int *maxalloced)
{
    if (currentalloced)
    {
	    *currentalloced = FMOD::gGlobal->gSystemPool->getCurrentAllocated();
    }
    if (maxalloced)
    {
	    *maxalloced = FMOD::gGlobal->gSystemPool->getMaxAllocated();
    }

    return FMOD_OK;
}


/*
[API]
[
	[DESCRIPTION]
    FMOD System creation function.  This must be called to create an FMOD System object before you can do anything else.
    Use this function to create 1, or multiple instances of FMOD System objects.

	[PARAMETERS]
    'system'    Address of a pointer that receives the new FMOD System object.
 
	[RETURN_VALUE]

	[REMARKS]
    Use System::release to free a system object.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    System::init
    System::release
]
*/
// VERIFY: d7326b16 auto
// OFFSET: 0x008D0660
FMOD_RESULT F_API FMOD_System_Create(FMOD_SYSTEM **system)
{
    FMOD::SystemI *sys, *current;
    int   count;
    bool usedsys[1 << FMOD::SYSTEMID_BITS];
    
    if (!system)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    sys = FMOD_Object_Calloc(FMOD::SystemI);

    *system = (FMOD_SYSTEM *)sys;
    if (!*system)
    {
        return FMOD_ERR_MEMORY;
    }

    FMOD_memset(usedsys, 0, sizeof(bool) * (1 << FMOD::SYSTEMID_BITS));
    for (current = (FMOD::SystemI *)(FMOD::gGlobal->gSystemHead->getNext()); current != FMOD::gGlobal->gSystemHead; current = (FMOD::SystemI *)(current->getNext()))
    {
        usedsys[current->mIndex - 1] = true;
    }

    for (count = 0; count < (1 << FMOD::SYSTEMID_BITS) - 1; count++)
    {
        if (!usedsys[count])
        {
            sys->mIndex = count + 1;
            break;
        }
    }

    if (count == (1 << FMOD::SYSTEMID_BITS) - 1)
    {
        FMOD_Memory_Free(sys);
        return FMOD_ERR_MEMORY;
    }

    sys->addAfter(FMOD::gGlobal->gSystemHead);

#ifdef FMOD_SUPPORT_CMDLOG
    FMOD_RESULT result = FMOD_CmdLog_Init();
    if (result != FMOD_OK)
    {
        return result;
    }
#endif

    return FMOD_OK;
}


/*
[API]
[
	[DESCRIPTION]
    Closes and frees a system object and its resources.

	[PARAMETERS]
    'system'    Address of variable that receives an FMOD::System object.
 
	[RETURN_VALUE]

	[REMARKS]
    This function also calls System::close, so calling close before this function is not necessary.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    System_Create
    System::init
    System::close
]
*/
FMOD_RESULT F_API FMOD_System_Release(FMOD_SYSTEM *system)
{
    FMOD::System *sys = (FMOD::System *)system;

    FMOD_RESULT result = sys->release();
    if (result != FMOD_OK)
    {
        return result;
    }

    return FMOD_OK;
}


/*$ preserve end $*/
/*
[API]
[
    [DESCRIPTION]
    This function selects the output mode for the platform.  This is for selecting different OS specific APIs which might have different features.

    [PARAMETERS]
    'output'    Output type to select.  See type list for different output types you can select.

    [RETURN_VALUE]

    [REMARKS]
    This function is not necessary to call.  It is only if you want to specifically switch away from the default output mode for the operating system.
    The most optimal mode is selected by default for the operating system.  For example <b>FMOD_OUTPUTTYPE_DSOUND</b> is selected on all operating systems except for Windows NT, where FMOD_OUTPUTTYPE_WINMM is selected because it is lower latency / faster.
    <br>
    <br>
    This function cannot be called after FMOD is already activated with System::init.<br>
    It must be called before System::init, or after System::close.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_OUTPUTTYPE
    System::init
    System::close
]
*/
FMOD_RESULT F_API FMOD_System_SetOutput(FMOD_SYSTEM *system, FMOD_OUTPUTTYPE output)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setOutput(output);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current output system FMOD is using to address the hardware.

    [PARAMETERS]
    'output'    Address of a variable that receives the current output type.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_OUTPUTTYPE
]
*/
FMOD_RESULT F_API FMOD_System_GetOutput(FMOD_SYSTEM *system, FMOD_OUTPUTTYPE *output)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getOutput(output);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of soundcard devices on the machine, specific to the output mode set with System::setOutput.

    [PARAMETERS]
    'numdrivers'    Address of a variable that receives the number of output drivers.

    [RETURN_VALUE]

    [REMARKS]
    If System::setOutput is not called it will return the number of drivers available for the default output type.
    Use this for enumerating sound devices.  Use System::getDriverInfo to get the device's name.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getDriver
    System::getDriverInfo
    System::setOutput
    System::getOutput
]
*/
FMOD_RESULT F_API FMOD_System_GetNumDrivers(FMOD_SYSTEM *system, int *numdrivers)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getNumDrivers(numdrivers);
}


FMOD_RESULT F_API FMOD_System_GetDriverName(FMOD_SYSTEM *system, int id, char *name, int namelen)
{
    FMOD::System *sys = (FMOD::System *)system;

    FMOD_RESULT result = sys->release();
    if (result != FMOD_OK)
    {
        return result;
    }

    return sys->getDriverName(id, name, namelen);
}


/*
[API]
[
    [DESCRIPTION]
    Returns information on capabilities of the current output mode for the selected sound device.

    [PARAMETERS]
    'id'                      Enumerated driver ID.  This must be in a valid range delimited by System::getNumDrivers.
    'caps'                    Address of a variable that receives the capabilities of the device.  Optional.  Specify 0 or NULL to ignore. 
    'minfrequency'            Address of a variable that receives the minimum frequency allowed with sounds created with FMOD_HARDWARE.  If Channel::setFrequency is used FMOD will clamp the frequency to this minimum.  Optional.  Specify 0 or NULL to ignore. 
    'maxfrequency'            Address of a variable that receives the maximum frequency allowed with sounds created with FMOD_HARDWARE.  If Channel::setFrequency is used FMOD will clamp the frequency to this maximum.  Optional.  Specify 0 or NULL to ignore. 
    'controlpanelspeakermode' Address of a variable that receives the speaker mode set by the operating system control panel.  Use this to pass to System::setSpeakerMode if you want to set up FMOD's software mixing engine to match.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]
    This function cannot be called after FMOD is already activated with System::init.<br>
    It must be called before System::init, or after System::close.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_CAPS
    System::init
    System::close
    System::getNumDrivers
    System::getHardwareChannels
    System::setSpeakerMode
    Channel::setFrequency
]
*/
FMOD_RESULT F_API FMOD_System_GetDriverCaps(FMOD_SYSTEM *system, int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency, FMOD_SPEAKERMODE *controlpanelspeakermode)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getDriverCaps(id, caps, minfrequency, maxfrequency, controlpanelspeakermode);
}


/*
[API]
[
    [DESCRIPTION]
    Selects a soundcard driver.
    This function is used when an output mode has enumerated more than one output device, and you need to select between them.

    [PARAMETERS]
    'driver'      Driver number to select.  0 = primary or main sound device as selected by the operating system settings.  Use System::getNumDrivers to select a specific device.

    [RETURN_VALUE]

    [REMARKS]
    If this function is called after FMOD is already initialized with System::init, the current driver will be shutdown and the newly selected driver will be initialized / started.<br>
    <br>
    When switching output driver after System::init there are a few considerations to make:<br>
    <br>
    All sounds must be created with FMOD_SOFTWARE, creating even one FMOD_HARDWARE sound will cause this function to return FMOD_ERR_NEEDSSOFTWARE.<br>
    <br>
    The driver that you wish to change to must support the current output format, sample rate, and number of channels. If it does not, FMOD_ERR_OUTPUT_INIT is returned and driver state is cleared. You should now call System::setDriver with your original driver index to restore driver state (providing that driver is still available / connected) or make another selection.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getDriver
    System::getNumDrivers
    System::getDriverInfo
    System::setOutput
    System::init
    System::close
]
*/
FMOD_RESULT F_API FMOD_System_SetDriver(FMOD_SYSTEM *system, int driver)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setDriver(driver);
}


/*
[API]
[
    [DESCRIPTION]
    Returns the currently selected driver number.  Drivers are enumerated when selecting a driver with System::setDriver or other driver related functions such as System::getNumDrivers or System::getDriverInfo

    [PARAMETERS]
    'driver'    Address of a variable that receives the currently selected driver ID.  0 = primary or main sound device as selected by the operating system settings.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::setDriver
    System::getNumDrivers 
    System::getDriverInfo
]
*/
FMOD_RESULT F_API FMOD_System_GetDriver(FMOD_SYSTEM *system, int *driver)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getDriver(driver);
}


/*
[API]
[
    [DESCRIPTION]
    This function allows the user to request a minimum number of hardware voices to be present on the soundcard to allow hardware 3D sound acceleration, or clamp the number of hardware 3D voices to a maximum value.

    [PARAMETERS]
    'min2d'       Minimum number of hardware voices on a soundcard required to actually support hardware 2D sound.  If the soundcard does not match this value for number of hardware voices possible, FMOD will place the sound into software mixed buffers instead hardware mixed buffers to guarantee the number of sounds playable at once is guaranteed.
    'max2d'       Maximum number of hardware voices to be used by FMOD.  This clamps the polyphony of hardware 2D voices to a user specified number.   This could be used to limit the number of 2D hardware voices possible at once so that it doesn't sound noisy, or the user might want to limit the number of channels used for 2D hardware support to avoid problems with certain buggy soundcard drivers that report they have many channels but actually don't.
    'min3d'       Minimum number of hardware voices on a soundcard required to actually support hardware 3D sound.  If the soundcard does not match this value for number of hardware voices possible, FMOD will place the sound into software mixed buffers instead hardware mixed buffers to guarantee the number of sounds playable at once is guaranteed.
    'max3d'       Maximum number of hardware voices to be used by FMOD.  This clamps the polyphony of hardware 3D voices to a user specified number.   This could be used to limit the number of 3D hardware voices possible at once so that it doesn't sound noisy, or the user might want to limit the number of channels used for 3D hardware support to avoid problems with certain buggy soundcard drivers that report they have many channels but actually don't.

    [RETURN_VALUE]

    [REMARKS]
    The 'min' value sets the minimum allowable hardware channels before FMOD drops back to 100 percent software based buffers for sounds even if they are allocated with FMOD_HARDWARE.<br>
    This is helpful for minimum spec cards, and not having to 'guess' how many hardware channels they might have.  This way you can guarantee and assume a certain number of channels for your application and always allocate with FMOD_HARDWARE | FMOD_3D without fear of the playsound failing.<br>
    <br>
    <br>
    The 'max' value function has nothing to do with the 'min' value, in that this is not a function that forces FMOD channels into software mode if a card has less than or more than a certain number of channels.<br>
    This parameter only sets a limit on hardware channels playable at once, so if your card has 96 hardware channels, and you set max to 10, then you will only have 10 hardware 3D channels to use.<br>
    The 'buggy soundcard driver' issue in the description for the 'max' parameter is to do with one known sound card driver in particular, the default Windows XP SoundBlaster Live drivers.  They report over 32 possible voices, but actually only support 32, and when you use the extra voices the driver can act unpredictably causing either sound dropouts or a crash.<br>
    <br>
    <br>
    This function cannot be called after FMOD is already activated with System::init.<br>
    It must be called before System::init, or after System::close.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getHardwareChannels
    System::init
    System::close
]
*/
FMOD_RESULT F_API FMOD_System_SetHardwareChannels(FMOD_SYSTEM *system, int min2d, int max2d, int min3d, int max3d)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setHardwareChannels(min2d, max2d, min3d, max3d);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the maximum number of software mixed channels possible.  Software mixed voices are used by sounds loaded with FMOD_SOFTWARE.

    [PARAMETERS]
    'numsoftwarechannels'   The maximum number of FMOD_SOFTWARE mixable voices to be allocated by FMOD.  If you don't require software mixed voices specify 0.  Default = 32.

    [RETURN_VALUE]

    [REMARKS]
    32 voices are allocated by default to be played simultaneously in software.<br>
    To turn off the software mixer completely including hardware resources used for the software mixer, specify FMOD_INIT_SOFTWARE_DISABLE in System::init.
    <br>
    <br>
    This function cannot be called after FMOD is already activated with System::init.<br>
    It must be called before System::init, or after System::close.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_MODE
    FMOD_INITFLAGS
    System::init
    System::close
    System::getSoftwareChannels
]
*/
FMOD_RESULT F_API FMOD_System_SetSoftwareChannels(FMOD_SYSTEM *system, int numsoftwarechannels)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setSoftwareChannels(numsoftwarechannels);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the maximum number of software mixed channels possible.  Software mixed voices are used by sounds loaded with FMOD_SOFTWARE.

    [PARAMETERS]
    'numsoftwarechannels'   Address of a variable that receives the current maximum number of software voices available.  Default = 32.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::setSoftwareChannels
]
*/
FMOD_RESULT F_API FMOD_System_GetSoftwareChannels(FMOD_SYSTEM *system, int *numsoftwarechannels)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getSoftwareChannels(numsoftwarechannels);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the output format for the software mixer.  This includes the bitdepth, sample rate and number of output channels.<br>
    Do not call this unless you explicity want to change something.  Calling this could have adverse impact on the performance and panning behaviour. <br>

    [PARAMETERS]
    'samplerate'        The soundcard's output rate.  default = 48000.
    'format'            The soundcard's output format.  default = FMOD_SOUND_FORMAT_PCM16.
    'numoutputchannels' The number of output channels / speakers to initialize the soundcard to.  0 = keep speakermode setting (set with System::setSpeakerMode).  If anything else than 0 is specified then the speakermode will be overriden and will become FMOD_SPEAKERMODE_RAW, meaning logical speaker assignments (as defined in FMOD_SPEAKER) become innefective and cannot be used.  Channel::setPan will also fail.  Default = 2 (FMOD_SPEAKERMODE_STEREO).  
    'maxinputchannels'  Optional.  Specify 0 to ignore.  Default = 6.  Maximum channel count in loaded/created sounds to be supported.  This is here purely for memory considerations and affects how much memory is used in the software mixer when allocating matrices for panning.  Do not confuse this with recording, or anything to do with how many voices you can play at once.  This is purely for setting the largest type of sound you can play (ie 1 = mono, 2 = stereo, etc.).  Most of the time the user will not play sounds any larger than mono or stereo, so setting this to 2 would save memory and cover most sounds that are playable.
    'resamplemethod'    Software engine resampling method.  default = FMOD_DSP_RESAMPLER_LINEAR.  See FMOD_DSP_RESAMPLER for different types.

    [RETURN_VALUE]

    [REMARKS]
    <b>Note!</b>  The settings in this function <i>may</i> be overriden by the output mode.<br>
    FMOD_OUTPUTTYPE_ASIO will always change the output mode to FMOD_SOUND_FORMAT_PCMFLOAT to be compatible with the output formats selectable by the ASIO control panel.<br>
    FMOD_OUTPUTTYPE_ASIO will also change the samplerate specified by the user to the one selected in the ASIO control panel.<br>
    Use System::getSoftwareFormat after System::init to determine what the output has possibly changed the format to.  Call it after System::init.<br>
    <br>
    It is dependant on the output whether it will force a format change and override these settings or not.<br>
    <br>
    If the output does not support the output mode specified System::init will fail, and you will have to try another setting.<br>
    <br>
    <b>Note!</b>  When this function is called with a output channel count greater than 0, the speaker mode is set to FMOD_SPEAKERMODE_RAW.  FMOD does not know when you specify a number of output channels what type of speaker system it is connected to, so Channel::setPan or Channel::setSpeakerMix will then fail to work.<br>
    Calling System::setSpeakerMode will override the output channel speaker count.<br>
    <br>
    This function cannot be called after FMOD is already activated with System::init.<br>
    It must be called before System::init, or after System::close.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getSoftwareFormat
    System::setSpeakerMode
    System::init
    System::close
    Channel::setPan
    Channel::setSpeakerMix
    FMOD_SPEAKER
    FMOD_SPEAKERMODE
    FMOD_SOUND_FORMAT
    FMOD_DSP_RESAMPLER
]
*/
FMOD_RESULT F_API FMOD_System_SetSoftwareFormat(FMOD_SYSTEM *system, int samplerate, FMOD_SOUND_FORMAT format, int numoutputchannels, int maxinputchannels, FMOD_DSP_RESAMPLER resamplemethod)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setSoftwareFormat(samplerate, format, numoutputchannels, maxinputchannels, resamplemethod);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the output format for the software mixer.

    [PARAMETERS]
    'samplerate'        Address of a variable that receives the mixer's output rate.  Optional.  Specify 0 or NULL to ignore. 
    'format'            Address of a variable that receives the mixer's output format.  Optional.  Specify 0 or NULL to ignore. 
    'numoutputchannels' Address of a variable that receives the number of output channels to initialize the mixer to, for example 1 = mono, 2 = stereo.  8 is the maximum for soundcards that can handle it.  Optional.  Specify 0 or NULL to ignore. 
    'maxinputchannels'  Address of a variable that receives the maximum channel depth on sounds that are loadable or creatable.  Specify 0 or NULL to ignore. 
    'resamplemethod'    Address of a variable that receives the current resampling (frequency conversion) method for software mixed sounds.  Specify 0 or NULL to ignore.
    'bits'              Address of a variable that receives the number of bits per sample.  Useful for byte->sample conversions.  for example FMOD_SOUND_FORMAT_PCM16 is 16.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]
    Note that the settings returned here may differ from the settings provided by the user with System::setSoftwareFormat.  This is because the driver may have changed it because it will not initialize to anything else.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::setSoftwareFormat
    FMOD_SOUND_FORMAT
    FMOD_DSP_RESAMPLER
]
*/
FMOD_RESULT F_API FMOD_System_GetSoftwareFormat(FMOD_SYSTEM *system, int *samplerate, FMOD_SOUND_FORMAT *format, int *numoutputchannels, int *maxinputchannels, FMOD_DSP_RESAMPLER *resamplemethod, int *bits)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getSoftwareFormat(samplerate, format, numoutputchannels, maxinputchannels, resamplemethod, bits);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the FMOD internal mixing buffer size.  This function is used if you need to control mixer latency or granularity.
    Smaller buffersizes lead to smaller latency, but can lead to stuttering/skipping/instable sound on slower machines or soundcards with bad drivers.
    
    [PARAMETERS]
    'bufferlength'  The mixer engine block size in samples.  Use this to adjust mixer update granularity.  Default = 1024.  (milliseconds = 1024 at 48khz = 1024 / 48000 * 1000 = 21.33ms).  This means the mixer updates every 21.33ms.
    'numbuffers'    The mixer engine number of buffers used.  Use this to adjust mixer latency.  Default = 4.  To get the total buffersize multiply the bufferlength by the numbuffers value.  By default this would be 4*1024.

    [RETURN_VALUE]

    [REMARKS]
    The FMOD software mixer mixes to a ringbuffer.  The size of this ringbuffer is determined here.  It mixes a block of sound data every 'bufferlength' number of samples, and there are 'numbuffers' number of these blocks that make up the entire ringbuffer.<br>
    Adjusting these values can lead to extremely low latency performance (smaller values), or greater stability in sound output (larger values).
    <br>
    <br>
    Warning!  The 'buffersize' is generally best left alone.  Making the granularity smaller will just increase CPU usage (cache misses and DSP network overhead).
    Making it larger affects how often you hear commands update such as volume/pitch/pan changes.  Anything above 20ms will be noticable and sound parameter changes will be obvious instead of smooth.
    <br>
    <br>
    FMOD chooses the most optimal size by default for best stability, depending on the output type, and if the drivers are emulated or not (for example DirectSound is emulated using waveOut on NT).
    It is not recommended changing this value unless you really need to.  You may get worse performance than the default settings chosen by FMOD.
    <br>
    <br>
    To convert from milliseconds to 'samples', simply multiply the value in milliseconds by the sample rate of the output (ie 48000 if that is what it is set to), then divide by 1000.
    <br>
    <br>
    The values in milliseconds and average latency expected from the settings can be calculated using the following code.<br>
    <br>
    <PRE>
    FMOD_RESULT result;
    unsigned int blocksize;
    int numblocks;
    float ms;
    <br>
    result = system->getDSPBufferSize(&blocksize, &numblocks);
    result = system->getSoftwareFormat(&frequency, 0, 0, 0, 0);
    <br>
    ms = (float)blocksize * 1000.0f / (float)frequency;
    <br>
    printf("Mixer blocksize        = %.02f ms\n", ms);
    printf("Mixer Total buffersize = %.02f ms\n", ms * numblocks);
    printf("Mixer Average Latency  = %.02f ms\n", ms * ((float)numblocks - 1.5f));
    </PRE>
    <br>
    <b>Platform notes:</b> Some output modes (such as FMOD_OUTPUTTYPE_ASIO) will change the buffer size to match their own internal optimal buffer size.  Use System::getDSPBufferSize after calling System::init to see if this is the case.<br>
    Linux output modes will ignore numbuffers and just write the buffer size to the output every time it can.  It does not use a ringbuffer.<br>
    Xbox 360 defaults to 256 sample buffersize and 4 for numblocks.  This gives a 5.333ms granularity with roughly a 10-15ms latency.<br>
    PS3 ignores this function. Check FMOD_PS3_EXTRADRIVERDATA to control output latency.
    <br>
    This function cannot be called after FMOD is already activated with System::init.<br>
    It must be called before System::init, or after System::close.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getDSPBufferSize
    System::getSoftwareFormat
    System::init
    System::close
]
*/
FMOD_RESULT F_API FMOD_System_SetDSPBufferSize(FMOD_SYSTEM *system, unsigned int bufferlength, int numbuffers)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setDSPBufferSize(bufferlength, numbuffers);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the buffer size settings for the FMOD software mixing engine.

    [PARAMETERS]
    'bufferlength'  Address of a variable that receives the mixer engine block size in samples.  Default = 1024.  (milliseconds = 1024 at 48khz = 1024 / 48000 * 1000 = 10.66ms).  This means the mixer updates every 21.3ms.  Optional.  Specify 0 or NULL to ignore. 
    'numbuffers'    Address of a variable that receives the mixer engine number of buffers used.  Default = 4.  To get the total buffersize multiply the bufferlength by the numbuffers value.  By default this would be 4*1024.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]
    See documentation on System::setDSPBufferSize for more information about these values.    

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::setDSPBufferSize
]
*/
FMOD_RESULT F_API FMOD_System_GetDSPBufferSize(FMOD_SYSTEM *system, unsigned int *bufferlength, int *numbuffers)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getDSPBufferSize(bufferlength, numbuffers);
}


/*
[API]
[
    [DESCRIPTION]
	Specify user callbacks for FMOD's internal file manipulation functions.<br>
	If ANY of the callback functions are set to 0/ NULL, then FMOD will switch back to its own file routines.<br>
	This function is useful for replacing FMOD's file system with a game system's own file reading API.<br>

    [PARAMETERS]
	'useropen'      Callback for opening a file.  Specifying 0 / null will disable file callbacks.
	'userclose'     Callback for closing a file.  Specifying 0 / null will disable file callbacks.
	'userread'      Callback for reading from a file.  Specifying 0 / null will disable file callbacks.
	'userseek'      Callback for seeking within a file.  Specifying 0 / null will disable file callbacks.
    'blockalign'    Internal minimum file block alignment.  FMOD will read data in at least chunks of this size if you ask it to.  Specifying 0 means there is no file buffering at all (this could adversely affect streaming).  Do NOT make this a large value, it is purely a setting for minimum sector size alignment to aid seeking and reading on certain media.  It is not for stream buffer sizes, that is what System::setStreamBufferSize is for.  It is recommened just to pass -1.  Large values just mean large memory usage with no benefit.  Specify -1 to not set this value.  Default = 2048.

    [RETURN_VALUE]

    [REMARKS]
	This has no effect on sounds loaded with FMOD_OPENMEMORY or FMOD_CREATEUSER.<br>
    <br>
    This function can be used to set user file callbacks, or if required, they can be turned off by specifying 0.<br>
    This function can be used purely to set the 'buffersize' parameter, and ignore the callback aspect of the function.<br>
    <br>
	Warning : This function can cause unpredictable behaviour if not used properly.  You must return the right values, and each command must work properly, or FMOD will not function, or it may even crash if you give it invalid data.<br>
    You must also return FMOD_ERR_FILE_EOF from a read callback if the number of bytes read is smaller than the number of bytes requested.<br>
    <br>
    FMOD's default filsystem buffers reads every 2048 bytes by default.  This means every time fmod reads one byte from the API (say if it was parsing a file format), it simply mem copies the byte from the 2k memory buffer, and every time it needs to, refreshes the 2k buffer resulting in a drastic reduction in file I/O.  Large reads go straight to the pointer instead of the 2k buffer if it is buffer aligned.  This value can be increased or decreased by the user.  A buffer of 0 means all reads go directly to the pointer specified.  2048 bytes is the size of a CD sector on most CD ISO formats so it is chosen as the default, for optimal reading speed from CD media.<br>
    <br>
    <b>NOTE!</b>  Do not force a cast from your function pointer to the FMOD_FILE_xxxCALLBACK type!  Never try to 'force' fmod to accept your function.  If there is an error then find out what it is.  Remember to include F_CALLBACK between the return type and the function name, this equates to stdcall which you must include otherwise (besides not compiling) it will cause problems such as crashing and callbacks not being called.
    <br>
    <b>NOTE!</b>  Your file callbacks must be thread safe. If not unexpected behaviour may occur.  FMOD calls file functions from asynchronous threads, such as the streaming thread, and thread related to FMOD_NONBLOCKING flag.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::init
    System::attachFileSystem
    FMOD_FILE_OPENCALLBACK
    FMOD_FILE_CLOSECALLBACK
    FMOD_FILE_READCALLBACK
    FMOD_FILE_SEEKCALLBACK
]
*/
FMOD_RESULT F_API FMOD_System_SetFileSystem(FMOD_SYSTEM *system, FMOD_FILE_OPENCALLBACK useropen, FMOD_FILE_CLOSECALLBACK userclose, FMOD_FILE_READCALLBACK userread, FMOD_FILE_SEEKCALLBACK userseek, int blockalign)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setFileSystem(useropen, userclose, userread, userseek, blockalign);
}


/*
[API]
[
    [DESCRIPTION]
    Function to allow a user to 'piggyback' on FMOD's file reading routines.  This allows users to capture data as FMOD reads it, which may be useful for ripping the raw data that FMOD reads for hard to support sources (for example internet streams or cdda streams).

    [PARAMETERS]
    'useropen'      Pointer to an open callback which is called after a file is opened by FMOD.
    'userclose'     Pointer to a close callback which is called after a file is closed by FMOD.
    'userread'      Pointer to a read callback which is called after a file is read by FMOD.
    'userseek'      Pointer to a seek callback which is called after a file is seeked into by FMOD.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::setFileSystem
    FMOD_FILE_OPENCALLBACK 
    FMOD_FILE_CLOSECALLBACK 
    FMOD_FILE_READCALLBACK
    FMOD_FILE_SEEKCALLBACK 

]
*/
FMOD_RESULT F_API FMOD_System_AttachFileSystem(FMOD_SYSTEM *system, FMOD_FILE_OPENCALLBACK useropen, FMOD_FILE_CLOSECALLBACK userclose, FMOD_FILE_READCALLBACK userread, FMOD_FILE_SEEKCALLBACK userseek)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->attachFileSystem(useropen, userclose, userread, userseek);
}


/*
[API]
[
    [DESCRIPTION]
    Sets advanced features like configuring memory and cpu usage for FMOD_CREATECOMPRESSEDSAMPLE usage.

	[PARAMETERS]
    'settings'      Pointer to FMOD_ADVANCEDSETTINGS structure.
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    FMOD_ADVANCEDSETTINGS
    System::getAdvancedSettings
    FMOD_MODE
]
*/
FMOD_RESULT F_API FMOD_System_SetAdvancedSettings(FMOD_SYSTEM *system, FMOD_ADVANCEDSETTINGS *settings)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setAdvancedSettings(settings);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the advanced settings value set for the system object.

    [PARAMETERS]
    'settings'      Address of a variable to receive the contents of the FMOD_ADVANCEDSETTINGS structure specified by the user.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_ADVANCEDSETTINGS
    System::setAdvancedSettings
]
*/
FMOD_RESULT F_API FMOD_System_GetAdvancedSettings(FMOD_SYSTEM *system, FMOD_ADVANCEDSETTINGS *settings)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getAdvancedSettings(settings);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the speaker mode in the hardware and FMOD software mixing engine.

    [PARAMETERS]
    'speakermode'  Speaker mode specified from the list in FMOD_SPEAKERMODE.

    [RETURN_VALUE]

    [REMARKS]
    Speaker modes that are supported on each platform are as follows.<br>
    <li>Win32/Win64     - All.
    <li>Linux/Linux64   - All.
    <li>Solaris         - All.
    <li>Macintosh       - All.
    <li>iPhone          - FMOD_SPEAKERMODE_RAW, FMOD_SPEAKERMODE_MONO, FMOD_SPEAKERMODE_STEREO, FMOD_SPEAKERMODE_PROLOGIC.
    <li>Wii             - FMOD_SPEAKERMODE_RAW, FMOD_SPEAKERMODE_MONO, FMOD_SPEAKERMODE_STEREO, FMOD_SPEAKERMODE_PROLOGIC.
    <li>PS2             - FMOD_SPEAKERMODE_RAW, FMOD_SPEAKERMODE_MONO, FMOD_SPEAKERMODE_STEREO, FMOD_SPEAKERMODE_PROLOGIC.
    <li>PSP             - FMOD_SPEAKERMODE_RAW, FMOD_SPEAKERMODE_MONO, FMOD_SPEAKERMODE_STEREO, FMOD_SPEAKERMODE_PROLOGIC.
    <li>PS3             - FMOD_SPEAKERMODE_7POINT1 only.  The user cannot change the speaker mode on this platform.
    <li>Xbox 360        - FMOD_SPEAKERMODE_5POINT1 only.  The user cannot change the speaker mode on this platform.
    <br>
    <br>
    <b>NOTE!</b> Calling this function resets any speaker positions set with System::set3DSpeakerPosition, therefore this function must be called before calling System::set3DSpeakerPosition.<br>
    If System::setSoftwareFormat is called after this function with a valid output channel count, the speakermode is set to FMOD_SPEAKERMODE_RAW.<br>
    If this function is called after System::setSoftwareFormat, then it will overwrite the channel count specified in that function.<br>
    The channel count that is overwritten for each speaker mode is as follows:<br>
    <li>FMOD_SPEAKERMODE_RAW        - Channel count is unaffected.
    <li>FMOD_SPEAKERMODE_MONO       - Channel count is set to 1.
    <li>FMOD_SPEAKERMODE_STEREO     - Channel count is set to 2.
    <li>FMOD_SPEAKERMODE_QUAD       - Channel count is set to 4.
    <li>FMOD_SPEAKERMODE_SURROUND   - Channel count is set to 5.
    <li>FMOD_SPEAKERMODE_5POINT1    - Channel count is set to 6.
    <li>FMOD_SPEAKERMODE_7POINT1    - Channel count is set to 8.
    <li>FMOD_SPEAKERMODE_PROLOGIC   - Channel count is set to 2.
    <br>
    <br>
    These channel counts are the channel width of the FMOD DSP system, and affect software mixed sounds (sounds created with FMOD_SOFTWARE flag) only.<br>
    Hardware sounds are not affected, but will still have the speaker mode appropriately set if possible.  (On Windows the speaker mode is set by the user in the control panel, not by FMOD).<br>
    <br>
    <b>NOTE! (ProLogic only)</b> Software 3D sounds will still be panned as though they are in FMOD_SPEAKERMODE_STEREO.<br>
    <br>
    <b>NOTE! (Windows only)</b> Sound will not behave correctly unless your control panel has set the speaker mode to the correct setup. For example if FMOD_SPEAKERMODE_7POINT1 is set on a speaker system that has been set to 'stereo' in the windows control panel, sounds can dissapear and come out of the wrong speaker.  Make sure your users know about this.<br>
    If using WinMM output, note that some soundcard drivers do not support multichannel output correctly (i.e. Creative cards).<br>
    If using WASAPI the speaker mode will be forced to the control panel setting, you can call System::getSpeakerMode after System::init to verify.<br>
    Only DirectSound, WASAPI and ASIO have reliably working multichannel output.<br>
    If the speaker mode is not actually supported (even though the user set the speaker mode to 7.1 in Windows) the soundcard might not be able to handle it. You will get FMOD_ERR_OUTPUT_CREATEBUFFER error.  Change the speaker mode to FMOD_SPEAKERMODE_STEREO and re-initialize if this happens.<br>
    <br>
    To set the speaker mode to that of the windows control panel, use System::getDriverCaps. For example:<br>
    <PRE>
    FMOD_SPEAKERMODE speakermode;
    FMOD_RESULT      result;

    result = system->getDriverCaps(0,0,0,0,&speakermode);   // Get speaker mode for default driver.
    ERRCHECK(result);

    result = system->setSpeakerMode(speakermode);
    ERRCHECK(result);
    </PRE>
    <br>
    This function cannot be called after FMOD is already activated with System::init.<br>
    It must be called before System::init, or after System::close.<br>
    
    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getSpeakerMode
    FMOD_SPEAKERMODE
    System::init
    System::close
    System::setSoftwareFormat
    System::set3DSpeakerPosition
    System::getDriverCaps
    FMOD_RESULT
]
*/
FMOD_RESULT F_API FMOD_System_SetSpeakerMode(FMOD_SYSTEM *system, FMOD_SPEAKERMODE speakermode)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setSpeakerMode(speakermode);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current speaker mode.

    [PARAMETERS]
    'speakermode'   Address of a variable that receives the current speaker mode.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::setSpeakerMode
    FMOD_SPEAKERMODE
]
*/
FMOD_RESULT F_API FMOD_System_GetSpeakerMode(FMOD_SYSTEM *system, FMOD_SPEAKERMODE *speakermode)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getSpeakerMode(speakermode);
}


/*
[API]
[
    [DESCRIPTION]
    Sets a system callback to catch various fatal or informational events.

    [PARAMETERS]
    'callback'      Pointer to a callback to receive the event callback when it happens.

    [RETURN_VALUE]

    [REMARKS]
    System callbacks are not asynchronous and are bound by the latency caused by the rate the user calls the update command.<br>
    <br>
    Callbacks are stdcall.  Use F_CALLBACK inbetween your return type and function name.<br>
    Example:
    <br>
    <PRE>
    FMOD_RESULT F_CALLBACK systemcallback(FMOD_SYSTEM *system, FMOD_SYSTEM_CALLBACKTYPE type, void *commanddata1, void *commanddata2)
    {
    <ul>FMOD::System *sys = (FMOD::System *)system;
        <br>
        switch (type)
        {
        <ul>case FMOD_SYSTEM_CALLBACKTYPE_DEVICELISTCHANGED:
            {
            <ul>int numdrivers;
                <br>
                printf("NOTE : FMOD_SYSTEM_CALLBACKTYPE_DEVICELISTCHANGED occured.\n");
                <br>
                sys->getNumDrivers(&numdrivers);
                <br>
                printf("Numdevices = %d\n", numdrivers);
                break;
            </ul>}
            case FMOD_SYSTEM_CALLBACKTYPE_MEMORYALLOCATIONFAILED:
            {
            <ul>printf("ERROR : FMOD_SYSTEM_CALLBACKTYPE_MEMORYALLOCATIONFAILED occured.\n");
                printf("%s.\n", commanddata1);
                printf("%d bytes.\n", commanddata2);
                break;
            </ul>}
            case FMOD_SYSTEM_CALLBACKTYPE_THREADCREATED:
            {
            <ul>printf("NOTE : FMOD_SYSTEM_CALLBACKTYPE_THREADCREATED occured.\n");
                printf("Thread ID = %d\n", (int)commanddata1);
                printf("Thread Name = %s\n", (char *)commanddata2);
                break;
            </ul>}
            case FMOD_SYSTEM_CALLBACKTYPE_BADDSPCONNECTION:
            {
            <ul>FMOD::DSP *source = (FMOD::DSP *)commanddata1;
                FMOD::DSP *dest = (FMOD::DSP *)commanddata2;
                <br>
                printf("ERROR : FMOD_SYSTEM_CALLBACKTYPE_BADDSPCONNECTION occured.\n");
                if (source)
                {
                <ul>char name[256];
                    source->getInfo(name, 0,0,0,0);
                    printf("SOURCE = %s\n", name);
                </ul>}
                if (dest)
                {
                <ul>char name[256];
                    dest->getInfo(name, 0,0,0,0);
                    printf("DEST = %s\n", name);
                </ul>}
                break;
            </ul>}
        </ul>}
        <br>
        return FMOD_OK;    
    </ul>}
    </PRE>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::update
    FMOD_SYSTEM_CALLBACK
    FMOD_SYSTEM_CALLBACKTYPE
]
*/
FMOD_RESULT F_API FMOD_System_SetCallback(FMOD_SYSTEM *system, FMOD_SYSTEM_CALLBACKTYPE type, FMOD_SYSTEM_CALLBACK callback)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setCallback(type, callback);
}


/*
[API]
[
    [DESCRIPTION]
    Specify a base search path for plugins so they can be placed somewhere else than the directory of the main executable.

    [PARAMETERS]
    'path'      A character string containing a correctly formatted path to load plugins from.

    [RETURN_VALUE]

    [REMARKS]
    The 'plugin' version of FMOD relies on plugins, so when System::init is called it tries to load all FMOD registered plugins.<br>
    This path is where it will attempt to load from.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::loadPlugin
    System::init
]
*/
FMOD_RESULT F_API FMOD_System_SetPluginPath(FMOD_SYSTEM *system, const char *path)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setPluginPath(path);
}


/*
[API]
[
    [DESCRIPTION]
    Loads an FMOD plugin.  This could be a DSP, file format or output plugin.

    [PARAMETERS]
    'filename'      Filename of the plugin to be loaded.
    'handle'        Pointer to an unsigned int to receive the plugin handle, for later use.
    'priority'      (FMOD_PLUGINTYPE_CODEC only) Priority of the codec compared to other codecs.  0 = most important.  higher numbers = less importance.

    [RETURN_VALUE]

    [REMARKS]
    Once the plugin is loaded, it can be enumerated and used.<br>
    For file format plugins, FMOD will automatically try to use them when System::createSound is used.<br>
    For DSP plugins, you can enumerate them with System::getNumPlugins, System::getPluginHandle and System::getPluginInfo.<br>
    Plugins can be created for FMOD by the user.  See the relevant section in the documentation on creating plugins.<br>
    The format of the plugin is dependant on the operating system.<br>
    On Win32 and Win64 the .dll format is used<br>
    On Linux, the .so format is used.<br>
    On Macintosh, the .shlib format is used<br>
    <br>
    The codecs internal to FMOD have the following priorities.<br>
    <br>
    <PRE>
    Tag         100
    CDDA        200
    FSB         300
    DSP         400
    VAG         500
    Wav         600 
    AT3         700
    OggVorbis   800
    Tremor      900
    AIFF        1000
    FLAC        1100
    MOD         1200
    S3M         1300
    XM          1400
    IT          1500
    MIDI        1600
    DLS         1700
    SF2         1800
    ASF         1900
    XMA         2000
    XWMA        2100
    Playlist    2200
    MPEGPSP     2300
    MPEG        2400
    Raw         2500
    </PRE>
    <b>Note:</b> Some codecs are only for certain platforms, ie XMA is xbox 360 only.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Solaris

    [SEE_ALSO]
    System::setPluginPath
    System::unloadPlugin
    System::getNumPlugins
    System::getPluginHandle
    System::getPluginInfo
    System::setOutputByPlugin
    System::getOutputByPlugin
    System::createDSPByPlugin
    System::createSound
]
*/
FMOD_RESULT F_API FMOD_System_LoadPlugin(FMOD_SYSTEM *system, const char *filename, FMOD_PLUGINTYPE *plugintype, int *index)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->loadPlugin(filename, plugintype, index);
}


/*
[API]
[
    [DESCRIPTION]
    Unloads a plugin from memory.

    [PARAMETERS]
    'handle'        Handle to a pre-existing plugin.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::loadPlugin
]
*/
FMOD_RESULT F_API FMOD_System_UnloadPlugin(FMOD_SYSTEM *system, FMOD_PLUGINTYPE plugintype, int index)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->unloadPlugin(plugintype, index);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of available plugins loaded into FMOD at the current time.

    [PARAMETERS]
    'plugintype'    The type of plugin type such as FMOD_PLUGINTYPE_OUTPUT, FMOD_PLUGINTYPE_CODEC or FMOD_PLUGINTYPE_DSP.
    'numplugins'    Address of a variable that receives the number of available plugins for the selected type.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_PLUGINTYPE
    System::getPluginHandle
]
*/
FMOD_RESULT F_API FMOD_System_GetNumPlugins(FMOD_SYSTEM *system, FMOD_PLUGINTYPE plugintype, int *numplugins)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getNumPlugins(plugintype, numplugins);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves information to display for the selected plugin.

    [PARAMETERS]
    'handle'        Handle to a pre-existing plugin.
    'plugintype'    Address of a variable that receives the type of the plugin, FMOD_PLUGINTYPE_OUTPUT, FMOD_PLUGINTYPE_CODEC or FMOD_PLUGINTYPE_DSP.
    'name'          Address of a variable that receives the name of the plugin.
    'namelen'       Length in bytes of the target buffer to receieve the string.
    'version'       Version number set by the plugin.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getNumPlugins
]
*/
FMOD_RESULT F_API FMOD_System_GetPluginInfo(FMOD_SYSTEM *system, FMOD_PLUGINTYPE plugintype, int index, char *name, int namelen, unsigned int *version)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getPluginInfo(plugintype, index, name, namelen, version);
}


/*
[API]
[
    [DESCRIPTION]
    Selects an output type based on the enumerated list of outputs including FMOD and 3rd party output plugins.

    [PARAMETERS]
    'handle'        Handle to a pre-existing output plugin.

    [RETURN_VALUE]

    [REMARKS]
    This function cannot be called after FMOD is already activated with System::init.<br>
    It must be called before System::init.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getNumPlugins
    System::getOutputByPlugin
    System::setOutput
    System::init
    System::close
]
*/
FMOD_RESULT F_API FMOD_System_SetOutputByPlugin(FMOD_SYSTEM *system, int index)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setOutputByPlugin(index);
}


/*
[API]
[
    [DESCRIPTION]
    Returns the currently selected output as an id in the list of output plugins.

    [PARAMETERS]
    'handle'        Handle to a pre-existing output plugin.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getNumPlugins
    System::setOutputByPlugin
    System::setOutput
]
*/
FMOD_RESULT F_API FMOD_System_GetOutputByPlugin(FMOD_SYSTEM *system, int *index)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getOutputByPlugin(index);
}


/*
[API]
[
    [DESCRIPTION]
    Creates an in memory file format codec to be used by FMOD by passing in a codec description structure.<br>
    Once this is created, FMOD will use it to open user defined file formats.

    [PARAMETERS]
    'description'   Address of a FMOD_CODEC_DESCRIPTION structure, containing information about the codec.
    'priority'      Priority of the codec compared to other codecs.  0 = most important.  higher numbers = less importance.

    [RETURN_VALUE]

    [REMARKS]
    The codecs internal to FMOD have the following priorities.<br>
    <br>
    <PRE>
    Tag         100
    CDDA        200
    FSB         300
    DSP         400
    VAG         500
    Wav         600 
    AT3         700
    OggVorbis   800
    Tremor      900
    AIFF        1000
    FLAC        1100
    MOD         1200
    S3M         1300
    XM          1400
    IT          1500
    MIDI        1600
    DLS         1700
    SF2         1800
    ASF         1900
    XMA         2000
    XWMA        2100
    Playlist    2200
    MPEGPSP     2300
    MPEG        2400
    Raw         2500
    </PRE>
    <b>Note:</b> Some codecs are only for certain platforms, ie XMA is xbox 360 only.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_CODEC_DESCRIPTION
]
*/
FMOD_RESULT F_API FMOD_System_CreateCodec(FMOD_SYSTEM *system, FMOD_CODEC_DESCRIPTION *description)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->createCodec(description);
}


/*
[API]
[
    [DESCRIPTION]
    Initializes the system object, and the sound device.  This has to be called at the start of the user's program.<br>
    <b>You must create a system object with FMOD::System_create.</b>

    [PARAMETERS]
    'maxchannels'           The maximum number of channels to be used in FMOD.  They are also called 'virtual channels' as you can play as many of these as you want, even if you only have a small number of hardware or software voices.  See remarks for more.
    'flags'                 See FMOD_INITFLAGS.  This can be a selection of flags bitwise OR'ed together to change the behaviour of FMOD at initialization time.
    'extradriverdata'       Driver specific data that can be passed to the output plugin.  For example the filename for the wav writer plugin.  See FMOD_OUTPUTTYPE for what each output mode might take here.  Optional.  Specify 0 or NULL to ignore.

    [RETURN_VALUE]

    [REMARKS]
    <u>Virtual channels.</u><br>
    These types of voices are the ones you work with using the FMOD::Channel API.  <br>
    The advantage of virtual channels are, unlike older versions of FMOD, you can now play as many sounds as you like without fear of ever running out of voices, or playsound failing.<br>
    You can also avoid 'channel stealing' if you specify enough virtual voices.<br>
    <br>
    As an example, you can play 1000 sounds at once, even on a 32 channel soundcard.<br>
    FMOD will only play the most important/closest/loudest (determined by volume/distance/geometry and priority settings) voices, and the other 968 voices will be virtualized without expense to the CPU.  The voice's cursor positions are updated.<br>
    When the priority of sounds change or emulated sounds get louder than audible ones, they will swap the actual voice resource over (ie hardware or software buffer) and play the voice from its correct position in time as it should be heard.<br>
    What this means is you can play all 1000 sounds, if they are scattered around the game world, and as you move around the world you will hear the closest or most important 32, and they will automatically swap in and out as you move.<br>
    <br>
    Currently the maximum channel limit is 4093.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_INITFLAGS
    System::close
    System_Create
    FMOD_OUTPUTTYPE
]
*/
FMOD_RESULT F_API FMOD_System_Init(FMOD_SYSTEM *system, int maxchannels, FMOD_INITFLAGS flags, void *extradriverdata)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->init(maxchannels, flags, extradriverdata);
}


/*
[API]
[
    [DESCRIPTION]
    Closes the system object without freeing the object's memory, so the system handle will still be valid.<br>
    Closing the output renders objects created with this system object invalid.  Make sure any sounds, channelgroups, geometry and dsp objects are released before closing the system object.<br>

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::init
    System::release
]
*/
FMOD_RESULT F_API FMOD_System_Close(FMOD_SYSTEM *system)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->close();
}


/*
[API]
[
    [DESCRIPTION]
    Updates the FMOD system.  This should be called once per 'game' tick, or once per frame in your application.    

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]
    This updates the following things.<br>
    <li> 3D Sound.  System::update must be called to get 3D positioning.
    <li> Virtual voices.  If more voices are played than there are real hardware/software voices, System::update must be called to handle the virtualization.
    <li> *_NRT output modes.  System::update must be called to drive the output for these output modes.
    <li> FMOD_INIT_STREAM_FROM_UPDATE.  System::update must be called to update the streamer if this flag has been used.
    <li> Callbacks.  System::update must be called to fire callbacks if they are specified.
    <li> FMOD_NONBLOCKING.  System::update must be called to make sounds opened with FMOD_NONBLOCKING flag to work properly.
    <li> FMOD_SYSTEM_CALLBACKTYPE_DEVICELISTCHANGED callback.  System::update must be called for this callback to trigger.
    <br>
    If FMOD_OUTPUTTYPE_NOSOUND_NRT or FMOD_OUTPUTTYPE_WAVWRITER_NRT output modes are used, this function also drives the software / DSP engine, instead of it running asynchronously in a thread as is the default behaviour.<br>
    This can be used for faster than realtime updates to the decoding or DSP engine which might be useful if the output is the wav writer for example.<br>
    <br>
    If FMOD_INIT_STREAM_FROM_UPDATE is used, this function will update the stream engine.  Combining this with the non realtime output will mean smoother captured output.<br>
    <br>
    <b>Warning!</b>  Do not be tempted to call this function from a different thread to other FMOD commands!  This is dangerous and will cause corruption/crashes.  This function is not thread safe, and should be called from the same thread as the rest of the FMOD commands.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::init
    FMOD_INITFLAGS
    FMOD_OUTPUTTYPE
    FMOD_MODE
]
*/
FMOD_RESULT F_API FMOD_System_Update(FMOD_SYSTEM *system)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->update();
}


/*
[API]
[
    [DESCRIPTION]
    Sets the global doppler scale, distance factor and log rolloff scale for all 3D sound in FMOD.

    [PARAMETERS]
    'dopplerscale'      Scaling factor for doppler shift.  Default = 1.0.
    'distancefactor'    Relative distance factor to FMOD's units.  Default = 1.0. (1.0 = 1 metre).
    'rolloffscale'      Scaling factor for 3D sound rolloff or attenuation for FMOD_3D_LOGROLLOFF based sounds only (which is the default type).  Default = 1.0.

    [RETURN_VALUE]

    [REMARKS]
    The <b>doppler scale</b> is a general scaling factor for how much the pitch varies due to doppler shifting in 3D sound. Doppler is the pitch 
    bending effect when a sound comes towards the listener or moves away from it, much like the effect you hear when a train goes past you with 
    its horn sounding. With "dopplerscale" you can exaggerate or diminish the effect. FMOD's effective speed of sound at a doppler factor of 1.0 is 340 m/s.
    <p>
	The <b>distance factor</b> is the FMOD 3D engine relative distance factor, compared to 1.0 meters. Another way to put it is that it equates to "how many units per meter does your engine have".
	For example, if you are using feet then "scale" would equal 3.28.
    <p>
    <b>Note!</b> This only affects doppler! If you keep your min/max distance, custom rolloff curves and positions in scale relative to each other the volume rolloff will not change.
    If you set this, the mindistance of a sound will automatically set itself to this value when it is created in case the user forgets to set the mindistance to match the new distancefactor.
    <p>
	The <b>rolloff scale</b> sets the global attenuation rolloff factor for FMOD_3D_LOGROLLOFF based sounds only (which is the default). Normally volume for a 
    sound will scale at mindistance / distance. This gives a logarithmic attenuation of volume as the source gets further away (or closer).
	Setting this value makes the sound drop off faster or slower.  The higher the value, the faster volume will attenuate, and conversely the lower 
    the value, the slower it will attenuate. For example a rolloff factor of 1 will simulate the real world, where as a value of 2 will make sounds attenuate 2 times quicker.
    <p>
    <b>Note!</b> "rolloffscale" has no effect when using FMOD_3D_LINEARROLLOFF or FMOD_3D_CUSTOMROLLOFF.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::get3DSettings
    Sound::set3DMinMaxDistance
    Sound::get3DMinMaxDistance
    Channel::set3DAttributes
    Channel::get3DAttributes
]
*/
FMOD_RESULT F_API FMOD_System_Set3DSettings(FMOD_SYSTEM *system, float dopplerscale, float distancefactor, float rolloffscale)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->set3DSettings(dopplerscale, distancefactor, rolloffscale);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the global doppler scale, distance factor and rolloff scale for all 3D sound in FMOD.

    [PARAMETERS]
    'dopplerscale'      Address of a variable that receives the scaling factor for doppler shift.  Optional.  Specify 0 or NULL to ignore. 
    'distancefactor'    Address of a variable that receives the relative distance factor to FMOD's units.  Optional.  Specify 0 or NULL to ignore. 
    'rolloffscale'      Address of a variable that receives the scaling factor for 3D sound rolloff or attenuation.   Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::set3DSettings
]
*/
FMOD_RESULT F_API FMOD_System_Get3DSettings(FMOD_SYSTEM *system, float *dopplerscale, float *distancefactor, float *rolloffscale)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->get3DSettings(dopplerscale, distancefactor, rolloffscale);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the number of 3D 'listeners' in the 3D sound scene.  This function is useful mainly for split-screen game purposes.

    [PARAMETERS]
    'numlisteners'      Number of listeners in the scene.  Valid values are from 1-4 inclusive.  Default = 1.

    [RETURN_VALUE]

    [REMARKS]
    If the number of listeners is set to more than 1, then panning and doppler are turned off.  All sound effects will be mono.<br>
    FMOD uses a 'closest sound to the listener' method to determine what should be heard in this case.<br>
    
    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::get3DNumListeners
    System::set3DListenerAttributes
]
*/
FMOD_RESULT F_API FMOD_System_Set3DNumListeners(FMOD_SYSTEM *system, int numlisteners)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->set3DNumListeners(numlisteners);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of 3D listeners.

    [PARAMETERS]
    'numlisteners'  Address of a variable that receives the current number of 3D listeners in the 3D scene.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::set3DNumListeners
]
*/
FMOD_RESULT F_API FMOD_System_Get3DNumListeners(FMOD_SYSTEM *system, int *numlisteners)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->get3DNumListeners(numlisteners);
}


/*
[API]
[
    [DESCRIPTION]
	This updates the position, velocity and orientation of the specified 3D sound listener.

    [PARAMETERS]
    'listener'  Listener ID in a multi-listener environment.  Specify 0 if there is only 1 listener.
	'pos'       The position of the listener in world space, measured in distance units.  You can specify 0 or NULL to not update the position.
	'vel'       The velocity of the listener measured in distance units <b>per second</b>.  You can specify 0 or NULL to not update the velocity of the listener.
	'forward'   The forwards orientation of the listener.  This vector must be of unit length and perpendicular to the up vector.    You can specify 0 or NULL to not update the forwards orientation of the listener.
	'up'        The upwards orientation of the listener.  This vector must be of unit length and perpendicular to the forwards vector.    You can specify 0 or NULL to not update the upwards orientation of the listener.

    [RETURN_VALUE]

    [REMARKS]
	By default, FMOD uses a left-handed co-ordinate system.  This means +X is right, +Y is up, and +Z is forwards.<br>
    To change this to a right-handed coordinate system, use FMOD_INIT_3D_RIGHTHANDED. This means +X is left, +Y is up, and +Z is forwards.<br>
    <br>
	To map to another coordinate system, flip/negate and exchange these values.<br>
    <br>
	Orientation vectors are expected to be of UNIT length. This means the magnitude of the vector should be 1.0.<br>
    <br>
	A 'distance unit' is specified by System::set3DSettings.  By default this is set to meters which is a distance scale of 1.0. <br>
    <br>
	Always remember to use <b>units per second</b>, <i>not</i> units per frame as this is a common mistake and will make the doppler effect sound wrong.<br>
	For example, Do not just use (pos - lastpos) from the last frame's data for velocity, as this is not correct.  You need to time compensate it so it is given in units per <b>second</b>.<br>
	You could alter your pos - lastpos calculation to something like this. <br>
    <PRE>
	vel = (pos-lastpos) / time_taken_since_last_frame_in_seconds.
    </PRE>
    I.e. at 60fps the formula would look like this vel = (pos-lastpos) / 0.0166667.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::get3DListenerAttributes
    FMOD_INITFLAGS
    System::set3DSettings
    System::get3DSettings
    FMOD_VECTOR
]
*/
FMOD_RESULT F_API FMOD_System_Set3DListenerAttributes(FMOD_SYSTEM *system, int listener, const FMOD_VECTOR *pos, const FMOD_VECTOR *vel, const FMOD_VECTOR *forward, const FMOD_VECTOR *up)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->set3DListenerAttributes(listener, pos, vel, forward, up);
}


/*
[API]
[
    [DESCRIPTION]
	This retrieves the position, velocity and orientation of the specified 3D sound listener.

    [PARAMETERS]
    'listener'  Listener ID in a multi-listener environment.  Specify 0 if there is only 1 listener.
	'pos'       Address of a variable that receives the position of the listener in world space, measured in distance units.    Optional.  Specify 0 or NULL to ignore. 
	'vel'       Address of a variable that receives the velocity of the listener measured in distance units <b>per second</b>.    Optional.  Specify 0 or NULL to ignore. 
	'forward'   Address of a variable that receives the forwards orientation of the listener.    Optional.  Specify 0 or NULL to ignore. 
	'up'        Address of a variable that receives the upwards orientation of the listener.    Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::set3DListenerAttributes
    FMOD_VECTOR
]
*/
FMOD_RESULT F_API FMOD_System_Get3DListenerAttributes(FMOD_SYSTEM *system, int listener, FMOD_VECTOR *pos, FMOD_VECTOR *vel, FMOD_VECTOR *forward, FMOD_VECTOR *up)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->get3DListenerAttributes(listener, pos, vel, forward, up);
}


/*
[API]
[
    [DESCRIPTION]
    When FMOD wants to calculate 3d volume for a channel, this callback can be used to override the internal volume calculation  based on distance.  

    [PARAMETERS]
    'callback'      Pointer to a C function of type FMOD_3D_ROLLOFFCALLBACK, that is used to override the FMOD volume calculation.  Default is 0 or NULL.  Setting the callback to null will return 3d calculation back to FMOD.

    [RETURN_VALUE]

    [REMARKS]
    This function overrides FMOD_3D_LOGROLLOFF, FMOD_3D_LINEARROLLOFF, FMOD_3D_CUSTOMROLLOFF.  To allow FMOD to calculate the 3d volume again, use 0 or NULL as the callback.<p>
    NOTE: When using the event system, call Channel::getUserData from your FMOD_3D_ROLLOFFCALLBACK to get the event instance handle of the event that spawned the channel in question.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_3D_ROLLOFFCALLBACK
    System::set3DListenerAttributes
    System::get3DListenerAttributes
    Channel::getUserData
]
*/
FMOD_RESULT F_API FMOD_System_Set3DRolloffCallback(FMOD_SYSTEM *system, FMOD_3D_ROLLOFFCALLBACK callback)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->set3DRolloffCallback(callback);
}


/*
[API]
[
    [DESCRIPTION]
    This function allows the user to specify the position of their actual physical speaker to account for non standard setups.<br>
    It also allows the user to disable speakers from 3D consideration in a game.<br>
    The funtion is for describing the 'real world' speaker placement to provide a more natural panning solution for 3d sound.  Graphical configuration screens in an application could draw icons for speaker placement that the user could position at their will.

    [PARAMETERS]
    'speaker'   The selected speaker of interest to position.
    'x'         The 2D X offset in relation to the listening position.  For example -1.0 would mean the speaker is on the left, and +1.0 would mean the speaker is on the right.  0.0 is the speaker is in the middle.
    'y'         The 2D Y offset in relation to the listening position.  For example -1.0 would mean the speaker is behind the listener, and +1 would mean the speaker is in front of the listener.
    'active'    Enables or disables speaker from 3D consideration.  Useful for disabling center speaker for vocals for example, or the LFE.  x and y can be anything in this case.

    [RETURN_VALUE]

    [REMARKS]
    <b>Note!</b>  This only affects software mixed 3d sounds, created with FMOD_SOFTWARE and FMOD_3D.<br>
    <br>
    A typical 7.1 setup would look like this.<br>
    <PRE>
    system->set3DSpeakerPosition(FMOD_SPEAKER_FRONT_LEFT,    -1.0f,  1.0f, true);
    system->set3DSpeakerPosition(FMOD_SPEAKER_FRONT_RIGHT,    1.0f,  1.0f, true);
    system->set3DSpeakerPosition(FMOD_SPEAKER_FRONT_CENTER,   0.0f,  1.0f, true);
    system->set3DSpeakerPosition(FMOD_SPEAKER_LOW_FREQUENCY,  0.0f,  0.0f, true);
    system->set3DSpeakerPosition(FMOD_SPEAKER_BACK_LEFT,     -1.0f, -1.0f, true);
    system->set3DSpeakerPosition(FMOD_SPEAKER_BACK_RIGHT,     1.0f, -1.0f, true);
    system->set3DSpeakerPosition(FMOD_SPEAKER_SIDE_LEFT,     -1.0f,  0.0f, true);
    system->set3DSpeakerPosition(FMOD_SPEAKER_SIDE_RIGHT,     1.0f,  0.0f, true);
    </PRE>
    A typical stereo setup would look like this.<br>
    <PRE>
    system->set3DSpeakerPosition(FMOD_SPEAKER_FRONT_LEFT,    -1.0f,  0.0f, true);
    system->set3DSpeakerPosition(FMOD_SPEAKER_FRONT_RIGHT,    1.0f,  0.0f, true);
    </PRE>
    You could use this function to make sounds in front of your come out of different physical speakers.  If you specified for example that FMOD_SPEAKER_SIDE_RIGHT was in front of you at <0.0, 1.0> and you organized the other speakers accordingly the 3d audio would come out of the side right speaker when it was in front instead of the default which is only to the side.<br>
    This function is also useful if speakers are not 'perfectly symmetrical'.  For example if the center speaker was closer to the front left than the front right, this function could be used to position that center speaker accordingly and FMOD would skew the panning appropriately to make it sound correct again.<br>
    <br>
    The 2d coordinates used are only used to generate angle information.  Size / distance does not matter in FMOD's implementation because it is not FMOD's job to attenuate or amplify the signal based on speaker distance.  If it amplified the signal in the digital domain the audio could clip/become distorted.  It is better to use the amplifier's analogue level capabilities to balance speaker volumes.
    <br>
    Calling System::setSpeakerMode overrides these values, so this function must be called after this.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::get3DSpeakerPosition
    System::setSpeakerMode
    FMOD_SPEAKERMODE
    FMOD_SPEAKER

]
*/
FMOD_RESULT F_API FMOD_System_Set3DSpeakerPosition(FMOD_SYSTEM *system, FMOD_SPEAKER speaker, float x, float y, FMOD_BOOL active)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->set3DSpeakerPosition(speaker, x, y, active ? true : false);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current speaker position information for the selected speaker.

    [PARAMETERS]
    'speaker'   The selected speaker of interest to return the x and y position.
    'x'         Address of a variable that receives the 2D X position relative to the listener.  Optional.  Specify 0 or NULL to ignore. 
    'y'         Address of a variable that receives the 2D Y position relative to the listener.  Optional.  Specify 0 or NULL to ignore. 
    'active'    Address of a variable that receives the active state of a speaker.

    [RETURN_VALUE]

    [REMARKS]
    See the System::set3DSpeakerPosition for more information on speaker positioning.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::set3DSpeakerPosition
    FMOD_SPEAKERMODE
    FMOD_SPEAKER
]
*/
FMOD_RESULT F_API FMOD_System_Get3DSpeakerPosition(FMOD_SYSTEM *system, FMOD_SPEAKER speaker, float *x, float *y, FMOD_BOOL *active)
{
    FMOD_RESULT result;
    bool active2;
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = _system->get3DSpeakerPosition(speaker, x, y, &active2);
    if (result == FMOD_OK)
    {
        if (active)
        {
            *active = active2 ? 1 : 0;
        }
    }

    return result;
}


/*
[API]
[
    [DESCRIPTION]
    Sets the internal buffersize for streams opened after this call.<br>
    Larger values will consume more memory (see remarks), whereas smaller values may cause buffer under-run/starvation/stuttering caused by large delays in disk access (ie CDROM or netstream), or cpu usage in slow machines, or by trying to play too many streams at once.<br>

    [PARAMETERS]
    'filebuffersize'        Size of stream file buffer.   Default is 16384 (FMOD_TIMEUNIT_RAWBYTES).
    'filebuffersizetype'    Type of unit for stream file buffer size.  Must be FMOD_TIMEUNIT_MS, FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_PCMBYTES or FMOD_TIMEUNIT_RAWBYTES.  Default is FMOD_TIMEUNIT_RAWBYTES.

    [RETURN_VALUE]

    [REMARKS]
    Note this function does not affect streams created with FMOD_OPENUSER, as the buffer size is specified in System::createSound.<br>
    This function does not affect latency of playback.  All streams are pre-buffered (unless opened with FMOD_OPENONLY), so they will always start immediately.<br>
    Seek and Play operations can sometimes cause a reflush of this buffer.<br>
    <br>
    If FMOD_TIMEUNIT_RAWBYTES is used, the memory allocated is 2 * the size passed in, because fmod allocates a double buffer.<br>
    If FMOD_TIMEUNIT_MS, FMOD_TIMEUNIT_PCM or FMOD_TIMEUNIT_PCMBYTES is used, and the stream is infinite (such as a shoutcast netstream), or VBR, then FMOD cannot calculate an accurate compression ratio to work with when the file is opened.  This means it will then base the buffersize on FMOD_TIMEUNIT_PCMBYTES, or in other words the number of PCM bytes, but this will be incorrect for some compressed formats.<br>
    Use FMOD_TIMEUNIT_RAWBYTES for these type (infinite / undetermined length) of streams for more accurate read sizes.<br>
    <br>
    Note to determine the actual memory usage of a stream, including sound buffer and other overhead, use Memory_GetStats before and after creating a sound.<br>
    <br>
    Note that the stream may still stutter if the codec uses a large amount of cpu time, which impacts the smaller, internal 'decode' buffer.<br>
    The decode buffer size is changeable via FMOD_CREATESOUNDEXINFO.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_TIMEUNIT
    System::createSound
    System::getStreamBufferSize
    Sound::getOpenState
    Channel::setMute
    Memory_GetStats
    FMOD_CREATESOUNDEXINFO
]
*/
FMOD_RESULT F_API FMOD_System_SetStreamBufferSize(FMOD_SYSTEM *system, unsigned int filebuffersize, FMOD_TIMEUNIT filebuffersizetype)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setStreamBufferSize(filebuffersize, filebuffersizetype);
}


/*
[API]
[
    [DESCRIPTION]
    Returns the current internal buffersize settings for streamable sounds.

    [PARAMETERS]
    'filebuffersize'        Address of a variable that receives the current stream file buffer size setting.   Default is 16384 (FMOD_TIMEUNIT_RAWBYTES).  Optional.  Specify 0 or NULL to ignore. 
    'filebuffersizetype'    Address of a variable that receives the type of unit for the current stream file buffer size setting.  Can be FMOD_TIMEUNIT_MS, FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_PCMBYTES or FMOD_TIMEUNIT_RAWBYTES.  Default is FMOD_TIMEUNIT_RAWBYTES.  Optional.  Specify 0 or NULL to ignore. 
    
    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_TIMEUNIT
    System::setStreamBufferSize
]
*/
FMOD_RESULT F_API FMOD_System_GetStreamBufferSize(FMOD_SYSTEM *system, unsigned int *filebuffersize, FMOD_TIMEUNIT *filebuffersizetype)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getStreamBufferSize(filebuffersize, filebuffersizetype);
}


/*
[API]
[
    [DESCRIPTION]
    Returns the current version of FMOD Ex being used.

    [PARAMETERS]
    'version'   Address of a variable that receives the current FMOD Ex version.

    [RETURN_VALUE]

    [REMARKS]
    The version is a 32bit hexadecimal value formated as 16:8:8, with the upper 16bits being the major version, the middle 8bits being the minor version and the bottom 8bits being the development version.  For example a value of 00040106h is equal to 4.01.06.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::init
]
*/
FMOD_RESULT F_API FMOD_System_GetVersion(FMOD_SYSTEM *system, unsigned int *version)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getVersion(version);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a pointer to the system level output device module.   This means a pointer to a DirectX "LPDIRECTSOUND", or a WINMM handle, or with something like with FMOD_OUTPUTTYPE_NOSOUND output, the handle will be null or 0.

    [PARAMETERS]
    'handle'        Address of a variable that receives the handle to the output mode's native hardware API object (see remarks).

    [RETURN_VALUE]

    [REMARKS]
    Must be called after System::init.<br>
    Cast the resulting pointer depending on what output system pointer you are after.<br>
    <br>
    <b>FMOD_OUTPUTTYPE_DSOUND</b>       Pointer to type DIRECTSOUND is returned.<br>
    <b>FMOD_OUTPUTTYPE_WINMM</b>        Pointer to type HWAVEOUT is returned.<br>
    <b>FMOD_OUTPUTTYPE_OPENAL</b>       Pointer to type ALCdevice is returned.<br>
    <b>FMOD_OUTPUTTYPE_WASAPI</b>       Pointer to type IAudioRenderClient is returned.<br>
    <b>FMOD_OUTPUTTYPE_ASIO</b>         Pointer to type AsioDrivers is returned.<br>
    <b>FMOD_OUTPUTTYPE_OSS</b>          File handle is returned, (cast to int).<br>
    <b>FMOD_OUTPUTTYPE_ALSA</b>         Pointer to type snd_pcm_t is returned.<br>
    <b>FMOD_OUTPUTTYPE_ESD</b>          Handle of type int is returned, as returned by so_esd_open_sound (cast to int).  <br>
    <b>FMOD_OUTPUTTYPE_COREAUDIO</b>    Handle of type AudioUnit is returned.<br>
    <b>FMOD_OUTPUTTYPE_PS2</b>          NULL / 0 is returned.<br>
    <b>FMOD_OUTPUTTYPE_PS3</b>          NULL / 0 is returned.<br>
    <b>FMOD_OUTPUTTYPE_XBOX360</b>      Pointer to type IXAudio2 is returned.<br>    
    <b>FMOD_OUTPUTTYPE_PSP</b>          NULL / 0 is returned.<br>
    <b>FMOD_OUTPUTTYPE_WII</b>          NULL / 0 is returned.<br>
    <b>FMOD_OUTPUTTYPE_NOSOUND</b>      NULL / 0 is returned.<br>
    <b>FMOD_OUTPUTTYPE_WAVWRITER</b>    NULL / 0 is returned.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_OUTPUTTYPE
    System::setOutput
    System::init
]
*/
FMOD_RESULT F_API FMOD_System_GetOutputHandle(FMOD_SYSTEM *system, void **handle)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getOutputHandle(handle);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of currently playing channels.

    [PARAMETERS]
    'channels'  Address of a variable that receives the number of currently playing channels.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
]
*/
FMOD_RESULT F_API FMOD_System_GetChannelsPlaying(FMOD_SYSTEM *system, int *channels)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getChannelsPlaying(channels);
}


/*
[API]
[
    [DESCRIPTION]
    Returns the number of available hardware mixed 2d and 3d channels.

    [PARAMETERS]
    'num2d'     Address of a variable that receives the number of available hardware mixed 2d channels.  Optional.  Specify 0 or NULL to ignore. 
    'num3d'     Address of a variable that receives the number of available hardware mixed 3d channels.  Optional.  Specify 0 or NULL to ignore. 
    'total'     Address of a variable that receives the total number of available hardware mixed channels.  Usually total = num2d + num3d, but on some platforms like PS2 and GameCube, 2D and 3D voices share the same channel pool so total, num2d and num3d will all be the same number.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]
    If total doesn't equal num2d + num3d, it usually means the 2d and 3d hardware voices share the same actual hardware voice pool.<br>
    For example if it said 32 for each value, then if 30 3d voices were playing, then only 2 voices total would be available, for 2d or 3d playback.  They are not separate.<br>
    <br>
    NOTE: If this is called before System::init, you must call System::getDriverCaps first.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::init
    System::getChannelsPlaying
    System::setHardwareChannels
    System::getDriverCaps
]
*/
FMOD_RESULT F_API FMOD_System_GetHardwareChannels(FMOD_SYSTEM *system, int *num2d, int *num3d, int *total)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getHardwareChannels(num2d, num3d, total);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves in percent of CPU time - the amount of cpu usage that FMOD is taking for streaming/mixing and System::update combined.

    [PARAMETERS]
    'dsp'      Address of a variable that receives the current dsp mixing engine cpu usage.  Result will be from 0 to 100.0f.  Optional.  Specify 0 or NULL to ignore. 
    'stream'   Address of a variable that receives the current streaming engine cpu usage.  Result will be from 0 to 100.0f.  Optional.  Specify 0 or NULL to ignore. 
    'update'   Address of a variable that receives the current System::update cpu usage.  Result will be from 0 to 100.0f.  Optional.  Specify 0 or NULL to ignore. 
    'geometry' Address of a variable that receives the current geometry engine cpu usage.  Result will be from 0 to 100.0f.  Optional.  Specify 0 or NULL to ignore. 
    'total'    Address of a variable that receives the current total cpu usage.  Result will be from 0 to 100.0f.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]
    This value is slightly smoothed to provide more stable readout (and to round off spikes that occur due to multitasking/operating system issues).<br>
    <br>
    NOTE!  On ps3 and xbox360, the dsp and stream figures are NOT main cpu/main thread usage.  On PS3 this is the percentage of SPU being used.  On Xbox 360 it is the percentage of a hardware thread being used which is on a totally different CPU than the main one.<br>
    Do not be alarmed if the usage for these platforms reaches over 50%, this is normal and should be ignored if you are playing a lot of compressed sounds and are using effects.  The only value on the main cpu / main thread to take note of here that will impact your framerate is the update value, and this is typically very low (ie less than 1%).<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::update
]
*/
FMOD_RESULT F_API FMOD_System_GetCPUUsage(FMOD_SYSTEM *system, float *dsp, float *stream, float *update, float *total)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getCPUUsage(dsp, stream, update, total);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the amount of dedicated sound ram available if the platform supports it.<br>
    Most platforms use main ram to store audio data, so this function usually isn't necessary.

    [PARAMETERS]
    'currentalloced'    Address of a variable that receives the currently allocated sound ram memory at time of call.  Optional.  Specify 0 or NULL to ignore. 
    'maxalloced'        Address of a variable that receives the maximum allocated sound ram memory since System::init.  Optional.  Specify 0 or NULL to ignore. 
    'total'             Address of a variable that receives the total amount of sound ram available on this device.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, PlayStation 2, PlayStation 3

    [SEE_ALSO]
    Memory_GetStats
]
*/
FMOD_RESULT F_API FMOD_System_GetSoundRAM(FMOD_SYSTEM *system, int *currentalloced, int *maxalloced, int *total)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getSoundRAM(currentalloced, maxalloced, total);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of available CDROM drives on the user's machine.

    [PARAMETERS]
    'numdrives'   Address of a variable that receives the number of CDROM drives.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Solaris

    [SEE_ALSO]
    System::getCDROMDriveName
]
*/
FMOD_RESULT F_API FMOD_System_GetNumCDROMDrives(FMOD_SYSTEM *system, int *numdrives)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getNumCDROMDrives(numdrives);
}


/*
[API]
[
    [DESCRIPTION]
    Gets information on the selected cdrom drive.

    [PARAMETERS]
    'drive'             The enumerated number of the CDROM drive to query.  0 based.
    'drivename'         Address of a variable that receives the name of the drive letter or name depending on the operating system.
    'drivenamelen'      Length in bytes of the target buffer to receieve the string.
    'scsiname'          Address of a variable that receives the SCSI address of the drive.   This could also be used to pass to System::createSound, or just used for information purposes.
    'scsinamelen'       Length in bytes of the target buffer to receieve the string.
    'devicename'        Address of a variable that receives the name of the physical device.  This is usually a string defined by the manufacturer.  It also contains the drive's vendor ID, product ID and version number.
    'devicenamelen'     Length in bytes of the target buffer to receieve the string.

    [RETURN_VALUE]

    [REMARKS]
    Enumerate CDROM drives by finding out how many there are with System::getNumCDROMDrives.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Solaris

    [SEE_ALSO]
    System::getNumCDROMDrives
    System::createSound
]
*/
FMOD_RESULT F_API FMOD_System_GetCDROMDriveName(FMOD_SYSTEM *system, int drive, char *drivename, int drivenamelen, char *scsiname, int scsinamelen, char *devicename, int devicenamelen)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getCDROMDriveName(drive, drivename, drivenamelen, scsiname, scsinamelen, devicename, devicenamelen);
}


/*
[API]
[
	[DESCRIPTION]
    Retrieves the spectrum from the currently playing output signal.

	[PARAMETERS]
    'spectrumarray'     Address of a variable that receives the spectrum data.  This is an array of floating point values.  Data will range is 0.0 to 1.0.  Decibels = 10.0f * (float)log10(val) * 2.0f;  See remarks for what the data represents.
    'numvalues'         Size of array in floating point values being passed to the function.  Must be a power of 2. (ie 128/256/512 etc).  Min = 64.  Max = 8192.
    'channeloffset'     Channel of the signal to analyze.  If the signal is multichannel (such as a stereo output), then this value represents which channel to analyze.  On a stereo signal 0 = left, 1 = right.
    'windowtype'        "Pre-FFT" window method.  This filters the PCM data before entering the spectrum analyzer to reduce transient frequency error for more accurate results.  See FMOD_DSP_FFT_WINDOW for different types of fft window techniques possible and for a more detailed explanation.
     
	[RETURN_VALUE]

	[REMARKS]
    The larger the numvalues, the more CPU the FFT will take.  Choose the right value to trade off between accuracy / speed.<br>
    The larger the numvalues, the more 'lag' the spectrum will seem to inherit.  This is because the FFT window size stretches the analysis back in time to what was already played.  For example if the window size happened to be 44100 and the output rate was 44100 it would be analyzing the past second of data, and giving you the average spectrum over that time period.<br>
    If you are not displaying the result in dB, then the data may seem smaller than it should be.  To display it you may want to normalize the data - that is, find the maximum value in the resulting spectrum, and scale all values in the array by 1 / max.  (ie if the max was 0.5f, then it would become 1).<br>
    To get the spectrum for both channels of a stereo signal, call this function twice, once with channeloffset = 0, and again with channeloffset = 1.  Then add the spectrums together and divide by 2 to get the average spectrum for both channels.<br>
    <br>
    <u>What the data represents.</u><br>
    To work out what each entry in the array represents, use this formula<br>
    <PRE>
    entry_hz = (output_rate / 2) / numvalues
    </PRE>
    The array represents amplitudes of each frequency band from 0hz to the nyquist rate.  The nyquist rate is equal to the output rate divided by 2.<br>
    For example when FMOD is set to 44100hz output, the range of represented frequencies will be 0hz to 22049hz, a total of 22050hz represented.<br>
    If in the same example, 1024 was passed to this function as the numvalues, each entry's contribution would be as follows.
    <PRE>
    entry_hz = (44100 / 2) / 1024
    entry_hz = 21.53 hz
    </PRE>
    <br>
    <b>Note:</b> This function only displays data for sounds playing that were created with FMOD_SOFTWARE.  FMOD_HARDWARE based sounds are played using the sound card driver and are not accessable.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    FMOD_DSP_FFT_WINDOW
    Channel::getSpectrum
    ChannelGroup::getSpectrum
    System::getWaveData
]
*/
FMOD_RESULT F_API FMOD_System_GetSpectrum(FMOD_SYSTEM *system, float *spectrumarray, int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getSpectrum(spectrumarray, numvalues, channeloffset, windowtype);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a pointer to a block of PCM data that represents the currently playing audio mix.<br>
    This function is useful for a very easy way to plot an oscilliscope.<br>

    [PARAMETERS]
    'wavearray'     Address of a variable that receives the currently playing waveform data.  This is an array of floating point values.
    'numvalues'     Number of floats to write to the array.  Maximum value = 16384.
    'channeloffset' Offset into multichannel data.  For mono output use 0.  Stereo output will use 0 = left, 1 = right.  More than stereo output - use the appropriate index.

    [RETURN_VALUE]

    [REMARKS]
    This is the actual resampled, filtered and volume scaled data of the final output, at the time this function is called.<br>
    <br>
    Do not use this function to try and display the whole waveform of the sound, as this is more of a 'snapshot' of the current waveform at the time it is called, and could return the same data if it is called very quickly in succession.<br>
    See the DSP API to capture a continual stream of wave data as it plays, or see Sound::lock / Sound::unlock if you want to simply display the waveform of a sound.<br>
    <br>
    This function allows retrieval of left and right data for a stereo sound individually.  To combine them into one signal, simply add the entries of each seperate buffer together and then divide them by 2.<br>
    <br>
    <b>Note:</b> This function only displays data for sounds playing that were created with FMOD_SOFTWARE.  FMOD_HARDWARE based sounds are played using the sound card driver and are not accessable.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getSpectrum
    Channel::getWaveData
    ChannelGroup::getWaveData
    Sound::lock
    Sound::unlock
]
*/
FMOD_RESULT F_API FMOD_System_GetWaveData(FMOD_SYSTEM *system, float *wavearray, int numvalues, int channeloffset)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getWaveData(wavearray, numvalues, channeloffset);
}


/*
[API]
[
    [DESCRIPTION]
    Loads a sound into memory, or opens it for streaming.

    [PARAMETERS]
    'name_or_data'      Name of the file or URL to open, or a pointer to a preloaded sound memory block if FMOD_OPENMEMORY/FMOD_OPENMEMORY_POINT is used.   For CD playback the name should be a drive letter with a colon, example "D:" (windows only).
    'mode'              Behaviour modifier for opening the sound.  See FMOD_MODE.  Also see remarks for more.
    'exinfo'            Pointer to a FMOD_CREATESOUNDEXINFO which lets the user provide extended information while playing the sound.  Optional.  Specify 0 or NULL to ignore. 
    'sound'             Address of a variable to receive a newly created FMOD::Sound object.

    [RETURN_VALUE]

    [REMARKS]
    <b>Important!</b> By default (FMOD_CREATESAMPLE) FMOD will try to load and decompress the whole sound into memory!  Use FMOD_CREATESTREAM to open it as a stream and have it play back in realtime!  FMOD_CREATECOMPRESSEDSAMPLE can also be used for certain formats.<br>    
    <br>
    <li>To open a file or URL as a stream, so that it decompresses / reads at runtime, instead of loading / decompressing into memory all at the time of this call, use the FMOD_CREATESTREAM flag.  This is like a 'stream' in FMOD 3.<br>
    <li>To open a file or URL as a compressed sound effect that is not streamed and is not decompressed into memory at load time, use FMOD_CREATECOMPRESSEDSAMPLE.  This is supported with MPEG (mp2/mp3), ADPCM (wav on all platforms and vag on ps2/psp) and XMA files only.  This is useful for those who want realtime compressed soundeffects, but not the overhead of disk access.<br>
    <li>To open a CD drive, use the drive as the name, for example on the windows platform, use "D:"<br>
    <li>To open a sound as 2D, so that it is not affected by 3D processing, use the FMOD_2D flag.  3D sound commands will be ignored on these types of sounds.<br>
    <li>To open a sound as 3D, so that it is treated as a 3D sound, use the FMOD_3D flag.  Calls to Channel::setPan will be ignored on these types of sounds.<br>
    <li>To use FMOD software mixing buffers, use the FMOD_SOFTWARE flag.  This gives certain benefits, such as DSP processing, spectrum analysis, loop points, 5.1 mix levels, 2d/3d morphing, and more.<br>
    <li>To use the soundcard's hardware to play the sound, use the FMOD_HARDWARE flag.  This gives certain benefits such as EAX reverb, dolby digital output on some devices, and better 3d sound virtualization using headphones.<br>
    <br>
    Note that FMOD_OPENRAW, FMOD_OPENMEMORY, FMOD_OPENMEMORY_POINT and FMOD_OPENUSER will not work here without the exinfo structure present, as more information is needed.<br>
    <br>
    Use FMOD_NONBLOCKING to have the sound open or load in the background.  You can use Sound::getOpenState to determine if it has finished loading / opening or not.  While it is loading (not ready), sound functions are not accessable for that sound.<br>
    <br>
    To account for slow devices or computers that might cause buffer underrun (skipping/stuttering/repeating blocks of audio), use System::setStreamBufferSize.
    <br>
    To play WMA files on Windows, the user must have the latest Windows media player codecs installed (Windows Media Player 9).  The user can download this as an installer (wmfdist.exe) from www.fmod.org download page if they desire or you may wish to redistribute it with your application (this is allowed).  This installer does NOT install windows media player, just the necessary WMA codecs needed.
    <br>
    <b>PlayStation 2 Note:</b> You can pre-pend "host0:" or "cdrom0:" if you like.  FMOD will automatically add "host0:" to the filename if it is not found.
    <br>
    Specifying FMOD_OPENMEMORY_POINT will POINT to your memory rather allocating its own sound buffers and duplicating it internally<br>
    <b><u>This means you cannot free the memory while FMOD is using it, until after Sound::release is called.</b></u>
    With FMOD_OPENMEMORY_POINT, for PCM formats, only WAV, FSB and RAW are supported.  For compressed formats, only those formats supported by FMOD_CREATECOMPRESSEDSAMPLE are supported.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_MODE
    FMOD_CREATESOUNDEXINFO
    Sound::getOpenState
    System::setStreamBufferSize
    Channel::setPan
]
*/
FMOD_RESULT F_API FMOD_System_CreateSound(FMOD_SYSTEM *system, const char *name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO *exinfo, FMOD_SOUND **sound)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->createSound(name_or_data, mode, exinfo, (FMOD::Sound **)sound);
}


/*
[API]
[
    [DESCRIPTION]
    Opens a sound for streaming.  This function is a helper function that is the same as System::createSound but has the FMOD_CREATESTREAM flag added internally.

    [PARAMETERS]
    'name_or_data'      Name of the file or URL to open.   For CD playback this may be a drive letter with a colon, example "D:".
    'mode'              Behaviour modifier for opening the sound.  See FMOD_MODE.  Also see remarks for more.
    'exinfo'            Pointer to a FMOD_CREATESOUNDEXINFO which lets the user provide extended information while playing the sound.  Optional.  Specify 0 or NULL to ignore. 
    'sound'             Address of a variable to receive a newly created FMOD::Sound object.

    [RETURN_VALUE]

    [REMARKS]
    Note that a stream only has 1 decode buffer and file handle, and therefore can only be played once.  It cannot play multiple times at once because it cannot share a stream buffer if the stream is playing at different positions.<br>
    Open multiple streams to have them play concurrently.<br>
    <br>
    <li>To open a file or URL as a stream, so that it decompresses / reads at runtime, instead of loading / decompressing into memory all at the time of this call, use the FMOD_CREATESTREAM flag.  This is like a 'stream' in FMOD 3.<br>
    <li>To open a file or URL as a compressed sound effect that is not streamed and is not decompressed into memory at load time, use FMOD_CREATECOMPRESSEDSAMPLE.  This is supported with MPEG (mp2/mp3), ADPCM (wav on all platforms and vag on ps2/psp) and XMA files only.  This is useful for those who want realtime compressed soundeffects, but not the overhead of disk access.<br>
    <li>To open a CD drive, use the drive as the name, for example on the windows platform, use "D:"<br>
    <li>To open a sound as 2D, so that it is not affected by 3D processing, use the FMOD_2D flag.  3D sound commands will be ignored on these types of sounds.<br>
    <li>To open a sound as 3D, so that it is treated as a 3D sound, use the FMOD_3D flag.  Calls to Channel::setPan will be ignored on these types of sounds.<br>
    <li>To use FMOD software mixing buffers, use the FMOD_SOFTWARE flag.  This gives certain benefits, such as DSP processing, spectrum analysis, loop points, 5.1 mix levels, 2d/3d morphing, and more.<br>
    <li>To use the soundcard's hardware to play the sound, use the FMOD_HARDWARE flag.  This gives certain benefits such as EAX reverb, dolby digital output on some devices, and better 3d sound virtualization using headphones.<br>
    <br>
    Note that FMOD_OPENRAW, FMOD_OPENMEMORY, FMOD_OPENMEMORY_POINT and FMOD_OPENUSER will not work here without the exinfo structure present, as more information is needed.<br>
    <br>
    Use FMOD_NONBLOCKING to have the sound open or load in the background.  You can use Sound::getOpenState to determine if it has finished loading / opening or not.  While it is loading (not ready), sound functions are not accessable for that sound.<br>
    <br>
    To account for slow devices or computers that might cause buffer underrun (skipping/stuttering/repeating blocks of audio), use System::setStreamBufferSize.
    <br>
    Note that FMOD_CREATESAMPLE will be ignored, overriden by this function because this is simply a wrapper to System::createSound that provides the FMOD_CREATESTREAM flag.  The FMOD_CREATESTREAM flag overrides FMOD_CREATESAMPLE.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_MODE
    FMOD_CREATESOUNDEXINFO
    Sound::getOpenState
    System::setStreamBufferSize
    System::createSound
    Channel::setPan
]
*/
FMOD_RESULT F_API FMOD_System_CreateStream(FMOD_SYSTEM *system, const char *name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO *exinfo, FMOD_SOUND **sound)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->createStream(name_or_data, mode, exinfo, (FMOD::Sound **)sound);
}


/*
[API]
[
    [DESCRIPTION]
    Creates a user defined DSP unit object to be inserted into a DSP network, for the purposes of sound filtering or sound generation.

    [PARAMETERS]
    'description'   Address of an FMOD_DSP_DESCRIPTION structure containing information about the unit to be created.
    'dsp'           Address of a variable to receive a newly created FMOD::DSP object.

    [RETURN_VALUE]

    [REMARKS]
    A DSP unit can generate or filter incoming data.<br>  
    The data is created or filtered through use of the read callback that is defined by the user.<br>
    See the definition for the FMOD_DSP_DESCRIPTION structure to find out what each member means.<br>
    To be active, a unit must be inserted into the FMOD DSP network to be heard.  Use functions such as System::addDSP, Channel::addDSP or DSP::addInput to do this.<br>
    For more information and a detailed description (with diagrams) see the tutorial on the DSP system in the documentation.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_DSP_DESCRIPTION
    System::createDSPByType
    System::createDSPByPlugin
    System::addDSP
    Channel::addDSP
    DSP::addInput
    DSP::setActive
]
*/
FMOD_RESULT F_API FMOD_System_CreateDSP(FMOD_SYSTEM *system, FMOD_DSP_DESCRIPTION *description, FMOD_DSP **dsp)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->createDSP(description, (FMOD::DSP **)dsp);
}


/*
[API]
[
    [DESCRIPTION]
    Creates an FMOD defined built in DSP unit object to be inserted into a DSP network, for the purposes of sound filtering or sound generation.<br>
    This function is used to create special effects that come built into FMOD.

    [PARAMETERS]
    'type'      A pre-defined DSP effect or sound generator described by a FMOD_DSP_TYPE.
    'dsp'       Address of a variable to receive a newly created FMOD::DSP object.

    [RETURN_VALUE]

    [REMARKS]
    A DSP unit can generate or filter incoming data.<br>  
    To be active, a unit must be inserted into the FMOD DSP network to be heard.  Use functions such as System::addDSP, Channel::addDSP , ChannelGroup::addDSP or DSP::addInput to do this.<br>
    For more information and a detailed description (with diagrams) see the tutorial on the DSP system in the documentation.<br>
    <br>
    <b>Note!</b> Winamp DSP and VST plugins will only return the first plugin of this type that was loaded!<br>
    To access all VST or Winamp DSP plugins the System::createDSPByPlugin function!  Use the index returned by System::loadPlugin if you don't want to enumerate them all.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_DSP_TYPE
    System::createDSP
    System::createDSPByPlugin
    System::addDSP
    System::loadPlugin
    Channel::addDSP
    ChannelGroup::addDSP
    DSP::addInput
    DSP::setActive
]
*/
FMOD_RESULT F_API FMOD_System_CreateDSPByType(FMOD_SYSTEM *system, FMOD_DSP_TYPE type, FMOD_DSP **dsp)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->createDSPByType(type, (FMOD::DSP **)dsp);
}


FMOD_RESULT F_API FMOD_System_CreateDSPByIndex(FMOD_SYSTEM *system, int index, FMOD_DSP **dsp)
{
    FMOD::System *sys = (FMOD::System *)system;

    FMOD_RESULT result = sys->release();
    if (result != FMOD_OK)
    {
        return result;
    }

    return sys->createDSPByIndex(index, (FMOD::DSP **)dsp);
}


/*
[API]
[
    [DESCRIPTION]
    Creates a channel group object.  These objects can be used to assign channels to for group channel settings, such as volume.<br>
    Channel groups are also used for sub-mixing.  Any channels that are assigned to a channel group get submixed into that channel group's DSP.

    [PARAMETERS]
    'name'          Label to give to the channel group for identification purposes.  Optional (can be null).
    'channelgroup'  Address of a variable to receive a newly created FMOD::ChannelGroup object.

    [RETURN_VALUE]

    [REMARKS]
    See the channel group class definition for the types of operations that can be performed on 'groups' of channels.<br>
    The channel group can for example be used to have 2 seperate groups of master volume, instead of one global master volume.<br>
    A channel group can be used for sub-mixing, ie so that a set of channels can be mixed into a channel group, then can have effects applied to it without affecting other channels.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getMasterChannelGroup
    Channel::setChannelGroup
    ChannelGroup::release
]
*/
FMOD_RESULT F_API FMOD_System_CreateChannelGroup(FMOD_SYSTEM *system, const char *name, FMOD_CHANNELGROUP **channelgroup)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->createChannelGroup(name, (FMOD::ChannelGroup **)channelgroup);
}


/*
[API]
[
    [DESCRIPTION]
    Creates a sound group, which can store handles to multiple Sound pointers.

    [PARAMETERS]
    'name'          Name of sound group.
    'soundgroup'    Address of a variable to recieve a pointer to a sound group.

    [RETURN_VALUE]

    [REMARKS]
    Once a SoundGroup is created, Sound::setSoundGroup is used to put a sound in a SoundGroup.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    SoundGroup::release
    Sound::setSoundGroup
]
*/
FMOD_RESULT F_API FMOD_System_CreateSoundGroup(FMOD_SYSTEM *system, const char *name, FMOD_SOUNDGROUP **soundgroup)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->createSoundGroup(name, (FMOD::SoundGroup **)soundgroup);
}


/*
[API]
[
    [DESCRIPTION]
    Creates a 'virtual reverb' object.  This object reacts to 3d location and morphs the reverb environment based on how close it is to the reverb object's center.<br>
    Multiple reverb objects can be created to achieve a multi-reverb environment.

    [PARAMETERS]
    'reverb'    Address of a pointer to a Reverb object to receive the newly created virtual reverb object.

    [RETURN_VALUE]

    [REMARKS]
    The 3D reverb object is a sphere having 3D attributes (position, minimum distance, maximum distance) and reverb properties.<br>
    The properties and 3D attributes of all reverb objects collectively determine, along with the listener's position, the settings of and input gains into a single 3D reverb DSP.<br>
    Please note that this only applies to software channels. When the listener is within the sphere of effect of one or more 3d reverbs, the listener's 3D reverb properties are
    a weighted combination of such 3d reverbs. When the listener is outside all of the reverbs, the 3D reverb setting is set to the default ambient reverb setting.<br>
    <br>
    Use System::setReverbAmbientProperties to set a 'background' default reverb environment.  This is a reverb that will be morphed to if the listener is not within any virtual reverb zones.<br>
    By default the ambient reverb is set to 'off'.
    <br>
    Creating multiple reverb objects does not impact performance.  These are 'virtual reverbs'.  There will still be only 1 physical reverb DSP running that just morphs between the different virtual reverbs.
    <br>
    System::setReverbProperties can still be used in conjunction with the 3d based virtual reverb system.  This allows 2d sounds to have reverb.  If this call is used at the same time virtual reverb objects are active, 2 physical reverb dsps will be used, incurring a small memory and cpu hit.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Reverb::release
    System::setReverbAmbientProperties
    System::getReverbAmbientProperties
    System::setReverbProperties
    System::getReverbProperties
]
*/
FMOD_RESULT F_API FMOD_System_CreateReverb(FMOD_SYSTEM *system, FMOD_REVERB **reverb)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->createReverb((FMOD::Reverb **)reverb);
}


/*
[API]
[
    [DESCRIPTION]
    Plays a sound object on a particular channel.

    [PARAMETERS]
    'channelid'     Use the value FMOD_CHANNEL_FREE to get FMOD to pick a free channel.  Otherwise specify a channel number from 0 to the 'maxchannels' value specified in System::init minus 1.
    'sound'         Pointer to the sound to play.  This is opened with System::createSound.
    'paused'        True or false flag to specify whether to start the channel paused or not.  Starting a channel paused allows the user to alter its attributes without it being audible, and unpausing with Channel::setPaused actually starts the sound.
    'channel'       Address of a channel handle pointer that receives the newly playing channel.   If FMOD_CHANNEL_REUSE is used, this can contain a previously used channel handle and FMOD will re-use it to play a sound on.

    [RETURN_VALUE]

    [REMARKS]
    When a sound is played, it will use the sound's default frequency, volume, pan, levels and priority.<br>
    <br>
    A sound defined as FMOD_3D will by default play at the position of the listener.<br>
    <br>
    To change channel attributes before the sound is audible, start the channel paused by setting the paused flag to true, and calling the relevant channel based functions.  Following that, unpause the channel with Channel::setPaused.<br>
    <br>
    If FMOD_CHANNEL_FREE is used as the channel index, it will pick an arbitrary free channel and use channel management. (As described below).<br>
    If FMOD_CHANNEL_REUSE is used as the channel index, FMOD Ex will re-use the channel handle that is passed in as the 'channel' parameter.  If NULL or 0 is passed in as the channel handle it will use the same logic as FMOD_CHANNEL_FREE and pick an arbitrary channel.<br>
    <br>
    Channels are reference counted.  If a channel is stolen by the FMOD priority system, then the handle to the stolen voice becomes invalid, and Channel based commands will not affect the new sound playing in its place.<br>
    If all channels are currently full playing a sound, FMOD will steal a channel with the lowest priority sound.<br>
    If more channels are playing than are currently available on the soundcard/sound device or software mixer, then FMOD will 'virtualize' the channel.  This type of channel is not heard, but it is updated as if it was playing.  When its priority becomes high enough or another sound stops that was using a real hardware/software channel, it will start playing from where it should be.  This technique saves CPU time (thousands of sounds can be played at once without actually being mixed or taking up resources), and also removes the need for the user to manage voices themselves.<br>
    An example of virtual channel usage is a dungeon with 100 torches burning, all with a looping crackling sound, but with a soundcard that only supports 32 hardware voices.  If the 3D positions and priorities for each torch are set correctly, FMOD will play all 100 sounds without any 'out of channels' errors, and swap the real voices in and out according to which torches are closest in 3D space.<br>
    Priority for virtual channels can be changed in the sound's defaults, or at runtime with Channel::setPriority.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_CHANNELINDEX
    System::createSound
    Channel::setPaused
    Channel::setPriority
    Sound::setDefaults
    Sound::setVariations
    System::init
]
*/
FMOD_RESULT F_API FMOD_System_PlaySound(FMOD_SYSTEM *system, FMOD_CHANNELINDEX channelid, FMOD_SOUND *sound, FMOD_BOOL paused, FMOD_CHANNEL **channel)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->playSound(channelid, (FMOD::Sound *)sound, paused ? true : false, (FMOD::Channel **)channel);
}


/*
[API]
[
    [DESCRIPTION]
    Plays a DSP unit object and its input network on a particular channel.

    [PARAMETERS]
    'channelid'     Use the value FMOD_CHANNEL_FREE to get FMOD to pick a free channel.  Otherwise specify a channel number from 0 to the 'maxchannels' value specified in System::init minus 1.
    'dsp'           Pointer to the dsp unit to play.  This is opened with System::createDSP, System::createDSPByType, System::createDSPByPlugin.
    'paused'        True or false flag to specify whether to start the channel paused or not.  Starting a channel paused allows the user to alter its attributes without it being audible, and unpausing with Channel::setPaused actually starts the dsp running.
    'channel'       Address of a channel handle pointer that receives the newly playing channel.   If FMOD_CHANNEL_REUSE is used, this can contain a previously used channel handle and FMOD will re-use it to play a dsp on.

    [RETURN_VALUE]

    [REMARKS]
    When a dsp is played, it will use the dsp's default frequency, volume, pan, levels and priority.<br>
    <br>
    A dsp defined as FMOD_3D will by default play at the position of the listener.<br>
    <br>
    To change channel attributes before the dsp is audible, start the channel paused by setting the paused flag to true, and calling the relevant channel based functions.  Following that, unpause the channel with Channel::setPaused.<br>
    <br>
    If FMOD_CHANNEL_FREE is used as the channel index, it will pick an arbitrary free channel and use channel management. (As described below).<br>
    If FMOD_CHANNEL_REUSE is used as the channel index, FMOD Ex will re-use the channel handle that is passed in as the 'channel' parameter.  If NULL or 0 is passed in as the channel handle it will use the same logic as FMOD_CHANNEL_FREE and pick an arbitrary channel.<br>
    <br>
    Channels are reference counted.  If a channel is stolen by the FMOD priority system, then the handle to the stolen voice becomes invalid, and Channel based commands will not affect the new channel playing in its place.<br>
    If all channels are currently full playing a dsp or sound, FMOD will steal a channel with the lowest priority dsp or sound.<br>
    If more channels are playing than are currently available on the soundcard/sound device or software mixer, then FMOD will 'virtualize' the channel.  This type of channel is not heard, but it is updated as if it was playing.  When its priority becomes high enough or another sound stops that was using a real hardware/software channel, it will start playing from where it should be.  This technique saves CPU time (thousands of sounds can be played at once without actually being mixed or taking up resources), and also removes the need for the user to manage voices themselves.<br>
    An example of virtual channel usage is a dungeon with 100 torches burning, all with a looping crackling sound, but with a soundcard that only supports 32 hardware voices.  If the 3D positions and priorities for each torch are set correctly, FMOD will play all 100 sounds without any 'out of channels' errors, and swap the real voices in and out according to which torches are closest in 3D space.<br>
    Priority for virtual channels can be changed in the sound's defaults, or at runtime with Channel::setPriority.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_CHANNELINDEX
    System::createDSP
    System::createDSPByType
    System::createDSPByPlugin
    Channel::setPaused
    Channel::setPriority
    DSP::setDefaults
    System::init
]
*/
FMOD_RESULT F_API FMOD_System_PlayDSP(FMOD_SYSTEM *system, FMOD_CHANNELINDEX channelid, FMOD_DSP *dsp, FMOD_BOOL paused, FMOD_CHANNEL **channel)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->playDSP(channelid, (FMOD::DSP *)dsp, paused ? true : false, (FMOD::Channel **)channel);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a handle to a channel by ID.

    [PARAMETERS]
    'channelid'     Index in the FMOD channel pool.  Specify a channel number from 0 to the 'maxchannels' value specified in System::init minus 1.
    'channel'       Address of a variable that receives a pointer to the requested channel.

    [RETURN_VALUE]

    [REMARKS]
    This function is mainly for getting handles to existing (playing) channels and setting their attributes.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::playSound
    System::init
]
*/
FMOD_RESULT F_API FMOD_System_GetChannel(FMOD_SYSTEM *system, int channelid, FMOD_CHANNEL **channel)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getChannel(channelid, (FMOD::Channel **)channel);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a handle to the internal master channel group.  This is the default channel group that all channels play on.<br>
    This channel group can be used to do things like set the master volume for all playing sounds.  See the ChannelGroup API for more functionality.

    [PARAMETERS]
    'channelgroup'  Address of a variable that receives a pointer to the master System object channel group.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createChannelGroup
    ChannelGroup::setVolume
    ChannelGroup::getVolume
]
*/
FMOD_RESULT F_API FMOD_System_GetMasterChannelGroup(FMOD_SYSTEM *system, FMOD_CHANNELGROUP **channelgroup)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getMasterChannelGroup((FMOD::ChannelGroup **)channelgroup);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the default sound group, where all sounds are placed when they are created.

    [PARAMETERS]
    'soundgroup'    Address of a pointer to a SoundGroup object to receive the master sound group.

    [RETURN_VALUE]

    [REMARKS]
    If a user based soundgroup is deleted/released, the sounds will be put back into this sound group.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    SoundGroup::release
    SoundGroup::getSystemObject
    SoundGroup::setMaxAudible
    SoundGroup::getMaxAudible
    SoundGroup::getName
    SoundGroup::getNumSounds
    SoundGroup::getSound
    SoundGroup::getNumPlaying
    SoundGroup::setUserData
    SoundGroup::getUserData
]
*/
FMOD_RESULT F_API FMOD_System_GetMasterSoundGroup(FMOD_SYSTEM *system, FMOD_SOUNDGROUP **soundgroup)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getMasterSoundGroup((FMOD::SoundGroup **)soundgroup);
}


/*
[API]
[
    [DESCRIPTION]
    Sets parameters for the global reverb environment.<br>
	Reverb parameters can be set manually, or automatically using the pre-defined presets given in the fmod.h header.

    [PARAMETERS]
    'prop'	    Address of an FMOD_REVERB_PROPERTIES structure which defines the attributes for the reverb.

    [RETURN_VALUE]

    [REMARKS]	
    With FMOD_HARDWARE on Windows using EAX, the reverb will only work on FMOD_3D based sounds.  FMOD_SOFTWARE does not have this problem and works on FMOD_2D and FMOD_3D based sounds.<br>
    <br>
    On PlayStation 2, the reverb is limited to only a few reverb types that are not configurable.  Use the FMOD_PRESET_PS2_xxx presets.
    <br>
    On Xbox, it is possible to apply reverb to FMOD_2D and FMOD_HARDWARE based voices using this function.  By default reverb is turned off for FMOD_2D hardware based voices.<br>
    <br>
    <b>Note!</b> It is important to specify the 'Instance' value in the FMOD_REVERB_PROPERTIES structure correctly, otherwise you will get an FMOD_ERR_REVERB_INSTANCE error.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_REVERB_PROPERTIES
    System::getReverbProperties
    Channel::setReverbProperties
    Channel::getReverbProperties
]
*/
FMOD_RESULT F_API FMOD_System_SetReverbProperties(FMOD_SYSTEM *system, const FMOD_REVERB_PROPERTIES *prop)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setReverbProperties(prop);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current reverb environment for the specified reverb instance.  You must specify the 'Instance' value (usually 0 unless you are using multiple reverbs) before calling this function.

    [PARAMETERS]
    'prop'     Address of a variable that receives the current reverb environment description.  Make sure the 'Instance' value is specified.

    [RETURN_VALUE]

    [REMARKS]
    <b>Note!</b> It is important to specify the 'Instance' value in the FMOD_REVERB_PROPERTIES structure correctly, otherwise you will get an FMOD_ERR_REVERB_INSTANCE error.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_REVERB_PROPERTIES
    System::setReverbProperties
    Channel::setReverbProperties
    Channel::getReverbProperties
]
*/
FMOD_RESULT F_API FMOD_System_GetReverbProperties(FMOD_SYSTEM *system, FMOD_REVERB_PROPERTIES *prop)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getReverbProperties(prop);
}


/*
[API]
[
    [DESCRIPTION]
    Sets a 'background' default reverb environment for the virtual reverb system.  This is a reverb preset that will be morphed to if the listener is not within any virtual reverb zones.<br>
    By default the ambient reverb is set to 'off'.

    [PARAMETERS]
    'prop'      Address of a FMOD_REVERB_PROPERTIES structure containing the settings for the desired ambient reverb setting.

    [RETURN_VALUE]

    [REMARKS]
    There is one reverb DSP dedicated to providing a 3D reverb effect. This DSP's properties are a weighted sum of all the contributing virtual reverbs.<br>
    The default 3d reverb properties specify the reverb properties in the 3D volumes which has no virtual reverbs defined. 

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_REVERB_PROPERTIES
    System::getReverbAmbientProperties
    System::createReverb
]
*/
FMOD_RESULT F_API FMOD_System_SetReverbAmbientProperties(FMOD_SYSTEM *system, FMOD_REVERB_PROPERTIES *prop)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setReverbAmbientProperties(prop);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the default reverb envrionment for the virtual reverb system.

    [PARAMETERS]
    'prop'      Address of a pointer to a FMOD_REVERB_PROPERTIES to receieve the settings for the current ambient reverb setting.

    [RETURN_VALUE]

    [REMARKS]
    By default the ambient reverb is set to 'off'.  This is the same as FMOD_REVERB_PRESET_OFF.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_REVERB_PROPERTIES
    System::setReverbAmbientProperties
    System::createReverb
]
*/
FMOD_RESULT F_API FMOD_System_GetReverbAmbientProperties(FMOD_SYSTEM *system, FMOD_REVERB_PROPERTIES *prop)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getReverbAmbientProperties(prop);
}


/*
[API]
[
    [DESCRIPTION]
    Returns a pointer to the head DSP unit of the DSP network.  This unit is the closest unit to the soundcard and all sound data comes through this unit.

    [PARAMETERS]
    'dsp'       Address of a variable that receives the pointer to the head DSP unit.

    [RETURN_VALUE]

    [REMARKS]
    Use this unit if you wish to connect custom DSP units to the output or filter the global mix by inserting filter units between this one and the incoming channel mixer unit.<br>
    Read the tutorial on DSP if you wish to know more about this.  It is not recommended using this if you do not understand how the FMOD Ex DSP network is connected.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getDSPHead
    ChannelGroup::getDSPHead
]
*/
FMOD_RESULT F_API FMOD_System_GetDSPHead(FMOD_SYSTEM *system, FMOD_DSP **dsp)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getDSPHead((FMOD::DSP **)dsp);
}


/*
[API]
[
    [DESCRIPTION]
    This function adds a pre-created DSP unit or effect to the head of the System DSP chain.

    [PARAMETERS]
    'dsp'               A pointer to a pre-created DSP unit to be inserted at the head of the System DSP chain.
    'connection'        A pointer to the connection involved between the System DSP head and the specified dsp unit.  Optional. Specify 0 or NULL to ignore.

    [RETURN_VALUE]

    [REMARKS]
    This function is a wrapper function to insert a DSP unit at the top of the System DSP chain.<br>
    It disconnects the head unit from its input, then inserts the unit at the head and reconnects the previously disconnected input back as as an input to the new unit.<br>
    <br>
    <b>Note:</b> The connection pointer retrieved here will become invalid if you disconnect the 2 dsp units that use it.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getDSPHead
    System::createDSP
    System::createDSPByType
    System::createDSPByPlugin
    Channel::addDSP
    ChannelGroup::addDSP
    DSP::remove
]
*/
FMOD_RESULT F_API FMOD_System_AddDSP(FMOD_SYSTEM *system, FMOD_DSP *dsp)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->addDSP((FMOD::DSP *)dsp);
}


/*
[API]
[
    [DESCRIPTION]
    Mutual exclusion function to lock the FMOD DSP engine (which runs asynchronously in another thread), so that it will not execute.  If the FMOD DSP engine is already executing, this function will block until it has completed.<br>
    The function may be used to synchronize DSP network operations carried out by the user.<br>
    An example of using this function may be for when the user wants to construct a DSP sub-network, without the DSP engine executing in the background while the sub-network is still under construction.<br>    

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]
    Once the user no longer needs the DSP engine locked, it must be unlocked with System::unlockDSP.<br>
    Note that the DSP engine should not be locked for a significant amount of time, otherwise inconsistency in the audio output may result. (audio skipping/stuttering).<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::unlockDSP
]
*/
FMOD_RESULT F_API FMOD_System_LockDSP(FMOD_SYSTEM *system)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->lockDSP();
}


/*
[API]
[
    [DESCRIPTION]
    Mutual exclusion function to unlock the FMOD DSP engine (which runs asynchronously in another thread) and let it continue executing.

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]
    The DSP engine must be locked with System::lockDSP before this function is called.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::lockDSP
]
*/
FMOD_RESULT F_API FMOD_System_UnlockDSP(FMOD_SYSTEM *system)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->unlockDSP();
}


FMOD_RESULT F_API FMOD_System_SetRecordDriver(FMOD_SYSTEM *system, int driver)
{
    FMOD::System *sys = (FMOD::System *)system;

    FMOD_RESULT result = sys->release();
    if (result != FMOD_OK)
    {
        return result;
    }

    return sys->setRecordDriver(driver);
}


FMOD_RESULT F_API FMOD_System_GetRecordDriver(FMOD_SYSTEM *system, int *driver)
{
    FMOD::System *sys = (FMOD::System *)system;

    FMOD_RESULT result = sys->release();
    if (result != FMOD_OK)
    {
        return result;
    }

    return sys->getRecordDriver(driver);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of recording devices available for this output mode.  Use this to enumerate all recording devices possible so that the user can select one.

    [PARAMETERS]
    'numdrivers'    Address of a variable that receives the number of recording drivers available for this output mode.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh360, PlayStation 3, Solaris, iPhone

    [SEE_ALSO]
    System::GetRecordDriverInfo
]
*/
FMOD_RESULT F_API FMOD_System_GetRecordNumDrivers(FMOD_SYSTEM *system, int *numdrivers)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getRecordNumDrivers(numdrivers);
}


FMOD_RESULT F_API FMOD_System_GetRecordDriverName(FMOD_SYSTEM *system, int id, char *name, int namelen)
{
    FMOD::System *sys = (FMOD::System *)system;

    FMOD_RESULT result = sys->release();
    if (result != FMOD_OK)
    {
        return result;
    }

    return sys->getRecordDriverName(id, name, namelen);
}


/*
[API]
[
    [DESCRIPTION]
    Returns information on capabilities of the current output mode for the selected recording sound device.

    [PARAMETERS]
    'id'           Enumerated driver ID.  This must be in a valid range delimited by System::getRecordNumDrivers.
    'caps'         Address of a variable that receives the capabilities of the device.  Optional.  Specify 0 or NULL to ignore. 
    'minfrequency' Address of a variable that receives the minimum frequency allowed for sounds used with recording.  Optional.  Specify 0 or NULL to ignore. 
    'maxfrequency' Address of a variable that receives the maximum frequency allowed for sounds used with recording.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh360, PlayStation 3, Solaris, iPhone

    [SEE_ALSO]
    FMOD_CAPS
    System::getRecordNumDrivers
]
*/
FMOD_RESULT F_API FMOD_System_GetRecordDriverCaps(FMOD_SYSTEM *system, int id, FMOD_CAPS *caps, int *minfrequency, int *maxfrequency)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getRecordDriverCaps(id, caps, minfrequency, maxfrequency);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current recording position of the record buffer in PCM samples.

    [PARAMETERS]
    'id'        Enumerated driver ID.  This must be in a valid range delimited by System::getRecordNumDrivers.
    'position'  Address of a variable to receieve the current recording position in PCM samples.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh360, PlayStation 3, Solaris, iPhone

    [SEE_ALSO]
]
*/
FMOD_RESULT F_API FMOD_System_GetRecordPosition(FMOD_SYSTEM *system, unsigned int *position)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getRecordPosition(position);
}


/*
[API]
[
    [DESCRIPTION]
    Starts the recording engine recording to the specified recording sound.

    [PARAMETERS]
    'id'            Enumerated driver ID.  This must be in a valid range delimited by System::getRecordNumDrivers.
    'sound'         User created sound for the user to record to.
    'loop'          Boolean flag to tell the recording engine whether to continue recording to the provided sound from the start again, after it has reached the end.  If this is set to true the data will be continually be overwritten once every loop.  See remarks.

    [RETURN_VALUE]

    [REMARKS]    

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh360, PlayStation 3, Solaris, iPhone

    [SEE_ALSO]
    System::recordStop
]
*/
FMOD_RESULT F_API FMOD_System_RecordStart(FMOD_SYSTEM *system, FMOD_SOUND *sound, FMOD_BOOL loop)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->recordStart((FMOD::Sound *)sound, loop ? true : false);
}


/*
[API]
[
    [DESCRIPTION]
    Stops the recording engine from recording to the specified recording sound.

    [PARAMETERS]
    'id'           Enumerated driver ID.  This must be in a valid range delimited by System::getRecordNumDrivers.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh360, PlayStation 3, Solaris, iPhone

    [SEE_ALSO]
    System::recordStart
]
*/
FMOD_RESULT F_API FMOD_System_RecordStop(FMOD_SYSTEM *system)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->recordStop();
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the state of the FMOD recording API, ie if it is currently recording or not.

    [PARAMETERS]
    'id'            Enumerated driver ID.  This must be in a valid range delimited by System::getRecordNumDrivers.
    'recording'     Address of a variable to receive the current recording state.  True or non zero if the FMOD recording api is currently in the middle of recording, false or zero if the recording api is stopped / not recording.

    [RETURN_VALUE]

    [REMARKS]
    Recording can be started with System::recordStart.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh360, PlayStation 3, Solaris, iPhone

    [SEE_ALSO]
    System::recordStart
    System::recordStop
]
*/
FMOD_RESULT F_API FMOD_System_IsRecording(FMOD_SYSTEM *system, FMOD_BOOL *recording)
{
    FMOD_RESULT result;
    bool recording2;
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = _system->isRecording(&recording2);
    if (result == FMOD_OK)
    {
        if (recording)
        {
            *recording = recording2 ? 1 : 0;
        }
    }

    return result;
}


/*
[API]
[
    [DESCRIPTION]
    Geometry creation function.  This function will create a base geometry object which can then have polygons added to it. 

    [PARAMETERS]
    'maxpolygons'    Maximum number of polygons within this object.
    'maxvertices'    Maximum number of vertices within this object.
    'geometry'       Address of a variable to receive a newly created FMOD::Geometry object.

    [RETURN_VALUE]

    [REMARKS]
    Polygons can be added to a geometry object using Geometry::AddPolygon.<br>
    <br>
    A geometry object stores its list of polygons in a structure optimized for quick line intersection testing and efficient insertion and updating.<br>
    The structure works best with regularly shaped polygons with minimal overlap.<br>
    Many overlapping polygons, or clusters of long thin polygons may not be handled efficiently.<br>
    Axis aligned polygons are handled most efficiently.<br>
    <br>
    The same type of structure is used to optimize line intersection testing with multiple geometry objects.<br>
    <br>
    It is important to set the value of maxworldsize to an appropriate value using System::setGeometrySettings.<br>
    Objects or polygons outside the range of maxworldsize will not be handled efficiently.<br>
    Conversely, if maxworldsize is excessively large, the structure may lose precision and efficiency may drop.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::setGeometrySettings
    System::loadGeometry
    Geometry::AddPolygon
]
*/
FMOD_RESULT F_API FMOD_System_CreateGeometry(FMOD_SYSTEM *system, int maxpolygons, int maxvertices, FMOD_GEOMETRY **geometry)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->createGeometry(maxpolygons, maxvertices, (FMOD::Geometry **)geometry);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the maximum world size for the geometry engine for performance / precision reasons.

    [PARAMETERS]
    'maxworldsize'  Maximum size of the world from the centerpoint to the edge using the same units used in other 3D functions.

    [RETURN_VALUE]

    [REMARKS]
	Setting maxworldsize should be done first before creating any geometry.<br>
	It can be done any time afterwards but may be slow in this case.<br>
    <br>
    Objects or polygons outside the range of maxworldsize will not be handled efficiently.<br>
    Conversely, if maxworldsize is excessively large, the structure may loose precision and efficiency may drop.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createGeometry
    System::getGeometrySettings
]
*/
FMOD_RESULT F_API FMOD_System_SetGeometrySettings(FMOD_SYSTEM *system, float maxworldsize)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setGeometrySettings(maxworldsize);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the maximum world size for the geometry engine.

    [PARAMETERS]
    'maxworldsize'  Pointer to a float to receieve the maximum world size.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::setGeometrySettings
]
*/
FMOD_RESULT F_API FMOD_System_GetGeometrySettings(FMOD_SYSTEM *system, float *maxworldsize)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getGeometrySettings(maxworldsize);
}


/*
[API]
[
    [DESCRIPTION]
    Creates a geometry object from a block of memory which contains pre-saved geometry data, saved by Geometry::save.

    [PARAMETERS]
    'data'      Address of data containing pre-saved geometry data.
    'datasize'  Size of geometry data block in bytes.
    'geometry'  Address of a variable to receive a newly created FMOD::Geometry object.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::save
    System::createGeometry
]
*/
FMOD_RESULT F_API FMOD_System_LoadGeometry(FMOD_SYSTEM *system, const void *data, int datasize, FMOD_GEOMETRY **geometry)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->loadGeometry(data, datasize, (FMOD::Geometry **)geometry);
}


/*
[API]
[
    [DESCRIPTION]
    Set a proxy server to use for all subsequent internet connections.

    [PARAMETERS]
    'proxy'     The name of a proxy server in host:port format e.g. www.fmod.org:8888 (defaults to port 80 if no port is specified).

    [RETURN_VALUE]

    [REMARKS]
    Basic authentication is supported. To use it, this parameter must be in user:password@host:port format e.g. bob:sekrit123@www.fmod.org:8888
    Set this parameter to 0 / NULL if no proxy is required.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Solaris, iPhone

    [SEE_ALSO]
    System::getNetworkProxy
]
*/
FMOD_RESULT F_API FMOD_System_SetNetworkProxy(FMOD_SYSTEM *system, const char *proxy)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setNetworkProxy(proxy);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the URL of the proxy server used in internet streaming.

    [PARAMETERS]
    'proxy'     Address of a variable that receives the proxy server URL.
    'proxylen'  Size of the buffer in bytes to receive the string.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Solaris, iPhone

    [SEE_ALSO]
    System::setNetworkProxy
]
*/
FMOD_RESULT F_API FMOD_System_GetNetworkProxy(FMOD_SYSTEM *system, char *proxy, int proxylen)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getNetworkProxy(proxy, proxylen);
}


/*
[API]
[
    [DESCRIPTION]
    Set the timeout for network streams.

    [PARAMETERS]
    'timeout'   The timeout value in ms.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Solaris, iPhone

    [SEE_ALSO]
    System::getNetworkTimeout
]
*/
FMOD_RESULT F_API FMOD_System_SetNetworkTimeout(FMOD_SYSTEM *system, int timeout)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setNetworkTimeout(timeout);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieve the timeout value for network streams

    [PARAMETERS]
    'timeout'   The timeout value in ms.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Solaris, iPhone

    [SEE_ALSO]
]
*/
FMOD_RESULT F_API FMOD_System_GetNetworkTimeout(FMOD_SYSTEM *system, int *timeout)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getNetworkTimeout(timeout);
}


/*
[API]
[
    [DESCRIPTION]
    Sets a user value that the System object will store internally.  Can be retrieved with System::getUserData.

    [PARAMETERS]
    'userdata'      Address of user data that the user wishes stored within the System object.

    [RETURN_VALUE]

    [REMARKS]
    This function is primarily used in case the user wishes to 'attach' data to an FMOD object.<br>
    It can be useful if an FMOD callback passes an object of this type as a parameter, and the user does not know which object it is (if many of these types of objects exist).  Using System::getUserData would help in the identification of the object.
    
    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getUserData
]
*/
FMOD_RESULT F_API FMOD_System_SetUserData(FMOD_SYSTEM *system, void *userdata)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->setUserData(userdata);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the user value that that was set by calling the System::setUserData function.

    [PARAMETERS]
    'userdata'  Address of a pointer that receives the data specified with the System::setUserData function.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::setUserData
]
*/
FMOD_RESULT F_API FMOD_System_GetUserData(FMOD_SYSTEM *system, void **userdata)
{
    FMOD::System *_system = (FMOD::System *)system;

    if (!FMOD::gGlobal->gSystemHead->exists((FMOD::SystemI *)_system))
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _system->getUserData(userdata);
}



/*
[API]
[
	[DESCRIPTION]
    Frees a sound object.

	[PARAMETERS]
    'sound'    Pointer to an FMOD::Sound object.
 
	[RETURN_VALUE]

	[REMARKS]
    This will free the sound object and everything created under it.<br>
    <br>
    If this is a stream that is playing as a subsound of another parent stream, then if this is the currently playing subsound (be it a normal subsound playback, or as part of a sentence), the whole stream will stop.
    <br>
    Note - This function will block if it was opened with FMOD_NONBLOCKING and hasn't finished opening yet.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    System::createSound
    Sound::getSubSound
]
*/
FMOD_RESULT F_API FMOD_Sound_Release(FMOD_SOUND *sound)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->release();
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the parent System object that was used to create this object.

    [PARAMETERS]
    'system'        Address of a pointer that receives the System object.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createSound
]
*/
FMOD_RESULT F_API FMOD_Sound_GetSystemObject(FMOD_SOUND *sound, FMOD_SYSTEM **system)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getSystemObject((FMOD::System **)system);
}


/*
[API]
[
    [DESCRIPTION]
	Returns a pointer to the beginning of the sample data for a sound.<br>

    [PARAMETERS]
	'offset'	Offset in <i>bytes</i> to the position you want to lock in the sample buffer.
	'length'	Number of <i>bytes</i> you want to lock in the sample buffer.
	'ptr1'		Address of a pointer that will point to the first part of the locked data.
	'ptr2'		Address of a pointer that will point to the second part of the locked data.  This will be null if the data locked hasn't wrapped at the end of the buffer.
	'len1'		Length of data in <i>bytes</i> that was locked for ptr1
	'len2'		Length of data in <i>bytes</i> that was locked for ptr2.  This will be 0 if the data locked hasn't wrapped at the end of the buffer.

    [RETURN_VALUE]

    [REMARKS]
	You must always unlock the data again after you have finished with it, using Sound::unlock.<br>
    With this function you get access to the RAW audio data, for example 8, 16, 24 or 32bit PCM data, mono or stereo data, and on consoles, vag, xadpcm or gcadpcm compressed data.  You must take this into consideration when processing the data within the pointer.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::unlock
    System::createSound
]
*/
FMOD_RESULT F_API FMOD_Sound_Lock(FMOD_SOUND *sound, unsigned int offset, unsigned int length, void **ptr1, void **ptr2, unsigned int *len1, unsigned int *len2)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->lock(offset, length, ptr1, ptr2, len1, len2);
}


/*
[API]
[
    [DESCRIPTION]
	Releases previous sample data lock from Sound::lock.

    [PARAMETERS]
	'ptr1'		Pointer to the 1st locked portion of sample data, from Sound::lock.
	'ptr2'		Pointer to the 2nd locked portion of sample data, from Sound::lock.
	'len1'		Length of data in <i>bytes</i> that was locked for ptr1
	'len2'		Length of data in <i>bytes</i> that was locked for ptr2.  This will be 0 if the data locked hasn't wrapped at the end of the buffer.

    [RETURN_VALUE]
    Call this function after data has been read/written to from Sound::lock.  This function will do any post processing necessary and if needed, send it to sound ram.

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::lock
    System::createSound
]
*/
FMOD_RESULT F_API FMOD_Sound_Unlock(FMOD_SOUND *sound, void *ptr1, void *ptr2, unsigned int len1, unsigned int len2)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->unlock(ptr1, ptr2, len1, len2);
}


/*
[API]
[
    [DESCRIPTION]
	Sets a sounds's default attributes, so when it is played it uses these values without having to specify them later for each channel each time the sound is played.

    [PARAMETERS]
    'frequency'     Default playback frequency for the sound, in hz.  (ie 44100hz).
    'volume'        Default volume for the sound.  0.0 to 1.0.  0.0 = Silent, 1.0 = full volume.  Default = 1.0.
    'pan'           Default pan for the sound.  -1.0 to +1.0.  -1.0 = Full left, 0.0 = center, 1.0 = full right.  Default = 0.0.
    'priority'      Default priority for the sound when played on a channel.  0 to 256.  0 = most important, 256 = least important.  Default = 128.

    [RETURN_VALUE]

    [REMARKS]
    There are no 'ignore' values for these parameters.  Use Sound::getDefaults if you want to change only 1 and leave others unaltered.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::getDefaults
    System::playSound
    System::createSound
]
*/
FMOD_RESULT F_API FMOD_Sound_SetDefaults(FMOD_SOUND *sound, float frequency, float volume, float pan, int priority)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->setDefaults(frequency, volume, pan, priority);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a sound's default attributes for when it is played on a channel with System::playSound.

    [PARAMETERS]
    'frequency'     Address of a variable that receives the default frequency for the sound.  Optional.  Specify 0 or NULL to ignore. 
    'volume'        Address of a variable that receives the default volume for the sound.  Result will be from 0.0 to 1.0.  0.0 = Silent, 1.0 = full volume.  Default = 1.0.  Optional.  Specify 0 or NULL to ignore. 
    'pan'           Address of a variable that receives the default pan for the sound.  Result will be from -1.0 to +1.0.  -1.0 = Full left, 0.0 = center, 1.0 = full right.  Default = 0.0.  Optional.  Specify 0 or NULL to ignore. 
    'priority'      Address of a variable that receives the default priority for the sound when played on a channel.  Result will be from 0 to 256.  0 = most important, 256 = least important.  Default = 128.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::setDefaults
    System::createSound
    System::playSound
]
*/
FMOD_RESULT F_API FMOD_Sound_GetDefaults(FMOD_SOUND *sound, float *frequency, float *volume, float *pan, int *priority)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getDefaults(frequency, volume, pan, priority);
}


/*
[API]
[
    [DESCRIPTION]
    Changes the playback behaviour of a sound by allowing random variations to playback parameters to be set.

    [PARAMETERS]
    'frequencyvar'  Frequency variation in hz.   Frequency will play at its default frequency, plus or minus a random value within this range.  Default = 0.0.
    'volumevar'     Volume variation.  0.0 to 1.0.  Sound will play at its default volume, plus or minus a random value within this range.  Default = 0.0.
    'panvar'        Pan variation. 0.0 to 2.0.  Sound will play at its default pan, plus or minus a random value within this range.  Pan is from -1.0 to +1.0 normally so the range can be a maximum of 2.0 in this case.  Default = 0.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::getVariations
]
*/
FMOD_RESULT F_API FMOD_Sound_SetVariations(FMOD_SOUND *sound, float frequencyvar, float volumevar, float panvar)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->setVariations(frequencyvar, volumevar, panvar);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current playback behaviour variations of a sound.

    [PARAMETERS]
    'frequencyvar'  Address of a variable to receive the frequency variation in hz.   Frequency will play at its default frequency, plus or minus a random value within this range.  Default = 0.0.  Specify 0 or NULL to ignore. 
    'volumevar'     Address of a variable to receive the volume variation.  0.0 to 1.0.  Sound will play at its default volume, plus or minus a random value within this range.  Default = 0.0.  Specify 0 or NULL to ignore. 
    'panvar'        Address of a variable to receive the pan variation. 0.0 to 2.0.  Sound will play at its default pan, plus or minus a random value within this range.  Pan is from -1.0 to +1.0 normally so the range can be a maximum of 2.0 in this case.  Default = 0.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::setVariations
]
*/
FMOD_RESULT F_API FMOD_Sound_GetVariations(FMOD_SOUND *sound, float *frequencyvar, float *volumevar, float *panvar)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getVariations(frequencyvar, volumevar, panvar);
}


/*
[API]
[
    [DESCRIPTION]
	Sets the minimum and maximum audible distance for a sound.<br>
	<br>
	MinDistance is the minimum distance that the sound emitter will cease to continue growing louder at (as it approaches the listener).<br>
    Within the mindistance it stays at the constant loudest volume possible.  Outside of this mindistance it begins to attenuate.<br>
	MaxDistance is the distance a sound stops attenuating at.  Beyond this point it will stay at the volume it would be at maxdistance units from the listener and will not attenuate any more.<br>
	MinDistance is useful to give the impression that the sound is loud or soft in 3d space.  An example of this is a small quiet object, such as a bumblebee, which you could set a mindistance of to 0.1 for example, which would cause it to attenuate quickly and dissapear when only a few meters away from the listener.<br>
    Another example is a jumbo jet, which you could set to a mindistance of 100.0, which would keep the sound volume at max until the listener was 100 meters away, then it would be hundreds of meters more before it would fade out.<br>
	<br>
	In summary, increase the mindistance of a sound to make it 'louder' in a 3d world, and decrease it to make it 'quieter' in a 3d world.<br>
    Maxdistance is effectively obsolete unless you need the sound to stop fading out at a certain point.  Do not adjust this from the default if you dont need to.<br>
    Some people have the confusion that maxdistance is the point the sound will fade out to, this is not the case.<br>

    [PARAMETERS]
	'min'		The sound's minimum volume distance in "units".  See remarks for more on units.
	'max'		The sound's maximum volume distance in "units".  See remarks for more on units.

    [RETURN_VALUE]

    [REMARKS]
	A 'distance unit' is specified by System::set3DSettings.  By default this is set to meters which is a distance scale of 1.0.<br>
    See System::set3DSettings for more on this.<br>
	The default units for minimum and maximum distances are 1.0 and 10,000.0f.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::get3DMinMaxDistance
    Channel::set3DMinMaxDistance
    Channel::get3DMinMaxDistance
    System::set3DSettings
]
*/
FMOD_RESULT F_API FMOD_Sound_Set3DMinMaxDistance(FMOD_SOUND *sound, float min, float max)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->set3DMinMaxDistance(min, max);
}


/*
[API]
[
    [DESCRIPTION]
	Retrieve the minimum and maximum audible distance for a sound.

    [PARAMETERS]
	'min'		Pointer to value to be filled with the minimum volume distance for the sound.  See remarks for more on units.  Optional.  Specify 0 or NULL to ignore. 
	'max'		Pointer to value to be filled with the maximum volume distance for the sound.  See remarks for more on units.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]
	A 'distance unit' is specified by System::set3DSettings.  By default this is set to meters which is a distance scale of 1.0.<br>
    See System::set3DSettings for more on this.<br>
	The default units for minimum and maximum distances are 1.0 and 10,000.0f.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::set3DMinMaxDistance
    Channel::set3DMinMaxDistance
    Channel::get3DMinMaxDistance
    System::set3DSettings
]
*/
FMOD_RESULT F_API FMOD_Sound_Get3DMinMaxDistance(FMOD_SOUND *sound, float *min, float *max)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->get3DMinMaxDistance(min, max);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the inside and outside angles of the sound projection cone, as well as the volume of the sound outside the outside angle of the sound projection cone.

    [PARAMETERS]
    'insideconeangle'   Inside cone angle, in degrees, from 0 to 360. This is the angle within which the sound is at its normal volume.  Must not be greater than outsideconeangle.  Default = 360.
    'outsideconeangle'  Outside cone angle, in degrees, from 0 to 360. This is the angle outside of which the sound is at its outside volume.  Must not be less than insideconeangle.  Default = 360.
    'outsidevolume'     Cone outside volume, from 0 to 1.0.  Default = 1.0.
    
    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::get3DConeSettings
    Channel::set3DConeSettings
]
*/
FMOD_RESULT F_API FMOD_Sound_Set3DConeSettings(FMOD_SOUND *sound, float insideconeangle, float outsideconeangle, float outsidevolume)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->set3DConeSettings(insideconeangle, outsideconeangle, outsidevolume);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the inside and outside angles of the sound projection cone.  

    [PARAMETERS]
    'insideconeangle'   Address of a variable that receives the inside angle of the sound projection cone, in degrees. This is the angle within which the sound is at its normal volume.  Optional.  Specify 0 or NULL to ignore. 
    'outsideconeangle'  Address of a variable that receives the outside angle of the sound projection cone, in degrees. This is the angle outside of which the sound is at its outside volume.  Optional.  Specify 0 or NULL to ignore. 
    'outsidevolume'     Address of a variable that receives the cone outside volume for this sound.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::set3DConeSettings
    Channel::set3DConeSettings
]
*/
FMOD_RESULT F_API FMOD_Sound_Get3DConeSettings(FMOD_SOUND *sound, float *insideconeangle, float *outsideconeangle, float *outsidevolume)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->get3DConeSettings(insideconeangle, outsideconeangle, outsidevolume);
}


/*
[API]
[
    [DESCRIPTION]
    Point a sound to use a custom rolloff curve.  Must be used in conjunction with FMOD_3D_CUSTOMROLLOFF flag to be activated.

    [PARAMETERS]
    'points'        An array of FMOD_VECTOR structures where x = distance and y = volume from 0.0 to 1.0.  z should be set to 0.
    'numpoints'     The number of points in the array.

    [RETURN_VALUE]

    [REMARKS]
    <b>Note!</b>  This function does not duplicate the memory for the points internally.  The pointer you pass to FMOD must remain valid until there is no more use for it.<br>
    Do not free the memory while in use, or use a local variable that goes out of scope while in use.<br>
    <br>
    Points must be sorted by distance!  Passing an unsorted list to FMOD will result in an error.<br>
    <br>
    Set the points parameter to 0 or NULL to disable the points.  If FMOD_3D_CUSTOMROLLOFF is set and the rolloff curve is 0, FMOD will revert to logarithmic curve rolloff.<br>
    <br>
    Min and maxdistance are meaningless when FMOD_3D_CUSTOMROLLOFF is used and the values are ignored.<br>
    <br>
    Here is an example of a custom array of points.<br>
    <PRE>
    FMOD_VECTOR curve[3] = 
    {
    <ul>{ 0.0f,  1.0f, 0.0f },
        { 2.0f,  0.2f, 0.0f },
        { 20.0f, 0.0f, 0.0f }
    /ul>};
    </PRE>
    x represents the distance, y represents the volume. z is always 0.<br>
    Distances between points are linearly interpolated.<br>
    Note that after the highest distance specified, the volume in the last entry is used from that distance onwards.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_MODE
    FMOD_VECTOR
    Sound::get3DCustomRolloff
    Channel::set3DCustomRolloff
    Channel::get3DCustomRolloff
]
*/
FMOD_RESULT F_API FMOD_Sound_Set3DCustomRolloff(FMOD_SOUND *sound, FMOD_VECTOR *points, int numpoints)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->set3DCustomRolloff(points, numpoints);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a pointer to the sound's current custom rolloff curve.

    [PARAMETERS]
    'points'        Address of a variable to receive the pointer to the current custom rolloff point list.  Optional.  Specify 0 or NULL to ignore.
    'numpoints'     Address of a variable to receive the number of points int he current custom rolloff point list.  Optional.  Specify 0 or NULL to ignore.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_VECTOR
    Sound::set3DCustomRolloff
    Channel::set3DCustomRolloff
    Channel::get3DCustomRolloff
]
*/
FMOD_RESULT F_API FMOD_Sound_Get3DCustomRolloff(FMOD_SOUND *sound, FMOD_VECTOR **points, int *numpoints)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->get3DCustomRolloff(points, numpoints);
}


/*
[API]
[

    [DESCRIPTION]
    Assigns a sound as a 'subsound' of another sound.  A sound can contain other sounds.  The sound object that is issuing the command will be the 'parent' sound.

	[PARAMETERS]
    'index'         Index within the sound to set the new sound to as a 'subsound'.
    'subsound'      Sound object to set as a subsound within this sound.
 
	[RETURN_VALUE]

	[REMARKS]
    If a subsound is set which is a member of a sentence (See Sound::setSubSoundSentence), the loop points of the sentence will be reset to a loopstart of 0 and a loopend of the length of the sentence minus 1.  
    This means any sentence must have its loop points re-set by the user if this function is called in this case.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

	[SEE_ALSO]
    Sound::getNumSubSounds
    Sound::getSubSound
    Sound::setSubSoundSentence
]
*/
FMOD_RESULT F_API FMOD_Sound_SetSubSound(FMOD_SOUND *sound, int index, FMOD_SOUND *subsound)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->setSubSound(index, (FMOD::Sound *)subsound);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a handle to a Sound object that is contained within the parent sound.

    [PARAMETERS]
    'index'     Index of the subsound to retrieve within this sound.
    'subsound'  Address of a variable that receives the sound object specified.  

    [RETURN_VALUE]

    [REMARKS]
    If the sound is a stream and FMOD_NONBLOCKING was not used, then this call will perform a blocking seek/flush to the specified subsound.<br>
    <br>
    If FMOD_NONBLOCKING was used to open this sound and the sound is a stream, FMOD will do a non blocking seek/flush and set the state of the subsound to FMOD_OPENSTATE_SEEKING.<br>
    The sound won't be ready to be used in this case until the state of the sound becomes FMOD_OPENSTATE_READY (or FMOD_OPENSTATE_ERROR).

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::getNumSubSounds
    Sound::setSubSound
    System::createSound
    FMOD_MODE
    FMOD_OPENSTATE
]
*/
FMOD_RESULT F_API FMOD_Sound_GetSubSound(FMOD_SOUND *sound, int index, FMOD_SOUND **subsound)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getSubSound(index, (FMOD::Sound **)subsound);
}


/*
[API]
[
    [DESCRIPTION]
    For any sound that has subsounds, this function will determine the order of playback of these subsounds, and it will play / stitch together the subsounds without gaps.<br>
    This is a very useful feature for those users wanting to do seamless / gapless stream playback.  (ie sports commentary, gapless playback media players etc).

    [PARAMETERS]
    'subsoundlist'  Pointer to an array of indicies which are the subsounds to play.   One subsound can be included in this list multiple times if required.
    'numsubsounds'  Number of indicies inside the subsoundlist array.

    [RETURN_VALUE]

    [REMARKS]
    The currently playing subsound in a sentence can be found with Channel::getPosition and the timeunit FMOD_TIMEUNIT_SENTENCE_SUBSOUND.
    This is useful for displaying the currently playing track of a cd in a whole CD sentence for example.
    <br>
    For realtime stitching purposes, it is better to know the buffered ahead of time subsound index.  This can be done by adding the flag (using bitwise OR) FMOD_TIMEUNIT_BUFFERED.
    <br>
    If FMOD_ERR_SUBSOUND_MODE is returned, then FMOD_CREATECOMPRESSEDSAMPLE has been used on a child sound, but not on the parent sound, or vice versa.  Another cause of this is trying to mix a static sample with a streaming sound.  Mode bits to do with this must match.
    <br>
    A user can change subsounds that are not playing at the time, to do dynamic stitching/sentencing of sounds.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::playSound
    Sound::getSubSound
    Channel::getPosition
    FMOD_TIMEUNIT
]
*/
FMOD_RESULT F_API FMOD_Sound_SetSubSoundSentence(FMOD_SOUND *sound, int *subsoundlist, int numsubsounds)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->setSubSoundSentence(subsoundlist, numsubsounds);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the name of a sound.

    [PARAMETERS]
    'name'        Address of a variable that receives the name of the sound.
    'namelen'     Length in bytes of the target buffer to receieve the string.

    [RETURN_VALUE]

    [REMARKS]
    if FMOD_LOWMEM has been specified in System::createSound, this function will return "(null)".

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createSound
    FMOD_MODE
]
*/
FMOD_RESULT F_API FMOD_Sound_GetName(FMOD_SOUND *sound, char *name, int namelen)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getName(name, namelen);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the length of the sound using the specified time unit.

    [PARAMETERS]
    'length'        Address of a variable that receives the length of the sound.
    'lengthtype'    Time unit retrieve into the length parameter.  See FMOD_TIMEUNIT.

    [RETURN_VALUE]

    [REMARKS]
    Certain timeunits do not work depending on the file format.  For example FMOD_TIMEUNIT_MODORDER will not work with an mp3 file.<br>
    A length of 0xFFFFFFFF usually means it is of unlimited length, such as an internet radio stream or MOD/S3M/XM/IT file which may loop forever.<br>
    <br>
    <b>Warning!</b>  Using a VBR source that does not have an associated length information in milliseconds or pcm samples (such as MP3 or MOD/S3M/XM/IT) may return inaccurate lengths specify FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCM.<br>
    If you want FMOD to retrieve an accurate length it will have to pre-scan the file first in this case.  You will have to specify FMOD_ACCURATETIME when loading or opening the sound.  This means there is a slight delay as FMOD scans the whole file when loading the sound to find the right length in millseconds or pcm samples, and this also creates a seek table as it does this for seeking purposes.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_TIMEUNIT
]
*/
FMOD_RESULT F_API FMOD_Sound_GetLength(FMOD_SOUND *sound, unsigned int *length, FMOD_TIMEUNIT lengthtype)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getLength(length, lengthtype);
}


/*
[API]
[
    [DESCRIPTION]
    Returns format information about the sound.

    [PARAMETERS]
    'type'          Address of a variable that receives the type of sound.  Optional.  Specify 0 or NULL to ignore. 
    'format'        Address of a variable that receives the format of the sound.  Optional.  Specify 0 or NULL to ignore. 
    'channels'      Address of a variable that receives the number of channels for the sound.  Optional.  Specify 0 or NULL to ignore. 
    'bits'          Address of a variable that receives the number of bits per sample for the sound.  This corresponds to FMOD_SOUND_FORMAT but is provided as an integer format for convenience.  Hardware compressed formats such as VAG, XADPCM, GCADPCM that stay compressed in memory will return 0.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_SOUND_TYPE
    FMOD_SOUND_FORMAT
]
*/
FMOD_RESULT F_API FMOD_Sound_GetFormat(FMOD_SOUND *sound, FMOD_SOUND_TYPE *type, FMOD_SOUND_FORMAT *format, int *channels, int *bits)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getFormat(type, format, channels, bits);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of subsounds stored within a sound.

    [PARAMETERS]
    'numsubsounds'  Address of a variable that receives the number of subsounds stored within this sound.

    [RETURN_VALUE]

    [REMARKS]
    A format that has subsounds is usually a container format, such as FSB, DLS, MOD, S3M, XM, IT.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::getSubSound
]
*/
FMOD_RESULT F_API FMOD_Sound_GetNumSubSounds(FMOD_SOUND *sound, int *numsubsounds)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getNumSubSounds(numsubsounds);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of tags belonging to a sound.

    [PARAMETERS]
    'numtags'           Address of a variable that receives the number of tags in the sound.  Optional.  Specify 0 or NULL to ignore. 
    'numtagsupdated'    Address of a variable that receives the number of tags updated since this function was last called.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]
    The 'numtagsupdated' parameter can be used to check if any tags have been updated since last calling this function.<br>
    This can be useful to update tag fields, for example from internet based streams, such as shoutcast or icecast where the name of the song might change.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::getTag
]
*/
FMOD_RESULT F_API FMOD_Sound_GetNumTags(FMOD_SOUND *sound, int *numtags, int *numtagsupdated)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getNumTags(numtags, numtagsupdated);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a descriptive tag stored by the sound, to describe things like the song name, author etc.

    [PARAMETERS]
    'name'          Optional.  Name of a tag to retrieve.  Used to specify a particular tag if the user requires it.  To get all types of tags leave this parameter as 0 or NULL.
    'index'         Index into the tag list.   If the name parameter is null, then the index is the index into all tags present, from 0 up to but not including the numtags value returned by Sound::getNumTags.<br>If name is not null, then index is the index from 0 up to the number of tags with the same name.  For example if there were 2 tags with the name "TITLE" then you could use 0 and 1 to reference them.<br>Specifying an index of -1 returns new or updated tags.  This can be used to pull tags out as they are added or updated.
    'tag'           Pointer to a tag structure.  This will receive 

    [RETURN_VALUE]

    [REMARKS]
    The number of tags available can be found with Sound::getNumTags.
    The way to display or retrieve tags can be done in 3 different ways.<br>
    All tags can be continuously retrieved by looping from 0 to the numtags value in Sound::getNumTags - 1.  Updated tags will refresh automatically, and the 'updated' member of the FMOD_TAG structure will be set to true if a tag has been updated, due to something like a netstream changing the song name for example.<br>
    Tags could also be retrieved by specifying -1 as the index and only updating tags that are returned.  If all tags are retrieved and this function is called the function will return an error of FMOD_ERR_TAGNOTFOUND.<br>
    Specific tags can be retrieved by specifying a name parameter.  The index can be 0 based or -1 in the same fashion as described previously.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::getNumTags
    FMOD_TAG
]
*/
FMOD_RESULT F_API FMOD_Sound_GetTag(FMOD_SOUND *sound, const char *name, int index, FMOD_TAG *tag)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getTag(name, index, tag);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the state a sound is in after FMOD_NONBLOCKING has been used to open it, or the state of the streaming buffer.

    [PARAMETERS]
    'openstate'         Address of a variable that receives the open state of a sound.  Optional.  Specify 0 or NULL to ignore. 
    'percentbuffered'   Address of a variable that receives the percentage of the file buffer filled progress of a stream.    Optional.  Specify 0 or NULL to ignore. 
    'starving'          Address of a variable that receives the starving state of a sound.  If a stream has decoded more than the stream file buffer has ready for it, it will return TRUE.    Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]
    <b>Note:</b> The return value will be the result of the asynchronous sound create.  Use this to determine what happened if a sound failed to open.
    <b>Note:</b> Always check 'openstate' to determine the state of the sound. Do not assume that if this function returns FMOD_OK then the sound has finished loading.

    [REMARKS]
    When a sound is opened with FMOD_NONBLOCKING, it is opened and prepared in the background, or asynchronously.<br>
    This allows the main application to execute without stalling on audio loads.<br>
    This function will describe the state of the asynchronous load routine i.e. whether it has succeeded, failed or is still in progress.<br>
    <br>
    If 'starving' is true, then you will most likely hear a stuttering/repeating sound as the decode buffer loops on itself and replays old data.<br>
    Now that this variable exists, you can detect buffer underrun and use something like Channel::setMute to keep it quiet until it is not starving any more.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_OPENSTATE
    FMOD_MODE
    Channel::setMute
]
*/
FMOD_RESULT F_API FMOD_Sound_GetOpenState(FMOD_SOUND *sound, FMOD_OPENSTATE *openstate, unsigned int *percentbuffered, FMOD_BOOL *starving)
{
    FMOD_RESULT result;
    bool starving2;
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = _sound->getOpenState(openstate, percentbuffered, &starving2);
    if (result == FMOD_OK)
    {
        if (starving)
        {
            *starving = starving2 ? 1 : 0;
        }
    }

    return result;
}


/*
[API]
[
    [DESCRIPTION]
    Reads data from an opened sound to a specified pointer, using the FMOD codec created internally.<br>
    This can be used for decoding data offline in small pieces (or big pieces), rather than playing and capturing it, or loading the whole file at once and having to lock / unlock the data.

    [PARAMETERS]
    'buffer'        Address of a buffer that receives the decoded data from the sound.
    'lenbytes'      Number of bytes to read into the buffer.
    'read'          Number of bytes actually read.

    [RETURN_VALUE]

    [REMARKS]
    If too much data is read, it is possible FMOD_ERR_FILE_EOF will be returned, meaning it is out of data.  The 'read' parameter will reflect this by returning a smaller number of bytes read than was requested.<br>
    As a sound already reads the whole file then closes it upon calling System::createSound (unless System::createStream or FMOD_CREATESTREAM is used), this function will not work because the file is no longer open.<br>
    Note that opening a stream makes it read a chunk of data and this will advance the read cursor.  You need to either use FMOD_OPENONLY to stop the stream pre-buffering or call Sound::seekData to reset the read cursor.<br>
    If FMOD_OPENONLY flag is used when opening a sound, it will leave the file handle open, and FMOD will not read any data internally, so the read cursor will be at position 0.  This will allow the user to read the data from the start.<br>
    As noted previously, if a sound is opened as a stream and this function is called to read some data, then you will 'miss the start' of the sound.<br>
    Channel::setPosition will have the same result.  These function will flush the stream buffer and read in a chunk of audio internally.  This is why if you want to read from an absolute position you should use Sound::seekData and not the previously mentioned functions.<br>
    Remember if you are calling readData and seekData on a stream it is up to you to cope with the side effects that may occur.  Information functions such as Channel::getPosition may give misleading results.  Calling Channel::setPosition will reset and flush the stream, leading to the time values returning to their correct position.<br>
    <br>
    NOTE!  Thread safety.  If you call this from another stream callback, or any other thread besides the main thread, make sure to put a criticalsection around the call, and another around Sound::release in case the sound is still being read from while releasing.<br>
    This function is thread safe to call from a stream callback or different thread as long as it doesnt conflict with a call to Sound::release.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::seekData
    FMOD_MODE
    Channel::setPosition
    System::createSound
    System::createStream
    Sound::release
]
*/
FMOD_RESULT F_API FMOD_Sound_ReadData(FMOD_SOUND *sound, void *buffer, unsigned int lenbytes, unsigned int *read)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->readData(buffer, lenbytes, read);
}


/*
[API]
[
    [DESCRIPTION]
    Seeks a sound for use with data reading.  This is not a function to 'seek a sound' for normal use.  This is for use in conjunction with Sound::readData.

    [PARAMETERS]
    'pcm'       Offset to seek to in PCM samples.

    [RETURN_VALUE]

    [REMARKS]
    Note.  If a stream is opened and this function is called to read some data, then it will advance the internal file pointer, so data will be skipped if you play the stream.  Also calling position / time information functions will lead to misleading results.<br>
    A stream can be reset before playing by setting the position of the channel (ie using Channel::setPosition), which will make it seek, reset and flush the stream buffer.  This will make it sound correct again.<br>
    Remember if you are calling readData and seekData on a stream it is up to you to cope with the side effects that may occur.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::readData
    Channel::setPosition
]
*/
FMOD_RESULT F_API FMOD_Sound_SeekData(FMOD_SOUND *sound, unsigned int pcm)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->seekData(pcm);
}


/*
[API]
[
    [DESCRIPTION]
    Moves the sound from its existing SoundGroup to the specified sound group.

    [PARAMETERS]
    'soundgroup'        Address of a SoundGroup object to move the sound to.

    [RETURN_VALUE]

    [REMARKS]
    By default a sound is located in the 'master sound group'.  This can be retrieved with System::getMasterSoundGroup.<br>
    Putting a sound in a sound group (or just using the master sound group) allows for functionality like limiting a group of sounds to a certain number of playbacks (see SoundGroup::setMaxAudible).

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::getSoundGroup
    System::getMasterSoundGroup
    System::createSoundGroup
    SoundGroup::setMaxAudible
]
*/
FMOD_RESULT F_API FMOD_Sound_SetSoundGroup(FMOD_SOUND *sound, FMOD_SOUNDGROUP *soundgroup)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->setSoundGroup((FMOD::SoundGroup *)soundgroup);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the sound's current soundgroup.

    [PARAMETERS]
    'soundgroup'        Address of a pointer to a SoundGroup to receive the sound's current soundgroup.

    [RETURN_VALUE]
    By default a sound is located in the 'master sound group'.  This can be retrieved with System::getMasterSoundGroup.<br>

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::setSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_Sound_GetSoundGroup(FMOD_SOUND *sound, FMOD_SOUNDGROUP **soundgroup)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getSoundGroup((FMOD::SoundGroup **)soundgroup);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of sync points stored within a sound.  These points can be user generated or can come from a wav file with embedded markers.

    [PARAMETERS]
    'numsyncpoints'     Address of a variable to receive the number of sync points within this sound.

    [RETURN_VALUE]

    [REMARKS]
    In sound forge, a marker can be added a wave file by clicking on the timeline / ruler, and right clicking then selecting 'Insert Marker/Region'.<br>
    Riff wrapped mp3 files are also supported.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::getSyncPoint
    Sound::getSyncPointInfo
    Sound::addSyncPoint
    Sound::deleteSyncPoint
]
*/
FMOD_RESULT F_API FMOD_Sound_GetNumSyncPoints(FMOD_SOUND *sound, int *numsyncpoints)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getNumSyncPoints(numsyncpoints);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieve a handle to a sync point.  These points can be user generated or can come from a wav file with embedded markers.

    [PARAMETERS]
    'index'     Index of the sync point to retrieve.  Use Sound::getNumSyncPoints to determine the number of syncpoints.
    'point'     Address of a variable to receive a pointer to a sync point.

    [RETURN_VALUE]

    [REMARKS]
    In sound forge, a marker can be added a wave file by clicking on the timeline / ruler, and right clicking then selecting 'Insert Marker/Region'.<br>
    Riff wrapped mp3 files are also supported.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::getNumSyncPoints
    Sound::getSyncPointInfo
    Sound::addSyncPoint
    Sound::deleteSyncPoint
]
*/
FMOD_RESULT F_API FMOD_Sound_GetSyncPoint(FMOD_SOUND *sound, int index, FMOD_SYNCPOINT **point)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getSyncPoint(index, point);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves information on an embedded sync point.  These points can be user generated or can come from a wav file with embedded markers.

    [PARAMETERS]
    'point'         Pointer to a sync point.  Use Sound::getSyncPoint to retrieve a syncpoint or Sound::addSyncPoint to create one.
    'name'          Address of a variable to receive the name of the syncpoint.  Optional.  Specify 0 or NULL to ignore. 
    'namelen'       Size of buffer in bytes for name parameter.  FMOD will only copy to this point if the string is bigger than the buffer passed in.  Specify 0 to ignore name parameter.
    'offset'        Address of a variable to receieve the offset of the syncpoint in a format determined by the offsettype parameter.  Optional.  Specify 0 or NULL to ignore. 
    'offsettype'    A timeunit parameter to determine a desired format for the offset parameter.  For example the offset can be specified as pcm samples, or milliseconds.

    [RETURN_VALUE]

    [REMARKS]
    In sound forge, a marker can be added a wave file by clicking on the timeline / ruler, and right clicking then selecting 'Insert Marker/Region'.<br>
    Riff wrapped mp3 files are also supported.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::getNumSyncPoints
    Sound::getSyncPoint
    Sound::addSyncPoint
    Sound::deleteSyncPoint
]
*/
FMOD_RESULT F_API FMOD_Sound_GetSyncPointInfo(FMOD_SOUND *sound, FMOD_SYNCPOINT *point, char *name, int namelen, unsigned int *offset, FMOD_TIMEUNIT offsettype)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getSyncPointInfo(point, name, namelen, offset, offsettype);
}


/*
[API]
[
    [DESCRIPTION]
    Adds a sync point at a specific time within the sound.  These points can be user generated or can come from a wav file with embedded markers.

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]
    In sound forge, a marker can be added a wave file by clicking on the timeline / ruler, and right clicking then selecting 'Insert Marker/Region'.<br>
    Riff wrapped mp3 files are also supported.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::getNumSyncPoints
    Sound::getSyncPoint
    Sound::getSyncPointInfo
    Sound::deleteSyncPoint
]
*/
FMOD_RESULT F_API FMOD_Sound_AddSyncPoint(FMOD_SOUND *sound, unsigned int offset, FMOD_TIMEUNIT offsettype, const char *name, FMOD_SYNCPOINT **point)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->addSyncPoint(offset, offsettype, name, point);
}


/*
[API]
[
    [DESCRIPTION]
    Deletes a syncpoint within the sound.  These points can be user generated or can come from a wav file with embedded markers.

    [PARAMETERS]
    'point'     Address of an FMOD_SYNCPOINT object.

    [RETURN_VALUE]

    [REMARKS]
    In sound forge, a marker can be added a wave file by clicking on the timeline / ruler, and right clicking then selecting 'Insert Marker/Region'.<br>
    Riff wrapped mp3 files are also supported.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::getNumSyncPoints
    Sound::getSyncPoint
    Sound::getSyncPointInfo
    Sound::addSyncPoint
]
*/
FMOD_RESULT F_API FMOD_Sound_DeleteSyncPoint(FMOD_SOUND *sound, FMOD_SYNCPOINT *point)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->deleteSyncPoint(point);
}


/*
[API]
[
    [DESCRIPTION]
    Sets or alters the mode of a sound.

    [PARAMETERS]
    'mode'      Mode bits to set.

    [RETURN_VALUE]

    [REMARKS]
    When calling this function, note that it will only take effect when the sound is played again with System::playSound.  Consider this mode the 'default mode' for when the sound plays, not a mode that will suddenly change all currently playing instances of this sound.
    <br>
    Flags supported:<br>
    FMOD_LOOP_OFF<br>
    FMOD_LOOP_NORMAL<br>
    FMOD_LOOP_BIDI (only works with sounds created with FMOD_SOFTWARE.  Otherwise it will behave as FMOD_LOOP_NORMAL) <br>
    FMOD_3D_HEADRELATIVE<br>
    FMOD_3D_WORLDRELATIVE<br>
    FMOD_2D (see notes for win32 hardware voices)<br>
    FMOD_3D (see notes for win32 hardware voices)<br>
    FMOD_3D_LOGROLLOFF<br>
    FMOD_3D_LINEARROLLOFF<br>
    FMOD_3D_CUSTOMROLLOFF<br>
    FMOD_3D_IGNOREGEOMETRY<br>
    FMOD_DONTRESTOREVIRTUAL<br>
    <br>
    <u>Issues with streamed audio.  (Sounds created with with System::createStream or FMOD_CREATESTREAM). </u>
    When changing the loop mode, sounds created with System::createStream or FMOD_CREATESTREAM may already have been pre-buffered and executed their loop logic ahead of time, before this call was even made.<br>
    This is dependant on the size of the sound versus the size of the stream <i>decode</i> buffer.  See FMOD_CREATESOUNDEXINFO.<br>
    If this happens, you may need to reflush the stream buffer.  To do this, you can call Channel::setPosition which forces a reflush of the stream buffer.<br>
    Note this will usually only happen if you have sounds or looppoints that are smaller than the stream decode buffer size.  Otherwise you will not normally encounter any problems.<br>
    <br>
    <b>Win32</b> FMOD_HARDWARE note.  Under DirectSound, you cannot change the mode of a sound between FMOD_2D and FMOD_3D.  If this is a problem create the sound as FMOD_3D initially, and use FMOD_3D_HEADRELATIVE and FMOD_3D_WORLDRELATIVE.  Alternatively just use FMOD_SOFTWARE.<br>
    <br>
    If FMOD_3D_IGNOREGEOMETRY is not specified, the flag will be cleared if it was specified previously.<br>


    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_MODE
    Sound::getMode
    System::setStreamBufferSize
    System::playSound
    System::createStream
    Channel::setPosition
    FMOD_CREATESOUNDEXINFO
]
*/
FMOD_RESULT F_API FMOD_Sound_SetMode(FMOD_SOUND *sound, FMOD_MODE mode)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->setMode(mode);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the mode bits set by the codec and the user when opening the sound.

    [PARAMETERS]
    'mode'      Address of a variable that receives the current mode for this sound.

    [RETURN_VALUE]

    [REMARKS]    

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::setMode
    System::createSound
    Channel::setMode
    Channel::getMode
]
*/
FMOD_RESULT F_API FMOD_Sound_GetMode(FMOD_SOUND *sound, FMOD_MODE *mode)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getMode(mode);
}


/*
[API]
[
    [DESCRIPTION]
    Sets a sound, by default, to loop a specified number of times before stopping if its mode is set to FMOD_LOOP_NORMAL or FMOD_LOOP_BIDI.

    [PARAMETERS]
    'loopcount'     Number of times to loop before stopping.  0 = oneshot.  1 = loop once then stop.  -1 = loop forever.  Default = -1

    [RETURN_VALUE]

    [REMARKS]
    This function does not affect FMOD_HARDWARE based sounds that are not streamable.<br>
    FMOD_SOFTWARE based sounds or any type of sound created with System::CreateStream or FMOD_CREATESTREAM will support this function.<br>
    <br>
    <u>Issues with streamed audio.  (Sounds created with with System::createStream or FMOD_CREATESTREAM). </u>
    When changing the loop count, sounds created with System::createStream or FMOD_CREATESTREAM may already have been pre-buffered and executed their loop logic ahead of time, before this call was even made.<br>
    This is dependant on the size of the sound versus the size of the stream <i>decode</i> buffer.  See FMOD_CREATESOUNDEXINFO.<br>
    If this happens, you may need to reflush the stream buffer.  To do this, you can call Channel::setPosition which forces a reflush of the stream buffer.<br>
    Note this will usually only happen if you have sounds or looppoints that are smaller than the stream decode buffer size.  Otherwise you will not normally encounter any problems.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::getLoopCount
    System::setStreamBufferSize
    FMOD_CREATESOUNDEXINFO
]
*/
FMOD_RESULT F_API FMOD_Sound_SetLoopCount(FMOD_SOUND *sound, int loopcount)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->setLoopCount(loopcount);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current loop count value for the specified sound.

    [PARAMETERS]
    'loopcount'     Address of a variable that receives the number of times a sound will loop by default before stopping.  0 = oneshot.  1 = loop once then stop.  -1 = loop forever.  Default = -1

    [RETURN_VALUE]

    [REMARKS]
    Unlike the channel loop count function, this function simply returns the value set with Sound::setLoopCount.  It does not decrement as it plays (especially seeing as one sound can be played multiple times).

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::setLoopCount
]
*/
FMOD_RESULT F_API FMOD_Sound_GetLoopCount(FMOD_SOUND *sound, int *loopcount)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getLoopCount(loopcount);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the loop points within a sound.

    [PARAMETERS]
    'loopstart'         The loop start point.  This point in time is played, so it is inclusive.
    'loopstarttype'     The time format used for the loop start point.  See FMOD_TIMEUNIT.
    'loopend'           The loop end point.  This point in time is played, so it is inclusive.
    'loopendtype'       The time format used for the loop end point.  See FMOD_TIMEUNIT.

    [RETURN_VALUE]

    [REMARKS]
    Not supported by static sounds created with FMOD_HARDWARE.<br>
    Supported by sounds created with FMOD_SOFTWARE, or sounds of any type (hardware or software) created with System::createStream or FMOD_CREATESTREAM.
    <br>
    If a sound was 44100 samples long and you wanted to loop the whole sound, loopstart would be 0, and loopend would be 44099, <br>not</b> 44100.  You wouldn't use milliseconds in this case because they are not sample accurate.<br>
    If loop end is smaller or equal to loop start, it will result in an error.<br>
    If loop start or loop end is larger than the length of the sound, it will result in an error.<br>
    <br>
    <u>Issues with streamed audio.  (Sounds created with with System::createStream or FMOD_CREATESTREAM). </u><br>
    When changing the loop points, sounds created with System::createStream or FMOD_CREATESTREAM may already have been pre-buffered and executed their loop logic ahead of time, before this call was even made.<br>
    This is dependant on the size of the sound versus the size of the stream <i>decode</i> buffer.  See FMOD_CREATESOUNDEXINFO.<br>
    If this happens, you may need to reflush the stream buffer.  To do this, you can call Channel::setPosition which forces a reflush of the stream buffer.<br>
    Note this will usually only happen if you have sounds or looppoints that are smaller than the stream decode buffer size.  Otherwise you will not normally encounter any problems.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_TIMEUNIT
    FMOD_MODE
    Sound::getLoopPoints
    Sound::setLoopCount
    System::createStream
    System::setStreamBufferSize
    Channel::setPosition
    FMOD_CREATESOUNDEXINFO
]
*/
FMOD_RESULT F_API FMOD_Sound_SetLoopPoints(FMOD_SOUND *sound, unsigned int loopstart, FMOD_TIMEUNIT loopstarttype, unsigned int loopend, FMOD_TIMEUNIT loopendtype)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->setLoopPoints(loopstart, loopstarttype, loopend, loopendtype);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the loop points for a sound.

    [PARAMETERS]
    'loopstart'         Address of a variable to receive the loop start point.  This point in time is played, so it is inclusive.  Optional.  Specify 0 or NULL to ignore. 
    'loopstarttype'     The time format used for the returned loop start point.  See FMOD_TIMEUNIT.
    'loopend'           Address of a variable to receive the loop end point.  This point in time is played, so it is inclusive.  Optional.  Specify 0 or NULL to ignore. 
    'loopendtype'       The time format used for the returned loop end point.  See FMOD_TIMEUNIT.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_TIMEUNIT
    Sound::setLoopPoints
]
*/
FMOD_RESULT F_API FMOD_Sound_GetLoopPoints(FMOD_SOUND *sound, unsigned int *loopstart, FMOD_TIMEUNIT loopstarttype, unsigned int *loopend, FMOD_TIMEUNIT loopendtype)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getLoopPoints(loopstart, loopstarttype, loopend, loopendtype);
}


/*
[API]
[
    [DESCRIPTION]
    Sets a user value that the Sound object will store internally.  Can be retrieved with Sound::getUserData.

    [PARAMETERS]
    'userdata'      Address of user data that the user wishes stored within the Sound object.

    [RETURN_VALUE]

    [REMARKS]
    This function is primarily used in case the user wishes to 'attach' data to an FMOD object.<br>
    It can be useful if an FMOD callback passes an object of this type as a parameter, and the user does not know which object it is (if many of these types of objects exist).  Using Sound::getUserData would help in the identification of the object.
    
    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::getUserData
]
*/
FMOD_RESULT F_API FMOD_Sound_SetUserData(FMOD_SOUND *sound, void *userdata)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->setUserData(userdata);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the user value that that was set by calling the Sound::setUserData function.

    [PARAMETERS]
    'userdata'  Address of a pointer that receives the data specified with the Sound::setUserData function.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Sound::setUserData
]
*/
FMOD_RESULT F_API FMOD_Sound_GetUserData(FMOD_SOUND *sound, void **userdata)
{
    FMOD::Sound *_sound = (FMOD::Sound *)sound;

    if (!_sound)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _sound->getUserData(userdata);
}



/*
[API]
[
    [DESCRIPTION]
    Retrieves the parent System object that was used to create this object.

    [PARAMETERS]
    'system'        Address of a variable that receives the System object.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::playSound
]
*/
FMOD_RESULT F_API FMOD_Channel_GetSystemObject(FMOD_CHANNEL *channel, FMOD_SYSTEM **system)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getSystemObject((FMOD::System **)system);
}


/*
[API]
[
    [DESCRIPTION]
    Stops the channel from playing.  Makes it available for re-use by the priority system.

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]
    
    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::playSound
]
*/
FMOD_RESULT F_API FMOD_Channel_Stop(FMOD_CHANNEL *channel)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->stop();
}


/*
[API]
[
    [DESCRIPTION]
    Sets the paused state of the channel.

    [PARAMETERS]
    'paused'        Paused state to set.  true = channel is paused.  false = channel is unpaused.

    [RETURN_VALUE]

    [REMARKS]
    If a channel belongs to a paused channelgroup, it will stay paused regardless of the channel pause state.  The channel pause state will still be reflected internally though, ie Channel::getPaused will still return the value you set.  If the channelgroup has paused set to false, this function will become effective again.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getPaused
    ChannelGroup::setPaused
]
*/
FMOD_RESULT F_API FMOD_Channel_SetPaused(FMOD_CHANNEL *channel, FMOD_BOOL paused)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setPaused(paused ? true : false);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the paused state of the channel.

    [PARAMETERS]
    'paused'        Address of a variable that receives the current paused state.  true = the sound is paused.  false = the sound is not paused.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setPaused
]
*/
FMOD_RESULT F_API FMOD_Channel_GetPaused(FMOD_CHANNEL *channel, FMOD_BOOL *paused)
{
    FMOD_RESULT result;
    bool paused2;
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = _channel->getPaused(&paused2);
    if (paused)
        {
            *paused = paused2 ? 1 : 0;
        }

    return result;
}


/*
[API]
[
    [DESCRIPTION]
    Sets the volume for the channel linearly.

    [PARAMETERS]
    'volume'        A linear volume level, from 0.0 to 1.0 inclusive.  0.0 = silent, 1.0 = full volume.  Default = 1.0.

    [RETURN_VALUE]

    [REMARKS]
    When a sound is played, it plays at the default volume of the sound which can be set by Sound::setDefaults.<br>
    For most file formats, the volume is determined by the audio format.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getVolume
    ChannelGroup::setVolume
    Sound::setDefaults
]
*/
FMOD_RESULT F_API FMOD_Channel_SetVolume(FMOD_CHANNEL *channel, float volume)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setVolume(volume);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the volume level for the channel.

    [PARAMETERS]
    'volume'        Address of a variable to receive the channel volume level, from 0.0 to 1.0 inclusive.  0.0 = silent, 1.0 = full volume.  Default = 1.0.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setVolume
]
*/
FMOD_RESULT F_API FMOD_Channel_GetVolume(FMOD_CHANNEL *channel, float *volume)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getVolume(volume);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the channel's frequency or playback rate, in HZ.

    [PARAMETERS]
    'frequency'        A frequency value in HZ.  This value can also be negative to play the sound backwards (negative frequencies allowed with FMOD_SOFTWARE based non-stream sounds only).   DirectSound hardware voices have limited frequency range on some soundcards.  Please see remarks for more on this.

    [RETURN_VALUE]

    [REMARKS]
    When a sound is played, it plays at the default frequency of the sound which can be set by Sound::setDefaults.<br>
    For most file formats, the volume is determined by the audio format.<br>
    <br>
    <u>Frequency limitations for sounds created with FMOD_HARDWARE in DirectSound.</u><br>
    Every hardware device has a minimum and maximum frequency.  This means setting the frequency above the maximum and below the minimum will have no effect.<br>
    FMOD clamps frequencies to these values when playing back on hardware, so if you are setting the frequency outside of this range, the frequency will stay at either the minimum or maximum.<br>
    Note that FMOD_SOFTWARE based sounds do not have this limitation.<br>
    To find out the minimum and maximum value before initializing FMOD (maybe to decide whether to use a different soundcard, output mode, or drop back fully to software mixing), you can use the System::getDriverCaps function.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getFrequency
    System::getDriverCaps
    Sound::setDefaults
]
*/
FMOD_RESULT F_API FMOD_Channel_SetFrequency(FMOD_CHANNEL *channel, float frequency)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setFrequency(frequency);
}


/*
[API]
[
    [DESCRIPTION]
    Returns the frequency in HZ of the channel.

    [PARAMETERS]
    'frequency'     Address of a variable that receives the current frequency of the channel in HZ.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setFrequency
]
*/
FMOD_RESULT F_API FMOD_Channel_GetFrequency(FMOD_CHANNEL *channel, float *frequency)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getFrequency(frequency);
}


/*
[API]
[
    [DESCRIPTION]
	Sets a channels pan position linearly.

    [PARAMETERS]
	'pan'     A left/right pan level, from -1.0 to 1.0 inclusive.  -1.0 = Full left, 0.0 = center, 1.0 = full right.  Default = 0.0.

    [RETURN_VALUE]

    [REMARKS]
    This function only works on sounds created with FMOD_2D.  3D sounds are not pannable and will return FMOD_ERR_NEEDS2D.<br>
    <br>
    Only sounds that are mono or stereo can be panned.  Multichannel sounds (ie >2 channels) cannot be panned.<br>
    Mono sounds are panned from left to right using constant power panning (non linear fade).  This means when pan = 0.0, the balance for the sound in each speaker is 71% left and 71% right, not 50% left and 50% right.  This gives (audibly) smoother pans.<br>
    Stereo sounds heave each left/right value faded up and down according to the specified pan position.  This means when pan = 0.0, the balance for the sound in each speaker is 100% left and 100% right.  When pan = -1.0, only the left channel of the stereo sound is audible, when pan = 1.0, only the right channel of the stereo sound is audible.<br>
    <br>
    Panning does not work if the speaker mode is FMOD_SPEAKERMODE_RAW.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getPan
    FMOD_SPEAKERMODE
]
*/
FMOD_RESULT F_API FMOD_Channel_SetPan(FMOD_CHANNEL *channel, float pan)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setPan(pan);
}


/*
[API]
[
    [DESCRIPTION]
    Returns the pan position of the channel.

    [PARAMETERS]
	'pan'     Address of a variable to receive the left/right pan level for the channel, from -1.0 to 1.0 inclusive.  -1.0 = Full left, 1.0 = full right.  Default = 0.0.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setPan
]
*/
FMOD_RESULT F_API FMOD_Channel_GetPan(FMOD_CHANNEL *channel, float *pan)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getPan(pan);
}


/*
[API]
[
    [DESCRIPTION]
    Sets an end delay for a sound (so that dsp can continue to process the finished sound), set the start of the sound according to the global DSP clock value which represents the time in the mixer timeline.

    [PARAMETERS]
    'delaytype'     See FMOD_DELAYTYPE.  This determines what delayhi and delaylo will represent.
    'delayhi'       Top (most significant) 32 bits of a 64bit number representing the time.
    'delaylo'       Bottom (least significant) 32 bits of a 64bit number representing the time.

    [RETURN_VALUE]

    [REMARKS]
    <b>Note!</b>  Only works with sounds created with FMOD_SOFTWARE.<br>
    <br>
    Using FMOD_DELAYTYPE_END_MS : Setting a delay after a sound ends is sometimes useful to prolong the sound, even though it has stopped, so that DSP effects can trail out, or render the last of their tails. (for example an echo or reverb effect).  Remember the delayhi parameter is the only parameter used here, and it is representing millseconds.<br>
    Using FMOD_DELAYTYPE_DSPCLOCK_START : <b>Note!<b>  Works with sounds created with FMOD_SOFTWARE only.  This allows a sound to be played in the future on an exact sample accurate boundary or DSP clock value.  This can be used for synchronizing sounds to start at an exact time in the overall timeline.<br>
    <br>
    What is the 'DSP clock'?  The DSP clock represents the output stream to the soundcard, and is incremented by the output rate every second (though of course with much finer granularity than this).  So if your output rate is 48khz, the DSP clock will increment by 48000 per second.<br>
    The hi and lo values represent this 64bit number, with the delaylo representing the least significant 32bits and the delayhi value representing the most significant 32bits.<br>
    <br>
    Use FMOD_64BIT_ADD or FMOD_64BIT_SUB to add a hi/lo combination together and cope with wraparound.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_DELAYTYPE
    Channel::getDelay
    Channel::isPlaying
]
*/
FMOD_RESULT F_API FMOD_Channel_SetDelay(FMOD_CHANNEL *channel, unsigned int startdelay, unsigned int enddelay)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setDelay(startdelay, enddelay);
}


/*
[API]
[
    [DESCRIPTION]
    Gets the currently set delay values.

    [PARAMETERS]
    'delaytype'     See FMOD_DELAYTYPE.  This determines what delayhi and delaylo will represent.
    'delayhi'       Address of a variable to receive the top (most significant) 32 bits of a 64bit number representing the time.
    'delaylo'       Address of a variable to receive the bottom (least significant) 32 bits of a 64bit number representing the time.

    [RETURN_VALUE]

    [REMARKS]
    <b>Note!</b>  Only works with sounds created with FMOD_SOFTWARE.<br>
    <br>
    If FMOD_DELAYTYPE_DSPCLOCK_START is used, this will be the value of the DSP clock time at the time System::playSound was called, if the user has not called Channel::setDelay..
    <br>
    What is the 'dsp clock'?  The DSP clock represents the output stream to the soundcard, and is incremented by the output rate every second (though of course with much finer granularity than this).  So if your output rate is 48khz, the DSP clock will increment by 48000 per second.<br>
    The hi and lo values represent this 64bit number, with the delaylo representing the least significant 32bits and the delayhi value representing the most significant 32bits.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_DELAYTYPE
    Channel::setDelay
    System::playSound
]
*/
FMOD_RESULT F_API FMOD_Channel_GetDelay(FMOD_CHANNEL *channel, unsigned int *startdelay, unsigned int *enddelay)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getDelay(startdelay, enddelay);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the channel's speaker volume levels for each speaker individually.

    [PARAMETERS]
    'frontleft'     Volume level for this channel in the front left speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = normal volume, 5.0 = 5x amplification.
    'frontright'    Volume level for this channel in the front right speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = normal volume, up to 5.0 = 5x amplification.
    'center'        Volume level for this channel in the center speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = normal volume, 5.0 = 5x amplification.
    'lfe'           Volume level for this channel in the subwoofer speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = normal volume, 5.0 = 5x amplification.
    'backleft'      Volume level for this channel in the back left speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = normal volume, 5.0 = 5x amplification.
    'backright'     Volume level for this channel in the back right speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = normal volume, 5.0 = 5x amplification.
    'sideleft'      Volume level for this channel in the side left speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = normal volume, 5.0 = 5x amplification.
    'sideright'     Volume level for this channel in the side right speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = normal volume, 5.0 = 5x amplification.


    [RETURN_VALUE]

    [REMARKS]
    This function only fully works on sounds created with FMOD_2D and FMOD_SOFTWARE.  FMOD_3D based sounds only allow setting of LFE channel, as all other speaker levels are calculated by FMOD's 3D engine.<br>
    <br>
    Speakers specified that don't exist will simply be ignored.<br>
    <br>
    For more advanced speaker control, including sending the different channels of a stereo sound to arbitrary speakers, see Channel::setSpeakerLevels.<br>
    <br>
    This function allows amplification!  You can go up to 5 times the volume of a normal sound, but warning this may cause clipping/distortion!  Useful for LFE boosting.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getSpeakerMix
    Channel::setSpeakerLevels
    FMOD_SPEAKERMODE
]
*/
FMOD_RESULT F_API FMOD_Channel_SetSpeakerMix(FMOD_CHANNEL *channel, float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setSpeakerMix(frontleft, frontright, center, lfe, backleft, backright, sideleft, sideright);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the channel's speaker volume levels for each speaker individually.

    [PARAMETERS]
    'frontleft'     Address of a variable to receive the current volume level for this channel in the front left speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume, up to 5.0 = 5x amplification.
    'frontright'    Address of a variable to receive the current volume level for this channel in the front right speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume, up to 5.0 = 5x amplification.
    'center'        Address of a variable to receive the current volume level for this channel in the center speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume, up to 5.0 = 5x amplification.
    'lfe'           Address of a variable to receive the current volume level for this channel in the subwoofer speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume, up to 5.0 = 5x amplification.
    'backleft'      Address of a variable to receive the current volume level for this channel in the back left speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume, up to 5.0 = 5x amplification.
    'backright'     Address of a variable to receive the current volume level for this channel in the back right speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume, up to 5.0 = 5x amplification.
    'sideleft'      Address of a variable to receive the current volume level for this channel in the side left speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume, up to 5.0 = 5x amplification.
    'sideright'     Address of a variable to receive the current volume level for this channel in the side right speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume, up to 5.0 = 5x amplification.

    [RETURN_VALUE]

    [REMARKS]
    For 3D sound, the values set here are not representative of the 3d mix.  For 3D sound this function is mainly for retrieving the LFE value if it was set by the user.
    This function is not affected by Channel::setSpeakerLevels.  This function only returns the levels set by Channel::setSpeakerMix.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setSpeakerMix
    Channel::setSpeakerLevels
]
*/
FMOD_RESULT F_API FMOD_Channel_GetSpeakerMix(FMOD_CHANNEL *channel, float *frontleft, float *frontright, float *center, float *lfe, float *backleft, float *backright, float *sideleft, float *sideright)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getSpeakerMix(frontleft, frontright, center, lfe, backleft, backright, sideleft, sideright);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the incoming sound levels for a particular speaker.  This is a literal setting of channel's internal pan matrix and does NOT perform downmixing depending on speaker mode.

    [PARAMETERS]
    'speaker'       The target speaker to modify the levels for.  This can be cast to an integer if you are using FMOD_SPEAKERMODE_RAW and want to access up to 15 speakers (output channels).
    'levels'        An array of floating point numbers from 0.0 to 1.0 representing the volume of each input channel of a sound.  See remarks for more.
    'numlevels'     The number of floats within the levels parameter being passed to this function.  In the case of the above mono or stereo sound, 1 or 2 could be used respectively.  If the sound being played was an 8 channel multichannel sound then 8 levels would be used.

    [RETURN_VALUE]

    [REMARKS]
    As an example of usage of this function, if the sound played on this speaker was mono, only 1 level would be needed. <br>
    If the sound played on this channel was stereo, then an array of 2 floats could be specified.  For example { 0, 1 } on a channel playing a stereo sound would mute the left part of the stereo sound when it is played on this speaker.<br>
    <br>
    <b>Note!</b>  This function requires the sound be created with FMOD_SOFTWARE in FMOD_OUTPUTTYPE_DSOUND.  Directsound hardware voices are not capable of this functionality.<br>
    <br>
    Only speakers that are usable with the current speaker mode will be accepted.  Anything else will return FMOD_ERR_INVALID_SPEAKER.<br>
    <br>
    When using FMOD_SPEAKERMODE_MONO it is preferable to use the alias FMOD_SPEAKER_MONO (or index 0), as FMOD_SPEAKER_FRONT_LEFT doesn't really make sense in that setting.<br>
    When using FMOD_SPEAKERMODE_RAW, the 'speaker' parameter can be cast to an integer and used as a raw speaker index, disregarding FMOD's speaker mappings.<br>
    <br>
    <b>Warning.</b>  This function will allocate memory for the speaker level matrix and attach it to the channel.  If you prefer not to have a dynamic memory allocation done at this point use Channel::setSpeakerMix instead.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getSpeakerLevels
    Channel::setSpeakerMix
    FMOD_SPEAKERMODE
    FMOD_SPEAKER
    FMOD_OUTPUTTYPE
]
*/
FMOD_RESULT F_API FMOD_Channel_SetSpeakerLevels(FMOD_CHANNEL *channel, FMOD_SPEAKER speaker, float *levels, int numlevels)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setSpeakerLevels(speaker, levels, numlevels);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current pan matrix level settings from Channel::setSpeakerLevels.

    [PARAMETERS]
    'speaker'       The speaker id to get the levels for.  This can be cast to an integer if you are using a device with more than the pre-defined speaker range.
    'levels'        Address of a variable that receives the current levels for the channel.  This is an array of floating point values.  The destination array size can be specified with the numlevels parameter.
    'numlevels'     Number of floats in the destination array.

    [RETURN_VALUE]

    [REMARKS]
    This function does not return level values reflecting Channel::setVolume.  Volume is a separate scalar to the pan matrix levels.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setSpeakerLevels
    Channel::setPan
    Channel::setVolume
]
*/
FMOD_RESULT F_API FMOD_Channel_GetSpeakerLevels(FMOD_CHANNEL *channel, FMOD_SPEAKER speaker, float *levels, int numlevels)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getSpeakerLevels(speaker, levels, numlevels);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the incoming levels in a sound.  This means if you have a multichannel sound you can turn channels on and off.<br>
    A mono sound has 1 input channel, a stereo has 2, etc.  It depends on what type of sound is playing on the channel at the time.

    [PARAMETERS]
    'levels'     Array of float volume levels, from 0.0 to 1.0.  These represent the incoming channels for the sound playing on the channel at the time.
    'numlevels'  Number of floats in the array.   Maximum = the maximum number of input channels specified in System::setSoftwareFormat.

    [RETURN_VALUE]

    [REMARKS]
    <b>Note!</b>  This function requires the sound be created with FMOD_SOFTWARE in FMOD_OUTPUTTYPE_DSOUND.  DirectSound hardware voices are not capable of this functionality.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getInputChannelMix
    Channel::setPan
    Channel::setSpeakerMix
    Channel::setSpeakerLevels
    System::setSoftwareFormat
]
*/
FMOD_RESULT F_API FMOD_Channel_SetInputChannelMix(FMOD_CHANNEL *channel, float *levels, int numlevels)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setInputChannelMix(levels, numlevels);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the incoming levels for a channel in a sound.<br>
    A mono sound has 1 input channel, a stereo has 2, etc.  It depends on what type of sound is playing on the channel at the time.

    [PARAMETERS]
    'levels'     Address of an array of float volume levels, from 0.0 to 1.0.  These represent the incoming channels for the sound playing on the channel at the time.
    'numlevels'  Number of floats to receive into the array.   Maximum = the maximum number of input channels specified in System::setSoftwareFormat.

    [RETURN_VALUE]

    [REMARKS]
    This does not affect which speakers the sound is routed to.  This can be used in conjunction with functions like Channel::setPan, Channel::setSpeakerMix, Channel::setSpeakerLevels.<br>
    This function only scales the input channels from the sound.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setInputChannelMix
    Channel::setPan
    Channel::setSpeakerMix
    Channel::setSpeakerLevels
    System::setSoftwareFormat
]
*/
FMOD_RESULT F_API FMOD_Channel_GetInputChannelMix(FMOD_CHANNEL *channel, float *levels, int numlevels)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getInputChannelMix(levels, numlevels);
}


/*
[API]
[
    [DESCRIPTION]
    Mutes / un-mutes a channel, effectively silencing it or returning it to its normal volume.

    [PARAMETERS]
    'mute'      true = channel becomes muted (silent), false = channel returns to normal volume.

    [RETURN_VALUE]

    [REMARKS]
    If a channel belongs to a muted channelgroup, it will stay muted regardless of the channel mute state.  The channel mute state will still be reflected internally though, ie Channel::getMute will still return the value you set.  If the channelgroup has mute set to false, this function will become effective again.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getMute
    ChannelGroup::setMute
]
*/
FMOD_RESULT F_API FMOD_Channel_SetMute(FMOD_CHANNEL *channel, FMOD_BOOL mute)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setMute(mute ? true : false);
}


/*
[API]
[
    [DESCRIPTION]
    Returns the current mute status of the channel.

    [PARAMETERS]
    'mute'      true = channel is muted (silent), false = channel is at normal volume.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setMute
]
*/
FMOD_RESULT F_API FMOD_Channel_GetMute(FMOD_CHANNEL *channel, FMOD_BOOL *mute)
{
    FMOD_RESULT result;
    bool mute2;
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = _channel->getMute(&mute2);
    if (mute)
        {
            *mute = mute2 ? 1 : 0;
        }

    return result;
}


/*
[API]
[
    [DESCRIPTION]
    Sets the priority for a channel after it has been played.  A sound with a higher priority than another sound will not be stolen or made virtual by that sound.

    [PARAMETERS]
    'priority'      priority for the channel.  0 to 256 inclusive.  0 = most important.  256 = least important.  Default = 128.

    [RETURN_VALUE]

    [REMARKS]
    Priority will make a channel more important or less important than its counterparts.  When virtual channels are in place, by default the importance of the sound (whether it is audible or not when more channels are playing than exist) is based on the volume, or audiblity of the sound.  This is determined by distance from the listener in 3d, the volume set with Channel::setVolume, channel group volume, and geometry occlusion factors.   To make a quiet sound more important, so that it isn't made virtual by louder sounds, you can use this function to increase its importance, and keep it audible.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getPriority
    Channel::setVolume
]
*/
FMOD_RESULT F_API FMOD_Channel_SetPriority(FMOD_CHANNEL *channel, int priority)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setPriority(priority);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current priority for this channel.

    [PARAMETERS]
    'priority'  Address of a variable that receives the current channel priority.  0 to 256 inclusive.  0 = most important.  256 = least important.  Default = 128.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setPriority
]
*/
FMOD_RESULT F_API FMOD_Channel_GetPriority(FMOD_CHANNEL *channel, int *priority)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getPriority(priority);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the current playback position for the currently playing sound to the specified PCM offset.

    [PARAMETERS]
    'position'      Position of the channel to set in units specified in the postype parameter.
    'postype'       Time unit to set the channel position by.  See FMOD_TIMEUNIT.

    [RETURN_VALUE]

    [REMARKS]
    Certain timeunits do not work depending on the file format.  For example FMOD_TIMEUNIT_MODORDER will not work with an mp3 file.<br>
    <br>
    Note that if you are calling this function on a stream, it has to possibly reflush its buffer to get zero latency playback when it resumes playing, therefore it could potentially cause a stall or take a small amount of time to do this.
    <br>
    <b>Warning!</b>  Using a VBR source that does not have an associated seek table or seek information (such as MP3 or MOD/S3M/XM/IT) may cause inaccurate seeking if you specify FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCM.<br>
    If you want FMOD to create a pcm vs bytes seek table so that seeking is accurate, you will have to specify FMOD_ACCURATETIME when loading or opening the sound.  This means there is a slight delay as FMOD scans the whole file when loading the sound to create this table.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getPosition
    FMOD_TIMEUNIT
    FMOD_MODE
    Sound::getLength
]
*/
FMOD_RESULT F_API FMOD_Channel_SetPosition(FMOD_CHANNEL *channel, unsigned int position, FMOD_TIMEUNIT postype)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setPosition(position, postype);
}


/*
[API]
[
    [DESCRIPTION]
    Returns the current PCM offset or playback position for the specified channel.

    [PARAMETERS]
    'position'      Address of a variable that receives the position of the sound.
    'postype'       Time unit to retrieve into the position parameter.  See FMOD_TIMEUNIT.

    [RETURN_VALUE]

    [REMARKS]
    Certain timeunits do not work depending on the file format.  For example FMOD_TIMEUNIT_MODORDER will not work with an mp3 file.<br>
    A PCM sample is a unit of measurement in audio that contains the data for one audible element of sound.  1 sample might be 16bit stereo, so 1 sample contains 4 bytes.  44,100 samples of a 44khz sound would represent 1 second of data.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setPosition
    FMOD_TIMEUNIT
    Sound::getLength
]
*/
FMOD_RESULT F_API FMOD_Channel_GetPosition(FMOD_CHANNEL *channel, unsigned int *position, FMOD_TIMEUNIT postype)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getPosition(position, postype);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the channel specific reverb properties, including things like wet/dry mix.

    [PARAMETERS]
    'prop'      A pointer to the a FMOD_REVERB_CHANNELPROPERTIES structure, with the relevant reverb instance specified in FMOD_REVERB_CHANNELFLAGS.

    [RETURN_VALUE]

    [REMARKS]
    With FMOD_HARDWARE on Windows using EAX, the reverb will only work on FMOD_3D based sounds.  FMOD_SOFTWARE does not have this problem and works on FMOD_2D and FMOD_3D based sounds.<br>
    <br>
    On PlayStation 2, the 'Room' parameter is the only parameter supported. The hardware only allows 'on' or 'off', so the reverb will be off when 'Room' is -10000 and on for every other value.<br>
    <br>
    On Xbox, it is possible to apply reverb to FMOD_2D and FMOD_HARDWARE based voices using this function.  By default reverb is turned off for FMOD_2D hardware based voices.<br>
    <br>
    <b>Note!</b> It is important to clear this structure to 0 before calling, or at least specify which reverb instance you are talking about by using the Flags member of FMOD_REVERB_CHANNELPROPERTIES, otherwise you may get an FMOD_ERR_REVERB_INSTANCE error.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getReverbProperties
    FMOD_REVERB_CHANNELPROPERTIES
    FMOD_REVERB_CHANNELFLAGS
]
*/
FMOD_RESULT F_API FMOD_Channel_SetReverbProperties(FMOD_CHANNEL *channel, const FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setReverbProperties(prop);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current reverb properties for this channel.

    [PARAMETERS]
    'prop'      Address of a variable to receive the FMOD_REVERB_CHANNELPROPERTIES information.  Remember to clear this structure before calling this function, or specify the correct reverb instance with FMOD_REVERB_CHANNELFLAGS.

    [RETURN_VALUE]

    [REMARKS]
    <b>Note!</b> It is important to clear this structure to 0 before calling, or at least specify which reverb instance you are talking about by using the Flags member of FMOD_REVERB_CHANNELPROPERTIES, otherwise you may get an FMOD_ERR_REVERB_INSTANCE error.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setReverbProperties
    FMOD_REVERB_CHANNELPROPERTIES
    FMOD_REVERB_CHANNELFLAGS
]
*/
FMOD_RESULT F_API FMOD_Channel_GetReverbProperties(FMOD_CHANNEL *channel, FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getReverbProperties(prop);
}


/*
[API]
[
    [DESCRIPTION]
    Sets a channel to belong to a specified channel group.  A channelgroup can contain many channels.<br>

    [PARAMETERS]
    'channelgroup'  Pointer to a ChannelGroup object.

    [RETURN_VALUE]

    [REMARKS]
    Setting a channel to a channel group removes it from any previous group, it does not allow sharing of channel groups.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getChannelGroup
]
*/
FMOD_RESULT F_API FMOD_Channel_SetChannelGroup(FMOD_CHANNEL *channel, FMOD_CHANNELGROUP *channelgroup)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setChannelGroup((FMOD::ChannelGroup *)channelgroup);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the currently assigned channel group for the channel.

    [PARAMETERS]
    'channelgroup'  Address of a variable to receive a pointer to the currently assigned channel group.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setChannelGroup
]
*/
FMOD_RESULT F_API FMOD_Channel_GetChannelGroup(FMOD_CHANNEL *channel, FMOD_CHANNELGROUP **channelgroup)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getChannelGroup((FMOD::ChannelGroup **)channelgroup);
}


/*
[API]
[
    [DESCRIPTION]
    Sets a callback for a channel for a specific event.

    [PARAMETERS]
    'type'          The callback type, for example an 'end of sound' callback.
    'callback'      Pointer to a callback to receive the event when it happens.

    [RETURN_VALUE]

    [REMARKS]
    Currently callbacks are driven by System::update and will only occur when this function is called.  This has the main advantage of far less complication due to thread issues, and allows all FMOD commands, including loading sounds and playing new sounds from the callback.<br>
    It also allows any type of sound to have an end callback, no matter what it is.  The only disadvantage is that callbacks are not asynchronous and are bound by the latency caused by the rate the user calls the update command.<br>
    Callbacks are stdcall.  Use F_CALLBACK inbetween your return type and function name.<br>
    Example:
    <br>
    <PRE>
    FMOD_RESULT F_CALLBACK mycallback(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2)
    { 
    <ul>FMOD::Channel *cppchannel = (FMOD::Channel *)channel;
        <br>
        // More code goes here.
        <br>
        return FMOD_OK;
    </ul>}
    </PRE>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::update
    FMOD_CHANNEL_CALLBACK
    FMOD_CHANNEL_CALLBACKTYPE
]
*/
FMOD_RESULT F_API FMOD_Channel_SetCallback(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, FMOD_CHANNEL_CALLBACK callback, int command)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setCallback(type, callback, command);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the position and velocity of a 3d channel.

    [PARAMETERS]
    'pos'           Position in 3D space of the channel.  Specifying 0 / null will ignore this parameter.
    'vel'           Velocity in 'distance units per second' in 3D space of the channel.  See remarks.   Specifying 0 / null will ignore this parameter.

    [RETURN_VALUE]

    [REMARKS]
	A 'distance unit' is specified by System::set3DSettings.  By default this is set to meters which is a distance scale of 1.0.<br>
    <br>
    For a stereo 3d sound, you can set the spread of the left/right parts in speaker space by using Channel::set3DSpread.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::get3DAttributes
    FMOD_VECTOR
    System::set3DSettings
    Channel::set3DSpread
]
*/
FMOD_RESULT F_API FMOD_Channel_Set3DAttributes(FMOD_CHANNEL *channel, const FMOD_VECTOR *pos, const FMOD_VECTOR *vel)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->set3DAttributes(pos, vel);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the position and velocity of a 3d channel.

    [PARAMETERS]
    'pos'           Address of a variable that receives the position in 3D space of the channel.  Optional.  Specify 0 or NULL to ignore. 
    'vel'           Address of a variable that receives the velocity in 'distance units per second' in 3D space of the channel.  See remarks.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]
	A 'distance unit' is specified by System::set3DSettings.  By default this is set to meters which is a distance scale of 1.0.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::set3DAttributes
    FMOD_VECTOR
    System::set3DSettings
]
*/
FMOD_RESULT F_API FMOD_Channel_Get3DAttributes(FMOD_CHANNEL *channel, FMOD_VECTOR *pos, FMOD_VECTOR *vel)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->get3DAttributes(pos, vel);
}


/*
[API]
[
    [DESCRIPTION]
	Sets the minimum and maximum audible distance for a channel.<br>

    [PARAMETERS]
	'mindistance'       The channel's minimum volume distance in "units".  See remarks for more on units.
	'maxdistance'       The channel's maximum volume distance in "units".  See remarks for more on units.

    [RETURN_VALUE]

    [REMARKS]
	<br>
	MinDistance is the minimum distance that the sound emitter will cease to continue growing louder at (as it approaches the listener).<br>
    Within the mindistance it stays at the constant loudest volume possible.  Outside of this mindistance it begins to attenuate.<br>
	MaxDistance is the distance a sound stops attenuating at.  Beyond this point it will stay at the volume it would be at maxdistance units from the listener and will not attenuate any more.<br>
	MinDistance is useful to give the impression that the sound is loud or soft in 3d space.  An example of this is a small quiet object, such as a bumblebee, which you could set a mindistance of to 0.1 for example, which would cause it to attenuate quickly and dissapear when only a few meters away from the listener.<br>
    Another example is a jumbo jet, which you could set to a mindistance of 100.0, which would keep the sound volume at max until the listener was 100 meters away, then it would be hundreds of meters more before it would fade out.<br>
	<br>
	In summary, increase the mindistance of a sound to make it 'louder' in a 3d world, and decrease it to make it 'quieter' in a 3d world.<br>
    maxdistance is effectively obsolete unless you need the sound to stop fading out at a certain point.  Do not adjust this from the default if you dont need to.<br>
    Some people have the confusion that maxdistance is the point the sound will fade out to, this is not the case.<br>
    <br>
	A 'distance unit' is specified by System::set3DSettings.  By default this is set to meters which is a distance scale of 1.0.<br>
	The default units for minimum and maximum distances are 1.0 and 10000.0f.<br>
	Volume drops off at mindistance / distance.<br>
    To define the min and max distance per sound and not per channel use Sound::set3DMinMaxDistance.<br>
    <br>
    If FMOD_3D_CUSTOMROLLOFF is used, then these values are stored, but ignored in 3d processing.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::get3DMinMaxDistance
    System::set3DSettings
    Sound::set3DMinMaxDistance
]
*/
FMOD_RESULT F_API FMOD_Channel_Set3DMinMaxDistance(FMOD_CHANNEL *channel, float mindistance, float maxdistance)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->set3DMinMaxDistance(mindistance, maxdistance);
}


/*
[API]
[
    [DESCRIPTION]
	Retrieves the current minimum and maximum audible distance for a channel.

    [PARAMETERS]
    'mindistance'   Pointer to a floating point value to store mindistance.  Optional.  Specify 0 or NULL to ignore. 
    'maxdistance'   Pointer to a floating point value to store maxdistance.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::set3DMinMaxDistance
    System::set3DSettings
    Sound::set3DMinMaxDistance
]
*/
FMOD_RESULT F_API FMOD_Channel_Get3DMinMaxDistance(FMOD_CHANNEL *channel, float *mindistance, float *maxdistance)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->get3DMinMaxDistance(mindistance, maxdistance);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the inside and outside angles of the sound projection cone, as well as the volume of the sound outside the outside angle of the sound projection cone.

    [PARAMETERS]
    'insideconeangle'   Inside cone angle, in degrees. This is the angle within which the sound is at its normal volume.  Must not be greater than outsideconeangle.  Default = 360.
    'outsideconeangle'  Outside cone angle, in degrees. This is the angle outside of which the sound is at its outside volume.  Must not be less than insideconeangle.  Default = 360.
    'outsidevolume'     Cone outside volume, from 0 to 1.0.  Default = 1.0.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::get3DConeSettings
    Channel::set3DConeOrientation
    Sound::set3DConeSettings
]
*/
FMOD_RESULT F_API FMOD_Channel_Set3DConeSettings(FMOD_CHANNEL *channel, float insideconeangle, float outsideconeangle, float outsidevolume)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->set3DConeSettings(insideconeangle, outsideconeangle, outsidevolume);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the inside and outside angles of the sound projection cone.

    [PARAMETERS]
    'insideconeangle'   Address of a variable that receives the inside angle of the sound projection cone, in degrees. This is the angle within which the sound is at its normal volume.  Optional.  Specify 0 or NULL to ignore. 
    'outsideconeangle'  Address of a variable that receives the outside angle of the sound projection cone, in degrees. This is the angle outside of which the sound is at its outside volume.  Optional.  Specify 0 or NULL to ignore. 
    'outsidevolume'     Address of a variable that receives the cone outside volume for this channel.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::set3DConeSettings
    Sound::get3DConeSettings
]
*/
FMOD_RESULT F_API FMOD_Channel_Get3DConeSettings(FMOD_CHANNEL *channel, float *insideconeangle, float *outsideconeangle, float *outsidevolume)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->get3DConeSettings(insideconeangle, outsideconeangle, outsidevolume);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the orientation of the sound projection cone.

    [PARAMETERS]
    'orientation'   Pointer to an FMOD_VECTOR defining the coordinates of the sound cone orientation vector.

    [RETURN_VALUE]

    [REMARKS]
    This function has no effect unless the cone angle and cone outside volume have also been set to values other than the default.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::get3DConeOrientation
    Channel::set3DConeSettings
    Sound::set3DConeSettings
    FMOD_VECTOR
]
*/
FMOD_RESULT F_API FMOD_Channel_Set3DConeOrientation(FMOD_CHANNEL *channel, FMOD_VECTOR *orientation)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->set3DConeOrientation(orientation);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the orientation of the sound projection cone for this channel.

    [PARAMETERS]
    'orientation'   Address of a variable that receives the orientation of the sound projection cone. The vector information represents the center of the sound cone.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::set3DConeOrientation
]
*/
FMOD_RESULT F_API FMOD_Channel_Get3DConeOrientation(FMOD_CHANNEL *channel, FMOD_VECTOR *orientation)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->get3DConeOrientation(orientation);
}


/*
[API]
[
    [DESCRIPTION]
    Point a channel to use a custom rolloff curve.  Must be used in conjunction with FMOD_3D_CUSTOMROLLOFF flag to be activated.

    [PARAMETERS]
    'points'        An array of FMOD_VECTOR structures where x = distance and y = volume from 0.0 to 1.0.  z should be set to 0.
    'numpoints'     The number of points in the array.

    [RETURN_VALUE]

    [REMARKS]
    <b>Note!</b>  This function does not duplicate the memory for the points internally.  The pointer you pass to FMOD must remain valid until there is no more use for it.<br>
    Do not free the memory while in use, or use a local variable that goes out of scope while in use.<br>
    <br>
    Points must be sorted by distance!  Passing an unsorted list to FMOD will result in an error.<br>
    <br>
    Set the points parameter to 0 or NULL to disable the points.  If FMOD_3D_CUSTOMROLLOFF is set and the rolloff curve is 0, FMOD will revert to logarithmic curve rolloff.<br>
    <br>
    Min and maxdistance are meaningless when FMOD_3D_CUSTOMROLLOFF is used and the values are ignored.<br>
    <br>
    Here is an example of a custom array of points.<br>
    <PRE>
    FMOD_VECTOR curve[3] = 
    {
    <ul>{ 0.0f,  1.0f, 0.0f },
        { 2.0f,  0.2f, 0.0f },
        { 20.0f, 0.0f, 0.0f } 
    </ul>};
    </PRE>
    x represents the distance, y represents the volume. z is always 0.<br>
    Distances between points are linearly interpolated.<br>
    Note that after the highest distance specified, the volume in the last entry is used from that distance onwards.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_MODE
    FMOD_VECTOR
    Channel::get3DCustomRolloff
    Sound::set3DCustomRolloff
    Sound::get3DCustomRolloff
]
*/
FMOD_RESULT F_API FMOD_Channel_Set3DCustomRolloff(FMOD_CHANNEL *channel, FMOD_VECTOR *points, int numpoints)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->set3DCustomRolloff(points, numpoints);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a pointer to the sound's current custom rolloff curve.

    [PARAMETERS]
    'points'        Address of a variable to receive the pointer to the current custom rolloff point list.  Optional.  Specify 0 or NULL to ignore.
    'numpoints'     Address of a variable to receive the number of points int he current custom rolloff point list.  Optional.  Specify 0 or NULL to ignore.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_VECTOR
    Channel::set3DCustomRolloff
    Sound::set3DCustomRolloff
    Sound::get3DCustomRolloff
]
*/
FMOD_RESULT F_API FMOD_Channel_Get3DCustomRolloff(FMOD_CHANNEL *channel, FMOD_VECTOR **points, int *numpoints)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->get3DCustomRolloff(points, numpoints);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the EAX or software based occlusion factors for a channel. If the FMOD geometry engine is not being used, this function can be called to produce the same audible effects, just without the built in polygon processing. FMOD's internal geometry engine calls this function.  

    [PARAMETERS]
    'directocclusion'   Occlusion factor for a voice for the direct path. 0.0 = not occluded. 1.0 = fully occluded. Default = 0.0. 
    'reverbocclusion'   Occlusion factor for a voice for the reverb mix. 0.0 = not occluded. 1.0 = fully occluded. Default = 0.0. 

    [RETURN_VALUE]

    [REMARKS]
    With EAX based sound cards and FMOD_HARDWARE based sounds, this will attenuate the sound using frequency filtering.<br>
    With non EAX sounds, then the volume is simply attenuated by the directOcclusion factor.<br>
    If FMOD_INIT_SOFTWARE_OCCLUSION is specified, FMOD_SOFTWARE based sounds will also use frequency filtering, with a small CPU hit.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::get3DOcclusion
    ChannelGroup::set3DOcclusion
    FMOD_INITFLAGS
]
*/
FMOD_RESULT F_API FMOD_Channel_Set3DOcclusion(FMOD_CHANNEL *channel, float directocclusion, float reverbocclusion)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->set3DOcclusion(directocclusion, reverbocclusion);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the the EAX or software based occlusion factors for a channel.  

    [PARAMETERS]
    'directocclusion'   Address of a variable that receives the occlusion factor for a voice for the direct path. 0.0 = not occluded. 1.0 = fully occluded. Default = 0.0. Optional. Specify 0 or NULL to ignore. 
    'reverbocclusion'   Address of a variable that receives the occlusion factor for a voice for the reverb mix. 0.0 = not occluded. 1.0 = fully occluded. Default = 0.0. Optional. Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]
    
    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::set3DOcclusion
    ChannelGroup::get3DOcclusion
]
*/
FMOD_RESULT F_API FMOD_Channel_Get3DOcclusion(FMOD_CHANNEL *channel, float *directocclusion, float *reverbocclusion)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->get3DOcclusion(directocclusion, reverbocclusion);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the spread of a 3d sound in speaker space.<br>
    Normally a 3d sound is aimed at one position in a speaker array depending on the 3d position, to give it direction.  Left and right parts of a stereo sound for example are consequently summed together and become 'mono'.<br>
    When increasing the 'spread' of a sound, the left and right parts of a stereo sound rotate away from their original position, to give it more 'stereoness'.   The rotation of the sound channels are done in 'speaker space'.<br>
    Mono sounds are also able to be 'spread' with this function.
   
    [PARAMETERS]
    'angle'         Speaker spread angle.  0 = all sound channels are located at the same speaker location and is 'mono'.  360 = all subchannels are located at the opposite speaker location to the speaker location that it should be according to 3D position.  Default = 0.

    [RETURN_VALUE]

    [REMARKS]
    Only affects sounds created with FMOD_SOFTWARE.<br>
    <br>
    By default, if a stereo sound was played in 3d, and it was directly in front of you, the left and right part of the stereo sound would be summed into the center speaker (on a 5.1 setup), making it sound mono.<br>
    This function lets you control the speaker spread of a stereo (and above) sound within the speaker array, to separate the left right part of a stereo sound for example.<br>
    In the above case, in a 5.1 setup, specifying a spread of 90 degrees would put the left part of the sound in the front left speaker, and the right part of the sound in the front right speaker.  This stereo separation remains intact as the listener rotates and the sound moves around the speakers.<br>
    To summarize (for a stereo sound).<br>
    1. A spread angle of 0 makes the stereo sound mono at the point of the 3d emitter.<br>
    2. A spread angle of 90 makes the left part of the stereo sound place itself at 45 degrees to the left and the right part 45 degrees to the right.<br>
    3. A spread angle of 180 makes the left part of the stero sound place itself at 90 degrees to the left and the right part 90 degrees to the right.<br>
    4. A spread angle of 360 makes the stereo sound mono at the opposite speaker location to where the 3d emitter should be located (by moving the left part 180 degrees left and the right part 180 degrees right).  So in this case, behind you when the sound should be in front of you!<br>
    <br>
    Multichannel sounds with channel counts greater than stereo have their sub-channels spread evently through the specified angle.  For example a 6 channel sound over a 90 degree spread has each subchannel located 15 degrees apart from each other in the speaker array.<br>
    <br>
    Mono sounds are spread as if they were a stereo signal, ie the signal is split into 2.  The power of the same will remain the same as it spreads around the speakers.
  
    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone
  
    [SEE_ALSO]
    Channel::get3DSpread
    FMOD_MODE
]
*/
FMOD_RESULT F_API FMOD_Channel_Set3DSpread(FMOD_CHANNEL *channel, float angle)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->set3DSpread(angle);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the stereo (and above) spread angle specified by Channel::set3DSpread.

    [PARAMETERS]
    'angle'     Address of a variable that receives the spread angle for subchannels.  0 = all subchannels are located at the same position.  360 = all subchannels are located at the opposite position.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::set3DSpread
]
*/
FMOD_RESULT F_API FMOD_Channel_Get3DSpread(FMOD_CHANNEL *channel, float *angle)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->get3DSpread(angle);
}


/*
[API]
[
    [DESCRIPTION]
    Sets how much the 3d engine has an effect on the channel, versus that set by Channel::setPan, Channel::setSpeakerMix, Channel::setSpeakerLevels.<br>

    [PARAMETERS]
    'level'   1 = Sound pans and attenuates according to 3d position.  0 = Attenuation is ignored and pan/speaker levels are defined by Channel::setPan, Channel::setSpeakerMix, Channel::setSpeakerLevels.  Default = 1 (all by 3D position).

    [RETURN_VALUE]

    [REMARKS]
    Only affects sounds created FMOD_3D.<br>
    <br>
    Useful for morhping a sound between 3D and 2D.  This is most common in volumetric sound, when the sound goes from directional, to 'all around you' (and doesn't pan according to listener position/direction).<br>
    FMOD_INIT_SOFTWARE_HRTF is also interpolated to be 'off' if level = 0, so that you do not get a muffling effect based on location when the sound is supposed to be effectively 2D.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::get3DPanLevel
    Channel::setSpeakerMix
    Channel::setPan
    Channel::setSpeakerLevels
]
*/
FMOD_RESULT F_API FMOD_Channel_Set3DPanLevel(FMOD_CHANNEL *channel, float level)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->set3DPanLevel(level);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current 3D mix level for the channel set by Channel::set3DPanLevel.

    [PARAMETERS]
    'level'   0 = Sound pans according to Channel::setSpeakerMix. 1 = Sound pans according to 3d position.  Default = 1 (all by 3d position).

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::set3DPanLevel
    Channel::setSpeakerMix
]
*/
FMOD_RESULT F_API FMOD_Channel_Get3DPanLevel(FMOD_CHANNEL *channel, float *level)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->get3DPanLevel(level);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the channel specific doppler scale for the channel.

    [PARAMETERS]
    'level'   0 = No doppler. 1 = Normal doppler. 5 = max.  Default = 1.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::get3DDopplerLevel
]
*/
FMOD_RESULT F_API FMOD_Channel_Set3DDopplerLevel(FMOD_CHANNEL *channel, float level)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->set3DDopplerLevel(level);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current 3D doppler level for the channel set by Channel::set3DDopplerLevel.

    [PARAMETERS]
    'level'   Address of a variable to receives the current doppler scale for this channel.  0 = No doppler. 1 = Normal doppler. 5 = max.  Default = 1.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::set3DDopplerLevel
]
*/
FMOD_RESULT F_API FMOD_Channel_Get3DDopplerLevel(FMOD_CHANNEL *channel, float *level)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->get3DDopplerLevel(level);
}


/*
[API]
[
    [DESCRIPTION]
    Returns a pointer to the DSP unit head node that handles software mixing for this channel.<br>
    Only applicable to channels playing sounds created with FMOD_SOFTWARE.

    [PARAMETERS]
    'dsp'       Address of a variable that receives pointer to the current head DSP unit for this channel.

    [RETURN_VALUE]

    [REMARKS]
    By default a channel DSP unit usually contains 1 input, which is the wavetable input.<br>
    If System::playDSP has been used then the input to the channel head unit will be the unit that was specified in the call.<br>
    See the tutorials for more information on DSP networks and how to manipulate them.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createDSP
    System::createDSPByType
    System::playDSP
]
*/
FMOD_RESULT F_API FMOD_Channel_GetDSPHead(FMOD_CHANNEL *channel, FMOD_DSP **dsp)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getDSPHead((FMOD::DSP **)dsp);
}


/*
[API]
[
    [DESCRIPTION]
    This function adds a pre-created DSP unit or effect to the head of the Channel DSP chain.

    [PARAMETERS]
    'dsp'               A pointer to a pre-created DSP unit to be inserted at the head of the Channel DSP chain.
    'connection'        A pointer to the connection involved between the Channel DSP head and the specified dsp unit.  Optional. Specify 0 or NULL to ignore.

    [RETURN_VALUE]

    [REMARKS]
    This function is a wrapper function to insert a DSP unit at the top of the Channel DSP chain.<br>
    It disconnects the head unit from its input, then inserts the unit at the head and reconnects the previously disconnected input back as as an input to the new unit.<br>
    <br>
    <b>Note:</b> The connection pointer retrieved here will become invalid if you disconnect the 2 dsp units that use it.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getDSPHead
    System::createDSP
    System::createDSPByType
    System::createDSPByPlugin
    System::addDSP
    ChannelGroup::addDSP
    DSP::remove
]
*/
FMOD_RESULT F_API FMOD_Channel_AddDSP(FMOD_CHANNEL *channel, FMOD_DSP *dsp)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->addDSP((FMOD::DSP *)dsp);
}


/*
[API]
[
    [DESCRIPTION]
    Returns the playing state for the current channel.

    [PARAMETERS]
    'isplaying'     Address of a variable that receives the current channel's playing status.  true = the channel is currently playing a sound.  false = the channel is not playing a sound.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::playSound
    System::playDSP
]
*/
FMOD_RESULT F_API FMOD_Channel_IsPlaying(FMOD_CHANNEL *channel, FMOD_BOOL *isplaying)
{
    FMOD_RESULT result;
    bool isplaying2;
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = _channel->isPlaying(&isplaying2);
    if (isplaying)
        {
            *isplaying = isplaying2 ? 1 : 0;
        }

    return result;
}


/*
[API]
[
    [DESCRIPTION]
    Returns the current channel's status of whether it is virtual (emulated) or not due to FMOD Ex's virtual channel management system.

    [PARAMETERS]
    'isvirtual'     Address of a variable that receives the current channel's virtual status.  true = the channel is inaudible and currently being emulated at no cpu cost.  false = the channel is a real hardware or software voice and should be audible.

    [RETURN_VALUE]

    [REMARKS]
    Virtual channels are not audible, because there are no more real hardware or software channels available.<br>
    If you are plotting virtual voices vs real voices graphically, and wondering why FMOD sometimes chooses seemingly random channels to be virtual that are usually far away, that is because they are probably silent.  It doesn't matter which are virtual and which are not if they are silent.  Virtual voices are not calculation on 'closest to listener' calculation, they are based on audibility.
    See the tutorial in the FMOD Ex documentation for more information on virtual channels.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::playSound
]
*/
FMOD_RESULT F_API FMOD_Channel_IsVirtual(FMOD_CHANNEL *channel, FMOD_BOOL *isvirtual)
{
    FMOD_RESULT result;
    bool isvirtual2;
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = _channel->isVirtual(&isvirtual2);
    if (isvirtual)
        {
            *isvirtual = isvirtual2 ? 1 : 0;
        }

    return result;
}


/*
[API]
[
    [DESCRIPTION]
    Returns the combined volume of the channel after 3d sound, volume, channel group volume and geometry occlusion calculations have been performed on it.

    [PARAMETERS]
    'audibility'    Address of a variable that receives the channel audibility value.

    [RETURN_VALUE]

    [REMARKS]
    This does not represent the waveform, just the calculated volume based on 3d distance, occlusion, volume and channel group volume.
    This value is used by the FMOD Ex virtual channel system to order its channels between real and virtual.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setVolume
    Channel::getVolume
    ChannelGroup::setVolume
    ChannelGroup::getVolume
    Channel::set3DOcclusion
    Channel::get3DOcclusion
    Channel::set3DAttributes
    Channel::get3DAttributes
]
*/
FMOD_RESULT F_API FMOD_Channel_GetAudibility(FMOD_CHANNEL *channel, float *audibility)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getAudibility(audibility);
}


/*
[API]
[
    [DESCRIPTION]
    Returns the currently playing sound for this channel.

    [PARAMETERS]
    'sound'         Address of a variable that receives the pointer to the currently playing sound for this channel.

    [RETURN_VALUE]

    [REMARKS]
    If a sound is not playing the returned pointer will be 0 or NULL.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::playSound
    System::playDSP
]
*/
FMOD_RESULT F_API FMOD_Channel_GetCurrentSound(FMOD_CHANNEL *channel, FMOD_SOUND **sound)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getCurrentSound((FMOD::Sound **)sound);
}


/*
[API]
[
	[DESCRIPTION]
    Retrieves the spectrum from the currently playing output signal for the current channel only.

	[PARAMETERS]
    'spectrumarray'     Address of a variable that receives the spectrum data.  This is an array of floating point values.  Data will range is 0.0 to 1.0.  Decibels = 10.0f * (float)log10(val) * 2.0f;  See remarks for what the data represents.
    'numvalues'         Size of array in floating point values being passed to the function.  Must be a power of 2. (ie 128/256/512 etc).  Min = 64.  Max = 8192.
    'channeloffset'     Channel of the signal to analyze.  If the signal is multichannel (such as a stereo output), then this value represents which channel to analyze.  On a stereo signal 0 = left, 1 = right.
    'windowtype'        "Pre-FFT" window method.  This filters the PCM data before entering the spectrum analyzer to reduce transient frequency error for more accurate results.  See FMOD_DSP_FFT_WINDOW for different types of fft window techniques possible and for a more detailed explanation.
     
	[RETURN_VALUE]

	[REMARKS]
    The larger the numvalues, the more CPU the FFT will take.  Choose the right value to trade off between accuracy / speed.<br>
    The larger the numvalues, the more 'lag' the spectrum will seem to inherit.  This is because the FFT window size stretches the analysis back in time to what was already played.  For example if the numvalues size happened to be 44100 and the output rate was 44100 it would be analyzing the past second of data, and giving you the average spectrum over that time period.<br>
    If you are not displaying the result in dB, then the data may seem smaller than it should be.  To display it you may want to normalize the data - that is, find the maximum value in the resulting spectrum, and scale all values in the array by 1 / max.  (ie if the max was 0.5f, then it would become 1).<br>
    To get the spectrum for both channels of a stereo signal, call this function twice, once with channeloffset = 0, and again with channeloffset = 1.  Then add the spectrums together and divide by 2 to get the average spectrum for both channels.<br>
    <br>
    <u>What the data represents.</u><br>
    To work out what each entry in the array represents, use this formula<br>
    <PRE>
    entry_hz = (output_rate / 2) / numvalues
    </PRE>
    The array represents amplitudes of each frequency band from 0hz to the nyquist rate.  The nyquist rate is equal to the output rate divided by 2.<br>
    For example when FMOD is set to 44100hz output, the range of represented frequencies will be 0hz to 22049hz, a total of 22050hz represented.<br>
    If in the same example, 1024 was passed to this function as the numvalues, each entry's contribution would be as follows.
    <PRE>
    entry_hz = (44100 / 2) / 1024
    entry_hz = 21.53 hz
    </PRE>
    <br>
    <b>Note:</b> This function only displays data for sounds playing that were created with FMOD_SOFTWARE.  FMOD_HARDWARE based sounds are played using the sound card driver and are not accessable.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_DSP_FFT_WINDOW
    System::getSpectrum
    ChannelGroup::getSpectrum
    System::getWaveData
]
*/
FMOD_RESULT F_API FMOD_Channel_GetSpectrum(FMOD_CHANNEL *channel, float *spectrumarray, int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getSpectrum(spectrumarray, numvalues, channeloffset, windowtype);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a pointer to a block of PCM data that represents the currently playing waveform on this channel.<br>
    This function is useful for a very easy way to plot an oscilliscope.

    [PARAMETERS]
    'wavearray'     Address of a variable that receives the currently playing waveform data.  This is an array of floating point values.
    'numvalues'     Number of floats to write to the array.  Maximum value = 16384.
    'channeloffset' Offset into multichannel data.  Mono channels use 0.  Stereo channels use 0 = left, 1 = right.  More than stereo use the appropriate index.

    [RETURN_VALUE]

    [REMARKS]
    This is the actual resampled pcm data window at the time the function is called.<br>
    <br>
    Do not use this function to try and display the whole waveform of the sound, as this is more of a 'snapshot' of the current waveform at the time it is called, and could return the same data if it is called very quickly in succession.<br>
    See the DSP API to capture a continual stream of wave data as it plays, or see Sound::lock / Sound::unlock if you want to simply display the waveform of a sound.<br>
    <br>
    This function allows retrieval of left and right data for a stereo sound individually.  To combine them into one signal, simply add the entries of each seperate buffer together and then divide them by 2.
    <br>
    <b>Note:</b> This function only displays data for sounds playing that were created with FMOD_SOFTWARE.  FMOD_HARDWARE based sounds are played using the sound card driver and are not accessable.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getSpectrum
    ChannelGroup::getWaveData
    System::getWaveData
    Sound::lock
    Sound::unlock
]
*/
FMOD_RESULT F_API FMOD_Channel_GetWaveData(FMOD_CHANNEL *channel, float *wavearray, int numvalues, int channeloffset)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getWaveData(wavearray, numvalues, channeloffset);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the internal channel index for a channel.

    [PARAMETERS]
    'index'     Address of a variable to receive the channel index.  This will be from 0 to the value specified in System::init minus 1.

    [RETURN_VALUE]

    [REMARKS]
    Note that working with channel indicies directly is not recommended.  It is recommended that you use FMOD_CHANNEL_FREE for the index in System::playSound to use FMOD's channel manager.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::playSound
    System::init
]
*/
FMOD_RESULT F_API FMOD_Channel_GetIndex(FMOD_CHANNEL *channel, int *index)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getIndex(index);
}


/*
[API]
[
    [DESCRIPTION]
    Changes some attributes for a channel based on the mode passed in.

    [PARAMETERS]
    'mode'      Mode bits to set.

    [RETURN_VALUE]

    [REMARKS]
    Flags supported:<br>
    FMOD_LOOP_OFF<br>
    FMOD_LOOP_NORMAL<br>
    FMOD_LOOP_BIDI (only works with sounds created with FMOD_SOFTWARE.  Otherwise it will behave as FMOD_LOOP_NORMAL) <br>
    FMOD_3D_HEADRELATIVE<br>
    FMOD_3D_WORLDRELATIVE<br>
    FMOD_2D (see notes for win32 hardware voices)<br>
    FMOD_3D (see notes for win32 hardware voices)<br>
    FMOD_3D_LOGROLLOFF<br>
    FMOD_3D_LINEARROLLOFF<br>
    FMOD_3D_CUSTOMROLLOFF<br>
    FMOD_3D_IGNOREGEOMETRY<br>
    FMOD_DONTRESTOREVIRTUAL<br>
    <br>
    <u>Issues with streamed audio.  (Sounds created with with System::createStream or FMOD_CREATESTREAM). </u><br>
    When changing the loop mode, sounds created with System::createStream or FMOD_CREATESTREAM may already have been pre-buffered and executed their loop logic ahead of time, before this call was even made.<br>
    This is dependant on the size of the sound versus the size of the stream <i>decode</i> buffer.  See FMOD_CREATESOUNDEXINFO.<br>
    If this happens, you may need to reflush the stream buffer.  To do this, you can call Channel::setPosition which forces a reflush of the stream buffer.<br>
    Note this will usually only happen if you have sounds or looppoints that are smaller than the stream decode buffer size.  Otherwise you will not normally encounter any problems.<br>
    <br>
    <u>Issues with PCM samples.  (Sounds created with with System::createSound or FMOD_CREATESAMPLE). </u><br>
    When changing the loop mode, if the sound was set up as FMOD_LOOP_OFF, then set to FMOD_LOOP_NORMAL with this function, the sound may click when playing the end of the sound.  This is because the sound needs to be pre-prepared for looping using Sound::setMode, by modifying the content of the pcm data (ie data past the end of the actual sample data) to allow the interpolators to read ahead without clicking.  If you use Channel::setMode it will not do this (because different channels may have different loop modes for the same sound) and may click if you try to set it to looping on an unprepared sound.  If you want to change the loop mode at runtime it may be better to load the sound as looping first (or use Sound::setMode), to let it pre-prepare the data as if it was looping so that it does not click whenever Channel::setMode is used to turn looping on.<br>
    <br>
    <b>Win32</b> FMOD_HARDWARE note.  Under DirectSound, you cannot change the loop mode of a channel while it is playing.  You must use Sound::setMode or pause the channel to get this to work.<br>
    <b>Win32</b> FMOD_HARDWARE note.  Under DirectSound, you cannot change the mode of a channel between FMOD_2D and FMOD_3D.  If this is a problem create the sound as FMOD_3D initially, and use FMOD_3D_HEADRELATIVE and FMOD_3D_WORLDRELATIVE.  Alternatively just use FMOD_SOFTWARE.<br>
    <br>
    If FMOD_3D_IGNOREGEOMETRY is not specified, the flag will be cleared if it was specified previously.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_MODE
    Channel::getMode
    Channel::setPosition
    Sound::setMode
    System::createStream
    System::createSound
    System::setStreamBufferSize
    FMOD_CREATESOUNDEXINFO
]
*/
FMOD_RESULT F_API FMOD_Channel_SetMode(FMOD_CHANNEL *channel, FMOD_MODE mode)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setMode(mode);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current mode bit flags for the current channel.

    [PARAMETERS]
    'mode'      Address of a an FMOD_MODE variable that receives the current mode for this channel.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setMode
]
*/
FMOD_RESULT F_API FMOD_Channel_GetMode(FMOD_CHANNEL *channel, FMOD_MODE *mode)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getMode(mode);
}


/*
[API]
[
    [DESCRIPTION]
    Sets a channel to loop a specified number of times before stopping.

    [PARAMETERS]
    'loopcount'     Number of times to loop before stopping.  0 = oneshot.  1 = loop once then stop.  -1 = loop forever.  Default = -1

    [RETURN_VALUE]

    [REMARKS]
    This function does not affect FMOD_HARDWARE based sounds that are not streamable.<br>
    FMOD_SOFTWARE based sounds or any type of sound created with System::CreateStream or FMOD_CREATESTREAM will support this function.<br>
    <br>
    <u>Issues with streamed audio.  (Sounds created with with System::createStream or FMOD_CREATESTREAM). </u>
    When changing the loop count, sounds created with System::createStream or FMOD_CREATESTREAM may already have been pre-buffered and executed their loop logic ahead of time, before this call was even made.<br>
    This is dependant on the size of the sound versus the size of the stream <i>decode</i> buffer.  See FMOD_CREATESOUNDEXINFO.<br>
    If this happens, you may need to reflush the stream buffer.  To do this, you can call Channel::setPosition which forces a reflush of the stream buffer.<br>
    Note this will usually only happen if you have sounds or looppoints that are smaller than the stream decode buffer size.  Otherwise you will not normally encounter any problems.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getLoopCount
    Channel::setPosition
    System::createStream
    FMOD_CREATESOUNDEXINFO
    FMOD_MODE
]
*/
FMOD_RESULT F_API FMOD_Channel_SetLoopCount(FMOD_CHANNEL *channel, int loopcount)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setLoopCount(loopcount);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current loop count for the specified channel.

    [PARAMETERS]
    'loopcount'     Address of a variable that receives the number of times a channel will loop before stopping.  0 = oneshot.  1 = loop once then stop.  -1 = loop forever.  Default = -1

    [RETURN_VALUE]

    [REMARKS]
    This function retrieves the <b>current</b> loop countdown value for the channel being played.<br>
    This means it will decrement until reaching 0, as it plays.  To reset the value, use Channel::setLoopCount.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setLoopCount
]
*/
FMOD_RESULT F_API FMOD_Channel_GetLoopCount(FMOD_CHANNEL *channel, int *loopcount)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getLoopCount(loopcount);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the loop points within a channel.

    [PARAMETERS]
    'loopstart'         The loop start point.  This point in time is played, so it is inclusive.
    'loopstarttype'     The time format used for the loop start point.  See FMOD_TIMEUNIT.
    'loopend'           The loop end point.  This point in time is played, so it is inclusive.
    'loopendtype'       The time format used for the loop end point.  See FMOD_TIMEUNIT.

    [RETURN_VALUE]

    [REMARKS]
    Not supported by static sounds created with FMOD_HARDWARE.<br>
    Supported by sounds created with FMOD_SOFTWARE, or sounds of any type (hardware or software) created with System::createStream or FMOD_CREATESTREAM.
    <br>
    If a sound was 44100 samples long and you wanted to loop the whole sound, loopstart would be 0, and loopend would be 44099, <br>not</b> 44100.  You wouldn't use milliseconds in this case because they are not sample accurate.<br>
    If loop end is smaller or equal to loop start, it will result in an error.<br>
    If loop start or loop end is larger than the length of the sound, it will result in an error.<br>
    <br>
    <u>Issues with streamed audio.  (Sounds created with with System::createStream or FMOD_CREATESTREAM). </u><br>
    When changing the loop points, sounds created with System::createStream or FMOD_CREATESTREAM may already have been pre-buffered and executed their loop logic ahead of time, before this call was even made.<br>
    This is dependant on the size of the sound versus the size of the stream <i>decode</i> buffer.  See FMOD_CREATESOUNDEXINFO.<br>
    If this happens, you may need to reflush the stream buffer.  To do this, you can call Channel::setPosition which forces a reflush of the stream buffer.<br>
    Note this will usually only happen if you have sounds or looppoints that are smaller than the stream decode buffer size.  Otherwise you will not normally encounter any problems.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_TIMEUNIT
    FMOD_MODE
    Channel::getLoopPoints
    Channel::setLoopCount
    System::createStream
    System::setStreamBufferSize
    FMOD_CREATESOUNDEXINFO
]
*/
FMOD_RESULT F_API FMOD_Channel_SetLoopPoints(FMOD_CHANNEL *channel, unsigned int loopstart, FMOD_TIMEUNIT loopstarttype, unsigned int loopend, FMOD_TIMEUNIT loopendtype)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setLoopPoints(loopstart, loopstarttype, loopend, loopendtype);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the loop points for a channel.

    [PARAMETERS]
    'loopstart'         Address of a variable to receive the loop start point.  This point in time is played, so it is inclusive.  Optional.  Specify 0 or NULL to ignore. 
    'loopstarttype'     The time format used for the returned loop start point.  See FMOD_TIMEUNIT.
    'loopend'           Address of a variable to receive the loop end point.  This point in time is played, so it is inclusive.  Optional.  Specify 0 or NULL to ignore. 
    'loopendtype'       The time format used for the returned loop end point.  See FMOD_TIMEUNIT.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_TIMEUNIT
    Channel::setLoopPoints
]
*/
FMOD_RESULT F_API FMOD_Channel_GetLoopPoints(FMOD_CHANNEL *channel, unsigned int *loopstart, FMOD_TIMEUNIT loopstarttype, unsigned int *loopend, FMOD_TIMEUNIT loopendtype)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getLoopPoints(loopstart, loopstarttype, loopend, loopendtype);
}


/*
[API]
[
    [DESCRIPTION]
    Sets a user value that the Channel object will store internally.  Can be retrieved with Channel::getUserData.

    [PARAMETERS]
    'userdata'      Address of user data that the user wishes stored within the Channel object.

    [RETURN_VALUE]

    [REMARKS]
    This function is primarily used in case the user wishes to 'attach' data to an FMOD object.<br>
    It can be useful if an FMOD callback passes an object of this type as a parameter, and the user does not know which object it is (if many of these types of objects exist).  Using Channel::getUserData would help in the identification of the object.<p>
    NOTE: If this channel was spawned by the event system then its user data field will be set, by the event system, to the event instance handle that spawned it and this function should NOT be called.
    
    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::getUserData
]
*/
FMOD_RESULT F_API FMOD_Channel_SetUserData(FMOD_CHANNEL *channel, void *userdata)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->setUserData(userdata);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the user value that that was set by calling the Channel::setUserData function.

    [PARAMETERS]
    'userdata'  Address of a pointer that receives the data specified with the Channel::setUserData function.

    [RETURN_VALUE]

    [REMARKS]
    NOTE: If this channel was spawned by the event system then its user data field will be set, by the event system, to the event instance handle that spawned it. Use this function to go from an arbitrary channel back up to the event that owns it.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setUserData
]
*/
FMOD_RESULT F_API FMOD_Channel_GetUserData(FMOD_CHANNEL *channel, void **userdata)
{
    FMOD::Channel *_channel = (FMOD::Channel *)channel;

    if (!_channel)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _channel->getUserData(userdata);
}



/*
[API]
[
    [DESCRIPTION]
    Frees a channel group.

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]
    All channels assigned to this group are returned back to the master channel group owned by the System object.  See System::getMasterChannelGroup.<br>
    All child groups assigned to this group are returned back to the master channel group owned by the System object.  See System::getMasterChannelGroup.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createChannelGroup
    System::getMasterChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_Release(FMOD_CHANNELGROUP *channelgroup)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->release();
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the parent System object that created this channel group.

    [PARAMETERS]
    'system'        Address of a variable that receives the System object.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createChannelGroup
    System::getMasterChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetSystemObject(FMOD_CHANNELGROUP *channelgroup, FMOD_SYSTEM **system)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getSystemObject((FMOD::System **)system);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the master volume for the channel group linearly.

    [PARAMETERS]
    'volume'        A linear volume level, from 0.0 to 1.0 inclusive.  0.0 = silent, 1.0 = full volume.  Default = 1.0.

    [RETURN_VALUE]

    [REMARKS]
    This function does not go through and overwrite the channel volumes.  It scales them by the channel group's volume.<br>
    That way when Channel::setVolume / Channel::getVolume is called the respective individual channel volumes will still be preserved.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::setVolume
    Channel::setVolume
    Channel::getVolume
    ChannelGroup::overrideVolume
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_SetVolume(FMOD_CHANNELGROUP *channelgroup, float volume)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->setVolume(volume);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the master volume level for the channel group.

    [PARAMETERS]
    'volume'        Address of a variable to receive the channel group volume level, from 0.0 to 1.0 inclusive.  0.0 = silent, 1.0 = full volume.  Default = 1.0.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::setVolume
    System::getMasterChannelGroup
    System::createChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetVolume(FMOD_CHANNELGROUP *channelgroup, float *volume)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getVolume(volume);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the master pitch for the channel group.

    [PARAMETERS]
    'pitch'        A pitch level, from 0.0 to 10.0 inclusive.  0.5 = half pitch, 2.0 = double pitch.  Default = 1.0.

    [RETURN_VALUE]

    [REMARKS]
    This function does not go through and overwrite the channel frequencies.  It scales them by the channel group's pitch.<br>
    That way when Channel::setFrequency / Channel::getFrequency is called the respective individual channel frequencies will still be preserved.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::overrideFrequency
    System::getMasterChannelGroup
    ChannelGroup::getPitch
    Channel::setFrequency
    Channel::getFrequency
    ChannelGroup::overrideFrequency
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_SetPitch(FMOD_CHANNELGROUP *channelgroup, float pitch)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->setPitch(pitch);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the master pitch level for the channel group.

    [PARAMETERS]
    'pitch'        Address of a variable to receive the channel group pitch value, from 0.0 to 10.0 inclusive.  0.0 = silent, 1.0 = full volume.  Default = 1.0.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::setPitch
    ChannelGroup::overrideFrequency
    System::getMasterChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetPitch(FMOD_CHANNELGROUP *channelgroup, float *pitch)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getPitch(pitch);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the master occlusion factors for the channel group.

    [PARAMETERS]
    'directocclusion'   Occlusion factor for the direct path. 0.0 = not occluded. 1.0 = fully occluded. Default = 0.0. 
    'reverbocclusion'   Occlusion factor for the reverb mix. 0.0 = not occluded. 1.0 = fully occluded. Default = 0.0. 

    [RETURN_VALUE]

    [REMARKS]
    This function does not go through and overwrite the channel occlusion factors. It scales them by the channel group's occlusion factors.<br>
    That way when Channel::set3DOcclusion / Channel::get3DOcclusion is called the respective individual channel occlusion factors will still be preserved.
    This means that final Channel occlusion values will be affected by both ChannelGroup occlusion and geometry (if any).
    
    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::get3DOcclusion
    Channel::set3DOcclusion
    Channel::get3DOcclusion
    System::getMasterChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_Set3DOcclusion(FMOD_CHANNELGROUP *channelgroup, float directocclusion, float reverbocclusion)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->set3DOcclusion(directocclusion, reverbocclusion);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the master occlusion factors for the channel group.

    [PARAMETERS]
    'directocclusion'   Address of a variable that receives the occlusion factor for the direct path. 0.0 = not occluded. 1.0 = fully occluded. Default = 0.0. Optional. Specify 0 or NULL to ignore. 
    'reverbocclusion'   Address of a variable that receives the occlusion factor for the reverb mix. 0.0 = not occluded. 1.0 = fully occluded. Default = 0.0. Optional. Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::set3DOcclusion
    Channel::set3DOcclusion
    Channel::get3DOcclusion
    System::getMasterChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_Get3DOcclusion(FMOD_CHANNELGROUP *channelgroup, float *directocclusion, float *reverbocclusion)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->get3DOcclusion(directocclusion, reverbocclusion);
}


/*
[API]
[
    [DESCRIPTION]
    Pauses a channelgroup, and the channels within it, or unpauses any unpaused channels if set to false.

    [PARAMETERS]
    'paused'        Paused state to set.  true = channelgroup state is set to paused.  false = channelgroup state is set to unpaused.

    [RETURN_VALUE]

    [REMARKS]
    A channelgroup maintains a paused state, that affects channelgroups and channels within it.  If a channelgroup is paused, all channelgroups and channels below it will become paused.<br>
    Channels will not have their per channel pause state overwritten, so that when a channelgroup is unpaused, the paused state of the channels will correct as they were set on a per channel basis.<br>
    This means even though a channel is paused, it can return false when you call Channel::getPaused on that channel, because that was the state of the channel at the time before the ChannelGroup was paused.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::getPaused
    Channel::setPaused
    Channel::getPaused
    System::getMasterChannelGroup
    System::createChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_SetPaused(FMOD_CHANNELGROUP *channelgroup, FMOD_BOOL paused)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->setPaused(paused ? true : false);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the pause state of a ChannelGroup.

    [PARAMETERS]
    'paused'    Address of a variable to receive the pause state of the channelgroup.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::setPaused
    Channel::setPaused
    Channel::getPaused
    System::getMasterChannelGroup
    System::createChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetPaused(FMOD_CHANNELGROUP *channelgroup, FMOD_BOOL *paused)
{
    FMOD_RESULT result;
    bool paused2;
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = _group->getPaused(&paused2);
    if (result == FMOD_OK)
    {
        if (paused)
        {
            *paused = paused2 ? 1 : 0;
        }
    }

    return result;
}


/*
[API]
[
    [DESCRIPTION]
    Mutes a channelgroup, and the channels within it, or unmutes any unmuted channels if set to false.

    [PARAMETERS]
    'mute'        Mute state to set.  true = channelgroup state is set to muted.  false = channelgroup state is set to unmuted.

    [RETURN_VALUE]

    [REMARKS]
    A channelgroup maintains a mute state, that affects channelgroups and channels within it.  If a channelgroup is muted, all channelgroups and channels below it will become muted.<br>
    Channels will not have their per channel mute state overwritten, so that when a channelgroup is unmuted, the muted state of the channels will correct as they were set on a per channel basis.<br>
    This means even though a channel is muted, it can return false when you call Channel::getMute on that channel, because that was the state of the channel at the time before the ChannelGroup was muted.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::getMute
    Channel::setMute
    Channel::getMute
    System::getMasterChannelGroup
    System::createChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_SetMute(FMOD_CHANNELGROUP *channelgroup, FMOD_BOOL mute)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->setMute(mute ? true : false);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the mute state of a ChannelGroup.

    [PARAMETERS]
    'mute'    Address of a variable to receive the pause state of the channelgroup.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::setMute
    Channel::setMute
    Channel::getMute
    System::getMasterChannelGroup
    System::createChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetMute(FMOD_CHANNELGROUP *channelgroup, FMOD_BOOL *mute)
{
    FMOD_RESULT result;
    bool mute2;
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = _group->getMute(&mute2);
    if (result == FMOD_OK)
    {
        if (mute)
        {
            *mute = mute2 ? 1 : 0;
        }
    }

    return result;
}


/*
[API]
[
    [DESCRIPTION]
    Stops all channels within the channelgroup.

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::playSound
    System::getMasterChannelGroup
    System::createChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_Stop(FMOD_CHANNELGROUP *channelgroup)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->stop();
}


/*
[API]
[
    [DESCRIPTION]
    Overrides the volume of all channels within this channel group and those of any sub channelgroups.

    [PARAMETERS]
    'volume'        A linear volume level, from 0.0 to 1.0 inclusive.  0.0 = silent, 1.0 = full volume.  Default = 1.0.

    [RETURN_VALUE]
    This is not to be used as a master volume for the group, as it will modify the volumes of the channels themselves.<br>
    If you want to scale the volume of the group, use ChannelGroup::setVolume.

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getMasterChannelGroup
    System::createChannelGroup
    ChannelGroup::setVolume
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_OverrideVolume(FMOD_CHANNELGROUP *channelgroup, float volume)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->overrideVolume(volume);
}


/*
[API]
[
    [DESCRIPTION]
    Overrides the frequency or playback rate, in HZ of all channels within this channel group and those of any sub channelgroups.
 
    [PARAMETERS]
    'frequency'        A frequency value in HZ.  This value can also be negative to play the sound backwards (negative frequencies allowed with FMOD_SOFTWARE based non-stream sounds only).   DirectSound hardware voices have limited frequency range on some soundcards.  Please see remarks for more on this.
 
    [RETURN_VALUE]
 
    [REMARKS]
    When a sound is played, it plays at the default frequency of the sound which can be set by Sound::setDefaults.<br>
    For most file formats, the volume is determined by the audio format.<br>
    <br>
    <u>Frequency limitations for sounds created with FMOD_HARDWARE in DirectSound.</u><br>
    Every hardware device has a minimum and maximum frequency.  This means setting the frequency above the maximum and below the minimum will have no effect.<br>
    FMOD clamps frequencies to these values when playing back on hardware, so if you are setting the frequency outside of this range, the frequency will stay at either the minimum or maximum.<br>
    Note that FMOD_SOFTWARE based sounds do not have this limitation.<br>
    To find out the minimum and maximum value before initializing FMOD (maybe to decide whether to use a different soundcard, output mode, or drop back fully to software mixing), you can use the System::getDriverCaps function.
 
    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone
 
    [SEE_ALSO]
    Channel::setFrequency
    Channel::getFrequency
    System::getDriverCaps
    System::getMasterChannelGroup
    System::createChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_OverrideFrequency(FMOD_CHANNELGROUP *channelgroup, float frequency)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->overrideFrequency(frequency);
}


/*
[API]
[
    [DESCRIPTION]
    Sets pan position linearly of all channels within this channel group and those of any sub channelgroups.

    [PARAMETERS]
	'pan'     A left/right pan level, from -1.0 to 1.0 inclusive.  -1.0 = Full left, 0.0 = center, 1.0 = full right.  Default = 0.0.

    [RETURN_VALUE]

    [REMARKS]
    Panning only works on sounds created with FMOD_2D.  3D sounds are not pannable.<br>
    Only sounds that are mono or stereo can be panned.  Multichannel sounds (ie >2 channels) cannot be panned.<br>
    <br>
    Mono sounds are panned from left to right using constant power panning.  This means when pan = 0.0, the balance for the sound in each speaker is 71% left and 71% right, not 50% left and 50% right.  This gives (audibly) smoother pans.<br>
    Stereo sounds heave each left/right value faded up and down according to the specified pan position.  This means when pan = 0.0, the balance for the sound in each speaker is 100% left and 100% right.  When pan = -1.0, only the left channel of the stereo sound is audible, when pan = 1.0, only the right channel of the stereo sound is audible.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getMasterChannelGroup
    System::createChannelGroup
    Channel::setPan
    Channel::getPan
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_OverridePan(FMOD_CHANNELGROUP *channelgroup, float pan)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->overridePan(pan);
}


/*
[API]
[
    [DESCRIPTION]
    Overrides the reverb properties of all channels within this channel group and those of any sub channelgroups. 
    
    [PARAMETERS]
    'prop'  Pointer to a FMOD_REVERB_CHANNELPROPERTIES structure definition.

    [RETURN_VALUE]

    [REMARKS]
    With FMOD_HARDWARE on Windows using EAX, the reverb will only work on FMOD_3D based sounds.  FMOD_SOFTWARE does not have this problem and works on FMOD_2D and FMOD_3D based sounds.<br>
    <br>
    On PlayStation 2, the 'Room' parameter is the only parameter supported. The hardware only allows 'on' or 'off', so the reverb will be off when 'Room' is -10000 and on for every other value.<br>
    <br>
    On Xbox, it is possible to apply reverb to FMOD_2D and FMOD_HARDWARE based voices using this function.  By default reverb is turned off for FMOD_2D hardware based voices, to make it compatible with EAX.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_REVERB_CHANNELPROPERTIES
    System::setReverbProperties
    System::getReverbProperties
    Channel::setReverbProperties
    Channel::getReverbProperties
    System::getMasterChannelGroup
    System::createChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_OverrideReverbProperties(FMOD_CHANNELGROUP *channelgroup, const FMOD_REVERB_CHANNELPROPERTIES *prop)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->overrideReverbProperties(prop);
}


/*
[API]
[
    [DESCRIPTION]
    Overrides the position and velocity of all channels within this channel group and those of any sub channelgroups.

    [PARAMETERS]
    'pos'           Position in 3D space of the channels in the group.  Specifying 0 / null will ignore this parameter.
    'vel'           Velocity in 'distance units per second' in 3D space of the group of channels.  See remarks.   Specifying 0 / null will ignore this parameter.

    [RETURN_VALUE]

    [REMARKS]
	A 'distance unit' is specified by System::set3DSettings.  By default this is set to meters which is a distance scale of 1.0.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::set3DAttributes
    Channel::get3DAttributes
    FMOD_VECTOR
    System::set3DSettings
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_Override3DAttributes(FMOD_CHANNELGROUP *channelgroup, const FMOD_VECTOR *pos, const FMOD_VECTOR *vel)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->override3DAttributes(pos, vel);
}


/*
[API]
[
    [DESCRIPTION]
    Overrides all channel speaker levels for each speaker individually.

    [PARAMETERS]
    'frontleft'     Level for this channel in the front left speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume.
    'frontright'    Level for this channel in the front right speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume.
    'center'        Level for this channel in the center speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume.
    'lfe'           Level for this channel in the subwoofer speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume.
    'backleft'      Level for this channel in the back left speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume.
    'backright'     Level for this channel in the back right speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume.
    'sideleft'      Level for this channel in the side left speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume.
    'sideright'     Level for this channel in the side right speaker of a multichannel speaker setup.  0.0 = silent, 1.0 = full volume.

    [RETURN_VALUE]

    [REMARKS]
    This function only works on sounds created with FMOD_2D.  3D sounds are not pannable and will return FMOD_ERR_NEEDS2D.<br>
    <br>
    Only sounds create with FMOD_SOFTWARE playing on this channel will allow this functionality.<br>
    <br>
    Speakers specified that don't exist will simply be ignored.<br>
    <br>
    For more advanced speaker control, including sending the different channels of a stereo sound to arbitrary speakers, see Channel::setSpeakerLevels.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Channel::setSpeakerMix
    Channel::getSpeakerMix
    Channel::setSpeakerLevels
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_OverrideSpeakerMix(FMOD_CHANNELGROUP *channelgroup, float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->overrideSpeakerMix(frontleft, frontright, center, lfe, backleft, backright, sideleft, sideright);
}


/*
[API]
[
    [DESCRIPTION]
    Adds a channel group as a child of the current channel group.

    [PARAMETERS]
    'group'     channel group to add as a child.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::getNumGroups
    ChannelGroup::getGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_AddGroup(FMOD_CHANNELGROUP *channelgroup, FMOD_CHANNELGROUP *group)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->addGroup((FMOD::ChannelGroup *)group);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of sub groups under this channel group.

    [PARAMETERS]
    'numgroups'     Address of a variable to receive the number of channel groups within this channel group.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::getGroup
    ChannelGroup::addGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetNumGroups(FMOD_CHANNELGROUP *channelgroup, int *numgroups)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getNumGroups(numgroups);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a handle to a specified sub channelgroup.

    [PARAMETERS]
    'index'     Index to specify which sub channelgroup to receieve.
    'group'     Address of a variable to receieve a pointer to a channelgroup.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::getNumGroups
    ChannelGroup::getParentGroup
    ChannelGroup::addGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetGroup(FMOD_CHANNELGROUP *channelgroup, int index, FMOD_CHANNELGROUP **group)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getGroup(index, (FMOD::ChannelGroup **)group);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a handle to this channelgroup's parent channelgroup.

    [PARAMETERS]
    'group'         Address of a variable to recieve a pointer to a channelgroup.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::getNumGroups
    ChannelGroup::getGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetParentGroup(FMOD_CHANNELGROUP *channelgroup, FMOD_CHANNELGROUP **group)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getParentGroup((FMOD::ChannelGroup **)group);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the DSP unit responsible for this channel group.  When channels are submixed to this channel group, this is the DSP unit they target.

    [PARAMETERS]
    'dsp'       Address of a variable to receive the pointer to the head DSP unit for this channel group.

    [RETURN_VALUE]

    [REMARKS]
    Use this unit if you wish to connect custom DSP units to the channelgroup or filter the channels in the channel group by inserting filter units between this one and the incoming channel mixer unit.<br>
    Read the tutorial on DSP if you wish to know more about this.  It is not recommended using this if you do not understand how the FMOD Ex DSP network is connected.<br>
    Alternatively you can simply add effects by using ChannelGroup::addDSP which does the connection / disconnection work for you.<br>
    <br>
    <b>Note:</b> If this function returns FMOD_ERR_DSP_NOTFOUND when called on an event's channelgroup,
    the channelgroup's DSP unit may have been optimized away by the event system.
    Use FMOD_EVENT_USERDSP when getting the event to force the channelgroup to contain a DSP unit.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::addDSP
    System::createDSP
    System::createDSPByType
    System::createDSPByPlugin
    System::getMasterChannelGroup
    System::createChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetDSPHead(FMOD_CHANNELGROUP *channelgroup, FMOD_DSP **dsp)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getDSPHead((FMOD::DSP **)dsp);
}


/*
[API]
[
    [DESCRIPTION]
    Adds a DSP effect to this channelgroup, affecting all channels that belong to it.  Because it is a submix, only one instance of the effect is added, and all subsequent channels are affected.
    
    [PARAMETERS]
    'dsp'               Pointer to the dsp effect to add.  This can be created with System::createDSP, System::createDSPByType, System::createDSPByPlugin. 
    'connection'        A pointer to the connection involved between the ChannelGroup DSP head and the specified dsp unit.  Optional. Specify 0 or NULL to ignore.

    [RETURN_VALUE]

    [REMARKS]
    This function is a wrapper function to insert a DSP unit at the top of the channel group DSP chain.<br>
    It disconnects the head unit from its input, then inserts the unit at the head and reconnects the previously disconnected input back as as an input to the new unit.<br>
    <br>
    <b>Note:</b> The connection pointer retrieved here will become invalid if you disconnect the 2 dsp units that use it.<br>
    <br>
    <b>Note:</b> If this function returns FMOD_ERR_DSP_NOTFOUND when called on an event's channelgroup,
    the channelgroup's DSP unit may have been optimized away by the event system.
    Use FMOD_EVENT_USERDSP when getting the event to force the channelgroup to contain a DSP unit.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::getDSPHead
    System::createDSP
    System::createDSPByType
    System::createDSPByPlugin
    System::getMasterChannelGroup
    System::createChannelGroup
    System::addDSP
    Channel::addDSP
    DSP::remove
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_AddDSP(FMOD_CHANNELGROUP *channelgroup, FMOD_DSP *dsp)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->addDSP((FMOD::DSP *)dsp);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the name of the channelgroup.  The name is set when the group is created.

    [PARAMETERS]
    'name'        Address of a variable that receives the name of the channel group.
    'namelen'     Length in bytes of the target buffer to receieve the string.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getMasterChannelGroup
    System::createChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetName(FMOD_CHANNELGROUP *channelgroup, char *name, int namelen)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getName(name, namelen);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current number of assigned channels to this channel group.

    [PARAMETERS]
    'numchannels'       Address of a variable to receive the current number of assigned channels in this channel group.

    [RETURN_VALUE]

    [REMARKS]
    Use this function to enumerate the channels within the channel group.  You can then use ChannelGroup::getChannel to retrieve each individual channel.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::getChannel
    System::getMasterChannelGroup
    System::createChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetNumChannels(FMOD_CHANNELGROUP *channelgroup, int *numchannels)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getNumChannels(numchannels);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the a handle to a channel from the current channel group.

    [PARAMETERS]
    'index'     Index of the channel inside the channel group, from 0 to the number of channels returned by ChannelGroup::getNumChannels.
    'channel'   Address of a variable to receieve a pointer to a Channel object.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::getNumChannels
    System::getMasterChannelGroup
    System::createChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetChannel(FMOD_CHANNELGROUP *channelgroup, int index, FMOD_CHANNEL **channel)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getChannel(index, (FMOD::Channel **)channel);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the spectrum from the currently playing channels assigned to this channel group.

	[PARAMETERS]
    'spectrumarray'     Address of a variable that receives the spectrum data.  This is an array of floating point values.  Data will range is 0.0 to 1.0.  Decibels = 10.0f * (float)log10(val) * 2.0f;  See remarks for what the data represents.
    'numvalues'         Size of array in floating point values being passed to the function.  Must be a power of 2. (ie 128/256/512 etc).  Min = 64.  Max = 8192.
    'channeloffset'     Channel of the signal to analyze.  If the signal is multichannel (such as a stereo output), then this value represents which channel to analyze.  On a stereo signal 0 = left, 1 = right.
    'windowtype'        "Pre-FFT" window method.  This filters the PCM data before entering the spectrum analyzer to reduce transient frequency error for more accurate results.  See FMOD_DSP_FFT_WINDOW for different types of fft window techniques possible and for a more detailed explanation.
     
	[RETURN_VALUE]

	[REMARKS]
    The larger the numvalues, the more CPU the FFT will take.  Choose the right value to trade off between accuracy / speed.<br>
    The larger the numvalues, the more 'lag' the spectrum will seem to inherit.  This is because the FFT window size stretches the analysis back in time to what was already played.  For example if the window size happened to be 44100 and the output rate was 44100 it would be analyzing the past second of data, and giving you the average spectrum over that time period.<br>
    If you are not displaying the result in dB, then the data may seem smaller than it should be.  To display it you may want to normalize the data - that is, find the maximum value in the resulting spectrum, and scale all values in the array by 1 / max.  (ie if the max was 0.5f, then it would become 1).<br>
    To get the spectrum for both channels of a stereo signal, call this function twice, once with channeloffset = 0, and again with channeloffset = 1.  Then add the spectrums together and divide by 2 to get the average spectrum for both channels.<br>
    <br>
    <u>What the data represents.</u><br>
    To work out what each entry in the array represents, use this formula<br>
    <PRE>
    entry_hz = (output_rate / 2) / numvalues
    </PRE>
    The array represents amplitudes of each frequency band from 0hz to the nyquist rate.  The nyquist rate is equal to the output rate divided by 2.<br>
    For example when FMOD is set to 44100hz output, the range of represented frequencies will be 0hz to 22049hz, a total of 22050hz represented.<br>
    If in the same example, 1024 was passed to this function as the numvalues, each entry's contribution would be as follows.
    <PRE>
    entry_hz = (44100 / 2) / 1024
    entry_hz = 21.53 hz
    </PRE>
    <br>
    <b>Note:</b> This function only displays data for sounds playing that were created with FMOD_SOFTWARE.  FMOD_HARDWARE based sounds are played using the sound card driver and are not accessable.
    <br>
    With the Event system, events are now optimized to remove the DSP unit from a channelgroup to save memory.  This will result in the error FMOD_ERR_DSP_NOTFOUND.  If you wish to force the DSP node to be created with an event, use FMOD_EVENT_USERDSP flag when getting an event.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_DSP_FFT_WINDOW
    System::getSpectrum
    Channel::getSpectrum
    System::getMasterChannelGroup
    System::createChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetSpectrum(FMOD_CHANNELGROUP *channelgroup, float *spectrumarray, int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getSpectrum(spectrumarray, numvalues, channeloffset, windowtype);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a pointer to a block of PCM data that represents the currently playing waveform for this channel group.<br>
    This function is useful for a very easy way to plot an oscilliscope.

    [PARAMETERS]
    'wavearray'     Address of a variable that receives the currently playing waveform data.  This is an array of floating point values.
    'numvalues'     Number of floats to write to the array.  Maximum value = 16384.
    'channeloffset' Offset into multichannel data.  Mono channels use 0.  Stereo channels use 0 = left, 1 = right.  More than stereo use the appropriate index.

    [RETURN_VALUE]

    [REMARKS]
    This is the actual resampled, filtered and volume scaled data, at the time this function is called.<br>
    <br>
    Do not use this function to try and display the whole waveform of the sound, as this is more of a 'snapshot' of the current waveform at the time it is called, and could return the same data if it is called very quickly in succession.<br>
    See the DSP API to capture a continual stream of wave data as it plays, or see Sound::lock / Sound::unlock if you want to simply display the waveform of a sound.<br>
    <br>
    This function allows retrieval of left and right data for a stereo sound individually.  To combine them into one signal, simply add the entries of each seperate buffer together and then divide them by 2.
    <br>
    <b>Note:</b> This function only displays data for sounds playing that were created with FMOD_SOFTWARE.  FMOD_HARDWARE based sounds are played using the sound card driver and are not accessable.
    <br>
    With the Event system, events are now optimized to remove the DSP unit from a channelgroup to save memory.  This will result in the error FMOD_ERR_DSP_NOTFOUND.  If you wish to force the DSP node to be created with an event, use FMOD_EVENT_USERDSP flag when getting an event.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::getMasterChannelGroup
    System::createChannelGroup
    Sound::lock
    Sound::unlock
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetWaveData(FMOD_CHANNELGROUP *channelgroup, float *wavearray, int numvalues, int channeloffset)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getWaveData(wavearray, numvalues, channeloffset);
}


/*
[API]
[
    [DESCRIPTION]
    Sets a user value that the ChannelGroup object will store internally.  Can be retrieved with ChannelGroup::getUserData.

    [PARAMETERS]
    'userdata'      Address of user data that the user wishes stored within the ChannelGroup object.

    [RETURN_VALUE]

    [REMARKS]
    This function is primarily used in case the user wishes to 'attach' data to an FMOD object.<br>
    It can be useful if an FMOD callback passes an object of this type as a parameter, and the user does not know which object it is (if many of these types of objects exist).  Using ChannelGroup::getUserData would help in the identification of the object.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::getUserData
    System::getMasterChannelGroup
    System::createChannelGroup
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_SetUserData(FMOD_CHANNELGROUP *channelgroup, void *userdata)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->setUserData(userdata);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the user value that that was set by calling the ChannelGroup::setUserData function.

    [PARAMETERS]
    'userdata'  Address of a pointer that receives the  to user data specified with the ChannelGroup::setUserData function.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    ChannelGroup::setUserData
]
*/
FMOD_RESULT F_API FMOD_ChannelGroup_GetUserData(FMOD_CHANNELGROUP *channelgroup, void **userdata)
{
    FMOD::ChannelGroup *_group = (FMOD::ChannelGroup *)channelgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getUserData(userdata);
}



/*
[API]
[
    [DESCRIPTION]
    Releases a soundgroup object and returns all sounds back to the master sound group.

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]
    You cannot release the master sound group.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_Release(FMOD_SOUNDGROUP *soundgroup)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->release();
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the parent System object that was used to create this object.

    [PARAMETERS]
    'system'        Address of a pointer that receives the System object.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_GetSystemObject(FMOD_SOUNDGROUP *soundgroup, FMOD_SYSTEM **system)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getSystemObject((FMOD::System **)system);
}


/*
[API]
[
    [DESCRIPTION]
    Limits the number of concurrent playbacks of sounds in a sound group to the specified value.<br>
    After this, if the sounds in the sound group are playing this many times, any attepts to play more of the sounds in the sound group will by default fail with FMOD_ERR_MAXAUDIBLE.<br>
    Use SoundGroup::setMaxAudibleBehavior to change the way the sound playback behaves when too many sounds are playing.  Muting, failing and stealing behaviors can be specified.<br>

    [PARAMETERS]
    'maxaudible'    Number of playbacks to be audible at once.  -1 = unlimited.  0 means no sounds in this group will succeed. Default = -1.

    [RETURN_VALUE]

    [REMARKS]
    SoundGroup::getNumPlaying can be used to determine how many instances of the sounds in the sound group are currently playing.<br>
    

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createSoundGroup
    SoundGroup::getMaxAudible
    SoundGroup::getNumPlaying
    SoundGroup::setMaxAudibleBehavior
    SoundGroup::getMaxAudibleBehavior
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_SetMaxAudible(FMOD_SOUNDGROUP *soundgroup, int maxaudible)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->setMaxAudible(maxaudible);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of concurrent playbacks of sounds in a sound group to the specified value.<br>
    If the sounds in the sound group are playing this many times, any attepts to play more of the sounds in the sound group will fail with FMOD_ERR_MAXAUDIBLE.

    [PARAMETERS]
    'maxaudible'    Address of a variable to recieve the number of playbacks to be audible at once.  -1 = unlimited.  0 means no sounds in this group will succeed. Default = -1.

    [RETURN_VALUE]

    [REMARKS]
    SoundGroup::getNumPlaying can be used to determine how many instances of the sounds in the sound group are playing.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    SoundGroup::setMaxAudible
    SoundGroup::getNumPlaying
    System::createSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_GetMaxAudible(FMOD_SOUNDGROUP *soundgroup, int *maxaudible)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getMaxAudible(maxaudible);
}


/*
[API]
[
    [DESCRIPTION]
    This function changes the way the sound playback behaves when too many sounds are playing in a soundgroup.  Muting, failing and stealing behaviors can be specified.<br>

    [PARAMETERS]
    'behavior'  Specify a behavior determined with a FMOD_SOUNDGROUP_BEHAVIOR flag.  Default is FMOD_SOUNDGROUP_BEHAVIOR_FAIL.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_SOUNDGROUP_BEHAVIOR
    SoundGroup::getMaxAudibleBehavior
    SoundGroup::setMaxAudible
    SoundGroup::getMaxAudible
    SoundGroup::setMuteFadeSpeed
    SoundGroup::getMuteFadeSpeed
    System::createSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_SetMaxAudibleBehavior(FMOD_SOUNDGROUP *soundgroup, FMOD_SOUNDGROUP_BEHAVIOR behavior)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->setMaxAudibleBehavior(behavior);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current max audible behavior method.  

    [PARAMETERS]
    'behavior'  Address of a variable to recieve the current sound group max playbacks behavior.  Default is FMOD_SOUNDGROUP_BEHAVIOR_FAIL.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_SOUNDGROUP_BEHAVIOR
    SoundGroup::setMaxAudibleBehavior
    SoundGroup::setMaxAudible
    SoundGroup::getMaxAudible
    SoundGroup::setMuteFadeSpeed
    SoundGroup::getMuteFadeSpeed
    System::createSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_GetMaxAudibleBehavior(FMOD_SOUNDGROUP *soundgroup, FMOD_SOUNDGROUP_BEHAVIOR *behavior)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getMaxAudibleBehavior(behavior);
}


/*
[API]
[
    [DESCRIPTION]
    Specify a time in seconds for FMOD_SOUNDGROUP_BEHAVIOR_MUTE behavior to fade with.  By default there is no fade.<br>
    When more sounds are playing in a SoundGroup than are specified with SoundGroup::setMaxAudible, the least important sound (ie lowest priority / lowest audible volume due to 3d position, volume etc) will fade to silence if FMOD_SOUNDGROUP_BEHAVIOR_MUTE is used, and any previous sounds that were silent because of this rule will fade in if they are more important.<br>

    [PARAMETERS]
    'speed'     Fade time in seconds (1.0 = 1 second).  Default = 0.0. (no fade).

    [RETURN_VALUE]

    [REMARKS]
    If a mode besides FMOD_SOUNDGROUP_BEHAVIOR_MUTE is used, the fade speed is ignored.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    SoundGroup::getMuteFadeSpeed
    SoundGroup::setMaxAudibleBehavior
    SoundGroup::getMaxAudibleBehavior
    SoundGroup::setMaxAudible
    SoundGroup::getMaxAudible
    System::createSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_SetMuteFadeSpeed(FMOD_SOUNDGROUP *soundgroup, float speed)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->setMuteFadeSpeed(speed);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current time in seconds for FMOD_SOUNDGROUP_BEHAVIOR_MUTE behavior to fade with.

    [PARAMETERS]
    'speed'     Address of a variable to receive the fade time in seconds (1.0 = 1 second).  Default = 0.0. (no fade).

    [RETURN_VALUE]

    [REMARKS]
    If a mode besides FMOD_SOUNDGROUP_BEHAVIOR_MUTE is used, the fade speed is ignored.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    SoundGroup::setMuteFadeSpeed
    SoundGroup::setMaxAudibleBehavior
    SoundGroup::getMaxAudibleBehavior
    SoundGroup::setMaxAudible
    SoundGroup::getMaxAudible
    System::createSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_GetMuteFadeSpeed(FMOD_SOUNDGROUP *soundgroup, float *speed)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getMuteFadeSpeed(speed);
}


/*
[API]
[
    [DESCRIPTION]
    Sets the volume for a sound group, affecting all channels playing the sounds in this soundgroup.

    [PARAMETERS]
    'volume'        A linear volume level, from 0.0 to 1.0 inclusive.  0.0 = silent, 1.0 = full volume.  Default = 1.0.

    [RETURN_VALUE]

    [REMARKS]    

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    SoundGroup::getVolume
    System::createSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_SetVolume(FMOD_SOUNDGROUP *soundgroup, float volume)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->setVolume(volume);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the volume for the sounds within a soundgroup.

    [PARAMETERS]
    'volume'        Address of a variable to receive the soundgroup volume level, from 0.0 to 1.0 inclusive.  0.0 = silent, 1.0 = full volume.  Default = 1.0.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    SoundGroup::setVolume
    System::createSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_GetVolume(FMOD_SOUNDGROUP *soundgroup, float *volume)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getVolume(volume);
}


/*
[API]
[
    [DESCRIPTION]
    Stops all sounds within this soundgroup.

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::playSound
    System::createSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_Stop(FMOD_SOUNDGROUP *soundgroup)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->stop();
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the name of the sound group.

    [PARAMETERS]
    'name'        Address of a variable that receives the name of the sound group.
    'namelen'     Length in bytes of the target buffer to receieve the string.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_GetName(FMOD_SOUNDGROUP *soundgroup, char *name, int namelen)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getName(name, namelen);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current number of sounds in this sound group.

    [PARAMETERS]
    'numsounds'     Address of a variable to receive the number of sounds in this sound group.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createSoundGroup
    System::getMasterSoundGroup
    SoundGroup::setMaxAudible
    SoundGroup::getSound
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_GetNumSounds(FMOD_SOUNDGROUP *soundgroup, int *numsounds)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getNumSounds(numsounds);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a pointer to a sound from within a sound group.

    [PARAMETERS]
    'index'     Index of the sound that is to be retrieved.
    'sound'     Address of a variable to receieve a pointer to a Sound object.

    [RETURN_VALUE]

    [REMARKS]
    Use SoundGroup::getNumSounds in conjunction with this function to enumerate all sounds in a sound group.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createSoundGroup
    System::createSound
    SoundGroup::getNumSounds
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_GetSound(FMOD_SOUNDGROUP *soundgroup, int index, FMOD_SOUND **sound)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getSound(index, (FMOD::Sound **)sound);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of currently playing channels for the sound group.

    [PARAMETERS]
    'numplaying'    Address of a variable to receive the number of actively playing channels from sounds in this sound group.

    [RETURN_VALUE]

    [REMARKS]
    This routine returns the number of channels playing.  If the sound group only has 1 sound, and that sound is playing twice, the figure returned will be 2.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_GetNumPlaying(FMOD_SOUNDGROUP *soundgroup, int *numplaying)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getNumPlaying(numplaying);
}


/*
[API]
[
    [DESCRIPTION]
    Sets a user value that the SoundGroup object will store internally.  Can be retrieved with SoundGroup::getUserData.

    [PARAMETERS]
    'userdata'      Address of user data that the user wishes stored within the sound group object.

    [RETURN_VALUE]

    [REMARKS]
    This function is primarily used in case the user wishes to 'attach' data to an FMOD object.<br>
    It can be useful if an FMOD callback passes an object of this type as a parameter, and the user does not know which object it is (if many of these types of objects exist).  Using SoundGroup::getUserData would help in the identification of the object.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    SoundGroup::getUserData
    System::createSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_SetUserData(FMOD_SOUNDGROUP *soundgroup, void *userdata)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->setUserData(userdata);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the user value that that was set by calling the SoundGroup::setUserData function.

    [PARAMETERS]
    'userdata'  Address of a pointer that receives the data specified with the SoundGroup::setUserData function.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    SoundGroup::setUserData
    System::createSoundGroup
    System::getMasterSoundGroup
]
*/
FMOD_RESULT F_API FMOD_SoundGroup_GetUserData(FMOD_SOUNDGROUP *soundgroup, void **userdata)
{
    FMOD::SoundGroup *_group = (FMOD::SoundGroup *)soundgroup;

    if (!_group)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _group->getUserData(userdata);
}



/*
[API]
[
  	[DESCRIPTION]
    Frees a DSP object.
   
   	[PARAMETERS]
    
   	[RETURN_VALUE]
   
   	[REMARKS]
    This will free the DSP object.
   
    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone
   
   	[SEE_ALSO]
    System::createDSP
    System::createDSPByType
    System::getDSPHead
    Channel::getDSPHead
    ChannelGroup::getDSPHead

]
*/
FMOD_RESULT F_API FMOD_DSP_Release(FMOD_DSP *dsp)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->release();
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the parent System object that was used to create this object.

    [PARAMETERS]
    'system'        Address of a variable that receives the System object.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createDSP
    System::createDSPByType
    System::getDSPHead
    Channel::getDSPHead
    ChannelGroup::getDSPHead
]
*/
FMOD_RESULT F_API FMOD_DSP_GetSystemObject(FMOD_DSP *dsp, FMOD_SYSTEM **system)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getSystemObject((FMOD::System **)system);
}


/*
[API]
[
    [DESCRIPTION]
    Adds the specified DSP unit as an input of the DSP object.

    [PARAMETERS]
    'target'        The DSP unit to add as an input of the current unit.
    'connection'    The connection between the 2 units.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]
    Adding a unit as an input means that there can be multiple units added to the target.<br>
    Inputs are automatically mixed together, then the mixed data is sent to the unit's output(s).<br>
    To find the number of inputs or outputs a unit has use DSP::getNumInputs or DSP::getNumOutputs.<br>
    <br>
    <b>Note:</b> The connection pointer retrieved here will become invalid if you disconnect the 2 dsp units that use it.<br>

    [REMARKS]
    If you want to add a unit as an output of another unit, then add 'this' unit as an input of that unit instead.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::getNumInputs
    DSP::getInput
    DSP::getNumOutputs
    DSP::disconnectFrom
]
*/
FMOD_RESULT F_API FMOD_DSP_AddInput(FMOD_DSP *dsp, FMOD_DSP *target)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->addInput((FMOD::DSP *)target);
}


/*
[API]
[
    [DESCRIPTION]
    Disconnect the DSP unit from the specified target.
   
    [PARAMETERS]
    'target'        The unit that this unit is to be removed from.  Specify 0 or NULL to disconnect the unit from all outputs and inputs.
   
    [RETURN_VALUE]
   
    [REMARKS]
    Note that when you disconnect a unit, it is up to you to reconnect the network so that data flow can continue.<br>
    <br>
    <b>Important note:</b> If you have a handle to the connection pointer that binds these 2 DSP units, then it will become invalid.  The connection is then sent back to a freelist to be re-used again by a later addInput command.<br>
   
    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone
   
    [SEE_ALSO]
    DSP::addInput
    DSP::disconnectAll
]
*/
FMOD_RESULT F_API FMOD_DSP_DisconnectFrom(FMOD_DSP *dsp, FMOD_DSP *target)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->disconnectFrom((FMOD::DSP *)target);
}


/*
[API]
[
    [DESCRIPTION]
    Helper function to disconnect either all inputs or all outputs of a dsp unit.

    [PARAMETERS]
    'inputs'    true = disconnect all inputs to this DSP unit.  false = leave input connections alone.
    'outputs'   true = disconnect all outputs to this DSP unit.  false = leave output connections alone.

    [RETURN_VALUE]

    [REMARKS]
    This function is optimized to be faster than disconnecting inputs and outputs manually one by one.<br>
    <br>
    <b>Important note:</b> If you have a handle to DSPConnection pointers that bind any of the inputs or outputs to this DSP unit, then they will become invalid.  The connections are sent back to a freelist to be re-used again by a later addInput command.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::disconnectFrom
]
*/
FMOD_RESULT F_API FMOD_DSP_DisconnectAll(FMOD_DSP *dsp, FMOD_BOOL inputs, FMOD_BOOL outputs)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->disconnectAll(inputs ? true : false, outputs ? true : false);
}


/*
[API]
[
    [DESCRIPTION]
    Removes a unit from a DSP chain and connects the unit's input and output together after it is gone.

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]
    This function is generally only used with units that have been added with System::addDSP or Channel::addDSP.<br>
    A unit that has been added in this way generally only has one input and one output, so this function assumes this and takes input 0 and connects it with output 0 after it has been removed, so that the data flow is not broken.<br>
    <br>
    <b>Important note:</b> If you have a handle to DSPConnection pointers that bind any of the inputs or outputs to this DSP unit, then they will become invalid.  The connections are sent back to a freelist to be re-used again by a later addInput command.<br>
    <br>
    <b>Note:</b> If the unit has not been added with addDSP it will not restore links, and will just disconnect all inputs and outputs, making it equivalent to 
    <pre>
    disconnectAll(true, true).
    </pre>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::addDSP
    Channel::addDSP
    ChannelGroup::addDSP
]
*/
FMOD_RESULT F_API FMOD_DSP_Remove(FMOD_DSP *dsp)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->remove();
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of inputs connected to the DSP unit.

    [PARAMETERS]
    'numinputs'     Address of a variable that receives the number of inputs connected to this unit.

    [RETURN_VALUE]

    [REMARKS]
    Inputs are units that feed data to this unit.  When there are multiple inputs, they are mixed together.<br>
    <br>
    <b>Performance warning!</b>  Because this function needs to flush the dsp queue before it can determine how many units are available, this function may block significantly while the background mixer thread operates.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::getNumOutputs
    DSP::getInput
]
*/
FMOD_RESULT F_API FMOD_DSP_GetNumInputs(FMOD_DSP *dsp, int *numinputs)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getNumInputs(numinputs);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of outputs connected to the DSP unit.

    [PARAMETERS]
    'numoutputs'     Address of a variable that receives the number of outputs connected to this unit.

    [RETURN_VALUE]

    [REMARKS]
    Outputs are units that this unit feeds data to.  When there are multiple outputs, the data is split and sent to each unit individually.<br>
    <br>
    <b>Performance warning!</b>  Because this function needs to flush the dsp queue before it can determine how many units are available, this function may block significantly while the background mixer thread operates.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::getNumInputs
    DSP::getOutput
]
*/
FMOD_RESULT F_API FMOD_DSP_GetNumOutputs(FMOD_DSP *dsp, int *numoutputs)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getNumOutputs(numoutputs);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a pointer to a DSP unit which is acting as an input to this unit.

    [PARAMETERS]
    'index'             Index of the input unit to retrieve.
    'input'             Address of a variable that receieves the pointer to the desired input unit.
    'inputconnection'   The connection between the 2 units.  Optional.  Specify 0 or NULL to ignore. 


    [RETURN_VALUE]

    [REMARKS]
    An input is a unit which feeds audio data to this unit.<br>
    If there are more than 1 input to this unit, the inputs will be mixed, and the current unit processes the mixed result.<br>
    Find out the number of input units to this unit by calling DSP::getNumInputs.<br>
    <br>
    <b>Performance warning!</b>  Because this function needs to flush the dsp queue before it can determine if the specified numerical input is available or not, this function may block significantly while the background mixer thread operates.<br>
    <br>
    <b>Note:</b> The connection pointer retrieved here will become invalid if you disconnect the 2 dsp units that use it.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::getNumInputs
    DSP::addInput
    DSP::getOutput
    DSPConnection::getMix
    DSPConnection::setMix
]
*/
FMOD_RESULT F_API FMOD_DSP_GetInput(FMOD_DSP *dsp, int index, FMOD_DSP **input)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getInput(index, (FMOD::DSP **)input);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a pointer to a DSP unit which is acting as an output to this unit.

    [PARAMETERS]
    'index'             Index of the output unit to retrieve.
    'output'            Address of a variable that receieves the pointer to the desired output unit.
    'outputconnection'  The connection between the 2 units.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]
    An output is a unit which this unit will feed data too once it has processed its data.<br>
    Find out the number of output units to this unit by calling DSP::getNumOutputs.<br>
    <br>
    <b>Performance warning!</b>  Because this function needs to flush the dsp queue before it can determine if the specified numerical output is available or not, this function may block significantly while the background mixer thread operates.<br>
    <br>
    <b>Note:</b> The connection pointer retrieved here will become invalid if you disconnect the 2 dsp units that use it.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::getNumOutputs
    DSP::addInput
    DSP::getInput
    DSPConnection::getMix
    DSPConnection::setMix
]
*/
FMOD_RESULT F_API FMOD_DSP_GetOutput(FMOD_DSP *dsp, int index, FMOD_DSP **output)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getOutput(index, (FMOD::DSP **)output);
}


FMOD_RESULT F_API FMOD_DSP_SetInputMix(FMOD_DSP *dsp, int index, float volume)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->setInputMix(index, volume);
}


FMOD_RESULT F_API FMOD_DSP_GetInputMix(FMOD_DSP *dsp, int index, float *volume)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getInputMix(index, volume);
}


FMOD_RESULT F_API FMOD_DSP_SetInputLevels(FMOD_DSP *dsp, int index, FMOD_SPEAKER speaker, float *levels, int numlevels)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->setInputLevels(index, speaker, levels, numlevels);
}


FMOD_RESULT F_API FMOD_DSP_GetInputLevels(FMOD_DSP *dsp, int index, FMOD_SPEAKER speaker, float *levels, int numlevels)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getInputLevels(index, speaker, levels, numlevels);
}


FMOD_RESULT F_API FMOD_DSP_SetOutputMix(FMOD_DSP *dsp, int index, float volume)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->setOutputMix(index, volume);
}


FMOD_RESULT F_API FMOD_DSP_GetOutputMix(FMOD_DSP *dsp, int index, float *volume)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getOutputMix(index, volume);
}


FMOD_RESULT F_API FMOD_DSP_SetOutputLevels(FMOD_DSP *dsp, int index, FMOD_SPEAKER speaker, float *levels, int numlevels)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->setOutputLevels(index, speaker, levels, numlevels);
}


FMOD_RESULT F_API FMOD_DSP_GetOutputLevels(FMOD_DSP *dsp, int index, FMOD_SPEAKER speaker, float *levels, int numlevels)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getOutputLevels(index, speaker, levels, numlevels);
}


/*
[API]
[
    [DESCRIPTION]
    Enables or disables a unit for being processed.  

    [PARAMETERS]
    'active'    true = unit is activated, false = unit is deactivated.

    [RETURN_VALUE]

    [REMARKS]
    This does not connect or disconnect a unit in any way, it just disables it so that it is not processed.<br>
    If a unit is disabled, and has inputs, they will also cease to be processed.<br>
    To disable a unit but allow the inputs of the unit to continue being processed, use DSP::setBypass instead.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::getActive
    DSP::setBypass
]
*/
FMOD_RESULT F_API FMOD_DSP_SetActive(FMOD_DSP *dsp, FMOD_BOOL active)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->setActive(active ? true : false);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the active state of a DSP unit.

    [PARAMETERS]
    'active'    Address of a variable that receives the active state of the unit.  true = unit is activated, false = unit is deactivated.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::setActive
    DSP::setBypass
]
*/
FMOD_RESULT F_API FMOD_DSP_GetActive(FMOD_DSP *dsp, FMOD_BOOL *active)
{
    FMOD_RESULT result;
    bool active2;
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = _dsp->getActive(&active2);
    if (result == FMOD_OK)
    {
        if (active)
        {
            *active = active2 ? 1 : 0;
        }
    }

    return result;
}


/*
[API]
[
    [DESCRIPTION]
    Enables or disables the read callback of a DSP unit so that it does or doesn't process the data coming into it.<br>
    A DSP unit that is disabled still processes its inputs, it will just be 'dry'.

    [PARAMETERS]
    'bypass'    Boolean to cause the read callback of the DSP unit to be bypassed or not.  Default = false.

    [RETURN_VALUE]

    [REMARKS]
    If a unit is bypassed, it will still process its inputs.<br>
    To disable the unit and all of its inputs, use DSP::setActive instead.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::getBypass
    DSP::setActive
]
*/
FMOD_RESULT F_API FMOD_DSP_SetBypass(FMOD_DSP *dsp, FMOD_BOOL bypass)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->setBypass(bypass ? true : false);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the bypass state of the DSP unit.

    [PARAMETERS]
    'bypass'        Address of a variable that receieves the bypass state for a DSP unit.  true = unit is not processing audio data, false = unit is processing audio data.  Default = false.

    [RETURN_VALUE]

    [REMARKS]
	If a unit is bypassed, it will still process its inputs, unlike DSP::setActive (when set to false) which causes inputs to stop processing as well.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
	DSP::setBypass
    DSP::setActive
]
*/
FMOD_RESULT F_API FMOD_DSP_GetBypass(FMOD_DSP *dsp, FMOD_BOOL *bypass)
{
    FMOD_RESULT result;
    bool bypass2;
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = _dsp->getBypass(&bypass2);
    if (result == FMOD_OK)
    {
        if (bypass)
        {
            *bypass = bypass2 ? 1 : 0;
        }
    }

    return result;
}


/*
[API]
[
    [DESCRIPTION]
    Calls the DSP unit's reset function, which will clear internal buffers and reset the unit back to an initial state.

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]
    Calling this function is useful if the DSP unit relies on a history to process itself (ie an echo filter).<br>
    If you disconnected the unit and reconnected it to a different part of the network with a different sound, you would want to call this to reset the units state (ie clear and reset the echo filter) so that you dont get left over artifacts from the place it used to be connected.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
]
*/
FMOD_RESULT F_API FMOD_DSP_Reset(FMOD_DSP *dsp)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->reset();
}


/*
[API]
[
    [DESCRIPTION]
    Sets a DSP unit's parameter by index.  To find out the parameter names and range, see the see also field.

    [PARAMETERS]
    'index'     Parameter index for this unit.  Find the number of parameters with DSP::getNumParameters.
    'value'     Parameter value.  The parameter properties can be retrieved with DSP::getParameterInfo.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::getParameterInfo
    DSP::getNumParameters
    DSP::getParameter
]
*/
FMOD_RESULT F_API FMOD_DSP_SetParameter(FMOD_DSP *dsp, int index, float value)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->setParameter(index, value);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves a DSP unit's parameter by index.  To find out the parameter names and range, see the see also field.

    [PARAMETERS]
    'index'         Parameter index for this unit.  Find the number of parameters with DSP::getNumParameters.
    'value'         Address of a variable that receives the parameter value.  The parameter properties can be retrieved with DSP::getParameterInfo.
    'valuestr'      Address of a variable that receives the string containing a formatted or more meaningful representation of the DSP parameter's value.   For example if a switch parameter has on and off (0.0 or 1.0) it will display "ON" or "OFF" by using this parameter.
    'valuestrlen'   Length of the user supplied memory in bytes that valuestr will write to.   This will not exceed 16 bytes.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::getParameterInfo
    DSP::getNumParameters
    DSP::setParameter
]
*/
FMOD_RESULT F_API FMOD_DSP_GetParameter(FMOD_DSP *dsp, int index, float *value, char *valuestr, int valuestrlen)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getParameter(index, value, valuestr, valuestrlen);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of parameters a DSP unit has to control its behaviour.

    [PARAMETERS]
    'numparams'     Address of a variable that receives the number of parameters contained within this DSP unit.

    [RETURN_VALUE]

    [REMARKS]
    Use this to enumerate all parameters of a DSP unit with DSP::getParameter and DSP::getParameterInfo.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::setParameter
    DSP::getParameter
    DSP::getParameterInfo
]
*/
FMOD_RESULT F_API FMOD_DSP_GetNumParameters(FMOD_DSP *dsp, int *numparams)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getNumParameters(numparams);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieve information about a specified parameter within the DSP unit.

    [PARAMETERS]
    'index'             Parameter index for this unit.  Find the number of parameters with DSP::getNumParameters.
    'name'              Address of a variable that receives the name of the parameter.  An example is "Gain".  This is a maximum string length of 16bytes (append \0 in case the plugin has used all 16 bytes for the string).
    'label'             Address of a variable that receives the label of the parameter (ie a parameter type that might go next to the parameter).  An example is "dB".  This is a maximum string length of 16bytes (append \0 in case the plugin has used all 16 bytes for the string).
    'description'       Address of a variable that receives the more descriptive text about the parameter (ie for a tooltip).  An example is "Controls the input level for the effect in decibels".
    'descriptionlen'    Maximum length of user supplied description string in bytes that FMOD will write to.
    'min'               Minimum range of the parameter.
    'max'               Maximum range of the parameter.

    [RETURN_VALUE]

    [REMARKS]
    Use DSP::getNumParameters to find out the number of parameters for this DSP unit.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::setParameter
    DSP::getParameter
    DSP::getNumParameters
]
*/
FMOD_RESULT F_API FMOD_DSP_GetParameterInfo(FMOD_DSP *dsp, int index, char *name, char *label, char *description, int descriptionlen, float *min, float *max)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getParameterInfo(index, name, label, description, descriptionlen, min, max);
}


/*
[API]
[
    [DESCRIPTION]
    Display or hide a DSP unit configuration dialog box inside the target window.

    [PARAMETERS]
    'hwnd'      Target HWND in windows to display configuration dialog.
    'show'      true = show dialog box inside target hwnd.  false = remove dialog from target hwnd.

    [RETURN_VALUE]

    [REMARKS]
    Dialog boxes are used by DSP plugins that prefer to use a graphical user interface to modify their parameters rather than using the other method of enumerating the parameters and using DSP::setParameter.<br>
    These are usually VST plugins.  FMOD Ex plugins do not have configuration dialog boxes.
    To find out what size window to create to store the configuration screen, use DSP::getInfo where you can get the width and height.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::getInfo
    DSP::setParameter
    DSP::getParameter
]
*/
FMOD_RESULT F_API FMOD_DSP_ShowConfigDialog(FMOD_DSP *dsp, void *hwnd, FMOD_BOOL show)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->showConfigDialog(hwnd, show ? true : false);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves information about the current DSP unit, including name, version, default channels and width and height of configuration dialog box if it exists.

    [PARAMETERS]
    'name'          Address of a variable that receives the name of the unit.  This will be a maximum of 32bytes.  If the DSP unit has filled all 32 bytes with the name with no terminating \0 null character it is up to the caller to append a null character.  Optional.  Specify 0 or NULL to ignore. 
    'version'       Address of a variable that receives the version number of the DSP unit.  Version number is usually formated as hex AAAABBBB where the AAAA is the major version number and the BBBB is the minor version number.  Optional.  Specify 0 or NULL to ignore. 
    'channels'      Address of a variable that receives the number of channels the unit was initialized with.  0 means the plugin will process whatever number of channels is currently in the network.  >0 would be mostly used if the unit is a unit that only generates sound, or is not flexible enough to take any number of input channels.  Optional.  Specify 0 or NULL to ignore. 
    'configwidth'   Address of a variable that receives the width of an optional configuration dialog box that can be displayed with DSP::showConfigDialog.  0 means the dialog is not present.  Optional.  Specify 0 or NULL to ignore. 
    'configheight'  Address of a variable that receives the height of an optional configuration dialog box that can be displayed with DSP::showConfigDialog.  0 means the dialog is not present.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::showConfigDialog
]
*/
FMOD_RESULT F_API FMOD_DSP_GetInfo(FMOD_DSP *dsp, char *name, unsigned int *version, int *channels, int *configwidth, int *configheight)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getInfo(name, version, channels, configwidth, configheight);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the pre-defined type of a FMOD registered DSP unit.

    [PARAMETERS]
    'type'  Address of a variable to recieve the FMOD dsp type.

    [RETURN_VALUE]

    [REMARKS]
    This is only valid for built in FMOD effects.  Any user plugins will simply return FMOD_DSP_TYPE_UNKNOWN.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_DSP_TYPE
]
*/
FMOD_RESULT F_API FMOD_DSP_GetType(FMOD_DSP *dsp, FMOD_DSP_TYPE *type)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getType(type);
}


/*
[API]
[
    [DESCRIPTION]
    If a DSP unit is to be played on a channel with System::playDSP, this will set the defaults for frequency, volume, pan and more for the channel.

    [PARAMETERS]
    'frequency'     Default playback frequency for the DSP unit, in hz.  (ie 44100hz).
    'volume'        Default volume for the DSP unit.  0.0 to 1.0.  0.0 = Silent, 1.0 = full volume.  Default = 1.0.
    'pan'           Default pan for the DSP unit.  -1.0 to +1.0.  -1.0 = Full left, 0.0 = center, 1.0 = full right.  Default = 0.0.
    'priority'      Default priority for the DSP unit when played on a channel.  0 to 256.  0 = most important, 256 = least important.  Default = 128.

    [RETURN_VALUE]

    [REMARKS]
    There are no 'ignore' values for these parameters.  Use DSP::getDefaults if you want to change only 1 and leave others unaltered.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::playDSP
    DSP::getDefaults
]
*/
FMOD_RESULT F_API FMOD_DSP_SetDefaults(FMOD_DSP *dsp, float frequency, float volume, float pan, int priority)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->setDefaults(frequency, volume, pan, priority);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the default frequency, volume, pan and more for this DSP unit if it was to ever be played on a channel using System::playDSP.

    [PARAMETERS]
    'frequency'     Address of a variable that receives the default frequency for the DSP unit.  Optional.  Specify 0 or NULL to ignore. 
    'volume'        Address of a variable that receives the default volume for the DSP unit.  Result will be from 0.0 to 1.0.  0.0 = Silent, 1.0 = full volume.  Default = 1.0.  Optional.  Specify 0 or NULL to ignore. 
    'pan'           Address of a variable that receives the default pan for the DSP unit.  Result will be from -1.0 to +1.0.  -1.0 = Full left, 0.0 = center, 1.0 = full right.  Default = 0.0.  Optional.  Specify 0 or NULL to ignore. 
    'priority'      Address of a variable that receives the default priority for the DSP unit when played on a channel.  Result will be from 0 to 256.  0 = most important, 256 = least important.  Default = 128.  Optional.  Specify 0 or NULL to ignore. 

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::setDefaults
    System::playDSP
]
*/
FMOD_RESULT F_API FMOD_DSP_GetDefaults(FMOD_DSP *dsp, float *frequency, float *volume, float *pan, int *priority)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getDefaults(frequency, volume, pan, priority);
}


/*
[API]
[
    [DESCRIPTION]
    Sets a user value that the DSP object will store internally.  Can be retrieved with DSP::getUserData.

    [PARAMETERS]
    'userdata'      Address of user data that the user wishes stored within the DSP object.

    [RETURN_VALUE]

    [REMARKS]
    This function is primarily used in case the user wishes to 'attach' data to an FMOD object.<br>
    It can be useful if an FMOD callback passes an object of this type as a parameter, and the user does not know which object it is (if many of these types of objects exist).  Using DSP::getUserData would help in the identification of the object.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::getUserData
]
*/
FMOD_RESULT F_API FMOD_DSP_SetUserData(FMOD_DSP *dsp, void *userdata)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->setUserData(userdata);
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the user value that that was set by calling the DSP::setUserData function.

    [PARAMETERS]
    'userdata'  Address of a pointer that receives the user data specified with the DSP::setUserData function.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    DSP::setUserData
]
*/
FMOD_RESULT F_API FMOD_DSP_GetUserData(FMOD_DSP *dsp, void **userdata)
{
    FMOD::DSP *_dsp = (FMOD::DSP *)dsp;

    if (!_dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _dsp->getUserData(userdata);
}



/*
[API]
[
    [DESCRIPTION]
    Frees a geometry object and releases its memory.

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
]
*/
FMOD_RESULT F_API FMOD_Geometry_Release(FMOD_GEOMETRY *geometry)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->release();
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Adds a polygon to an existing geometry object.

    [PARAMETERS]
    'directocclusion'   Occlusion value from 0.0 to 1.0 which affects volume or audible frequencies.  0.0 = The polygon does not occlude volume or audible frequencies (sound will be fully audible), 1.0 = The polygon fully occludes (sound will be silent).
    'reverbocclusion'   Occlusion value from 0.0 to 1.0 which affects the reverb mix.  0.0 = The polygon does not occlude reverb (reverb reflections still travel through this polygon), 1.0 = The polyfully fully occludes reverb (reverb reflections will be silent through this polygon).
    'doublesided'       Description of polygon if it is double sided or single sided.  true = polygon is double sided, false = polygon is single sided, and the winding of the polygon (which determines the polygon's normal) determines which side of the polygon will cause occlusion.
    'numvertices'       Number of vertices in this polygon.  This must be at least 3.  Polygons (more than 3 sides) are supported.
    'vertices'          A pointer to an array of vertices located in object space, with the count being the number of vertices described using the numvertices parameter.
    'polygonindex'      Address of a variable to receieve the polygon index for this object.  This index can be used later with other per polygon based geometry functions.
    
    [RETURN_VALUE]

    [REMARKS]
    <b>Note!</b><br>
    - All vertices must lay in the same plane otherwise behaviour may be unpredictable.<br>
    - The polygon is assumed to be convex. A non convex polygon will produce unpredictable behaviour.<br>
    - Polygons with zero area will be ignored.<br>
    <br>
    Vertices of an object are in object space, not world space, and so are relative to the position, or center of the object.  See Geometry::setPosition.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::getNumPolygons
    Geometry::setPosition
    FMOD_VECTOR
]
*/
FMOD_RESULT F_API FMOD_Geometry_AddPolygon(FMOD_GEOMETRY *geometry, float directocclusion, float reverbocclusion, FMOD_BOOL doublesided, int numvertices, const FMOD_VECTOR *vertices, int *polygonindex)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->addPolygon(directocclusion, reverbocclusion, doublesided ? true : false, numvertices, vertices, polygonindex);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the number of polygons stored within this geometry object.

    [PARAMETERS]
    'numpolygons'   Address of a variable to receive the number of polygons within this object.

    [RETURN_VALUE]

    [REMARKS]
    Polygons are added to a geometry object via Geometry::addPolygon.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::AddPolygon
]
*/
FMOD_RESULT F_API FMOD_Geometry_GetNumPolygons(FMOD_GEOMETRY *geometry, int *numpolygons)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->getNumPolygons(numpolygons);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the maximum number of polygons and vertices allocatable for this object.  This is not the number of polygons or vertices currently present.<br>
    The maximum number was set with System::createGeometry.

    [PARAMETERS]
    'maxpolygons'   Address of a variable to receieve the maximum possible number of polygons in this object.
    'maxvertices'   Address of a variable to receieve the maximum possible number of vertices in this object.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createGeometry
    System::loadGeometry
]
*/
FMOD_RESULT F_API FMOD_Geometry_GetMaxPolygons(FMOD_GEOMETRY *geometry, int *maxpolygons, int *maxvertices)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->getMaxPolygons(maxpolygons, maxvertices);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Gets the number of vertices in a polygon which is part of the geometry object.

    [PARAMETERS]
    'index'         Polygon index.  This must be in the range of 0 to Geometry::getNumPolygons minus 1.
    'numvertices'   Address of a variable to receive the number of vertices for the selected polygon.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::getNumPolygons
]
*/
FMOD_RESULT F_API FMOD_Geometry_GetPolygonNumVertices(FMOD_GEOMETRY *geometry, int index, int *numvertices)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->getPolygonNumVertices(index, numvertices);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Alters the position of a polygon's vertex inside a geometry object.

    [PARAMETERS]
    'index'         Polygon index.  This must be in the range of 0 to Geometry::getNumPolygons minus 1.
    'vertexindex'   Vertex index inside the polygon.  This must be in the range of 0 to Geometry::getPolygonNumVertices minus 1.
    'vertex'        Address of an FMOD_VECTOR which holds the new vertex location.

    [RETURN_VALUE]

    [REMARKS]
    <b>Note!</b> There may be some significant overhead with this function as it may cause some reconfiguration of internal data structures used to speed up sound-ray testing.<br>
    You may get better results if you want to modify your object by using Geometry::setPosition, Geometry::setScale and Geometry::setRotation.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::getPolygonNumVertices
    Geometry::getPolygonNumVertices
    Geometry::setPosition
    Geometry::setScale
    Geometry::setRotation
    Geometry::getNumPolygons
    FMOD_VECTOR
]
*/
FMOD_RESULT F_API FMOD_Geometry_SetPolygonVertex(FMOD_GEOMETRY *geometry, int index, int vertexindex, const FMOD_VECTOR *vertex)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->setPolygonVertex(index, vertexindex, vertex);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the position of the vertex inside a geometry object.

    [PARAMETERS]
    'index'         Polygon index.  This must be in the range of 0 to Geometry::getNumPolygons minus 1.
    'vertexindex'   Vertex index inside the polygon.  This must be in the range of 0 to Geometry::getPolygonNumVertices minus 1.
    'vertex'        Address of an FMOD_VECTOR structure which will receive the new vertex location in object space.

    [RETURN_VALUE]

    [REMARKS]
    Vertices are relative to the position of the object.  See Geometry::setPosition.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::getPolygonNumVertices
    Geometry::setPosition
    Geometry::getNumPolygons
    FMOD_VECTOR
]
*/
FMOD_RESULT F_API FMOD_Geometry_GetPolygonVertex(FMOD_GEOMETRY *geometry, int index, int vertexindex, FMOD_VECTOR *vertex)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->getPolygonVertex(index, vertexindex, vertex);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Sets individual attributes for each polygon inside a geometry object.

    [PARAMETERS]
    'index'             Polygon index inside the object.
    'directocclusion'   Occlusion value from 0.0 to 1.0 which affects volume or audible frequencies.  0.0 = The polygon does not occlude volume or audible frequencies (sound will be fully audible), 1.0 = The polygon fully occludes (sound will be silent).
    'reverbocclusion'   Occlusion value from 0.0 to 1.0 which affects the reverb mix.  0.0 = The polygon does not occlude reverb (reverb reflections still travel through this polygon), 1.0 = The polyfully fully occludes reverb (reverb reflections will be silent through this polygon).
    'doublesided'       Description of polygon if it is double sided or single sided.  true = polygon is double sided, false = polygon is single sided, and the winding of the polygon (which determines the polygon's normal) determines which side of the polygon will cause occlusion.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::getPolygonAttributes
    Geometry::getNumPolygons
]
*/
FMOD_RESULT F_API FMOD_Geometry_SetPolygonAttributes(FMOD_GEOMETRY *geometry, int index, float directocclusion, float reverbocclusion, FMOD_BOOL doublesided)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->setPolygonAttributes(index, directocclusion, reverbocclusion, doublesided ? true : false);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the attributes for a particular polygon inside a geometry object.

    [PARAMETERS]
    'index'             Polygon index inside the object.
    'directocclusion'   Address of a variable to receieve the occlusion value from 0.0 to 1.0 which affects volume or audible frequencies.  0.0 = The polygon does not occlude volume or audible frequencies (sound will be fully audible), 1.0 = The polygon fully occludes (sound will be silent).
    'reverbocclusion'   Address of a variable to receieve the occlusion value from 0.0 to 1.0 which affects the reverb mix.  0.0 = The polygon does not occlude reverb (reverb reflections still travel through this polygon), 1.0 = The polyfully fully occludes reverb (reverb reflections will be silent through this polygon).
    'doublesided'       Address of a variable to receieve the description of polygon if it is double sided or single sided.  true = polygon is double sided, false = polygon is single sided, and the winding of the polygon (which determines the polygon's normal) determines which side of the polygon will cause occlusion.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::getPolygonAttributes
    Geometry::getNumPolygons
]
*/
FMOD_RESULT F_API FMOD_Geometry_GetPolygonAttributes(FMOD_GEOMETRY *geometry, int index, float *directocclusion, float *reverbocclusion, FMOD_BOOL *doublesided)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD_RESULT result;
    bool doublesided2;
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = _geometry->getPolygonAttributes(index, directocclusion, reverbocclusion, &doublesided2);
    if (result == FMOD_OK)
    {
        if (doublesided)
        {
            *doublesided = doublesided2 ? 1 : 0;
        }
    }

    return result;
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Enables or disables an object from being processed in the geometry engine.

    [PARAMETERS]
    'active'    true = active, false = not active.  Default = true.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::getActive
]
*/
FMOD_RESULT F_API FMOD_Geometry_SetActive(FMOD_GEOMETRY *geometry, FMOD_BOOL active)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->setActive(active ? true : false);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the user set active state of the geometry object.

    [PARAMETERS]
    'active'    Address of a variable to receive the active state of the object.  true = active, false = not active.  Default = true.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::setActive
]
*/
FMOD_RESULT F_API FMOD_Geometry_GetActive(FMOD_GEOMETRY *geometry, FMOD_BOOL *active)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD_RESULT result;
    bool active2;
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = _geometry->getActive(&active2);
    if (result == FMOD_OK)
    {
        if (active)
        {
            *active = active2 ? 1 : 0;
        }
    }

    return result;
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Sets the orientation of the geometry object.

    [PARAMETERS]
	'forward'   The forwards orientation of the geometry object.  This vector must be of unit length and perpendicular to the up vector.    You can specify 0 or NULL to not update the forwards orientation of the geometry object.
	'up'        The upwards orientation of the geometry object.  This vector must be of unit length and perpendicular to the forwards vector.    You can specify 0 or NULL to not update the upwards orientation of the geometry object.

    [RETURN_VALUE]

    [REMARKS]
    See remarks in System::set3DListenerAttributes for more description on forward and up vectors.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::getRotation
    System::set3DListenerAttributes
    FMOD_VECTOR
]
*/
FMOD_RESULT F_API FMOD_Geometry_SetRotation(FMOD_GEOMETRY *geometry, const FMOD_VECTOR *forward, const FMOD_VECTOR *up)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->setRotation(forward, up);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the orientation of the geometry object.

    [PARAMETERS]
	'forward'   Address of a variable that receives the forwards orientation of the geometry object.  Specify 0 or NULL to ignore.
	'up'        Address of a variable that receives the upwards orientation of the geometry object.  Specify 0 or NULL to ignore.

    [RETURN_VALUE]

    [REMARKS]
    See remarks in System::set3DListenerAttributes for more description on forward and up vectors.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::setRotation
    System::set3DListenerAttributes
    FMOD_VECTOR
]
*/
FMOD_RESULT F_API FMOD_Geometry_GetRotation(FMOD_GEOMETRY *geometry, FMOD_VECTOR *forward, FMOD_VECTOR *up)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->getRotation(forward, up);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Sets the position of the object in world space, which is the same space FMOD sounds and listeners reside in.

    [PARAMETERS]
    'position'      Pointer to a vector containing the 3d position of the object.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::getPosition
    Geometry::setRotation
    Geometry::setScale
    FMOD_VECTOR
]
*/
FMOD_RESULT F_API FMOD_Geometry_SetPosition(FMOD_GEOMETRY *geometry, const FMOD_VECTOR *position)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->setPosition(position);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the position of the object in 3D world space.

    [PARAMETERS]
    'position'      Address of a variable to receive the 3d position of the object.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::setPosition
    FMOD_VECTOR
]
*/
FMOD_RESULT F_API FMOD_Geometry_GetPosition(FMOD_GEOMETRY *geometry, FMOD_VECTOR *position)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->getPosition(position);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Sets the relative scale vector of the geometry object.  An object can be scaled/warped in all 3 dimensions separately using the vector without having to modify polygon data.

    [PARAMETERS]
    'scale'     The scale vector of the object.  Default = 1.0, 1.0, 1.0.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::getScale
    Geometry::setRotation
    Geometry::setPosition
    FMOD_VECTOR
]
*/
FMOD_RESULT F_API FMOD_Geometry_SetScale(FMOD_GEOMETRY *geometry, const FMOD_VECTOR *scale)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->setScale(scale);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the relative scale vector of the geometry object.  An object can be scaled/warped in all 3 dimensions separately using the vector.

    [PARAMETERS]
    'scale'     Address of a variable to receieve the scale vector of the object.  Default = 1.0, 1.0, 1.0.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::setScale
    FMOD_VECTOR
]
*/
FMOD_RESULT F_API FMOD_Geometry_GetScale(FMOD_GEOMETRY *geometry, FMOD_VECTOR *scale)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->getScale(scale);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Saves the geometry object as a serialized binary block, to a user memory buffer.  This can then be saved to a file if required and loaded later with System::loadGeometry.

    [PARAMETERS]
    'data'      Address of a variable to receive the serialized geometry object.  Specify 0 or NULL to have the datasize parameter return the size of the memory required for this saved object.
    'datasize'  Address of a variable to receive the size in bytes required to save this object when 'data' parameter is 0 or NULL.

    [RETURN_VALUE]

    [REMARKS]
    To use this function you will normally need to call it twice.  Once to get the size of the data, then again to write the data to your pointer.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::loadGeometry
    System::createGeometry
]
*/
FMOD_RESULT F_API FMOD_Geometry_Save(FMOD_GEOMETRY *geometry, void *data, int *datasize)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->save(data, datasize);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Sets a user value that the Geometry object will store internally.  Can be retrieved with Geometry::getUserData.

    [PARAMETERS]
    'userdata'      Address of user data that the user wishes stored within the Geometry object.

    [RETURN_VALUE]

    [REMARKS]
    This function is primarily used in case the user wishes to 'attach' data to an FMOD object.<br>
    It can be useful if an FMOD callback passes an object of this type as a parameter, and the user does not know which object it is (if many of these types of objects exist).  Using Geometry::getUserData would help in the identification of the object.
    
    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::getUserData
]
*/
FMOD_RESULT F_API FMOD_Geometry_SetUserData(FMOD_GEOMETRY *geometry, void *userdata)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->setUserData(userdata);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the user value that that was set by calling the Geometry::setUserData function.

    [PARAMETERS]
    'userdata'  Address of a pointer that receives the data specified with the Geometry::setUserData function.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Geometry::setUserData
]
*/
FMOD_RESULT F_API FMOD_Geometry_GetUserData(FMOD_GEOMETRY *geometry, void **userdata)
{
#ifdef FMOD_SUPPORT_GEOMETRY
    FMOD::Geometry *_geometry = (FMOD::Geometry *)geometry;

    if (!_geometry)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _geometry->getUserData(userdata);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}



/*
[API]
[
    [DESCRIPTION]
    Releases the memory for a reverb object and makes it inactive.

    [PARAMETERS]

    [RETURN_VALUE]

    [REMARKS]
    If no reverb objects are created, the ambient reverb will be the only audible reverb.  By default this ambient reverb setting is set to OFF.

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    System::createReverb
    System::setReverbAmbientProperties
]
*/
FMOD_RESULT F_API FMOD_Reverb_Release(FMOD_REVERB *reverb)
{
#ifdef FMOD_SUPPORT_SFXREVERB
    FMOD::Reverb *_reverb = (FMOD::Reverb *)reverb;

    if (!_reverb)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _reverb->release();
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Sets the 3d properties of a 'virtual' reverb object.

    [PARAMETERS]
    'position'      Pointer to a vector containing the 3d position of the center of the reverb in 3d space.   Default = { 0,0,0 }.
    'mindistance'   The distance from the centerpoint that the reverb will have full effect at.  Default = 0.0.
    'maxdistance'   The distance from the centerpoint that the reverb will not have any effect.  Default = 0.0.

    [RETURN_VALUE]

    [REMARKS]
    The 3D reverb object is a sphere having 3D attributes (position, minimum distance, maximum distance) and reverb properties.<br>
    The properties and 3D attributes of all reverb objects collectively determine, along with the listener's position, the settings of and input gains into a single 3D reverb DSP.<br>
    Please note that this only applies to software channels. When the listener is within the sphere of effect of one or more 3d reverbs, the listener's 3D reverb properties are
    a weighted combination of such 3d reverbs. When the listener is outside all of the reverbs, the 3D reverb setting is set to the default ambient reverb setting.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Reverb::get3DAttributes
    System::createReverb
]
*/
FMOD_RESULT F_API FMOD_Reverb_Set3DAttributes(FMOD_REVERB *reverb, const FMOD_VECTOR *position, float mindistance, float maxdistance)
{
#ifdef FMOD_SUPPORT_SFXREVERB
    FMOD::Reverb *_reverb = (FMOD::Reverb *)reverb;

    if (!_reverb)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _reverb->set3DAttributes(position, mindistance, maxdistance);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the 3d attributes of a Reverb object.

    [PARAMETERS]
    'position'      Address of a variable that will receive the 3d position of the center of the reverb in 3d space.   Default = { 0,0,0 }.
    'mindistance'   Address of a variable that will receive the distance from the centerpoint that the reverb will have full effect at.  Default = 0.0.
    'maxdistance'   Address of a variable that will receive the distance from the centerpoint that the reverb will not have any effect.  Default = 0.0.

    [RETURN_VALUE]

    [REMARKS]
    The 3D reverb object is a sphere having 3D attributes (position, minimum distance, maximum distance) and reverb properties.<br>
    The properties and 3D attributes of all reverb objects collectively determine, along with the listener's position, the settings of and input gains into a single 3D reverb DSP.<br>
    Please note that this only applies to software channels. When the listener is within the sphere of effect of one or more 3d reverbs, the listener's 3D reverb properties are
    a weighted combination of such 3d reverbs. When the listener is outside all of the reverbs, the 3D reverb setting is set to the default ambient reverb setting.<br>

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Reverb::set3DAttributes
    System::createReverb
]
*/
FMOD_RESULT F_API FMOD_Reverb_Get3DAttributes(FMOD_REVERB *reverb, FMOD_VECTOR *position, float *mindistance, float *maxdistance)
{
#ifdef FMOD_SUPPORT_SFXREVERB
    FMOD::Reverb *_reverb = (FMOD::Reverb *)reverb;

    if (!_reverb)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _reverb->get3DAttributes(position, mindistance, maxdistance);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Sets reverb parameters for the current reverb object.<br>
	Reverb parameters can be set manually, or automatically using the pre-defined presets given in the fmod.h header.

    [PARAMETERS]
    'properties'	    Address of an FMOD_REVERB_PROPERTIES structure which defines the attributes for the reverb.

    [RETURN_VALUE]

    [REMARKS]	

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    FMOD_REVERB_PROPERTIES
    Reverb::getProperties
    System::createReverb
]
*/
FMOD_RESULT F_API FMOD_Reverb_SetProperties(FMOD_REVERB *reverb, const FMOD_REVERB_PROPERTIES *properties)
{
#ifdef FMOD_SUPPORT_SFXREVERB
    FMOD::Reverb *_reverb = (FMOD::Reverb *)reverb;

    if (!_reverb)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _reverb->setProperties(properties);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the current reverb environment.

    [PARAMETERS]
    'properties'     Address of a variable that receives the current reverb environment description.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Reverb::setProperties
    System::createReverb
]
*/
FMOD_RESULT F_API FMOD_Reverb_GetProperties(FMOD_REVERB *reverb, FMOD_REVERB_PROPERTIES *properties)
{
#ifdef FMOD_SUPPORT_SFXREVERB
    FMOD::Reverb *_reverb = (FMOD::Reverb *)reverb;

    if (!_reverb)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _reverb->getProperties(properties);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Disables or enables a reverb object so that it does or does not contribute to the 3d scene.

    [PARAMETERS]
    'active'    true = active, false = not active.  Default = true.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Reverb::setActive
    System::createReverb
]
*/
FMOD_RESULT F_API FMOD_Reverb_SetActive(FMOD_REVERB *reverb, FMOD_BOOL active)
{
#ifdef FMOD_SUPPORT_SFXREVERB
    FMOD::Reverb *_reverb = (FMOD::Reverb *)reverb;

    if (!_reverb)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _reverb->setActive(active ? true : false);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the active state of the reverb object.

    [PARAMETERS]
    'active'    Address of a variable to receive the current active state of the reverb object.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Reverb::setActive
    System::createReverb
]
*/
FMOD_RESULT F_API FMOD_Reverb_GetActive(FMOD_REVERB *reverb, FMOD_BOOL *active)
{
#ifdef FMOD_SUPPORT_SFXREVERB
    FMOD_RESULT result;
    bool active2;
    FMOD::Reverb *_reverb = (FMOD::Reverb *)reverb;

    if (!_reverb)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    result = _reverb->getActive(&active2);
    if (result == FMOD_OK)
    {
        if (active)
        {
            *active = active2 ? 1 : 0;
        }
    }

    return result;
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Sets a user value that the Reverb object will store internally.  Can be retrieved with Reverb::getUserData.

    [PARAMETERS]
    'userdata'      Address of user data that the user wishes stored within the Reverb object.

    [RETURN_VALUE]

    [REMARKS]
    This function is primarily used in case the user wishes to 'attach' data to an FMOD object.<br>
    It can be useful if an FMOD callback passes an object of this type as a parameter, and the user does not know which object it is (if many of these types of objects exist).  Using Reverb::getUserData would help in the identification of the object.
    
    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Reverb::getUserData
]
*/
FMOD_RESULT F_API FMOD_Reverb_SetUserData(FMOD_REVERB *reverb, void *userdata)
{
#ifdef FMOD_SUPPORT_SFXREVERB
    FMOD::Reverb *_reverb = (FMOD::Reverb *)reverb;

    if (!_reverb)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _reverb->setUserData(userdata);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}


/*
[API]
[
    [DESCRIPTION]
    Retrieves the user value that that was set by calling the Reverb::setUserData function.

    [PARAMETERS]
    'userdata'  Address of a pointer that receives the data specified with the Reverb::setUserData function.

    [RETURN_VALUE]

    [REMARKS]

    [PLATFORMS]
    Win32, Win64, Linux, Linux64, Macintosh, Xbox360, PlayStation 2, PlayStation Portable, PlayStation 3, Wii, Solaris, iPhone

    [SEE_ALSO]
    Reverb::setUserData
]
*/
FMOD_RESULT F_API FMOD_Reverb_GetUserData(FMOD_REVERB *reverb, void **userdata)
{
#ifdef FMOD_SUPPORT_SFXREVERB
    FMOD::Reverb *_reverb = (FMOD::Reverb *)reverb;

    if (!_reverb)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    return _reverb->getUserData(userdata);
#else
    return FMOD_ERR_UNSUPPORTED;
#endif
}

