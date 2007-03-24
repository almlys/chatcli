#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# $Id$
#
#    Copyright (C) 2007 Alberto Montañola Lacort
#    See the file AUTHORS for more info
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
#    USA.
#
#    Please see the file COPYING for the full license.
#

import ConfigParser

class MyConfigParser(ConfigParser.SafeConfigParser):
    """A config parser in the way that I want it to work"""

    def writeFile(self,name):
        """
        Writes the configuration to the file by name
        @param name Destination file name
        """
        fo=file(name,"w")
        self.write(fo)
        fo.close()

    def set(self,section,name,value):
        """Control the NoSectionError"""
        if not self.has_section(section):
            self.add_section(section)
        ConfigParser.SafeConfigParser.set(self,section,name,value)

    def get(self,section,name,default=None):
        """We want to return default or None instead of an Exception"""
        try:
            return ConfigParser.SafeConfigParser.get(self,section,name)
        except (ConfigParser.NoSectionError, ConfigParser.NoOptionError):
            if default!=None:
                self.set(section,name,default)
            return default


class BaseApplication(object):
    """
    Base Aplication, with configuration stuff, etc...
    Intended to be used with any type of GUI framework
    (Gtk, Qt, wxPython, Tk, CEGUI, etc...)
    """

    def __init__(self,configFile=None):
        """Constructor"""
        self.__configFile=config
        self.config=None
        self._loadConfig()

    def _loadConfig(self,cfg=None):
        """Load App configuration"""
        if cfg==None:
            cfg=self.__configFile
        self.config=MyConfigParser()
        self._setConfigDefaults()
        if cfg!=None:
            self.config.read(cfg)

    def _setConfigDefaults(self):
        """
        Sets the configuration defaults
        Must be overrided
        """
        pass

    def _saveConfig(self,cfg=None):
        """Save App configuration"""
        if cfg==None:
            cfg=self.__configFile
        if cfg!=None:
            self.config.writeFile(cfg)

    def GetCfg(self,section,name,default=None):
        """self.config.get Shortcut (that my be overrided for other unknown purposes)"""
        return self.config.get(section,name,default)

    def _installGettext(self,lang=None):
        """Installs GetText"""
        import gettext
        if lang==None:
            gettext.install(self.GetCfg("global","app.gettext.domain"),
                            self.GetCfg("global","app.gettext.locales"),True)
        else:
            gettext.translation(self.GetCfg("global","app.gettext.domain"),
                                self.GetCfg("global","app.gettext.locales"),
                                (lang,)).install(True)

    def GetLanguages(self):
        try:
            return self.__Languages
        except:
            import dircache
            self.__Languages={}
            # Dirty hardcoded ugly LangDict
            _ = lambda x : x
            LangDict={"en":_("English"),"es":_("Spanish"),"ca":_("Catalan")}
            for lan in dircache.listdir(self.GetCfg("global","app.gettext.locales")):
                if lan in LangDict:
                    self.__Languages[lan]=LangDict[lan]
                else:
                    self.__Languages[lan]=lan
            return self.__Languages

    def SetLanguage(self,lang):
        #import locale
        #locale.setlocale(locale.LC_ALL, (lang,"utf-8"))
        self.__Language=lang
        self._installGettext(lang)

    def GetLanguage(self):
        try:
            return self.__Language
        except:
            import locale
            self.__Language=locale.getdefaultlocale()[0][:2]
            return self.__Language

    def GetAppVersion(self):
        return "$Id$"


if __name__ == "__main__":
    a=MyConfigParser()
    a.set("global","hola","feo")
    assert(a.get("global","hola")=="feo")
    assert(a.get("global","hola2")==None)
    assert(a.get("global","whola3","xxxx")=="xxxx")
    assert(a.get("global","whola3")=="xxxx")
    a.set("main","yes","no")
    assert(a.get("main","yes")=="no")
    assert(a.get("null","wth")==None)

