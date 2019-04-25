# Pretty Printer for Modern C++

## Highlights

* Header-only library
* Requires C++17
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
printer.print(...);
```

Here's a quick example showing how to print a vector of ```std::variant``` objects

```cpp
// Construct a vector of values
std::vector<std::variant<bool, int, float, std::string, std::vector<int>,
                         std::map<std::string, std::map<std::string, int>>, std::pair<double, double>>> foo;
foo.push_back(5);
foo.push_back(3.14f);
foo.push_back(std::string{"Hello World"});
foo.push_back(std::vector<int>{1, 2, 3, 4});
foo.push_back(std::map<std::string, std::map<std::string, int>>{{"a",{{"b",1}}}, {"c",{{"d",2}, {"e",3}}}});
foo.push_back(true);
foo.push_back(std::pair<double, double>{1.1, 2.2});

// Print the vector
pprint::PrettyPrinter printer;
printer.indent(2);
printer.print(foo);
```

```bash
[
  5, 
  3.14f, 
  "Hello World", 
  [1, 2, 3, 4], 
  {"a" : {"b" : 1}, "c" : {"d" : 2, "e" : 3}}, 
  true, 
  (1.1, 2.2)
]
```

## Table of Contents

* Usage Examples
  - [Fundamental Types](#fundamental-types)
  - [STL Containers](#stl-containers)
  - [Classes](#classes)
* [Contribution](#contribution)
* [License](#license)

## Fundamental Types

```cpp
printer.print(5);
printer.print(3.14f);
printer.print(2.718);
printer.print(true);
printer.print('x');
printer.print("Hello World!");
printer.print(nullptr);
```

```bash
5
3.14f
2.718
true
'x'
"Hello World!"
null
```

## STL Containers

### Sequence Containers: std::vector

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

pprint can handle other containers inside vectors:

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

### Associative Containers: std::map

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

Here's an example of an std::map with compound mapped_type values:

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

## Classes

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

## Contribution
Contributions are welcome, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License
The project is available under the [MIT](https://opensource.org/licenses/MIT) license.
