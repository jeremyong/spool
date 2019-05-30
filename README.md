<p align="center"><img src="https://raw.githubusercontent.com/jeremyong/assets/master/spool.png" height="330px"/></p>

> Spool - A C or C++ cmake *s*tring *pool*ing preprocessor.

## 🚩 Table of Contents
- [Motivation](#motivation)
- [Features](#features)
- [Usage](#usage)
    - [Requirements](#requirements)
    - [Cmake Integration](#cmake-integration)
    - [Code Integration](#code-integration)
- [Caveats](#caveats)
- [FAQ](#faq)
- [License](#license)

## Motivation

Most C and C++ programmers know the perils of relying on the following practices:

```c++
    // Comparing string literals by comparing their pointer values
    const char* name1 = "Alice";
    const char* name2 = "Alice";

    // Yikes!
    assert(name1 == name2);
```

Obviously, you need to do a `strcmp` or equivalent if you want this sort of comparison operation to make sense.

```c++
    // Using string literals as keys in a map

    const char* name1 = "Alice";
    const char* name2 = "Alice";
    std::unordered_map<const char*, bool> people_present;
    people_present[name1] = true;
    people_present[name2] = true;
    
    // Now people_present will have two entries... both keyed to "Alice" stored in different locations
    // ...
    // Or not! Where they are stored is technically implementation defined
```

Now, various compilers can of course opt to pool this memory in a common section. The usual trick is to generate
a mangled name based on the string contents (placed in the same data segment). That way, at link-time, the linker
will identify that two segments of memory actually are the same, and the constants will be coalesced. Relying on
this is a huge no-no because it's compiler dependent, and possibly dependent on your optimization flags and
compile mode.

With this library, you can do the following:

```cmake
    # In your cmake file
    add_subdirectory(spool)

    spool(your_lib)
    spool(your_exe) # suppose your_exe depends on your_lib
```

And now, all the following code works

```c++
    #include <spool.h>

    const char* name1 = SP("Gerald");
    const char* name2 = SP("Gerald");

    // Will always be pass
    assert(name1 == name2);

    std::unordered_map<const char*, bool> people_present;
    people_present[name1] = true;
    people_present[name2] = true;

    // Now you only have the one entry for Gerald
    assert(people_present.size() == 1);
```

All of the above is done *without* incurring any runtime cost (even the indirections on initialization are optimized away).

Now, so long as you've appropriately spooled your targets, you can rely on string literals to use as keys in maps, do
`O(1)` comparisons to check equality, and more.

## 🎨 Features

**Spool** uses a couple levels of indirection to ensure that when you declare a spooled string literal, they refer to the
same address in memory located within a single translation unit in your project. This enables fast comparisons, keying,
and other nice properties.

Spool uses a custom preprocessor to achieve this, and takes care to support incremental builds where only one or a
few files have been modified. This is done by carefully injecting itself within your cmake dependency tree.

Spool supports full target spooling or spooling only individual files. In addition, you can spool different targets or
files into different "domains" to provide isolation between modules as necessary. By default though, all spooled strings
and placed and referenced from a single translation unit.

## 🔨 Usage

### Requirements

To use spool, you need:

- A relatively recent version of Cmake
- A C++17 capable compiler
- A recent [Sqlite](https://www.sqlite.org/index.html) installed and in your path along with its development libraries

Also **please please** read the [caveats](#caveats) section below before using spool, as this library is very early stages.

### Cmake Integration

Spool was written to be super easy to integrate provided you already use `cmake`. Simply call `add_subdirectory` from your
source tree and supply the path to this project (e.g. `add_subdirectory(spool)`).

This automatically adds `spool/cmake` to your `CMAKE_MODULE_PATH` which provides you with two functions: `spool` and `spool_file`.

The `spool` function works on library and executable targets and takes the target name as the first argument. Thereafter,
it injects a conditioning step (aka "spooling") for each constituent source file to scan for strings that need to be pooled.

```cmake
    # Sample spool usage
    add_library(my_lib ${MY_LIB_SOURCES})
    spool(my_lib)

    add_executable(my_exe ${MY_EXE_SOURCES})
    target_link_libraries(my_exe PUBLIC my_lib)

    # IMPORTANT: spool(my_lib) is invoked first
    spool(my_exe)
 ```


The `spool_file` function takes as an additional argument the path to a specific source file you wish to be spooled.
Use this if you only want the spooling preprocessor to only run on a subset of files in a target.

```cmake
    # Sample spool_file usage
    add_library(my_lib src1.cpp src2.cpp src3.cpp)
    spool_file(my_lib src1.cpp)
    spool_file(my_lib src2.cpp)

    # Now, string literals between src1.cpp and src2.cpp can be mapped to the same location in memory
    # but src3.cpp will be excluded
```

Both `spool` and `spool_file` accept an optional argument at the end denoting the spool domain. By default, this is
set to `spool_default`. If you specify a different name, then all strings will be spooled under the domain by that name.

For example, suppose we have two source files like so:

```c++
    // A.cpp
    #include <spool.h>

    const char* a_foo = SP("hi");
```

```c++
    // B.cpp
    #include <spool.h>

    const char* b_foo = SP("hi");
```

If both `A.cpp` and `B.cpp` belong to the same spool domain, then we can expect that `a_foo == b_foo` will evaluate truthy.
However, if `A.cpp` and `B.cpp` were spooled with, say `spool_file(my_lib A.cpp spoolA)` and `spool_file(my_lib B.cpp spoolB)`,
we are guaranteed that `a_foo != b_foo`.

### Code Integration

In code, you will need to do two things:

1. First, include the header `#include <spool.h>` which is already available in your include path for targets that have been spooled
2. Second, wrap literals you wish to be spooled with the macro `SP`

Feel free to look at the `test` folder (which is a simple executable, no fancy test frameworks or anything) to understand the usage.

## Caveats

It is **important** that you read the contents of this section.

- First and foremost, *THIS IS PRE-ALPHA QUALITY*. I primarily needed it for my own projects where I (abuse) this technique
  extensively to get better performance. I am publishing this mainly because there is a fair bit of machinery here (especially
  with the Cmake aspects) that I could not find decent examples for online, and also because it may be a good starting point.
- Bugs you will likely run into:
    - Windows/MacOS support (largely untested at this time)
    - Bugs if you use a wildly different compiler than I did (recent `g++` and `clang++`)
    - Issues with projects that have more exotic linking strategies or mixed shared/static linkage
- Obviously, this only works when working with statically defined strings. Strings that are constructed dynamically
  or mutated cannot be pooled
- The `SP` spooling macro does not work in headers (typically, you would use this with C++17's `inline` keyword)
- The `SP` spooling macro relies on `__COUNTER__` and usage of this macro in your translation units *will break* the spooling
- When using the cmake `spool` function, you must do it in dependency order. Meaning, if `A` depends on `B`, `spool` must
  be applied to `A` first.

## FAQ

**How does this work?**

The macro `SP` is defined in [spool.h](https://github.com/jeremyong/spool/blob/master/public/spool.h) as follows:

```c++
    #define SP(...) *spool_strings_[SPOOL_ID][__COUNTER__]
```

The `spool_strings_` constant there is defined as

```c++
    extern const char*** spool_strings_[];
```

It's not everyday you see a lot of triple pointer arrays, but seeing this should give you a decent idea of some of the "magic."

As I have time available, I may publish a blog post detailing the various techniques in the library in the future.

**But why?**

This is a fair question. I think having well-defined semantics for where literals are held in static storage is always something
I wished C++ and C had. As a programmer, you have very little control in managing the various data segments, and the C++ abstract
memory model exists on a much higher plane. The payoff of using a feature like this is quite high when you consider how much
time is wasted hashing and comparing strings that are honestly known at compile time.

**Why not use enums?**

Even with this library, an `enum` or an `enum class` will often absolutely be the right tool for the job. The thing that this requires
though, is visibility and coupling you may not necessarily want. Also, sometimes the same string is a useful identifier in a lot
of different contexts. Sometimes you work with assets that have string representations. Sometimes, you actually want to do string
operations as well (on top of using the "enum" as a key and whatnot). Regardless, even keeping `enum`s in mind,
I have run into situations where a string was easier to work with or more debuggable. Needless to say, that choice is still there
even if this library exists and it isn't going anywhere.

**Is this indirection expensive?**

No. When your code executes, it goes through an initialization sequence to initialize static storage. You are likely doing this
all over the place without even realizing it. After the indirections occur for each string, there is no further runtime penalty.
I haven't used this technique in a large enough application that the difference in startup time was measurable. The payoffs later
can be small to great, depending on how you used strings before. If you're a programmer that uses data structures like
`std::map<std::string, T>` all over the place, boxing `const char*`s as you go, first of all, that's unfortunate, but second of all,
you'll likely see an enormous benefit.

**Really though, is this library for real?**

It's really tough to say. In it's current state, I am worried that it hasn't had remotely enough coverage/usage in the wild to be
anything close to production ready. Also, it's a weird idea, and I'm sure there will be plenty of applications that have no need
for Spool (and possibly even people who are offended by the mere idea of Spool). Either way, I had fun hacking it together, at
least as a proof of concept, and I've been using an even hackier version for a while already with modest success. How "for real"
the library is will be up to you.


## License

This software is licensed under the [MIT](https://github.com/jeremyong/spool/blob/master/LICENSE) license.

Attribution, support, bugfixes, comments, critique, and suggestions are all appreciated and loved, but not required.
