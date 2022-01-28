# octave-ir

This is a work-in-progress implementation of a JIT intended to (eventually) target the Octave language. Currently it is not doing any translation of the actual AST coming from Octave, but it can compile pieces of code specified with C++. It can currently handle only very simple constructs like non-nested loops.

## Concept

The idea behind this JIT is to eschew the SSA dominance tree algorithms for determining defs in favor of a well-defined hierarchy of components to be iterated over with visitors. We can do this because Octave does not have any arbitrary jumping control structures, so the basic control structures may be defined by a small set of components. 

For example, a loop component consists of four subcomponents: `start`, `condition`, `body`, and `update`. If we need to figure out the location of a def for a use located from within the `body` block, a visitor will first visit `condition`. If there is not def located there, then it will check `update` and `start`. If a totally dominating def is located in the `start` component then the def is determined. Otherwise, the process continues to the parent.

## Methods

This project has been a bit of a playground for me, so you can expect to see various exotic/esoteric expressions. Some are more useful than others. The least useful is probably the whole visitor system because I wanted to save a dispatch call or two.

A useful construct that I wrote and have been using are the `>>=` operators for `std::optional` and `gch::optional_ref` types. It works as the bind operator for the optional (maybe) monad. It allows one to save a few lines when unwrapping optionals. For example, given 

```c++
gch::optional_ref<char> f (void);
std::optional<int> g (char&);
long& h (int&&)
```

you can go from 

```c++
gch::optional_ref<long> res;
if (gch::optional_ref<char> opt { f () })
{
  if (std::optional<int> opt2 { g (*opt) })
    res.emplace (h (std::move (*opt2)));
}
```

to

```c++
gch::optional_ref res { (f () >>= g) >>= h };
```

Specializations of `>>=` for `std::optional` may be found in `ir-optional-util.hpp`.

Heavy usage of `gch::nonnull_ptr` and `gch::optional_ref` can be found throughout. Both are just pointer wrappers for semantics, but `nonnull_ptr` can only be initialized with a reference and can never be null, and `optional_ref` indicates that the state of the object should be checked before proceeding.

If you are interested in methods for mapping runtime values to template instantiations, you can check out the `map_pack` function, and usage of `ir_type`, `ir_metadata`, and the LLVM translation mappings.

## Other projects

Other projects that are utilized by this project include 

- [gch::nonnull_ptr](https://github.com/gharveymn/nonnull_ptr) - A pointer wrapper which is not nullable.
- [gch::optional_ref](https://github.com/gharveymn/optional_ref) - A class encapsulating an optional reference (with rebinding semantics). It also includes specializations for `operator>>=`.
- [gch::partition](https://github.com/gharveymn/partition) - Several wrapper classes for containers which allow for lightweight partitioning of their contents. This allows for iteration over and manipulation of defined subsections of an underlying container.
- [gch::select-iterator](https://github.com/gharveymn/select-iterator) - Iterators which select tuple elements when dereferencing.
- [gch::small_vector](https://github.com/gharveymn/small_vector) - An implementation of a vector with a small buffer optimization.
- [gch::tracker](https://github.com/gharveymn/tracker) - A system for automatic tracking of remote object lifetimes.
