# pprint

## Quick Start

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
printer.print(std::map<std::string, std::vector<int>>{ {"a", {1, 2, 3}}, {"b", {4, 5, 6}} });
```

```bash
{
  "a" : [1, 2, 3],
  "b" : [4, 5, 6]
}
```
