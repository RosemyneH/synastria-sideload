#ifndef _FMOD_AUTOCLEANUP_H
#define _FMOD_AUTOCLEANUP_H

#include "fmod_os_misc.h"
#include "fmod_memory.h"
#include "fmod_linkedlist.h"

#ifdef PLATFORM_WINDOWS
// disable "identifier was truncated in the debug info" warnings
#pragma warning(push)
#pragma warning(disable: 4786)
#endif

namespace FMOD
{

/*! \brief Parameterised resource cleanup.
    An object of this class stores a value of type \a T. \a T must provide a convesion
    to bool. If the value is valid (converts to true), the destructor calls the
    \a CleanupPolicy::cleanup function on it.
    Copying is disabled for this class, as it doesn't make sense and could easily
    lead to bugs.
*/
template <class T, class CleanupPolicy>
class AutoCleanup : public CleanupPolicy
{
public:
    AutoCleanup(const T &val = T()) : m_val(val) { }
    ~AutoCleanup() { if(m_val) CleanupPolicy::cleanup(m_val); }

    /*! Sets the internal pointer.
    */
    AutoCleanup<T, CleanupPolicy> &operator=(T val) { m_val = val; return *this; }
    bool operator!() { return !m_val; }

    /*! Releases the internal pointer, so the cleanup policy will not be invoked.
    */
    T releasePtr() { T tmp = m_val; m_val = T(); return tmp; }
private:
    AutoCleanup(const AutoCleanup<T, CleanupPolicy> &other); // disabled
    void operator=(const AutoCleanup<T, CleanupPolicy> &other); // disabled
    T m_val;
};


/*
    Convenience cleanup types
    These types wrap the AutoCleanup class for common cleanup policies so we
    have nicer template instantiation code everywhere.
*/

/*! Stores a value of type T*.
    Cleanup: Calls T::close() on the stored pointer.
*/
template <class T>
class AutoClose;

/*! Stores a value of type T*.
    Cleanup: Calls T::release() on the stored pointer.
*/
template <class T>
class AutoRelease;

/*! Stores a value of type T*, and an FMOD_OS_CRITICALSECTION*.
    Cleanup: Enters the stored critical section, calls T::release() on the
    stored pointer and then exits the critical section.
*/
template <class T>
class AutoCritRelease;

/*! Stores a value of type T*, and a cleanup function pointer.
    Cleanup: Calls the stored function pointer with the stored pointer as its argument.
*/
template <class T>
class AutoCleanupFunc;

/*! Stores a value of type T**.
    Cleanup: Calls T::release() on *value, then sets *value to 0, where value
    is the stored T**.
*/
template <class T>
class AutoReleaseClear;

/*! Stores a pointer of type T*, and a value of type T.
    Cleanup: Sets *pointer to the stored value.
*/
template <class T>
class AutoSet;

/*! Stores a value of type T**.
    Cleanup: Calls FMOD_Memory_Free on *value, then sets *value to 0.
*/
template <class T>
class AutoFreeClear;

/*! \class AutoFree
    Stores a value of type void*.
    Cleanup: Calls FMOD_Memory_Free on the stored value.
*/

/*! \class AutoFreeCrit
    Stores a value of type FMOD_OS_CRITICALSECTION*.
    Cleanup: Calls FMOD_OS_CriticalSection_Free on the stored value.
*/


/*
    Cleanup policy and convenience class implementations
*/

/*
    Cleanup policy classes
*/

template <class T>
class CleanupRelease
{
public:
    static void cleanup(T ptr) { ptr->release(); }
};

template <class T>
class CleanupClose
{
public:
    static void cleanup(T ptr) { ptr->close(); }
};

template <class T, class CleanupPolicy>
class CleanupClear : public CleanupPolicy
{
public:
    static void cleanup(T ptr)
    {
        if(*ptr)
        {
            CleanupPolicy::cleanup(*ptr);
            *ptr = 0;
        }
    }
};

template <class T>
class CleanupSet
{
public:
    CleanupSet() : mSetValue() { }

    void setValue(const T &value) { mSetValue = value; }
    void cleanup(T *ptr) { *ptr = mSetValue; }

private:
    T mSetValue;
};

template <class T>
class CleanupFree
{
public:
    static void cleanup(T ptr) { FMOD_Memory_Free(ptr); }
};

template <>
class CleanupFree<FMOD_OS_CRITICALSECTION*>
{
public:
    static void cleanup(FMOD_OS_CRITICALSECTION *ptr)
    { FMOD_OS_CriticalSection_Free(ptr); }
};

template <>
class CleanupFree<FMOD_OS_SEMAPHORE*>
{
public:
    static void cleanup(FMOD_OS_SEMAPHORE *ptr)
    { FMOD_OS_Semaphore_Free(ptr); }
};

template <class T, class CleanupPolicy>
class CleanupCrit : public CleanupPolicy
{
public:
    CleanupCrit() : mCrit(0) { }

