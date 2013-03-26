henet - high availability networking library
============================================

&#x5D4;et
=========

Rationale
---------
The goal is to create a general purpose networking library based on the
Linux network stack.

Components
----------
* address
* socket
* mutex
* scoped_resource
* server
* client

How to build
------------

    make clean all


Links
-----

### C++11 concurrency: locks & mutexes
    http://www.boost.org/doc/libs/1_38_0/doc/html/thread/synchronization.html#thread.synchronization.mutex_concepts.lockable.try_lock
    http://locklessinc.com/articles/mutex_cv_futex/
    http://bartoszmilewski.com/2008/09/01/thin-lock-vs-futex/
    http://home.roadrunner.com/~hinnant/mutexes/locking.html
    http://codepen.io/stuffit/pen/KrAwx
    http://www.k-press.ru/cs/2008/3/multithreading/multithreading.asp
    http://codexpert.ro/blog/2013/02/20/cpp11-concurrency-locks/
    http://nick-black.com/dankwiki/index.php/Fast_UNIX_Servers
    http://opensource.apple.com/source/lldb/lldb-159/include/lldb/Utility/
    http://opensource.apple.com/source/lldb/lldb-69/include/lldb/Host/Mutex.h
    http://en.m.wikipedia.org/wiki/Wrapper_library#section_1
    http://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Atomic-Builtins.html
    http://preshing.com/20120226/roll-your-own-lightweight-mutex
    http://en.cppreference.com/w/cpp/concept
    http://en.cppreference.com/w/cpp/concept/BasicLockable
    http://stackoverflow.com/questions/6089917/how-to-achieve-lock-free-but-blocking-behavior
    http://yarchive.net/comp/linux/everything_is_file.html

