/** ir-traversal.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_TRAVERSER_HPP
#define OCTAVE_IR_IR_TRAVERSER_HPP

#include "ir-structure.hpp"

namespace gch
{
  class ir_structure_descender
  {
  public:
    using comp_list = std::vector<nonnull_ptr<ir_component>>;
    using iter       = comp_list::iterator;
    using citer      = comp_list::const_iterator;
    using riter      = comp_list::reverse_iterator;
    using criter     = comp_list::const_reverse_iterator;
    using ref        = comp_list::reference;
    using cref       = comp_list::const_reference;

    [[nodiscard]] iter   begin (void)       noexcept   { return m_components.begin ();   }
    [[nodiscard]] citer  begin (void) const noexcept   { return m_components.begin ();   }
    [[nodiscard]] citer  cbegin (void) const noexcept  { return m_components.cbegin ();  }

    [[nodiscard]] iter   end (void)       noexcept     { return m_components.end ();     }
    [[nodiscard]] citer  end (void) const noexcept     { return m_components.end ();     }
    [[nodiscard]] citer  cend (void) const noexcept    { return m_components.cend ();    }

    [[nodiscard]] riter  rbegin (void)       noexcept  { return m_components.rbegin ();  }
    [[nodiscard]] criter rbegin (void) const noexcept  { return m_components.rbegin ();  }
    [[nodiscard]] criter crbegin (void) const noexcept { return m_components.crbegin (); }

    [[nodiscard]] riter  rend (void)       noexcept    { return m_components.rend ();    }
    [[nodiscard]] criter rend (void) const noexcept    { return m_components.rend ();    }
    [[nodiscard]] criter crend (void) const noexcept   { return m_components.crend ();   }

    [[nodiscard]] ref    front (void)                  { return m_components.front ();   }
    [[nodiscard]] cref   front (void) const            { return m_components.front ();   }

    [[nodiscard]] ref    back (void)                   { return m_components.back ();    }
    [[nodiscard]] cref   back (void) const             { return m_components.back ();    }

    ir_structure_descender (void)                              = delete;
    ir_structure_descender (const ir_structure_descender&)     = default;
    ir_structure_descender (ir_structure_descender&&) noexcept = default;
    ir_structure_descender& operator= (const ir_structure_descender&)     = default;
    ir_structure_descender& operator= (ir_structure_descender&&) noexcept = default;
    ~ir_structure_descender (void)                              = default;

    ir_structure_descender (comp_list&& l, iter start, bool is_fork)
      : m_components (std::move (l)),
        m_start      (start),
        m_is_fork    (is_fork)
    { }

    template <typename F, typename Ret, typename... Args>
    std::enable_if_t<std::is_same_v<std::invoke_result_t<F, ir_block&, Args&&...>,
                                    bool>,
                     std::vector<Ret>>
    collect (F func, Args&&... args)
    {
      auto f = std::bind (func, std::placeholders::_1, std::forward<Args> (args)...);
      return select_collect (f);
    }

  private:

    template <typename F, typename Ret>
    constexpr std::pair<bool, std::vector<Ret>> select_collect (F f)
    {
      return m_is_fork ? sequence_collect (f) : fork_collect (f);
    }

    template <typename B, typename Ret>
    std::pair<bool, std::vector<Ret>> sequence_collect (B unary_func)
    {
      std::vector<Ret> ret_vec { };
      return { std::any_of (m_start, m_components.end (),
                             [&ret_vec, unary_func] (nonnull_ptr<ir_component> c)
                             {
                               auto curr_ret = single_collect (unary_func, c);
                               auto curr_ret_vec = std::get<std::vector<Ret>> (curr_ret);
                               if (! curr_ret_vec.empty ())
                                 std::move (curr_ret_vec.begin (), curr_ret_vec.end (),
                                            std::back_inserter (ret_vec));
                               return std::get<bool> (curr_ret);
                             }), std::move (ret_vec) };
    }

    template <typename B, typename Ret>
    std::pair<bool, std::vector<Ret>> fork_collect (B unary_func)
    {
      auto beg = m_start;
      if (m_components.empty ())
        return { false, { } };

      auto p = single_collect (unary_func, *beg);
      if (beg != m_components.begin () || std::get<bool> (p))
        return p;

      bool stop_branch = true;
      auto& ret_vec = std::get<std::vector<Ret>> (p);
      std::for_each (++beg, m_components.end (),
                     [&stop_branch, &ret_vec, unary_func] (nonnull_ptr<ir_component> c)
                     {
                       auto curr_ret = single_collect (unary_func, c);
                       auto curr_ret_vec = std::get<std::vector<Ret>> (curr_ret);
                       if (! curr_ret_vec.empty ())
                         std::move (curr_ret_vec.begin (), curr_ret_vec.end (),
                                    std::back_inserter (ret_vec));
                       stop_branch &= std::get<bool> (curr_ret);
                     });
      return { stop_branch, ret_vec };
    }

    template <typename B, typename Ret>
    static std::pair<bool, std::vector<Ret>>
    single_collect (B unary_func, nonnull_ptr<ir_component> c)
    {
      if (auto *block = dynamic_cast<ir_block *> (c.get ()))
      {
        if (auto ret = unary_func (*block))
          return { true, { *ret } };
        else
          return { false, { } };
      }
      auto sub_des = static_cast<ir_structure&> (*c).create_descender ();
      return sub_des.select_collect (unary_func);
    }

    comp_list  m_components;
    iter       m_start;
    bool       m_is_fork;
  };

  class ir_structure_ascender
  {
  public:
    using comp_list = std::vector<nonnull_ptr<ir_component>>;
    using iter       = comp_list::iterator;
    using citer      = comp_list::const_iterator;
    using riter      = comp_list::reverse_iterator;
    using criter     = comp_list::const_reverse_iterator;
    using ref        = comp_list::reference;
    using cref       = comp_list::const_reference;

    [[nodiscard]] iter   begin (void)       noexcept   { return m_components.begin ();   }
    [[nodiscard]] citer  begin (void) const noexcept   { return m_components.begin ();   }
    [[nodiscard]] citer  cbegin (void) const noexcept  { return m_components.cbegin ();  }

    [[nodiscard]] iter   end (void)       noexcept     { return m_components.end ();     }
    [[nodiscard]] citer  end (void) const noexcept     { return m_components.end ();     }
    [[nodiscard]] citer  cend (void) const noexcept    { return m_components.cend ();    }

    [[nodiscard]] riter  rbegin (void)       noexcept  { return m_components.rbegin ();  }
    [[nodiscard]] criter rbegin (void) const noexcept  { return m_components.rbegin ();  }
    [[nodiscard]] criter crbegin (void) const noexcept { return m_components.crbegin (); }

    [[nodiscard]] riter  rend (void)       noexcept    { return m_components.rend ();    }
    [[nodiscard]] criter rend (void) const noexcept    { return m_components.rend ();    }
    [[nodiscard]] criter crend (void) const noexcept   { return m_components.crend ();   }

    [[nodiscard]] ref    front (void)                  { return m_components.front ();   }
    [[nodiscard]] cref   front (void) const            { return m_components.front ();   }

    [[nodiscard]] ref    back (void)                   { return m_components.back ();    }
    [[nodiscard]] cref   back (void) const             { return m_components.back ();    }

    ir_structure_ascender (void)                              = delete;
    ir_structure_ascender (const ir_structure_ascender&)     = default;
    ir_structure_ascender (ir_structure_ascender&&) noexcept = default;
    ir_structure_ascender& operator= (const ir_structure_ascender&)     = default;
    ir_structure_ascender& operator= (ir_structure_ascender&&) noexcept = default;
    ~ir_structure_ascender (void)                              = default;

    ir_structure_ascender (comp_list&& l, riter rstart, ir_structure& structure, bool is_fork)
      : m_components (std::move (l)),
        m_rstart     (rstart),
        m_structure  (structure),
        m_is_fork    (is_fork)
    { }

    template <typename F, typename Ret, typename... Args>
    std::enable_if_t<std::is_same_v<std::invoke_result_t<F, ir_block&, Args&&...>,
                                    bool>,
                     std::vector<Ret>>
    collect (F func, Args&&... args)
    {
      auto f = std::bind (func, std::placeholders::_1, std::forward<Args> (args)...);
      return select_collect (f);
    }

  private:

    template <typename F, typename Ret>
    constexpr std::pair<bool, std::vector<Ret>> select_collect (F f)
    {
      return m_is_fork ? sequence_collect (f) : fork_collect (f);
    }

    template <typename B, typename Ret>
    std::pair<bool, std::vector<Ret>> sequence_collect (B unary_func)
    {
      std::vector<Ret> ret_vec { };
      bool branch_stop = std::any_of (m_rstart, m_components.rend (),
                                      [&ret_vec, unary_func] (nonnull_ptr<ir_component> c)
                                      {
                                        auto curr_ret = single_collect (unary_func, c);
                                        auto& curr_ret_vec = std::get<std::vector<Ret>> (curr_ret);
                                        if (! curr_ret_vec.empty ())
                                          std::move (curr_ret_vec.begin (), curr_ret_vec.end (),
                                                     std::back_inserter (ret_vec));
                                        return std::get<bool> (curr_ret);
                                      });
      if (! branch_stop && m_structure->has_parent ())
      {
        ir_structure& parent = m_structure->get_parent ();
        auto sub_asc = parent.create_ascender (*m_structure);
        auto parent_ret = sub_asc.select_collect (unary_func);
        branch_stop = std::get<bool> (parent_ret);
        auto& parent_ret_vec = std::get<std::vector<Ret>> (parent_ret);
        if (! parent_ret_vec.empty ())
          std::move (parent_ret_vec.begin (), parent_ret_vec.end (),
                     std::back_inserter (ret_vec));
      }
      return { branch_stop, std::move (ret_vec) };
    }

    template <typename B, typename Ret>
    std::pair<bool, std::vector<Ret>> fork_collect (B unary_func)
    {
      auto beg = m_rstart;
      if (m_components.empty())
        return { false, { } };

      auto p = single_descend_collect (unary_func, *beg);
      if (beg == --m_components.rend () || std::get<bool> (p))
        return p;

      auto& curr_ret_vec = std::get<std::vector<Ret>> (p);
      auto cond_ret = single_collect (unary_func, m_components.front ());
      auto& cond_ret_vec = std::get<std::vector<Ret>> (p);

      std::move (cond_ret_vec.begin (), cond_ret_vec.end (), std::back_inserter (curr_ret_vec));

      bool branch_stop = std::get<bool> (cond_ret);
      if (! branch_stop && m_structure->has_parent ())
      {
        ir_structure& parent = m_structure->get_parent ();
        auto sub_asc = parent.create_ascender (*m_structure);
        auto parent_ret = sub_asc.select_collect (unary_func);
        branch_stop = std::get<bool> (parent_ret);
        auto& parent_ret_vec = std::get<std::vector<Ret>> (parent_ret);
        if (! parent_ret_vec.empty ())
          std::move (parent_ret_vec.begin (), parent_ret_vec.end (),
                     std::back_inserter (curr_ret_vec));
      }
      return { branch_stop, std::move (curr_ret_vec) };
    }

    template <typename B, typename Ret>
    static std::pair<bool, std::vector<Ret>>
    single_collect (B unary_func, nonnull_ptr<ir_component> c)
    {
      if (auto *block = dynamic_cast<ir_block *> (c.get ()))
      {
        if (auto ret = unary_func (*block))
          return { true, { *ret } };
        else
          return { false, { } };
      }
      auto sub_asc = static_cast<ir_structure&> (*c).create_ascender ();
      return sub_asc.select_collect (unary_func);
    }

    comp_list                 m_components;
    riter                     m_rstart;
    nonnull_ptr<ir_structure> m_structure;
    bool                      m_is_fork;
  };
}

#endif // OCTAVE_IR_IR_TRAVERSER_HPP
