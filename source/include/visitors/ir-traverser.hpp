/** ir-traversal.hpp
 * Copyright © 2021 Gene Harvey
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef OCTAVE_IR_IR_TRAVERSER_HPP
#define OCTAVE_IR_IR_TRAVERSER_HPP

#include "components/ir-component.hpp"
#include "components/ir-structure.hpp"
#include "components/ir-component-fork.hpp"
#include "components/ir-component-loop.hpp"
#include "components/ir-component-sequence.hpp"
#include "components/ir-function.hpp"

#include <algorithm>

namespace gch
{
  class ir_component;
  class ir_block;

  class ir_structure;
  class ir_function;

  class ir_substructure;
  class ir_component_fork;
  class ir_component_loop;
  class ir_component_sequence;

  enum class ir_traverser_state : bool
  {
    run  = false,
    stop = true ,
  };

  template <typename Derived>
  class ir_descender;

  template <template <typename> typename DerivedT, typename BlockVisitor>
  class ir_descender<DerivedT<BlockVisitor>>
  {
    using derived_type = DerivedT<BlockVisitor>;

  public:
    using block_visitor_type = BlockVisitor;

    explicit
    ir_descender (BlockVisitor& block_visitor) noexcept
      : m_block_visitor (block_visitor),
        m_state   (ir_traverser_state::run)
    { }

    void
    visit (ir_block& c)
    {
      set_state (m_block_visitor.visit (c));
    }

    [[nodiscard]]
    ir_traverser_state
    get_state (void) const noexcept
    {
      return m_state;
    }

    ir_traverser_state
    set_state (bool state) noexcept
    {
      return (m_state = static_cast<ir_traverser_state> (state));
    }

    [[nodiscard]]
    bool
    is_stopped (void) const noexcept
    {
      return get_state () == ir_traverser_state::stop;
    }

    [[nodiscard]]
    explicit
    operator bool (void) const noexcept
    {
      return is_stopped ();
    }

    bool
    operator() (ir_component& c)
    {
      c.accept (static_cast<derived_type&> (*this));
      return is_stopped ();
    }

    bool
    dispatch_child (ir_component& c)
    {
      return m_block_visitor.dispatch_child (c);
    }

    bool
    dispatch_child (ir_component_ptr comp)
    {
      return dispatch_child (*comp);
    }

  private:
    BlockVisitor&      m_block_visitor;
    ir_traverser_state m_state         = ir_traverser_state::stop;
  };

  template <typename BlockVisitor>
  class ir_forward_descender
    : public ir_descender<ir_forward_descender<BlockVisitor>>
  {
    using base = ir_descender<ir_forward_descender<BlockVisitor>>;

  public:
    using block_visitor_type = BlockVisitor;

    using ir_descender<ir_forward_descender<BlockVisitor>>::ir_descender;

    using base::dispatch_child;
    using base::set_state;

    void
    visit (ir_component_fork& fork)
    {
      set_state (dispatch_child (fork.get_condition ())
             ||  std::all_of (fork.cases_begin (), fork.cases_end (),
                              [&](ir_component& sub) { return dispatch_child (sub); }));
    }

    void
    visit (ir_component_loop& loop)
    {
      set_state (dispatch_child (loop.get_start ()    )
             ||  dispatch_child (loop.get_condition ())
             ||  dispatch_child (loop.get_body ()     )
             ||  dispatch_child (loop.get_update ()   ));
    }

    void
    visit (ir_component_sequence& seq)
    {
      set_state (std::any_of (seq.begin (), seq.end (),
                              [&](ir_component& sub) { return dispatch_child (sub); }));
    }

    void
    visit (ir_function& func)
    {
      dispatch_child (func.get_body ());
      set_state (ir_traverser_state::stop);
    }
  };

  template <typename BlockVisitor>
  class ir_backward_descender
    : public ir_descender<ir_backward_descender<BlockVisitor>>
  {
    using base = ir_descender<ir_backward_descender<BlockVisitor>>;

  public:
    using block_visitor_type = BlockVisitor;

    using ir_descender<ir_backward_descender<BlockVisitor>>::
      ir_descender;

    using base::dispatch_child;
    using base::set_state;
    using base::is_stopped;

    void
    visit (ir_component_fork& fork)
    {
      set_state (std::all_of (fork.cases_begin (), fork.cases_end (),
                              [&](ir_component& sub) { return dispatch_child (sub); })
             ||  dispatch_child (fork.get_condition ()));
    }

    void
    visit (ir_component_loop& loop)
    {
      set_state (dispatch_child (loop.get_condition ()));
      if (! is_stopped ())
      {
        if (! dispatch_child (loop.get_update ()))
          dispatch_child (loop.get_body ());
        set_state (dispatch_child (loop.get_start ()));
      }
    }

    void
    visit (ir_component_sequence& seq)
    {
      set_state (std::any_of (seq.rbegin (), seq.rend (),
                              [&](ir_component& sub) { return dispatch_child (sub); }));
    }

    void
    visit (ir_function& func)
    {
      dispatch_child (func.get_body ());
      set_state (ir_traverser_state::stop);
    }
  };

  template <typename Derived>
  class ir_ascender;

  template <template <typename> typename DerivedT, typename BlockVisitor>
  class ir_ascender<DerivedT<BlockVisitor>>
  {
    using derived_type = DerivedT<BlockVisitor>;

  public:
    ir_ascender (BlockVisitor& block_visitor, ir_component& subcomponent) noexcept
      : m_block_visitor (block_visitor),
        m_subcomponent  (subcomponent)
    { }

    BlockVisitor&
    operator() (ir_structure& s)
    {
      s.accept (static_cast<derived_type&> (*this));
      return m_block_visitor;
    }

    [[nodiscard]]
    ir_component&
    get_subcomponent (void) const noexcept
    {
      return m_subcomponent;
    }

  protected:
    bool
    dispatch_child (ir_component& c)
    {
      return m_block_visitor.dispatch_child (c);
    }

    bool
    dispatch_child (ir_component_ptr comp)
    {
      return dispatch_child (*comp);
    }

    void
    dispatch_parent (ir_substructure& s)
    {
      m_block_visitor.dispatch_parent (s);
    }

    void
    visit (ir_function&)
    { }

  private:
    BlockVisitor   m_block_visitor;
    ir_component&  m_subcomponent;
  };

  // descend after the subcomponent
  template <typename BlockVisitor>
  class ir_forward_ascender
    : public ir_ascender<ir_forward_ascender<BlockVisitor>>
  {
    using base = ir_ascender<ir_forward_ascender<BlockVisitor>>;

  public:
    using block_visitor_type = BlockVisitor;

    using ir_ascender<ir_forward_ascender<BlockVisitor>>::
      ir_ascender;

    using base::get_subcomponent;
    using base::dispatch_child;
    using base::dispatch_parent;

    void
    visit (ir_component_fork& fork)
    {
      if (fork.is_condition (get_subcomponent ()))
      {
        if (! std::all_of (fork.cases_begin (), fork.cases_end (),
                           [&](ir_component& sub) { return dispatch_child (sub); }))
        {
          return dispatch_parent (fork);
        }
      }
      else
      {
        assert (std::any_of (fork.cases_begin (), fork.cases_end (),
                             [&](const ir_component& sub) { return &sub == &get_subcomponent (); }));
      }
    }

    void
    visit (ir_component_loop& loop)
    {
      using id = ir_component_loop::subcomponent_id;

      // we could do some fallthroughs here, but I think it's less readable
      bool stop = false;
      switch (loop.get_id (get_subcomponent ()))
      {
        case id::start:
        {
          stop = dispatch_child (loop.get_condition ())
             ||  dispatch_child (loop.get_body ()     )
             ||  dispatch_child (loop.get_update ()   );
          break;
        }
        case id::condition:
        {
          stop = dispatch_child (loop.get_body ()  )
             ||  dispatch_child (loop.get_update ());
          break;
        }
        case id::body:
        {
          stop = dispatch_child (loop.get_update ());
          break;
        }
        case id::update:
        {
          stop = false;
          break;
        }
      }

      if (! stop)
        dispatch_parent (loop);
    }

    void
    visit (ir_component_sequence& seq)
    {
      ir_component_ptr pos = seq.get_ptr (get_subcomponent ());
      assert (pos != seq.end ());

      if (std::none_of (std::next (pos), seq.end (),
                        [&](ir_component& sub) { return dispatch_child (sub); }))
      {
        return dispatch_parent (seq);
      }
    }
  };

  // ascend before the subcomponent
  template <typename BlockVisitor>
  class ir_backward_ascender
    : public ir_ascender<ir_backward_ascender<BlockVisitor>>
  {
    using base = ir_ascender<ir_backward_ascender<BlockVisitor>>;

  public:
    using block_visitor_type = BlockVisitor;

    using ir_ascender<ir_backward_ascender<BlockVisitor>>::
      ir_ascender;

    using base::get_subcomponent;
    using base::dispatch_child;
    using base::dispatch_parent;

    void
    visit (ir_component_fork& fork)
    {
      if (! fork.is_condition (get_subcomponent ()))
      {
        assert (std::any_of (fork.cases_begin (), fork.cases_end (),
                             [&](const ir_component& sub)
                             {
                               return &sub == &get_subcomponent ();
                             }));

        if (! dispatch_child (fork.get_condition ()))
          return dispatch_parent (fork);
      }
      else
        return dispatch_parent (fork);
    }

    void
    visit (ir_component_loop& loop)
    {
      using id = ir_component_loop::subcomponent_id;
      switch (loop.get_id (get_subcomponent ()))
      {
        case id::condition:
        {
          if (! dispatch_child (loop.get_update ()))
            dispatch_child (loop.get_body ());

          if (! dispatch_child (loop.get_start ()))
            dispatch_parent (loop);
          break;
        }
        case id::update:
        {
          if (! dispatch_child (loop.get_body ())
            &&! dispatch_child (loop.get_condition ())
            &&! dispatch_child (loop.get_start ()))
          {
            dispatch_parent (loop);
          }
          break;
        }
        case id::body:
        {
          if (! dispatch_child (loop.get_condition ()))
          {
            dispatch_child (loop.get_update ());
            if (! dispatch_child (loop.get_start ()))
              dispatch_parent (loop);
          }
          break;
        }
        case id::start:
        {
          dispatch_parent (loop);
          break;
        }
      }
    }

    void
    visit (ir_component_sequence& seq)
    {
      std::reverse_iterator<ir_component_ptr> rpos { seq.get_ptr (get_subcomponent ()) };
      assert (rpos != seq.rend ());

      if (std::none_of (std::next (rpos), seq.rend (),
                        [&](ir_component& sub) { return dispatch_child (sub); }))
      {
        return dispatch_parent (seq);
      }
    }
  };

  template <typename BlockVisitor>
  ir_forward_descender (BlockVisitor&) -> ir_forward_descender<BlockVisitor>;

  template <typename BlockVisitor>
  ir_backward_descender (BlockVisitor&) -> ir_backward_descender<BlockVisitor>;

  template <typename BlockVisitor>
  ir_forward_ascender (BlockVisitor&, ir_component&) -> ir_forward_ascender<BlockVisitor>;

  template <typename BlockVisitor>
  ir_backward_ascender (BlockVisitor&, ir_component&) -> ir_backward_ascender<BlockVisitor>;

  class ir_block_visitor_prototype
  {
    ir_block_visitor_prototype&
    operator() (ir_component& c)
    {
      if (! ir_forward_descender<ir_block_visitor_prototype> { *this } (c))
        dispatch_parent (c);
      return *this;
    }

    bool
    visit (ir_block&)
    {
      return false;
    }

    bool
    dispatch_child (ir_component& c)
    {
      ir_block_visitor_prototype sub;
      // do operations needed to prepare class data for sub-scope

      return ir_forward_descender<ir_block_visitor_prototype> { sub } (c);
    }

    void
    dispatch_parent (ir_component& c)
    {
      ir_block_visitor_prototype super;
      // do operations needed to prepare class data for super-scope

      ir_forward_ascender<ir_block_visitor_prototype> parent { super, c };
      parent (*c.maybe_get_parent ());
    }
  };

}

#endif // OCTAVE_IR_IR_TRAVERSER_HPP
