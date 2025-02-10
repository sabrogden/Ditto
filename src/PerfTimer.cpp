// CPerfTimer - a simple Win32 performance counter wrapper
// by Dean Wyant dwyant@mindspring.com

#include "stdafx.h"
#include "PerfTimer.h"

// Declare and initialize static member vars that get set only once and never change
__int64 CPerfTimer::m_Freq = 0; 
__int64 CPerfTimer::m_Adjust = 0; 

// All functions defined inline for speed. After all, the performance counter is 
// supposed to be able to time very short events fairly accurately.



BOOL CPerfTimer::IsSupported()
{ // Returns FALSE if performance counter not supported.
  // Call after constructing at least one CPerfTimer
  return (m_Freq > 1);
}

const double CPerfTimer::Resolution()   
{ // Returns timer resolution in seconds
  return 1.0/(double)m_Freq; 
}

const double CPerfTimer::Resolutionms() 
{ // Returns timer resolution in milliseconds
  return 1000.0/(double)m_Freq; 
}

const double CPerfTimer::Resolutionus() 
{ // Returns timer resolution in microseconds
  return 1000000.0/(double)m_Freq; 
}



