// CPerfTimer - a simple Win32 performance counter wrapper
// by Dean Wyant dwyant@mindspring.com

/*

  This class is simple to use. Just declare a variable(s) as type CPerfTimer,
  call Start() to start timimg and call Stop() to stop timimg. You can pause a
  timer by calling Stop() and then you can call Start() to resume. Retrieve the
  elapsed time by calling an Elapsed..() function. Assignment, addition, 
  subtraction and comparison are supported. There are a few information calls
  available also. All calls except Start and Stop can be performed on a timer
  without stopping it.
  
*/

#ifndef __PERFTIMER_H__
#define __PERFTIMER_H__

class CPerfTimer
{
public:
  CPerfTimer(BOOL bStart = FALSE) {Init(bStart);}

  CPerfTimer(const CPerfTimer& Src); 

  virtual ~CPerfTimer() {;}

  void Start(BOOL bReset = FALSE);   // Start from current value or optionally from 0
  void Stop();                       // Stop timing. Use Start afterwards to continue.
 
  BOOL IsRunning();                  // Returns FALSE if stopped.
  
  BOOL IsSupported();                // Returns FALSE if performance counter not supported.
                                     // Call after constructing at least one CPerfTimer

  const double Resolution();         // Returns timer resolution in seconds
  const double Resolutionms();       // Returns timer resolution in milliseconds
  const double Resolutionus();       // Returns timer resolution in microseconds
  
  const double Elapsed();            // Returns elapsed time in seconds
  const double Elapsedms();          // Returns elapsed time in milliseconds 
  const double Elapsedus();          // Returns elapsed time in microseconds

  const CPerfTimer& operator=(const CPerfTimer& Src); // Assignment operator 

  // Math operators
  CPerfTimer operator+(const CPerfTimer& Src) const;
	CPerfTimer operator-(const CPerfTimer& Src) const;
	const CPerfTimer& operator+=(const CPerfTimer& Src);
	const CPerfTimer& operator-=(const CPerfTimer& Src);
  // For time in seconds
  CPerfTimer operator+(const double Secs) const;
	CPerfTimer operator-(const double Secs) const;
	const CPerfTimer& operator+=(const double Secs);
	const CPerfTimer& operator-=(const double Secs);

  // Boolean comparison operators
	BOOL operator<(const CPerfTimer& Src);
	BOOL operator>(const CPerfTimer& Src);
	BOOL operator<=(const CPerfTimer& Src);
	BOOL operator>=(const CPerfTimer& Src);
  // For time in seconds
  BOOL operator<(const double Secs);
	BOOL operator>(const double Secs);
	BOOL operator<=(const double Secs);
	BOOL operator>=(const double Secs);

  virtual void Lock() const {;}     // Override for thread safe operation
  virtual void Unlock() const {;}     // Override for thread safe operation
protected:
  void Init(BOOL bStart);
  void Copy(const CPerfTimer& Src);

private:
  __int64 m_Start;
  static __int64 m_Freq;   // does not change while system is running
  static __int64 m_Adjust; // Adjustment time it takes to Start and Stop
};

class CPerfTimerT : public CPerfTimer
{ // You only need to use types of this class if a timer is going to be shared between threads
public:
  CPerfTimerT(BOOL bStart = FALSE)
  {
    m_hMutex = CreateMutex(NULL, FALSE, _T(""));
    Init(bStart);
  }

  CPerfTimerT(const CPerfTimerT& Src) 
  { 
    m_hMutex = CreateMutex(NULL,FALSE,_T(""));
    Copy(Src); 
  }

  CPerfTimerT(const CPerfTimer& Src) 
  { 
    m_hMutex = CreateMutex(NULL,FALSE,_T(""));
    Copy(Src); 
  }

  virtual ~CPerfTimerT() 
  { CloseHandle(m_hMutex); }

  const CPerfTimerT& operator=(const CPerfTimerT& Src) // Assignment operator 
  {
    Copy(Src);
    return *this; 
  }
 
