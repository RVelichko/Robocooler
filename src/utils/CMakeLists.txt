add_library(log STATIC
  Log.cpp
  )

add_library(chart STATIC
  Demangle.cpp
  )

add_library(sig_dispatcher STATIC
  SignalDispatcher.cpp
  )

add_library(thread_pool STATIC
  ThreadPool.cpp
  )

add_library(algorithm STATIC
  FibonacciReduction.cpp
  )

add_library(tty_io STATIC
  TtyIo.cpp
  )

add_library(strhex2hex STATIC
  Strhex2Hex.cpp
  )
