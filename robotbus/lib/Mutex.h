#ifndef MMUTEX_H_  
#define MMUTEX_H_  

#ifdef WIN32
   #include <windows.h>
#else
   #include <stdio.h>
   #include <pthread.h>
#endif

#ifdef WIN32
typedef CRITICAL_SECTION        MKIT_MUTEX_SECTION;
   #define MKIT_MUTEX_INIT         ::InitializeCriticalSection
   #define MKIT_MUTEX_DESTROY      ::DeleteCriticalSection
   #define MKIT_MUTEX_LOCK         ::EnterCriticalSection
   #define MKIT_MUTEX_UNLOCK       ::LeaveCriticalSection
#else  
typedef pthread_mutex_t         MKIT_MUTEX_SECTION;  
   #define MKIT_MUTEX_INIT         pthread_mutex_init
   #define MKIT_MUTEX_DESTROY      pthread_mutex_destroy
   #define MKIT_MUTEX_LOCK         pthread_mutex_lock
   #define MKIT_MUTEX_UNLOCK       pthread_mutex_unlock
#endif  

class Mutex
{
public:
    Mutex()
    {
        MKIT_MUTEX_INIT(&m_mutex
#ifndef _WIN32
            , NULL
#endif
            );
    }

    virtual ~Mutex() {MKIT_MUTEX_DESTROY(&m_mutex);}
    virtual void Lock() const {MKIT_MUTEX_LOCK(&m_mutex);}
    virtual void Unlock() const { MKIT_MUTEX_UNLOCK(&m_mutex); }

private:
    mutable MKIT_MUTEX_SECTION m_mutex;
};

#endif