    void setCrit(FMOD_OS_CRITICALSECTION *crit) { mCrit = crit; }

    void cleanup(T ptr)
    {
        if(mCrit) FMOD_OS_CriticalSection_Enter(mCrit);
        CleanupPolicy::cleanup(ptr);
        if(mCrit) FMOD_OS_CriticalSection_Leave(mCrit);
    }

private:
    FMOD_OS_CRITICALSECTION *mCrit;
};

template <class T>
class CleanupFuncPtr
{
public:
    typedef void (F_STDCALL *CleanupFunction)(T);

    CleanupFuncPtr() : mFunc(0) { }

    void setFunc(CleanupFunction func) { mFunc = func; }

    void cleanup(T val)
    {
        if(mFunc) mFunc(val);
    }

private:
    CleanupFunction mFunc;
};

/*
    Convenience cleanup type implementations
*/

template <class T>
class AutoClose : public AutoCleanup<T*, CleanupClose<T*> >
{
public:
    typedef AutoCleanup<T*, CleanupClose<T*> > Base;

    AutoClose(T *ptr = 0) : Base(ptr) { }
    AutoClose<T> &operator=(T *ptr) { Base::operator=(ptr); return *this; }
};

template <class T>
class AutoRelease : public AutoCleanup<T*, CleanupRelease<T*> >
{
public:
    typedef AutoCleanup<T*, CleanupRelease<T*> > Base;

    AutoRelease(T *ptr = 0) : Base(ptr) { }
    AutoRelease<T> &operator=(T *ptr) { Base::operator=(ptr); return *this; }
};

template <class T>
class AutoCritRelease : public AutoCleanup<T*, CleanupCrit<T*, CleanupRelease<T*> > >
{
public:
    typedef AutoCleanup<T*, CleanupCrit<T*, CleanupRelease<T*> > > Base;

    AutoCritRelease(FMOD_OS_CRITICALSECTION *crit, T *ptr = 0) : Base(ptr)
    { Base::setCrit(crit); }
    AutoCritRelease<T> &operator=(T *ptr) { Base::operator=(ptr); return *this; }
};

template <class T>
class AutoCleanupFunc : public AutoCleanup<T*, CleanupFuncPtr<T*> >
{
public:
    typedef void (F_STDCALL *CleanupFunction)(T *ptr);
    typedef AutoCleanup<T*, CleanupFuncPtr<T*> > Base;

    AutoCleanupFunc(CleanupFunction func, T *ptr = 0) : Base(ptr)
    { setFunc(func); }
    AutoCleanupFunc<T> &operator=(T *ptr) { Base::operator=(ptr); return *this; }
};

template <class T>
class AutoReleaseClear : public AutoCleanup<T*, CleanupClear<T*, CleanupRelease<T> > >
{
public:
    typedef AutoCleanup<T*, CleanupClear<T*, CleanupRelease<T> > > Base;

    AutoReleaseClear(T *ptr = 0) : Base(ptr) { }
    AutoReleaseClear<T> &operator=(T *ptr) { Base::operator=(ptr); return *this; }
};

template <class T>
class AutoSet : public AutoCleanup<T*, CleanupSet<T> >
{
public:
    typedef AutoCleanup<T*, CleanupSet<T> > Base;

    AutoSet(const T &value, T *ptr = 0) : Base(ptr)
    { setValue(value); }
    AutoSet<T> &operator=(T *ptr) { Base::operator=(ptr); return *this; }
};

template <class T>
class AutoFreeClear : public AutoCleanup<T*, CleanupClear<T*, CleanupFree<T> > >
{
public:
    typedef AutoCleanup<T*, CleanupClear<T*, CleanupFree<T> > > Base;

    AutoFreeClear(T *ptr = 0) : Base(ptr) { }
    AutoFreeClear<T> &operator=(T *ptr) { Base::operator=(ptr); return *this; }
};

typedef AutoCleanup<void*, CleanupFree<void*> > AutoFree;
typedef AutoCleanup<FMOD_OS_CRITICALSECTION*,
                    CleanupFree<FMOD_OS_CRITICALSECTION*> > AutoFreeCrit;
typedef AutoCleanup<FMOD_OS_SEMAPHORE*,
                    CleanupFree<FMOD_OS_SEMAPHORE*> > AutoFreeSema;

} // namespace FMOD

#ifdef PLATFORM_WINDOWS
#pragma warning(pop)
#endif

#endif // _FMOD_AUTOCLEANUP_H