  virtual void Lock() const { WaitForSingleObject(m_hMutex,10000); }   
  virtual void Unlock() const { ReleaseMutex(m_hMutex); }   
private:
  HANDLE m_hMutex;
};

inline void CPerfTimer::Init(BOOL bStart)
{
  if (!m_Freq) 
  { // Initialization should only run once
    QueryPerformanceFrequency((LARGE_INTEGER *)&m_Freq); 
    if (!m_Freq)
      m_Freq = 1; // Timer will be useless but will not cause divide by zero
    m_Start = 0; 
    m_Adjust = 0; 
    Start();            // Time a Stop
    Stop(); 
    m_Adjust = m_Start;
  }
  // This is the only part that normally runs
  m_Start = 0; 
  if (bStart)
    Start(); 
}

inline CPerfTimer::CPerfTimer(const CPerfTimer& Src)  
{
  Copy(Src);
}

inline void CPerfTimer::Copy(const CPerfTimer& Src)
{
  if (&Src == this) 
    return; // avoid deadlock if someone tries to copy it to itself
  Src.Lock();
  Lock();
  m_Start = Src.m_Start; 
  Unlock();
  Src.Unlock();
}

inline void CPerfTimer::Start(BOOL bReset) 
{ // Start from current value or optionally from 0
  __int64 i;
  QueryPerformanceCounter((LARGE_INTEGER *)&i);
  Lock();
  if ((!bReset) && (m_Start < 0))
    m_Start += i;   // We are starting with an accumulated time
  else 
    m_Start = i;    // Starting from 0
  Unlock();
} 

inline void CPerfTimer::Stop() 
{ // Stop timing. Use Start afterwards to continue
  Lock();
  if (m_Start <= 0)
  {
    Unlock();
    return;          // Was not running
  }
  __int64 i;
  QueryPerformanceCounter((LARGE_INTEGER *)&i); 
  m_Start += -i;          // Stopped timer keeps elapsed timer ticks as a negative 
  if (m_Start < m_Adjust) // Do not overflow
    m_Start -= m_Adjust;  // Adjust for time timer code takes to run
  else 
    m_Start = 0;          // Stop must have been called directly after Start
  Unlock();
} 

inline BOOL CPerfTimer::IsRunning() 
{ // Returns FALSE if stopped.
  Lock();
  BOOL bRet = (m_Start > 0); // When < 0, holds elpased clicks
  Unlock();
  return bRet;   
}
 inline const double CPerfTimer::Elapsed()
{ // Returns elapsed time in seconds
  CPerfTimer Result(*this);
  Result.Stop();
  return (double)(-Result.m_Start)/(double)m_Freq; 
}

inline const double CPerfTimer::Elapsedms() 
{ // Returns elapsed time in milliseconds
  CPerfTimer Result(*this);
  Result.Stop();
  return (-Result.m_Start*1000.0)/(double)m_Freq; 
}

inline const double CPerfTimer::Elapsedus() 
{ // Returns elapsed time in microseconds
  CPerfTimer Result(*this);
  Result.Stop();
  return (-Result.m_Start * 1000000.0)/(double)m_Freq; 
}


// Assignment operator
inline const CPerfTimer& CPerfTimer::operator=(const CPerfTimer& Src) 
{
  Copy(Src);
  return *this; 
}


// Math operators
inline CPerfTimer CPerfTimer::operator+(const CPerfTimer& Src) const
{
  CPerfTimer Result(*this);
  Result += Src; 
  return Result; 
}

inline CPerfTimer CPerfTimer::operator-(const CPerfTimer& Src) const
{
  CPerfTimer Result(*this);
  Result -= Src; 
  return Result; 
}

inline const CPerfTimer& CPerfTimer::operator+=(const CPerfTimer& Src)
{
  CPerfTimer SrcStop(Src);  // Temp is necessary in case Src is not stopped
  SrcStop.Stop();
  Lock();
  m_Start += SrcStop.m_Start;
  Unlock();
  return *this; 
}

