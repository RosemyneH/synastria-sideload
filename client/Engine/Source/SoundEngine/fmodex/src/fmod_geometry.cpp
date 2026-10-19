/*$ preserve start $*/

#include "fmod_settings.h"

#ifdef FMOD_SUPPORT_GEOMETRY

#include "fmod_geometryi.h"
#include "fmod.hpp"

namespace FMOD
{
/*$ preserve end $*/


FMOD_RESULT Geometry::release()
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->release();
    }
}


FMOD_RESULT Geometry::addPolygon(float directocclusion, float reverbocclusion, bool doublesided, int numvertices, const FMOD_VECTOR *vertices, int *polygonindex)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->addPolygon(directocclusion, reverbocclusion, doublesided, numvertices, vertices, polygonindex);
    }
}


FMOD_RESULT Geometry::getNumPolygons(int *numpolygons)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->getNumPolygons(numpolygons);
    }
}


FMOD_RESULT Geometry::getMaxPolygons(int *maxpolygons, int *maxvertices)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->getMaxPolygons(maxpolygons, maxvertices);
    }
}


FMOD_RESULT Geometry::getPolygonNumVertices(int index, int *numvertices)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->getPolygonNumVertices(index, numvertices);
    }
}


FMOD_RESULT Geometry::setPolygonVertex(int index, int vertexindex, const FMOD_VECTOR *vertex)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->setPolygonVertex(index, vertexindex, vertex);
    }
}


FMOD_RESULT Geometry::getPolygonVertex(int index, int vertexindex, FMOD_VECTOR *vertex)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->getPolygonVertex(index, vertexindex, vertex);
    }
}


FMOD_RESULT Geometry::setPolygonAttributes(int index, float directocclusion, float reverbocclusion, bool doublesided)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->setPolygonAttributes(index, directocclusion, reverbocclusion, doublesided);
    }
}


FMOD_RESULT Geometry::getPolygonAttributes(int index, float *directocclusion, float *reverbocclusion, bool *doublesided)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->getPolygonAttributes(index, directocclusion, reverbocclusion, doublesided);
    }
}


FMOD_RESULT Geometry::setActive(bool active)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->setActive(active);
    }
}


FMOD_RESULT Geometry::getActive(bool *active)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->getActive(active);
    }
}


FMOD_RESULT Geometry::setRotation(const FMOD_VECTOR *forward, const FMOD_VECTOR *up)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->setRotation(forward, up);
    }
}


FMOD_RESULT Geometry::getRotation(FMOD_VECTOR *forward, FMOD_VECTOR *up)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->getRotation(forward, up);
    }
}


FMOD_RESULT Geometry::setPosition(const FMOD_VECTOR *position)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->setPosition(position);
    }
}


FMOD_RESULT Geometry::getPosition(FMOD_VECTOR *position)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->getPosition(position);
    }
}


FMOD_RESULT Geometry::setScale(const FMOD_VECTOR *scale)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->setScale(scale);
    }
}


FMOD_RESULT Geometry::getScale(FMOD_VECTOR *scale)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->getScale(scale);
    }
}


FMOD_RESULT Geometry::save(void *data, int *datasize)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->save(data, datasize);
    }
}


FMOD_RESULT Geometry::setUserData(void *_userdata)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->setUserData(_userdata);
    }
}


FMOD_RESULT Geometry::getUserData(void **_userdata)
{
    FMOD_RESULT result;
    GeometryI *geometryi;

    result = GeometryI::validate(this, &geometryi);
    if (result != FMOD_OK)
    {
        return result;
    }
    else
    {
        return geometryi->getUserData(_userdata);
    }
}



/*$ preserve start $*/
}

#endif // FMOD_SUPPORT_GEOMETRY

/*$ preserve end $*/
