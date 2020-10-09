// Taken from:
// http://muq.mit.edu/master-muq2-docs/PyDictConversion_8h_source.html
#pragma once

#include <Python.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
//#include <pybind11/eigen.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>



inline void AddDictToPtree(pybind11::dict dict, std::string basePath, boost::property_tree::ptree &pt)
{
	pybind11::object keys = pybind11::list(dict.attr("keys")());
	std::vector<std::string> keysCpp = keys.cast<std::vector<std::string>>();

	for (auto& key : keysCpp) {

		// Recursively add dictionaries
		if (pybind11::isinstance<pybind11::dict>(dict.attr("get")(key))) {
			AddDictToPtree(dict.attr("get")(key), basePath + key + ".", pt);

			// Convert lists in the comma-separated strings
		}
		else if (pybind11::isinstance<pybind11::list>(dict.attr("get")(key))) {
			std::string val = "";
			for (auto comp : pybind11::list(dict.attr("get")(key)))
				val += "," + std::string(pybind11::str(comp));
			pt.put(basePath + key, val.substr(1));

			// Add all the other objects through their "str" interpretation
		}
		else {
			if (((std::string)pybind11::str(dict.attr("get")(key))).compare("False") == 0) {
				pt.put(basePath + key, false);
			}
			else if (((std::string)pybind11::str(dict.attr("get")(key))).compare("True") == 0) {
				pt.put(basePath + key, true);
			}
			else {
				pt.put(basePath + key, pybind11::str(dict.attr("get")(key)));
			}
		}
	}
};

inline boost::property_tree::ptree ConvertDictToPtree(pybind11::dict dict)
{
	boost::property_tree::ptree pt;
	AddDictToPtree(dict, "", pt);
	return pt;
};