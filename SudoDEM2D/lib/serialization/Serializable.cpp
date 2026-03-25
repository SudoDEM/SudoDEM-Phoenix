/*************************************************************************
*  Copyright (C) 2004 by Olivier Galizzi                                 *
*  olivier.galizzi@imag.fr                                               *
*  Copyright (C) 2004 by Janek Kozicki                                   *
*  cosurgi@berlios.de                                                    *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#include <sudodem/lib/serialization/Serializable.hpp>


void Serializable::pyRegisterClass(pybind11::module_ _module) {
	checkPyClassRegistersItself("Serializable");
	pybind11::class_<Serializable, shared_ptr<Serializable>>(_module, "Serializable", pybind11::dynamic_attr())
		.def("__str__",&Serializable::pyStr)
		.def("__repr__",&Serializable::pyStr)
		.def("dict",&Serializable::pyDict,"Return dictionary of attributes.")
		.def("updateAttrs",&Serializable::pyUpdateAttrs,"Update object attributes from given dictionary")
		#if 1
			/* pybind11 pickling support */
			.def("__getstate__",&Serializable::pyDict)
			.def("__setstate__",&Serializable::pyUpdateAttrs)
		#endif
		// constructor with dictionary of attributes
		.def(pybind11::init([](pybind11::args args, pybind11::kwargs kwargs) {
			return Serializable_ctor_kwAttrs<Serializable>(args, kwargs);
		}))
		// comparison operators
		.def("__eq__", [](const Serializable& a, const Serializable& b) { return false; }) // placeholder
		.def("__ne__", [](const Serializable& a, const Serializable& b) { return true; }) // placeholder
		;
}

void Serializable::checkPyClassRegistersItself(const std::string& thisClassName) const {
	// Check removed: classes now use pyRegisterClass() methods for pybind11 registration
	// This check was for the old SUDODEM_CLASS_BASE_DOC_ATTR* macro system
}


void Serializable::pyUpdateAttrs(const pybind11::dict& d){
	pybind11::list items = d.attr("items")();
	for (size_t i = 0; i < items.size(); i++){
		pybind11::tuple item = items[i].cast<pybind11::tuple>();
		string key = item[0].cast<string>();
		pySetAttr(key, item[1].cast<pybind11::object>());
	}
	callPostLoad();
}