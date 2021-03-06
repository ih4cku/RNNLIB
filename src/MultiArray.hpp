/*Copyright 2009 Alex Graves

This file is part of RNNLIB.

RNNLIB is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RNNLIB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RNNLIB.  If not, see <http://www.gnu.org/licenses/>.*/

#ifndef _INCLUDED_MultiArray_h  
#define _INCLUDED_MultiArray_h  

#include <iostream>
#include <numeric>
#include <vector>
#include <algorithm>
#include <boost/array.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/optional.hpp>
#include <boost/range.hpp>
#include "list_of.hpp"
#include "Helpers.hpp"

using namespace std;
using namespace boost;
using namespace boost::assign;

namespace rnnlib {

template <class T> struct MultiArray
{
	//data
	vector<T> data;
	vector<size_t> shape;
	vector<size_t> strides;
	
	//functions
	MultiArray()
	{
	}
	MultiArray(const vector<size_t>& s)
	{
		reshape(s);
	}
	MultiArray(const vector<size_t>& s, const T& fillval)
	{
		reshape(s, fillval);
	}
	virtual ~MultiArray()
	{
	}
	virtual size_t size() const
	{
		return data.size();
	}
	virtual size_t num_dims() const
	{
		return shape.size();
	}
	virtual bool empty() const
	{
		return data.empty();
	}
	virtual void resize_data()
	{
		data.resize(product(shape));
		strides.resize(shape.size());
		strides.back() = 1;
		for (int i = shape.size()-2; i >= 0; --i)
		{
			strides.at(i) = strides.at(i+1) * shape.at(i+1);
		}
	}
	template<class R> void reshape(const R& newShape)
	{
		assert(newShape.size());
		vector_assign(newShape, shape);
		resize_data();
	}
	template<class R> void reshape(const R& dims, const T& fillval)
	{
		reshape(dims);
		fill(data, fillval);
	}
	bool in_range(const vector<int>& coords) const
	{
		if (coords.size() > shape.size())
		{
			return false;
		}
		VSTCI shapeIt = shape.begin();
		for (VICI coordIt = coords.begin(); coordIt != coords.end(); ++coordIt, ++shapeIt)
		{
			int c = *coordIt;
			if (c < 0 || c >= *shapeIt)
			{
				return false;
			}
		}
		return true;
	}
	T& get(const vector<int>& coords)
	{
		check(boost::size(coords) == shape.size(), "get(" + str(coords) + ") called with shape " + str(shape));
		return *((*this)[coords].begin());
	}
	const T& get(const vector<int>& coords) const
	{
		check(boost::size(coords) == shape.size(), "get(" + str(coords) + ") called with shape " + str(shape));
		return (*this)[coords].front();
	}
	const View<T> operator[](const vector<int>& coords)
	{
		check(coords.size() <= shape.size(), "operator [" + str(coords) + "] called with shape " + str(shape));
		if (coords.empty())
		{
			return View<T>(&data.front(), &*data.end());
		}
		T* start = &data.front(); 
		VSTCI strideIt = strides.begin();
		for (VICI coordIt = coords.begin(); coordIt != coords.end(); ++coordIt, ++strideIt)
		{
			start += (*coordIt * *strideIt);
		}
		T* end = start + *(--strideIt);
		return View<T>(start, end);
	}
	const View<const T> operator[](const vector<int>& coords) const
	{
		check(coords.size() <= shape.size(), "operator [" + str(coords) + "] called with shape " + str(shape));
		if (coords.empty())
		{
			return View<const T>(&data.front(), &*data.end());
		}
		const T* start = &data.front(); 
		VSTCI strideIt = strides.begin();
		for (VICI coordIt = coords.begin(); coordIt != coords.end(); ++coordIt, ++strideIt)
		{
			start += (*coordIt * *strideIt);
		}
		const T* end = start + *(--strideIt);
		return View<const T>(start, end);
	}
	const View<T> at(const vector<int>& coords)
	{
		if (in_range(coords))
		{
			return (*this)[coords];
		}
		return View<T>(0, 0);
	}
	const View<const T> at(const vector<int>& coords) const
	{
		if (in_range(coords))
		{
			return (*this)[coords];
		}
		return View<const T>(0, 0);
	}
	template<class T2> void assign(const MultiArray<T2>& a)
	{
		reshape(a.shape);
		copy(a.data.begin(), a.data.end(), data.begin());
	}
	template<class T2> MultiArray<T>& operator=(const MultiArray<T2>& a)
	{
		assign(a);
		return *this;
	}
};

template <class T> static bool operator == (const MultiArray<T>& a, const MultiArray<T>& b)
{
	return (a.data == b.data && a.shape == b.shape);
}

};

#endif
