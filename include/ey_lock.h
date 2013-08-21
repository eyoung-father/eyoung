#ifndef EY_LOCK_H
#define EY_LOCK_H 1

#include <pthread.h>
#define ey_spinlock_t pthread_mutex_t
#define EY_SPINLOCK_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#define ey_spinlock_init(lock) pthread_mutex_init(lock,NULL)
#define ey_spinlock_lock(lock) pthread_mutex_lock(lock)
#define ey_spinlock_trylock(lock) pthread_mutex_trylock(lock)
#define ey_spinlock_unlock(lock) pthread_mutex_unlock(lock)

#define ey_rwlock_t pthread_rwlock_t
#define EY_RWLOCK_INITIALIZER PTHREAD_RWLOCK_INITIALIZER
#define ey_rwlock_init(lock) pthread_rwlock_init(lock,NULL)
#define ey_rwlock_rdlock(lock) pthread_rwlock_rdlock(lock)
#define ey_rwlock_rdtrylock(lock) pthread_rwlock_rdtrylock(lock)
#define ey_rwlock_rdunlock(lock) pthread_rwlock_unlock(lock)
#define ey_rwlock_wtlock(lock) pthread_rwlock_wrlock(lock)
#define ey_rwlock_wttrylock(lock) pthread_rwlock_wrtrylock(lock)
#define ey_rwlock_wtunlock(lock) pthread_rwlock_unlock(lock)
#endif
