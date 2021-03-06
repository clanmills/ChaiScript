// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef __bad_boxed_cast_hpp__
#define __bad_boxed_cast_hpp__

#include "type_info.hpp"

namespace chaiscript 
{
  
  /**
   * class that is thrown in the event of a bad_boxed_cast. That is,
   * in the case that a Boxed_Value cannot be cast to the desired type
   */
  class bad_boxed_cast : public std::bad_cast
  {
    public:
      bad_boxed_cast(const Type_Info &t_from, const std::type_info &t_to,
          const std::string &what)
        : from(t_from), to(&t_to), m_what(what)
      {
      }

      bad_boxed_cast(const Type_Info &t_from, const std::type_info &t_to) throw()
        : from(t_from), to(&t_to), m_what("Cannot perform boxed_cast")
      {
      }

      bad_boxed_cast(const std::string &w) throw()
        : m_what(w)
      {
      }

      virtual ~bad_boxed_cast() throw() {}

      virtual const char * what() const throw()
      {
        return m_what.c_str();
      }

      Type_Info from;
      const std::type_info *to;

    private:
      std::string m_what;
  };
}



#endif

