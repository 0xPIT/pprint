# Pretty Printer for Modern C++

## Highlights

* Header-only library
* Requires C++11
* MIT License

## Quick Start

Simply include pprint.hpp and you're good to go.

```cpp
#include <pprint.hpp>
```

To start printing, create an ```PrettyPrinter```. 

```cpp
pprint::PrettyPrinter printer;
printer.indent(2);
```

## Examples

### Fundamental Types

```cpp
printer.print(5);
printer.print(3.14f);
printer.print(2.718);
printer.print(true);
printer.print('x');
```

```bash
5
3.14f
2.718
true
'x'
```

### std::string

```cpp
printer.print("Hello World!");
```

```bash
"Hello World!"
```

### std::vector

```cpp
printer.print(std::vector<int>{1, 2, 3, 4, 5});
```

```bash
[
  1,
  2,
  3,
  4,
  5
]
```

### Nested vectors

```cpp
std::cout << "foo = ";
printer.print(std::vector<std::vector<int>>{{1, 2, 3}, {4, 5, 6}, {}, {7, 8}});
```

```bash
foo = [
  [1, 2, 3],
  [4, 5, 6],
  [],
  [7, 8]
]
```

### std::map

```cpp
printer.print(std::map<std::string, int>{ {"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}, {"e", 5}});
```

```bash
{
  "a" : 1,
  "b" : 2,
  "c" : 3,
  "d" : 4,
  "e" : 5
}
```

### Compound maps

```cpp
  printer.print(std::map<std::string, std::vector<std::pair<int, std::string>>>{ 
    {"foo", {{1, "b"}, {2, "c"}, {3, "d"}}}, 
    {"bar", {{4, "e"}, {5, "f"}, {6, "g"}}}
  });
  ```

```bash
{
  "foo" : [(1, "b"), (2, "c"), (3, "d")], 
  "bar" : [(4, "e"), (5, "f"), (6, "g")]
}
```

### Pointer to member function

```cpp
class Bar {
public:
  void Foo() {}
};
printer.print(&Bar::Foo);
```

```bash
<Object.method void (main::Bar::*)()>
```
