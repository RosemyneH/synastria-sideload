#include "fmod_settings.h"

#include "fmod_pluginfactory.h"
#include "fmod_codeci.h"
#ifdef FMOD_SUPPORT_DSPCODEC
#include "fmod_dsp_codec.h"
#endif
#include "fmod_dsp_oscillator.h"
#include "fmod_dsp_resampler.h"
#include "fmod_dsp_resampler_multiinput.h"
#include "fmod_dsp_soundcard.h"
#ifdef FMOD_SUPPORT_VSTPLUGIN
#include "fmod_dsp_vstplugin.h"
#endif
#include "fmod_dsp_wavetable.h"
#ifdef FMOD_SUPPORT_WINAMPPLUGIN
#include "fmod_dsp_winampplugin.h"
#endif
#ifdef FMOD_SUPPORT_OPENAL
#include "fmod_output_openal.h"
#endif
#include "fmod_output_polled.h"
#include "fmod_memory.h"
#include "fmod_file.h"
#include "fmod_os_misc.h"
#include "fmod_systemi.h"


#include <stdio.h>

namespace FMOD
{

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 5fc12a06 auto
// OFFSET: 0x008F4130
PluginFactory::PluginFactory()
{
    mSystem = 0;
    FMOD_memset(mPluginPath, 0, FMOD_STRING_MAXPATHLEN);

    mDSPHead.initNode();
    mCodecHead.initNode();
    mOutputHead.initNode();
}



/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 7d1b8a8a manual
// OFFSET: 0x008F4190
FMOD_RESULT PluginFactory::release()
{
    FMOD_RESULT result;
    int num, count;

    /*
        Cleanup codecs
    */
    result = getNumCodecs(&num);
    if (result != FMOD_OK)
    {
        return result;
    }

    for (count = 0; count < num; count++)
    {
        result = unloadPlugin(FMOD_PLUGINTYPE_CODEC, 0);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    /*
        Cleanup DSPs
    */
    result = getNumDSPs(&num);
    if (result != FMOD_OK)
    {
        return result;
    }

    for (count = 0; count < num; count++)
    {
        result = unloadPlugin(FMOD_PLUGINTYPE_DSP, 0);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    /*
        Cleanup Outputs
    */
    result = getNumOutputs(&num);
    if (result != FMOD_OK)
    {
        return result;
    }

    for (count = 0; count < num; count++)
    {
        result = unloadPlugin(FMOD_PLUGINTYPE_OUTPUT, 0);
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    FMOD_Memory_Free(this);

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 176fa232 auto
// OFFSET: 0x008F38D0
FMOD_RESULT PluginFactory::setSystem(SystemI *system)  
{ 
    mSystem = system;  
    return FMOD_OK; 
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
    FMOD_OK

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT PluginFactory::getSystem(SystemI **system) 
{ 
    *system = mSystem; 
    return FMOD_OK; 
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 3ef3bd77 auto
// OFFSET: 0x008F38F0
FMOD_RESULT PluginFactory::setPluginPath(const char *path)
{
    if (FMOD_strlen(path) >= FMOD_STRING_MAXPATHLEN)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    FMOD_strncpy(mPluginPath, path, FMOD_STRING_MAXPATHLEN);

    return FMOD_OK;
}

#ifdef FMOD_SUPPORT_DLLS

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT PluginFactory::loadPlugin(const char *dllname, FMOD_PLUGINTYPE *plugintype, int *handle)
{
    FMOD_RESULT                 result;
    FMOD_OS_LIBRARY            *dllhandle;
    FMOD_CODEC_DESCRIPTION     *(F_CALLBACK *getcodecdesc   )() = 0;
    FMOD_CODEC_DESCRIPTION_EX  *(F_CALLBACK *getcodecdescex )() = 0;
    FMOD_DSP_DESCRIPTION       *(F_CALLBACK *getdspdesc     )() = 0;
    FMOD_DSP_DESCRIPTION_EX    *(F_CALLBACK *getdspdescex   )() = 0;
    FMOD_OUTPUT_DESCRIPTION    *(F_CALLBACK *getoutputdesc  )() = 0;
    FMOD_OUTPUT_DESCRIPTION_EX *(F_CALLBACK *getoutputdescex)() = 0;
#ifdef FMOD_SUPPORT_VSTPLUGIN
    AEffect                    *(VSTCALLBACK *VSTmain)(audioMasterCallback) = 0;
#endif
#ifdef FMOD_SUPPORT_WINAMPPLUGIN
    winampDSPHeader            *( *winampGetHeader)() = 0;
#endif
    char                        path[FMOD_STRING_MAXPATHLEN];
    const char                 *beginning;
    const char                 *end;
    char                        symbol[50];

    FMOD_strncpy(path, mPluginPath, FMOD_STRING_MAXPATHLEN);
    if (FMOD_strlen(path) && path[FMOD_strlen(path) - 1] != '\\' && path[FMOD_strlen(path) - 1] != '/')
    {
        FMOD_strcat(path, "/");
    }
    FMOD_strcat(path, dllname);

    if (FMOD_strlen(path))
    {
        #if defined(PLATFORM_WINDOWS)
        if (FMOD_strncmp(&path[FMOD_strlen(path)-4], ".dll", 4))
        {
            FMOD_strcat(path, ".dll");
        }
        #elif defined(PLATFORM_MAC)
        if (FMOD_strncmp(&path[FMOD_strlen(path)-6], ".dylib", 6))
        {
            /*
                Could be a VST / Bundle
            */
            if ((FMOD_strncmp(&path[FMOD_strlen(path)-4], ".vst", 4)) && (FMOD_strncmp(&path[FMOD_strlen(path)-7], ".bundle", 7)))
            {
                FMOD_strcat(path, ".dylib");
            }
        }
        #elif defined(PLATFORM_LINUX) || defined(PLATFORM_SOLARIS)
        if (FMOD_strncmp(&path[FMOD_strlen(path)-3], ".so", 3))
        {
            FMOD_strcat(path, ".so");
        }
        #endif
    }

    result = FMOD_OS_Library_Load(path, &dllhandle);
    if (result != FMOD_OK)
    {
        result = FMOD_OS_Library_Load(dllname, &dllhandle);    /* Try without the search path, in case they were actually using a relative path to the app */
        if (result != FMOD_OK)
        {
            return result;
        }
    }

    #if defined(PLATFORM_LINUX) || defined(PLATFORM_WINDOWS64)
    beginning = "";
    end       = "";
    #elif defined(PLATFORM_WINDOWS)
    beginning = "_";
    end       = "@0";
    #elif defined(PLATFORM_MAC_PPC)
    beginning = "_";
    end       = "";
    #else
    beginning = "";
    end       = "";
    #endif
    
    sprintf(symbol, "%sFMODGetCodecDescription%s", beginning, end); 

    result = FMOD_OS_Library_GetProcAddress(dllhandle, symbol, (void **)&getcodecdesc);
    if (result == FMOD_OK)
    {
        return registerCodec(getcodecdesc(), plugintype, handle);
    }

    sprintf(symbol, "%sFMODGetCodecDescriptionEx%s", beginning, end);
    result = FMOD_OS_Library_GetProcAddress(dllhandle, symbol, (void **)&getcodecdescex);
    if (result == FMOD_OK)
    {
        return registerCodec(getcodecdescex(), plugintype, handle);
    }

    sprintf(symbol, "%sFMODGetDSPDescription%s", beginning, end);
    result = FMOD_OS_Library_GetProcAddress(dllhandle, symbol, (void **)&getdspdesc);
    if (result == FMOD_OK)
    {
        return registerDSP(getdspdesc(), plugintype, handle);
    }

    sprintf(symbol, "%sFMODGetDSPDescriptionEx%s", beginning, end);
    result = FMOD_OS_Library_GetProcAddress(dllhandle, symbol, (void **)&getdspdescex);
    if (result == FMOD_OK)
    {
        return registerDSP(getdspdescex(), plugintype, handle);
    }

    sprintf(symbol, "%sFMODGetOutputDescription%s", beginning, end);
    result = FMOD_OS_Library_GetProcAddress(dllhandle, symbol, (void **)&getoutputdesc);
    if (result == FMOD_OK)
    {
        return registerOutput(getoutputdesc(), plugintype, handle);
    }

    sprintf(symbol, "%sFMODGetOutputDescriptionEx%s", beginning, end);
    result = FMOD_OS_Library_GetProcAddress(dllhandle, symbol, (void **)&getoutputdescex);
    if (result == FMOD_OK)
    {
        return registerOutput(getoutputdescex(), plugintype, handle);
    }

#ifdef FMOD_SUPPORT_VSTPLUGIN
    result = FMOD_OS_Library_GetProcAddress(dllhandle, "VSTPluginMain", (void **)&VSTmain);     // VST 2.4 and onward use VSTPluginMain
    if (result != FMOD_OK)
    {    
        #ifdef PLATFORM_MAC
        result = FMOD_OS_Library_GetProcAddress(dllhandle, "main_macho", (void **)&VSTmain);
        #else
        result = FMOD_OS_Library_GetProcAddress(dllhandle, "main", (void **)&VSTmain);
        #endif
    }
    if (result == FMOD_OK)
    {
        AEffect *effect;
        
        effect = VSTmain(DSPVSTPlugin::audioMasterCB);

        if (effect->magic == kEffectMagic)
        {
            struct ERect
            {
	            short top;
	            short left;
	            short bottom;
	            short right;
            } *rect = 0;
            FMOD_DSP_DESCRIPTION_EX *dspdesc;
            FMOD_DSP_PARAMETERDESC  *dspparams = (FMOD_DSP_PARAMETERDESC *)FMOD_Memory_Calloc(sizeof(FMOD_DSP_PARAMETERDESC) * effect->numParams);

            dspdesc           = DSPVSTPlugin::getDescriptionEx();
            dspdesc->mAEffect = effect;

            dspdesc->version  = effect->dispatcher(effect, effGetVendorVersion, 0, 0, NULL, 0);

            sprintf(dspdesc->name, "VST ");
            effect->dispatcher(effect, effGetEffectName, 0, 0, (void *)(dspdesc->name + 4), 0);
            effect->dispatcher(effect, effEditGetRect, 0, 0, &rect, 0);

            /*
                If this plugin has no name, derive one from its filename
            */
            if (FMOD_strlen(dspdesc->name) == 4)
            {
                int i;

                for (i=FMOD_strlen(dllname) - 1; i > 0; i--)
                {
                    if ((dllname[i] == '/') || (dllname[i] == '\\'))
                    {
                        i++;
                        break;
                    }
                }

                int   len = FMOD_strlen(&dllname[i]);
                char *p   = (char *)&dllname[i] + len - 4;
                if (p >= &dllname[i])
                {
                    if ((FMOD_tolower(p[0]) == '.') &&
                        (FMOD_tolower(p[1]) == 'd') &&
                        (FMOD_tolower(p[2]) == 'l') &&
                        (FMOD_tolower(p[3]) == 'l'))
                    {
                        len -= 4;
                    }
                }

                FMOD_strncat(dspdesc->name, &dllname[i], len);
            }

            if (rect)
            {
                dspdesc->configheight = rect->bottom - rect->top;
                dspdesc->configwidth  = rect->right  - rect->left;
            }

            /*
                Fill in dspparams
            */
            for (int count = 0; count < effect->numParams; count++)
            {
                dspparams[count].min         = 0.0f;
                dspparams[count].max         = 1.0f;
                dspparams[count].defaultval  = effect->getParameter(effect, count);
                dspparams[count].description = NULL;

                effect->dispatcher(effect, effGetParamName, count, 0, dspparams[count].name, 0);
                effect->dispatcher(effect, effGetParamLabel, count, 0, dspparams[count].label, 0);
            }
            dspdesc->paramdesc      = dspparams;
            dspdesc->numparameters  = effect->numParams;

            return registerDSP(dspdesc, plugintype, handle);
        }
    }
#endif

#ifdef FMOD_SUPPORT_WINAMPPLUGIN
    result = FMOD_OS_Library_GetProcAddress(dllhandle, "winampDSPGetHeader2", (void **)&winampGetHeader);
    if (result == FMOD_OK)
    {
        winampDSPHeader         *header;

        header = winampGetHeader();

        /*
            Register each module as a seperate plugin
        */
        for (int count = 0; ; count++)
        {
            FMOD_DSP_DESCRIPTION_EX *dspdesc = NULL;
            winampDSPModule         *module  = NULL;

            module = header->getModule(count);
            if (!module)
            {
                break;
            }

            module->hDllInstance = dllhandle;

            dspdesc           = DSPWinampPlugin::getDescriptionEx();
            dspdesc->mAEffect = module;

            dspdesc->version  = header->version;

            sprintf(dspdesc->name, "WINAMP ");

            FMOD_strncpy((dspdesc->name + 7), module->description, 25);

            dspdesc->name[31] = '\0';

            /*
                If this plugin has no name, derive one from its filename
            */
            if (FMOD_strlen(dspdesc->name) == 7)
            {
                int i;

                for (i=FMOD_strlen(dllname) - 1; i > 0; i--)
                {
                    if ((dllname[i] == '/') || (dllname[i] == '\\'))
                    {
                        i++;
                        break;
                    }
                }

                int   len = FMOD_strlen(&dllname[i]);
                char *p   = (char *)&dllname[i] + len - 4;
                if (p >= &dllname[i])
                {
                    if ((FMOD_tolower(p[0]) == '.') &&
                        (FMOD_tolower(p[1]) == 'd') &&
                        (FMOD_tolower(p[2]) == 'l') &&
                        (FMOD_tolower(p[3]) == 'l'))
                    {
                        len -= 4;
                    }
                }

                FMOD_strncat(dspdesc->name, &dllname[i], len);
            }

            dspdesc->numparameters  = 0;

            result = registerDSP(dspdesc, plugintype, handle);
        }
    }
#endif

    return result;
}

#endif

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT PluginFactory::unloadPlugin(FMOD_PLUGINTYPE plugintype, int handle)
{
    FMOD_RESULT result;

    switch (plugintype)
    {
        case FMOD_PLUGINTYPE_OUTPUT:
        {
            FMOD_OUTPUT_DESCRIPTION_EX *output;

            result = getOutput(handle, &output);
            if (result != FMOD_OK)
            {
                return result;
            }

            #ifdef FMOD_SUPPORT_DLLS
            if (output->mModule)
            {
                FMOD_OS_Library_Free(output->mModule);
            }
            #endif
            output->removeNode();
            FMOD_Memory_Free(output);
            break;
        }
        case FMOD_PLUGINTYPE_CODEC:
        {
            FMOD_CODEC_DESCRIPTION_EX *codec;

            result = getCodec(handle, &codec);
            if (result != FMOD_OK)
            {
                return result;
            }

            #ifdef FMOD_SUPPORT_DLLS
            if (codec->mModule)
            {
                FMOD_OS_Library_Free(codec->mModule);
            }
            #endif
            codec->removeNode();
            FMOD_Memory_Free(codec);
            break;
        }
        case FMOD_PLUGINTYPE_DSP:
        {
            FMOD_DSP_DESCRIPTION_EX *dsp;

            result = getDSP(handle, &dsp);
            if (result != FMOD_OK)
            {
                return result;
            }

            #ifdef FMOD_SUPPORT_DLLS
            if (dsp->mAEffect)
            {
                if (dsp->paramdesc)
                {
                    FMOD_Memory_Free(dsp->paramdesc);
                }
            }
            #endif

            #ifdef FMOD_SUPPORT_DLLS
            if (dsp->mModule)
            {
                FMOD_OS_Library_Free(dsp->mModule);
            }
            #endif
            dsp->removeNode();
            FMOD_Memory_Free(dsp);
            break;
        }
        default:
        {
            return FMOD_ERR_INVALID_PARAM;
        }
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 77af32c2 manual
// OFFSET: 0x008F3930
FMOD_RESULT PluginFactory::registerCodec(FMOD_CODEC_DESCRIPTION_EX *description, FMOD_PLUGINTYPE *plugintype, int *handle)
{
    FMOD_CODEC_DESCRIPTION_EX *newdesc;

    if (!description)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    newdesc = (FMOD_CODEC_DESCRIPTION_EX *)FMOD_Memory_Calloc(sizeof(FMOD_CODEC_DESCRIPTION_EX));
    if (!newdesc)
    {
        return FMOD_ERR_MEMORY;
    }

    newdesc->name                    = description->name;
    newdesc->version                 = description->version;
    newdesc->timeunits               = description->timeunits;
    newdesc->defaultasstream         = description->defaultasstream;
    newdesc->open                    = description->open;
    newdesc->close                   = description->close;
    newdesc->read                    = description->read;
    newdesc->getlength               = description->getlength;
    newdesc->setposition             = description->setposition;
    newdesc->getposition             = description->getposition;
    newdesc->soundcreate             = description->soundcreate;
    newdesc->getwaveformat           = description->getwaveformat;
    newdesc->mType                   = description->mType;
    newdesc->mSize                   = description->mSize;
    newdesc->mModule                 = description->mModule;
    newdesc->reset                   = description->reset;
    newdesc->canpoint                = description->canpoint;

    newdesc->addBefore(&mCodecHead);

    if (plugintype)
    {
        *plugintype = FMOD_PLUGINTYPE_CODEC;
    }
    if (handle)
    {
        *handle = mCodecHead.getNodeIndex(newdesc);
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT PluginFactory::registerCodec(FMOD_CODEC_DESCRIPTION *description, FMOD_PLUGINTYPE *plugintype, int *handle)
{
    FMOD_CODEC_DESCRIPTION_EX *newdesc;

    if (!description)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    newdesc = (FMOD_CODEC_DESCRIPTION_EX *)FMOD_Memory_Calloc(sizeof(FMOD_CODEC_DESCRIPTION_EX));
    if (!newdesc)
    {
        return FMOD_ERR_MEMORY;
    }

    newdesc->name                    = description->name;
    newdesc->version                 = description->version;
    newdesc->timeunits               = description->timeunits;
    newdesc->defaultasstream         = description->defaultasstream;
    newdesc->open                    = description->open;
    newdesc->close                   = description->close;
    newdesc->read                    = description->read;
    newdesc->getlength               = description->getlength;
    newdesc->setposition             = description->setposition;
    newdesc->getposition             = description->getposition;
    newdesc->soundcreate             = description->soundcreate;
    newdesc->getwaveformat           = description->getwaveformat;
    newdesc->mType                   = FMOD_SOUND_TYPE_UNKNOWN;
    newdesc->mSize                   = sizeof(Codec);
    newdesc->mModule                 = 0;
    newdesc->reset                   = 0;
    newdesc->canpoint                = 0;

    newdesc->addBefore(&mCodecHead);

    if (plugintype)
    {
        *plugintype = FMOD_PLUGINTYPE_CODEC;
    }
    if (handle)
    {
        *handle = mCodecHead.getNodeIndex(newdesc);
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 73c34682 manual
// OFFSET: 0x008F3A30
FMOD_RESULT PluginFactory::registerDSP(FMOD_DSP_DESCRIPTION_EX *description, FMOD_PLUGINTYPE *plugintype, int *handle)
{
    FMOD_DSP_DESCRIPTION_EX *newdesc;

    if (!description)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    newdesc = (FMOD_DSP_DESCRIPTION_EX *)FMOD_Memory_Calloc(sizeof(FMOD_DSP_DESCRIPTION_EX));
    if (!newdesc)
    {
        return FMOD_ERR_MEMORY;
    }

    FMOD_strcpy(newdesc->name, description->name);
    newdesc->version              = description->version;
    newdesc->channels             = description->channels;
    newdesc->create               = description->create;
    newdesc->release              = description->release;
    newdesc->reset                = description->reset;
    newdesc->read                 = description->read;
    newdesc->setposition          = description->setposition;

    newdesc->numparameters         = description->numparameters;
    newdesc->paramdesc             = description->paramdesc;
    newdesc->setparameter          = description->setparameter;
    newdesc->getparameter          = description->getparameter;
    newdesc->config                = description->config;
    newdesc->configwidth           = description->configwidth;
    newdesc->configheight          = description->configheight;
    newdesc->userdata              = description->userdata;

    newdesc->mType                 = description->mType;
    newdesc->mCategory             = description->mCategory;
    newdesc->mSize                 = description->mSize;
#ifdef FMOD_SUPPORT_DLLS
    newdesc->mModule               = description->mModule;
    newdesc->mAEffect              = description->mAEffect;
#endif
    newdesc->mFormat               = description->mFormat;
    newdesc->mResamplerBlockLength = description->mResamplerBlockLength;

    newdesc->addBefore(&mDSPHead);

    if (plugintype)
    {
        *plugintype = FMOD_PLUGINTYPE_DSP;
    }
    if (handle)
    {
        *handle = mDSPHead.getNodeIndex(newdesc);
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT PluginFactory::registerDSP(FMOD_DSP_DESCRIPTION *description, FMOD_PLUGINTYPE *plugintype, int *handle)
{
    FMOD_DSP_DESCRIPTION_EX *newdesc;

    if (!description)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    newdesc = (FMOD_DSP_DESCRIPTION_EX *)FMOD_Memory_Calloc(sizeof(FMOD_DSP_DESCRIPTION_EX));
    if (!newdesc)
    {
        return FMOD_ERR_MEMORY;
    }

    FMOD_strcpy(newdesc->name, description->name);
    newdesc->version               = description->version;
    newdesc->channels              = description->channels;
    newdesc->create                = description->create;
    newdesc->release               = description->release;
    newdesc->reset                 = description->reset;
    newdesc->read                  = description->read;
    newdesc->setposition           = description->setposition;

    newdesc->numparameters         = description->numparameters;
    newdesc->paramdesc             = description->paramdesc;
    newdesc->setparameter          = description->setparameter;
    newdesc->getparameter          = description->getparameter;
    newdesc->config                = description->config;
    newdesc->configwidth           = description->configwidth;
    newdesc->configheight          = description->configheight;
    newdesc->userdata              = description->userdata;

    newdesc->mType                 = FMOD_DSP_TYPE_UNKNOWN;
    newdesc->mSize                 = sizeof(DSPI);
    newdesc->mFormat               = FMOD_SOUND_FORMAT_PCMFLOAT;

    newdesc->addBefore(&mDSPHead);

    if (plugintype)
    {
        *plugintype = FMOD_PLUGINTYPE_DSP;
    }
    if (handle)
    {
        *handle = mDSPHead.getNodeIndex(newdesc);
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: b2045be1 manual
// OFFSET: 0x008F3B60
FMOD_RESULT PluginFactory::registerOutput(FMOD_OUTPUT_DESCRIPTION_EX *description, FMOD_PLUGINTYPE *plugintype, int *handle)
{
    FMOD_OUTPUT_DESCRIPTION_EX *newdesc;

    if (!description)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    newdesc = (FMOD_OUTPUT_DESCRIPTION_EX *)FMOD_Memory_Calloc(sizeof(FMOD_OUTPUT_DESCRIPTION_EX));
    if (!newdesc)
    {
        return FMOD_ERR_MEMORY;
    }

    newdesc->name                   = description->name;
    newdesc->version                = description->version;
    newdesc->polling                = description->polling;
    newdesc->getnumdrivers          = description->getnumdrivers;
    newdesc->getdrivername          = description->getdrivername;
    newdesc->getdrivercaps          = description->getdrivercaps;
    newdesc->init                   = description->init;
    newdesc->close                  = description->close;
    newdesc->start                  = description->start;
    newdesc->stop                   = description->stop;
    newdesc->update                 = description->update;
    newdesc->gethandle              = description->gethandle;
    newdesc->getposition            = description->getposition;
    newdesc->lock                   = description->lock;
    newdesc->unlock                 = description->unlock;

    newdesc->getsamplemaxchannels   = description->getsamplemaxchannels;
    newdesc->getdrivercapsex        = description->getdrivercapsex;
    newdesc->getdrivercapsex2       = description->getdrivercapsex2;
    newdesc->initex                 = description->initex;
    newdesc->start                  = description->start;
    newdesc->stop                   = description->stop;
    newdesc->updatefinished         = description->updatefinished;
    newdesc->createsample           = description->createsample;
    newdesc->getsoundram            = description->getsoundram;
    newdesc->record_getnumdrivers   = description->record_getnumdrivers;
    newdesc->record_getdrivername   = description->record_getdrivername;
    newdesc->record_getdrivercaps   = description->record_getdrivercaps;
    newdesc->record_start           = description->record_start;
    newdesc->record_stop            = description->record_stop;
    newdesc->record_getposition     = description->record_getposition;
    newdesc->record_lock            = description->record_lock;
    newdesc->record_unlock          = description->record_unlock;
    newdesc->reverb_setproperties   = description->reverb_setproperties;
	newdesc->postmixcallback		= description->postmixcallback;
    newdesc->mType                  = description->mType;
    newdesc->mSize                  = description->mSize;
    newdesc->mModule                = description->mModule;

    newdesc->addBefore(&mOutputHead);

    if (plugintype)
    {
        *plugintype = FMOD_PLUGINTYPE_OUTPUT;
    }
    if (handle)
    {
        *handle = mOutputHead.getNodeIndex(newdesc);
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
FMOD_RESULT PluginFactory::registerOutput(FMOD_OUTPUT_DESCRIPTION *description, FMOD_PLUGINTYPE *plugintype, int *handle)
{
    FMOD_OUTPUT_DESCRIPTION_EX *newdesc;

    if (!description)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    newdesc = (FMOD_OUTPUT_DESCRIPTION_EX *)FMOD_Memory_Calloc(sizeof(FMOD_OUTPUT_DESCRIPTION_EX));
    if (!newdesc)
    {
        return FMOD_ERR_MEMORY;
    }

    newdesc->name           = description->name;
    newdesc->version        = description->version;
    newdesc->polling        = description->polling;
    newdesc->getnumdrivers  = description->getnumdrivers;
    newdesc->getdrivername  = description->getdrivername;
    newdesc->getdrivercaps  = description->getdrivercaps;
    newdesc->init           = description->init;
    newdesc->close          = description->close;
    newdesc->update         = description->update;
    newdesc->gethandle      = description->gethandle;
    newdesc->getposition    = description->getposition;
    newdesc->lock           = description->lock;
    newdesc->unlock         = description->unlock;

    newdesc->mType          = FMOD_OUTPUTTYPE_UNKNOWN;
    newdesc->mSize          = sizeof(Output);
    newdesc->mModule        = 0;

    newdesc->addBefore(&mOutputHead);

    if (plugintype)
    {
        *plugintype = FMOD_PLUGINTYPE_OUTPUT;
    }
    if (handle)
    {
        *handle = mOutputHead.getNodeIndex(newdesc);
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 052ea932 manual
// OFFSET: 0x008F3D00
FMOD_RESULT PluginFactory::getNumCodecs(int *numcodecs)
{
    FMOD_CODEC_DESCRIPTION_EX *current;

    if (!numcodecs)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *numcodecs = 0;

    current = (FMOD_CODEC_DESCRIPTION_EX *)mCodecHead.getNext();
    while (current != &mCodecHead)
    {
        (*numcodecs)++;
        current = (FMOD_CODEC_DESCRIPTION_EX *)current->getNext();
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 7d93ae82 manual
// OFFSET: 0x008F3D50
FMOD_RESULT PluginFactory::getNumDSPs(int *numdsps)
{
    FMOD_DSP_DESCRIPTION_EX *current;

    if (!numdsps)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *numdsps = 0;

    current = (FMOD_DSP_DESCRIPTION_EX *)mDSPHead.getNext();
    while (current != &mDSPHead)
    {
        (*numdsps)++;
        current = (FMOD_DSP_DESCRIPTION_EX *)current->getNext();
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: d89fe209 manual
// OFFSET: 0x008F3DA0
FMOD_RESULT PluginFactory::getNumOutputs(int *numoutputs)
{
    FMOD_OUTPUT_DESCRIPTION_EX *current;

    if (!numoutputs)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *numoutputs = 0;

    current = (FMOD_OUTPUT_DESCRIPTION_EX *)mOutputHead.getNext();
    while (current != &mOutputHead)
    {
        (*numoutputs)++;
        current = (FMOD_OUTPUT_DESCRIPTION_EX *)current->getNext();
    }

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: db4cb658 manual
// OFFSET: 0x008F3DF0
FMOD_RESULT PluginFactory::getCodec(int handle, FMOD_CODEC_DESCRIPTION_EX **codecdesc)
{
    FMOD_CODEC_DESCRIPTION_EX *current;
    int count;

    if (!codecdesc)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *codecdesc = 0;

    count = 0;
    current = (FMOD_CODEC_DESCRIPTION_EX *)mCodecHead.getNext();
    while (current != &mCodecHead)
    {
        if (count == handle)
        {
            *codecdesc = current;
            return FMOD_OK;
        }

        count++;
        current = (FMOD_CODEC_DESCRIPTION_EX *)current->getNext();
    }

    return FMOD_ERR_PLUGIN_MISSING;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 247d7f2f manual
// OFFSET: 0x008F3E60
FMOD_RESULT PluginFactory::getDSP(int handle, FMOD_DSP_DESCRIPTION_EX **dspdesc)
{
    FMOD_DSP_DESCRIPTION_EX *current;
    int count;

    if (!dspdesc)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *dspdesc = 0;

    count = 0;
    current = (FMOD_DSP_DESCRIPTION_EX *)mDSPHead.getNext();
    while (current != &mDSPHead)
    {
        if (count == handle)
        {
            *dspdesc = current;
            return FMOD_OK;
        }

        count++;
        current = (FMOD_DSP_DESCRIPTION_EX *)current->getNext();
    }

    return FMOD_ERR_PLUGIN_MISSING;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 326c50b3 manual
// OFFSET: 0x008F3ED0
FMOD_RESULT PluginFactory::getOutput(int handle, FMOD_OUTPUT_DESCRIPTION_EX **outputdesc)
{
    FMOD_OUTPUT_DESCRIPTION_EX *current;
    int count;

    if (!outputdesc)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    *outputdesc = 0;

    count = 0;
    current = (FMOD_OUTPUT_DESCRIPTION_EX *)mOutputHead.getNext();
    while (current != &mOutputHead)
    {
        if (count == handle)
        {
            *outputdesc = current;
            return FMOD_OK;
        }

        count++;
        current = (FMOD_OUTPUT_DESCRIPTION_EX *)current->getNext();
    }

    return FMOD_ERR_PLUGIN_MISSING;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 9343f814 auto
// OFFSET: 0x008F43B0
FMOD_RESULT PluginFactory::createCodec(FMOD_CODEC_DESCRIPTION_EX *codecdesc, Codec **codec)
{
    Codec *newcodec;

    if (!codecdesc || !codec)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    newcodec = FMOD_Object_CallocSize(Codec, (unsigned int)codecdesc->mSize);
    if (!newcodec)
    {
        return FMOD_ERR_MEMORY;
    }

    /*
        Copy the description out of the source codec to the new codec.
    */
    FMOD_memcpy(&(newcodec->mDescription), codecdesc, sizeof(FMOD_CODEC_DESCRIPTION_EX));

    *codec = newcodec;

    if (!newcodec->mDescription.getwaveformat)
    {
        newcodec->mDescription.getwaveformat = &Codec::defaultGetWaveFormat;
    }

    return FMOD_OK;
}

#ifdef FMOD_SUPPORT_SOFTWARE

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: a545de65 manual
// OFFSET: 0x008F4670
FMOD_RESULT PluginFactory::createDSP(FMOD_DSP_DESCRIPTION_EX *dspdesc, DSPI **dsp)
{
    FMOD_RESULT result;
    DSPI *newdsp;
    unsigned int size = 0;

    if (!dspdesc || !dsp)
    {
        return FMOD_ERR_INVALID_PARAM;
    }

    size = dspdesc->mSize;

    if (*dsp)
    {
        newdsp = *dsp;
    }
    else
    {
        switch (dspdesc->mCategory)
        {
            case FMOD_DSP_CATEGORY_SOUNDCARD:
            {
                newdsp = FMOD_Object_CallocSize(DSPSoundCard, size);
                break;
            }
            case FMOD_DSP_CATEGORY_FILTER:
            {
                size = size < sizeof(DSPFilter) ? sizeof(DSPFilter) : size;
                newdsp = (DSPI *)FMOD_Memory_Calloc(size);
                if (!newdsp)
                {
                    return FMOD_ERR_MEMORY;
                }
                new (newdsp) DSPFilter;
                break;
            }
#ifdef FMOD_SUPPORT_DSPCODEC
            case FMOD_DSP_CATEGORY_DSPCODECMPEG:
            case FMOD_DSP_CATEGORY_DSPCODECADPCM:
            case FMOD_DSP_CATEGORY_DSPCODECXMA:
            case FMOD_DSP_CATEGORY_DSPCODECRAW:
            {
                size = size < sizeof(DSPCodec) ? sizeof(DSPCodec) : size;
                newdsp = (DSPI *)FMOD_Memory_Calloc(size);
				if (!newdsp)
                {
                    return FMOD_ERR_MEMORY;
                }

                if (0)
                {
                }
                #ifdef FMOD_SUPPORT_MPEG
                else if (dspdesc->mCategory == FMOD_DSP_CATEGORY_DSPCODECMPEG)
                {
                    new (newdsp) DSPCodecMPEG;
                }
                #endif
                #ifdef FMOD_SUPPORT_IMAADPCM
                else if (dspdesc->mCategory == FMOD_DSP_CATEGORY_DSPCODECADPCM)
                {
                    new (newdsp) DSPCodecADPCM;
                }
                #endif
                #ifdef FMOD_SUPPORT_XMA
                else if (dspdesc->mCategory == FMOD_DSP_CATEGORY_DSPCODECXMA)
                {
                    new (newdsp) DSPCodecXMA;
                }
                #endif
                #ifdef FMOD_SUPPORT_RAW
                else if (dspdesc->mCategory == FMOD_DSP_CATEGORY_DSPCODECRAW)
                {
                    new (newdsp) DSPCodecRaw;
                }
                #endif
                else
                {
                    return FMOD_ERR_FORMAT;
                }
                break;
            }
#endif
            case FMOD_DSP_CATEGORY_RESAMPLER:
            {
                size = size < sizeof(DSPResamplerMultiInput) ? sizeof(DSPResamplerMultiInput) : size;
                newdsp = (DSPI *)FMOD_Memory_Calloc(size);
                if (!newdsp)
                {
                    return FMOD_ERR_MEMORY;
                }
                new (newdsp) DSPResamplerMultiInput;
                break;
            }
            case FMOD_DSP_CATEGORY_WAVETABLE:
            {
                size = size < sizeof(DSPWaveTable) ? sizeof(DSPWaveTable) : size;
                newdsp = (DSPI *)FMOD_Memory_Calloc(size);
                if (!newdsp)
                {
                    return FMOD_ERR_MEMORY;
                }
                new (newdsp) DSPWaveTable;
                break;
            }
            default:
            {
                return FMOD_ERR_INVALID_PARAM;
            }
        }
    }

    if (!newdsp)
    {
        *dsp = 0;
        return FMOD_ERR_MEMORY;
    }

    newdsp->mSystem = mSystem;

    result = newdsp->alloc(dspdesc);
    if (result != FMOD_OK)
    {
        FMOD_Memory_Free(newdsp);
        return result;
    }

    if (dspdesc->create)
    {
#ifdef FMOD_SUPPORT_DLLS
        if (dspdesc->mAEffect)
        {
            #ifdef FMOD_SUPPORT_VSTPLUGIN
            {
                AEffect *effect = (AEffect *)dspdesc->mAEffect;

                if (effect->magic == kEffectMagic)
                {
                    ((DSPVSTPlugin *)newdsp)->mEffect = effect;
                }
            }
            #endif
            #ifdef FMOD_SUPPORT_WINAMPPLUGIN
            {
                winampDSPModule *effect = (winampDSPModule *)dspdesc->mAEffect;

                if (!FMOD_strncmp(dspdesc->name, "WINAMP", 6))
                {
                    ((DSPWinampPlugin *)newdsp)->mEffect = effect;
                }
            }
            #endif
        }
#endif

        newdsp->instance = (FMOD_DSP *)newdsp;

        result = dspdesc->create((FMOD_DSP_STATE *)newdsp);
        if (result != FMOD_OK)
        {
            FMOD_Memory_Free(newdsp);
            return result;
        }
    }

    *dsp = newdsp;

    return FMOD_OK;
}

#endif

/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]

	[REMARKS]

    [PLATFORMS]
    
	[SEE_ALSO]
]
*/
// VERIFY: 2f77cebc auto
// OFFSET: 0x008F3FC0
FMOD_RESULT PluginFactory::createOutput(FMOD_OUTPUT_DESCRIPTION_EX *outputdesc, Output **output)
{
    Output *newoutput;

    if (!outputdesc || !output)
    {
        return FMOD_ERR_INVALID_PARAM;      
    }

#if defined(FMOD_SUPPORT_OPENAL) && !defined(FMOD_USE_PLUGINS)
	if (outputdesc->mType == FMOD_OUTPUTTYPE_OPENAL)	/* OpenAL has a special virtual function getFreeChannel so we need to detect it and object alloc it to set up vtables. */
	{
		newoutput = FMOD_Object_Calloc(OutputOpenAL);
	}
	else
#endif
    if (outputdesc->polling)
    {
        newoutput = FMOD_Object_CallocSize(OutputPolled, (unsigned int)outputdesc->mSize);
    }
    else
    {
        newoutput = FMOD_Object_CallocSize(Output, (unsigned int)outputdesc->mSize);
    }

    if (!newoutput)
    {
        *output = 0;
        return FMOD_ERR_MEMORY;
    }
    
    /*
        Copy the description out of the source output to the new output.
    */
    FMOD_memcpy(&newoutput->mDescription, outputdesc, sizeof(FMOD_OUTPUT_DESCRIPTION_EX));

    newoutput->mSystem       = mSystem;

#ifdef FMOD_SUPPORT_SOFTWARE
    newoutput->readfrommixer = Output::mixCallback;
#endif

    *output = newoutput;

    return FMOD_OK;
}


/*
[
	[DESCRIPTION]

	[PARAMETERS]
 
	[RETURN_VALUE]
 
	[REMARKS]

    [PLATFORMS]
 
	[SEE_ALSO]
]
*/


}
