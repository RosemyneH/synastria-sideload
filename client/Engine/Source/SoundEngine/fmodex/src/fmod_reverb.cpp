/*$ preserve start $*/
#include "fmod_settings.h"
#include "fmod.hpp"

#include "fmod_reverbi.h"

namespace FMOD
{


/*$ preserve end $*/


FMOD_RESULT Reverb::release()
{
    FMOD_RESULT result;
    ReverbI *reverbi;

    result = ReverbI::validate(this, &reverbi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return reverbi->release();
    }
}


FMOD_RESULT Reverb::set3DAttributes(const FMOD_VECTOR *position, float mindistance, float maxdistance)
{
    FMOD_RESULT result;
    ReverbI *reverbi;

    result = ReverbI::validate(this, &reverbi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return reverbi->set3DAttributes(position, mindistance, maxdistance);
    }
}


FMOD_RESULT Reverb::get3DAttributes(FMOD_VECTOR *position, float *mindistance, float *maxdistance)
{
    FMOD_RESULT result;
    ReverbI *reverbi;

    result = ReverbI::validate(this, &reverbi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return reverbi->get3DAttributes(position, mindistance, maxdistance);
    }
}


FMOD_RESULT Reverb::setProperties(const FMOD_REVERB_PROPERTIES *properties)
{
    FMOD_RESULT result;
    ReverbI *reverbi;

    result = ReverbI::validate(this, &reverbi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return reverbi->setProperties(properties);
    }
}


FMOD_RESULT Reverb::getProperties(FMOD_REVERB_PROPERTIES *properties)
{
    FMOD_RESULT result;
    ReverbI *reverbi;

    result = ReverbI::validate(this, &reverbi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return reverbi->getProperties(properties);
    }
}


FMOD_RESULT Reverb::setActive(bool active)
{
    FMOD_RESULT result;
    ReverbI *reverbi;

    result = ReverbI::validate(this, &reverbi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return reverbi->setActive(active);
    }
}


FMOD_RESULT Reverb::getActive(bool *active)
{
    FMOD_RESULT result;
    ReverbI *reverbi;

    result = ReverbI::validate(this, &reverbi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return reverbi->getActive(active);
    }
}


FMOD_RESULT Reverb::setUserData(void *_userdata)
{
    FMOD_RESULT result;
    ReverbI *reverbi;

    result = ReverbI::validate(this, &reverbi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return reverbi->setUserData(_userdata);
    }
}


FMOD_RESULT Reverb::getUserData(void **_userdata)
{
    FMOD_RESULT result;
    ReverbI *reverbi;

    result = ReverbI::validate(this, &reverbi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return reverbi->getUserData(_userdata);
    }
}



/*$ preserve start $*/
}

/*$ preserve end $*/
