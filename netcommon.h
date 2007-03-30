/******************************************************************************
* $Revision$                                                            *
*                                                                             *
*    Copyright (C) 2007 Ignasi Barri Vilardell                                *
*    Copyright (C) 2007 Alberto Montañola Lacort                              *
*    Copyright (C) 2007 Josep Rius Torrento                                   *
*    See the file AUTHORS for more info                                       *
*                                                                             *
*    This program is free software; you can redistribute it and/or modify     *
*    it under the terms of the GNU General Public License as published by     *
*    the Free Software Foundation; either version 2 of the License, or        *
*    (at your option) any later version.                                      *
*                                                                             *
*    This program is distributed in the hope that it will be useful,          *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
*    GNU General Public License for more details.                             *
*                                                                             *
*    You should have received a copy of the GNU General Public License        *
*    along with this program; if not, write to the Free Software              *
*    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA            *
*    02110-1301,USA.                                                          *
*                                                                             *
*    Please see the file COPYING for the full license.                        *
*                                                                             *
*******************************************************************************
*                                                                             *
* Xarxes II                                                                   *
* Màster en Enginyeria de Programari Lliure                                   *
*                                                                             *
* Pràctica I                                                                  *
*                                                                             *
******************************************************************************/
#ifndef NETCOMMON_H
#define NETCOMMON_H

/*
	Common stuff for the server/client aplications
	Please see netcommon.py as a reference for this file
*/

/// This class registers descriptors to be monitored for reads
/// Note, this implementation can register only the file descriptors when 
/// the python implementation allows to register any object associated to a file 
/// descriptor.
class selectInterface {
private:
	fd_set _master;
	fd_set _readfs;
	int _fdmax;
	std::queue<int> _descs;
public:
	/// Constructor
	selectInterface();
	/// Register a new descriptor
	/// @param fdesc Descriptor to register
	void register2(int fdesc);
	/// Unregister a descriptor
	/// @param fdesc Descriptor to unmonitor
	void unregister(int fdesc);
	/// Waits for new data in one of the registered descriptors
	/// @returns A queue with descriptors with available data
	std::queue<int> & wait();
};

#endif
