/*
  ZynAddSubFX - a software synthesizer

  PresetsArray.h - PresetsArray and Clipboard management
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2 or later) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#ifndef PRESETSARRAY_H
#define PRESETSARRAY_H

#include "../Misc/XMLwrapper.h"

#include "Presets.h"

/**PresetsArray and Clipboard management*/
class PresetsArray:public Presets
{
    public:
        PresetsArray();
        virtual ~PresetsArray();

        void copy(PresetsStore &ps, const char *name); /**<if name==NULL, the clipboard is used*/
        void copy(PresetsStore &ps, int elm, const char *name); /**<if name==NULL, the clipboard is used*/
        //void paste(PresetsStore &ps, int npreset); //npreset==0 for clipboard
        //bool checkclipboardtype(PresetsStore &ps);
        //void setelement(int n);
    protected:
        void setpresettype(const char *type);
    private:
        virtual void add2XMLsection(XMLwrapper *xml, int n)    = 0;
        //virtual void getfromXMLsection(XMLwrapper *xml, int n) = 0;
        //virtual void defaults() = 0;
        //virtual void defaults(int n) = 0;
};

#endif
