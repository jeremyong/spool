#pragma once

#ifndef SPOOL_ID
// No spool id, just pass the contents through intact
#define SP(str) str
#else

extern const char*** spool_strings_[];
#define SP(...) *spool_strings_[SPOOL_ID][__COUNTER__]
#endif