inline const CPerfTimer& CPerfTimer::operator-=(const CPerfTimer& Src)
{
  CPerfTimer SrcStop(Src);  // Temp is necessary in case Src is not stopped
  SrcStop.Stop();
  Lock();
  m_Start -= SrcStop.m_Start; 
  Unlock();
  return *this; 
}

// For time in seconds
inline CPerfTimer CPerfTimer::operator+(const double Secs) const
{
  CPerfTimer Result(*this);
  Result += Secs; 
  return Result; 
}

inline CPerfTimer CPerfTimer::operator-(const double Secs) const
{
  CPerfTimer Result(*this);
  Result += Secs; 
  return Result; 
}

inline const CPerfTimer& CPerfTimer::operator+=(const double Secs)
{
  Lock();
  m_Start -= (__int64)(Secs*(double)m_Freq);
  Unlock();
  return *this; 
}

inline const CPerfTimer& CPerfTimer::operator-=(const double Secs)
{
  Lock();
  m_Start += (__int64)(Secs*(double)m_Freq);
  Unlock();
  return *this; 
}



// Boolean comparison operators
inline BOOL CPerfTimer::operator<(const CPerfTimer& Src)
{ 
  BOOL bRet; 
  CPerfTimer Temp(Src);
  Lock();
  if (m_Start <= 0)
  {
    Temp.Stop();
    bRet = (m_Start > Temp.m_Start); 
    Unlock();
    return bRet;
  }
  else
  if (Temp.m_Start > 0)
  {
    bRet = (m_Start < Temp.m_Start); 
    Unlock();
    return bRet;
  }
  else
  {
    Unlock();
    CPerfTimer ThisStop(*this);
    ThisStop.Stop();
    return (ThisStop.m_Start > Temp.m_Start); 
  }
}

inline BOOL CPerfTimer::operator>(const CPerfTimer& Src)
{ 
  BOOL bRet; 
  CPerfTimer Temp(Src);
  Lock();
  if (m_Start <= 0)
  {
    Temp.Stop();
    bRet = (m_Start < Temp.m_Start); 
    Unlock();
    return bRet;
  }
  else
  if (Temp.m_Start > 0)
  {
    bRet = (m_Start > Temp.m_Start); 
    Unlock();
    return bRet;
  }
  else
  {
    Unlock();
    CPerfTimer ThisStop(*this);
    ThisStop.Stop();
    return (ThisStop.m_Start < Temp.m_Start); 
  }
}

inline BOOL CPerfTimer::operator<=(const CPerfTimer& Src)
{ 
  return !(*this > Src);
}

inline BOOL CPerfTimer::operator>=(const CPerfTimer& Src)
{ 
  return !(*this < Src);
}

// For time in seconds
inline BOOL CPerfTimer::operator<(const double Secs)
{ 
  BOOL bRet; 
  Lock();
  if (m_Start <= 0)
  {
    bRet = (m_Start > (__int64)(-Secs*(double)m_Freq)); 
    Unlock();
    return bRet;
  }
  else
  {
    Unlock();
    CPerfTimer ThisStop(*this);
    ThisStop.Stop();
    return (ThisStop.m_Start > (__int64)(-Secs*(double)m_Freq)); 
  }
}

inline BOOL CPerfTimer::operator>(const double Secs)
{ 
  BOOL bRet; 
  Lock();
  if (m_Start <= 0)
  {
    bRet = (m_Start < (__int64)(-Secs*(double)m_Freq)); 
    Unlock();
    return bRet;
  }
  else
  {
    Unlock();
    CPerfTimer ThisStop(*this);
    ThisStop.Stop();
    return (ThisStop.m_Start < (__int64)(-Secs*(double)m_Freq)); 
  }
}

inline BOOL CPerfTimer::operator<=(const double Secs)
{ 
  return !(*this > Secs);
}

inline BOOL CPerfTimer::operator>=(const double Secs)
{ 
  return !(*this < Secs);
}


#endif //__PERFTIMER_H__