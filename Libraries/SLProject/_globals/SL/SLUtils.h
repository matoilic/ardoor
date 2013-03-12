//#############################################################################
//  File:      _globals/SLUtils.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>

#ifndef SLUTILS_H
#define SLUTILS_H

//-----------------------------------------------------------------------------
//! SLUtils provides static utility functions string handling
class SLUtils
{  public:

      //! SLUtils::toLower returns a string in lower case
      static SLstring toLower(SLstring s)
      {  SLstring cpy(s);
         transform(cpy.begin(), cpy.end(), cpy.begin(),::tolower);
         return cpy;
      }
      
      //! SLUtils::toUpper returns a string in upper case
      static SLstring toUpper(SLstring s)
      {  SLstring cpy(s);
         transform(cpy.begin(), cpy.end(), cpy.begin(),::toupper);
         return cpy;
      }
      
      //! SLUtils::getPath returns the path w. '\\' of path-filename string
      static SLstring getPath(const SLstring& pathFilename) 
      {
         size_t i;
         i = pathFilename.rfind('\\', pathFilename.length( ));
         if (i != string::npos) 
         {  return(pathFilename.substr(0, i+1));
         }
         
         i = pathFilename.rfind('/', pathFilename.length( ));
         if (i != string::npos) 
         {  return(pathFilename.substr(0, i+1));
         }
         return pathFilename;
      }
      
      //! SLUtils::getFileName returns the filename of path-filename string
      static SLstring getFileName(const SLstring& pathFilename) 
      {
         size_t i;
         i = pathFilename.rfind('\\', pathFilename.length( ));
         if (i != string::npos) 
         {  return(pathFilename.substr(i+1, pathFilename.length( ) - i));
         }
         
         i = pathFilename.rfind('/', pathFilename.length( ));
         if (i != string::npos) 
         {  return(pathFilename.substr(i+1, pathFilename.length( ) - i));
         }
         return pathFilename;
      }
      
      //! SLUtils::getFileNameWOExt returns the filename without extension
      static SLstring getFileNameWOExt(const SLstring& pathFilename) 
      {
         SLstring filename = getFileName(pathFilename);
         
         size_t i;
         i = filename.rfind('.', filename.length( ));
         if (i != string::npos) 
         {  return(filename.substr(0, i));
         }
         
         return(filename);
      }    
      
      //! SLUtils::getFileExt returns the file extension without dot in lower case
      static SLstring getFileExt(SLstring filename) 
      {
         size_t i;
         i = filename.rfind('.', filename.length( ));
         if (i != string::npos) 
            return toLower(filename.substr(i+1, filename.length() - i));
         return("");
      }
            
      //! SLUtils::trims a string at the end
      static SLstring trim(SLstring& s, const SLstring& drop = " ")
      {  SLstring r=s.erase(s.find_last_not_of(drop)+1);
         return r.erase(0,r.find_first_not_of(drop));
      }
      
      //! SLUtils::splits an input string at a delimeter string into an array
      static int split(const SLstring&        in, 
                       const SLstring&        delimiter, 
                       std::vector<SLstring>& out, 
                       bool                   includeEmpties)
      {
         int iPos   = 0;
         int newPos = -1;
         int sizeS2 = (int)delimiter.size();
         int iSize  = (int)in.size();

         if (iSize==0 || sizeS2==0) return 0;

         std::vector<SLint> pos;

         newPos = in.find (delimiter, 0);

         if (newPos < 0) return 0;

         int numFound = 0;

         while (newPos >= iPos)
         {  numFound++;
            pos.push_back(newPos);
            iPos = newPos;
            newPos = in.find (delimiter, iPos+sizeS2);
         }

         if (numFound == 0) return 0;

         for (SLuint i=0; i <= pos.size(); ++i)
         {  SLstring s("");
            if (i == 0) s = in.substr(i, pos[i]); 
            int offset = pos[i-1] + sizeS2;
            if (offset < iSize)
            {  if (i == pos.size()) 
                  s = in.substr(offset);
               else if (i > 0) 
                  s = in.substr(pos[i-1] + sizeS2, pos[i] - pos[i-1] - sizeS2);
            }
            if (includeEmpties || s.size()>0) out.push_back(s);
         }
         return numFound;
      }

      //! SLUtils::removeComments for C/C++ comments removal from shader code
      static SLstring removeComments(SLstring src)
      {  
         SLstring dst;
         SLint len = src.length();
         SLint i = 0;

         while (i < len)
         {  if (src[i]=='/' && src[i+1]=='/')
            {  dst += '\n';
               while (i<len && src[i] != '\n') i++;
               i++; 
            } 
            else if (src[i]=='/' && src[i+1]=='*')
            {  while (i<len && !(src[i]=='*' && src[i+1]=='/'))
               { 
                  if (src[i]=='\n') dst += '\n';
                  i++; 
               }
               i+=2;
            } 
            else
            {  dst += src[i++];
            } 
         }
         //cout << dst << "|" << endl;
         return dst;
      }
};
//-----------------------------------------------------------------------------
#endif



