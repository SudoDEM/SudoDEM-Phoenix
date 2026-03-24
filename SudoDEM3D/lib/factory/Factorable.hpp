/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*  Copyright (C) 2004 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#pragma once

#include <string>
#include <sstream>
#include <sudodem/lib/base/PluginMacros.hpp>


//! macro for registering both class and its base
#define REGISTER_CLASS_AND_BASE(cn,bcn) REGISTER_CLASS_NAME_DERIVED(cn); REGISTER_BASE_CLASS_NAME_DERIVED(bcn);

// Macro for base classes (like Factorable itself) - no override
#define REGISTER_CLASS_NAME_BASE(cn)								\
	public : virtual string getClassName() const { return #cn; };

// Macro for derived classes - adds override
#define REGISTER_CLASS_NAME_DERIVED(cn)								\
	public : virtual string getClassName() const override { return #cn; };

// FIXME[1] - that macro below should go to another class! factorable has nothing to do with inheritance tree.

// Macro for base classes (like Factorable itself) - no override
#define REGISTER_BASE_CLASS_NAME_BASE(bcn)							\
	public : virtual string getBaseClassName(unsigned int i=0) const		\
	{										\
		string token;								\
		vector<string> tokens;							\
		string str=#bcn;							\
		istringstream iss(str);							\
		while (!iss.eof())							\
		{									\
			iss >> token;							\
			tokens.push_back(token);					\
		}									\
		if (i>=token.size())							\
			return "";							\
		else									\
			return tokens[i];						\
	}										\
	public : virtual int getBaseClassNumber()		 			\
	{										\
		string token;								\
		vector<string> tokens;							\
		string str=#bcn;							\
		istringstream iss(str);							\
		while (!iss.eof())							\
		{									\
			iss >> token;							\
			tokens.push_back(token);					\
		}									\
		return tokens.size();							\
	}

// Macro for derived classes - adds override
#define REGISTER_BASE_CLASS_NAME_DERIVED(bcn)							\
	public : virtual string getBaseClassName(unsigned int i=0) const override		\
	{										\
		string token;								\
		vector<string> tokens;							\
		string str=#bcn;							\
		istringstream iss(str);							\
		while (!iss.eof())							\
		{									\
			iss >> token;							\
			tokens.push_back(token);					\
		}									\
		if (i>=token.size())							\
			return "";							\
		else									\
			return tokens[i];						\
	}										\
	public : virtual int getBaseClassNumber() override		 			\
	{										\
		string token;								\
		vector<string> tokens;							\
		string str=#bcn;							\
		istringstream iss(str);							\
		while (!iss.eof())							\
		{									\
			iss >> token;							\
			tokens.push_back(token);					\
		}									\
		return tokens.size();							\
	}


class Factorable
{
	public :
		Factorable() {}
		virtual ~Factorable() {}

		virtual string getBaseClassName(unsigned int i=0) const { return "";}	// FIXME[1]
		virtual int getBaseClassNumber() { return 0;}				// FIXME[1]

	REGISTER_CLASS_NAME_BASE(Factorable);

// FIXME - virtual function to return version, long and short description, OR
//         maybe just a file with the same name as class with description inside
//	public    : virtual std::string getVersion();					// FIXME[1] -	we can make a class Plugin for all that extra stuff: 
											//		shortDescription(), longDescription(),  baseClassName(), baseClassNumber()
};


