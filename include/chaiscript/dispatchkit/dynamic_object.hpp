#ifndef __DYNAMIC_OBJECT_HPP__
#define __DYNAMIC_OBJECT_HPP__

#include <boost/optional.hpp>

namespace chaiscript
{
  class Dynamic_Object
  {
    public:
      Dynamic_Object(const std::string &t_type_name)
        : m_type_name(t_type_name)
      {
      }

      std::string get_type_name() const
      {
        return m_type_name;
      }

      Boxed_Value get_attr(const std::string &t_attr_name)
      {
        return m_attrs[t_attr_name];
      }

      std::map<std::string, Boxed_Value> get_attrs()
      {
        return m_attrs;
      }

    private:
      std::string m_type_name;

      std::map<std::string, Boxed_Value> m_attrs;
  };

  struct Dynamic_Object_Attribute
  {
    static Boxed_Value func(const std::string &t_type_name, const std::string &t_attr_name,
        Dynamic_Object &t_do)
    {
      if (t_do.get_type_name() != t_type_name) 
      {
        throw bad_boxed_cast("Dynamic object type mismatch");
      }

      return t_do.get_attr(t_attr_name);
    }
  };

  /**
   * A Proxy_Function implementation designed for calling a function
   * that is automatically guarded based on the first param based on the
   * param's type name
   */
  class Dynamic_Object_Function : public Proxy_Function_Base
  {
    public:
      Dynamic_Object_Function(
          const std::string &t_type_name,
          const Proxy_Function &t_func,
          const boost::optional<Type_Info> &t_ti = boost::optional<Type_Info>())
        : Proxy_Function_Base(build_param_types(t_func->get_param_types(), t_ti)),
          m_type_name(t_type_name), m_func(t_func), m_ti(t_ti)
      {
        assert( (t_func->get_arity() > 0 || t_func->get_arity() < 0)
            && "Programming error, Dynamic_Object_Function must have at least one parameter (this)");
      }

      virtual ~Dynamic_Object_Function() {}

      virtual bool operator==(const Proxy_Function_Base &f) const
      {
        const Dynamic_Object_Function *df = dynamic_cast<const Dynamic_Object_Function *>(&f);
        if (df)
        {
          return df->m_type_name == m_type_name && (*df->m_func) == (*m_func);
        } else {
          return false;
        }
      }

      virtual bool call_match(const std::vector<Boxed_Value> &vals) const
      {
        if (dynamic_object_typename_match(vals, m_type_name, m_ti))
        {
          return m_func->call_match(vals);
        } else {
          return false;
        }
      }    

      virtual std::vector<Const_Proxy_Function> get_contained_functions() const
      {
        std::vector<Const_Proxy_Function> fs;
        fs.push_back(m_func);
        return fs;
      }


      virtual int get_arity() const
      {
        return m_func->get_arity();
      }

      virtual std::string annotation() const
      {
        return m_func->annotation();
      }


    protected:
      virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params) const
      {
        if (dynamic_object_typename_match(params, m_type_name, m_ti))
        {
          return (*m_func)(params);
        } else {
          throw guard_error();
        } 
      }

      virtual bool compare_first_type(const Boxed_Value &bv) const
      {
        return dynamic_object_typename_match(bv, m_type_name, m_ti);
      }

    private:
      static std::vector<Type_Info> build_param_types(
          const std::vector<Type_Info> &t_inner_types, boost::optional<Type_Info> t_objectti)
      {
        if (t_objectti)
        {
          std::vector<Type_Info> types(t_inner_types);

          assert(types.size() > 1);
          assert(types[1].bare_equal(user_type<Boxed_Value>()));
          types[1] = *t_objectti;
          return types;
        } else {
          return t_inner_types;
        }
      }

      static bool dynamic_object_typename_match(const Boxed_Value &bv, const std::string &name,
        const boost::optional<Type_Info> &ti)
      {
        static Type_Info doti = user_type<Dynamic_Object>();
        if (bv.get_type_info().bare_equal(doti))
        {
          try {
            const Dynamic_Object &d = boxed_cast<const Dynamic_Object &>(bv);
            return name == "Dynamic_Object" || d.get_type_name() == name;
          } catch (const std::bad_cast &) {
            return false;
          } 
        } else {
          if (ti)
          {
            return bv.get_type_info().bare_equal(*ti);
          } else {
            return false;
          }
        }

      }

      static bool dynamic_object_typename_match(const std::vector<Boxed_Value> &bvs, const std::string &name,
          const boost::optional<Type_Info> &ti) 
      {
        if (bvs.size() > 0)
        {
          return dynamic_object_typename_match(bvs[0], name, ti);
        } else {
          return false;
        }
      }

      std::string m_type_name;
      Proxy_Function m_func;
      boost::optional<Type_Info> m_ti;

  };


  /**
   * A Proxy_Function implementation designed for creating a new
   * Dynamic_Object
   * that is automatically guarded based on the first param based on the
   * param's type name
   */
  class Dynamic_Object_Constructor : public Proxy_Function_Base
  {
    public:
      Dynamic_Object_Constructor(
          const std::string &t_type_name,
          const Proxy_Function &t_func)
        : Proxy_Function_Base(build_type_list(t_func->get_param_types())),
          m_type_name(t_type_name), m_func(t_func)
      {
        assert( (t_func->get_arity() > 0 || t_func->get_arity() < 0)
            && "Programming error, Dynamic_Object_Function must have at least one parameter (this)");
      }

      static std::vector<Type_Info> build_type_list(const std::vector<Type_Info> &tl)
      {
        std::vector<Type_Info>::const_iterator begin = tl.begin();
        std::vector<Type_Info>::const_iterator end = tl.end();

        if (begin != end)
        {
          ++begin;
        }

        return std::vector<Type_Info>(begin, end);
      }

      virtual ~Dynamic_Object_Constructor() {}

      virtual bool operator==(const Proxy_Function_Base &f) const
      {
        const Dynamic_Object_Constructor *dc = dynamic_cast<const Dynamic_Object_Constructor*>(&f);
        if (dc)
        {
          return dc->m_type_name == m_type_name && (*dc->m_func) == (*m_func);
        } else {
          return false;
        }
      }

      virtual bool call_match(const std::vector<Boxed_Value> &vals) const
      {
        std::vector<Boxed_Value> new_vals;
        new_vals.push_back(Boxed_Value(Dynamic_Object(m_type_name)));
        new_vals.insert(new_vals.end(), vals.begin(), vals.end());

        return m_func->call_match(new_vals);
      }    


      virtual int get_arity() const
      {
        // "this" is not considered part of the arity
        return m_func->get_arity() - 1; 
      }

      virtual std::string annotation() const
      {
        return m_func->annotation();
      }

    protected:
      virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params) const
      {
        std::vector<Boxed_Value> new_params;
        chaiscript::Boxed_Value bv = var(Dynamic_Object(m_type_name));
        new_params.push_back(bv);
        new_params.insert(new_params.end(), params.begin(), params.end());

        (*m_func)(new_params);

        return bv;
      }

    private:
      std::string m_type_name;
      Proxy_Function m_func;

  };
}
#endif

