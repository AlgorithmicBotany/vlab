/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */




#ifndef __POST_EXEC_H__
#define __POST_EXEC_H__

#include <qobject.h>
#include <qtimer.h>

class PostExec : public QObject
{
	Q_OBJECT

public:
	PostExec (QObject * parent, int t, void (* func) (int, char **)
		  , int argc, char * argv [])
		: QObject (parent)
		{
                        setObjectName("postexec");
			QTimer::singleShot (
				t, this, SLOT (func_slot ()));
			pfunc = func;
			pargc = argc;
			pargv = argv;
		}
	
public slots:
        void func_slot ()
		{
			pfunc (pargc, pargv);
		}


private:
	int pargc;
	char ** pargv;
	void (* pfunc) (int, char **);

	
};

#endif
