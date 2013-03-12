//#############################################################################
//  File:      _globals/SLTimer.h
//  Author:    Marcus Hudritsch
//             Copied from: Song Ho Ahn (song.ahn@gmail.com), www.songho.ca
//  Purpose:   High Resolution Timer that is able to measure the elapsed time 
//             with 1 micro-second accuracy on Windows, Linux and Unix system
//  Date:      February 2013
//  Copyright: Song Ho Ahn (song.ahn@gmail.com)
//#############################################################################

#ifndef SLTIMER
#define SLTIMER

#include <stdafx.h>

//-----------------------------------------------------------------------------
//! High Resolution Timer class
/*!
High Resolution Timer that is able to measure the elapsed time with 1 
micro-second accuracy on Windows, Linux and Unix system
*/
class SLTimer
{
   public:
               SLTimer();
              ~SLTimer();

      void     start();                   
      void     stop();
      double   getElapsedTimeInSec();
      double   getElapsedTimeInMilliSec();
      double   getElapsedTimeInMicroSec();
       
   private:
      double   startTimeInMicroSec;       // starting time in micro-second
      double   endTimeInMicroSec;         // ending time in micro-second
      int      stopped;                   // stop flag

      #ifdef WIN32
      LARGE_INTEGER frequency;            //!< ticks per second
      LARGE_INTEGER startCount;           //!< ticks at start
      LARGE_INTEGER endCount;             //!< ticks at end
      #else
      timeval startCount;
      timeval endCount;
      #endif
};
//---------------------------------------------------------------------------
#endif
