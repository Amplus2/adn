## adn
_Adscript Data Notation_ is a modified version of
[EDN](https://github.com/edn-format/edn). This spec is currently very sparse,
but still usable.

### Data types
| Type    | Compile-time Implementation (C++) | Example            |
|---------|-----------------------------------|--------------------|
| `id`    | `std::u32string`                  | `f`                |
| `char`  | `char32_t`                        | `\A`               |
| `int`   | `int64_t`                         | `42`               |
| `float` | `double`                          | `13.37`            |
| `str`   | `std::u32string`                  | `"hi"`             |
| `list`  | `std::vector<void*>`              | `(f 1)`            |
| `hevec` | `std::vector<void*>`              | `["hi" 2]`         |
| `hovec` | `std::vector<T>`                  | `#[1 2]`           |
| `hemap` | `std::map<hash, void*>`           | `{0 "hi" "bye" 2}` |
| `homap` | `std::map<K, V>`                  | `{\x 1 \y 2}`      |

<!--TODO: go into detail about those-->
