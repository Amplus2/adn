## adn
_Adscript Data Notation_ is a modified version of
[EDN](https://github.com/edn-format/edn). This spec is currently very sparse,
but still usable.

### Data types
| Type    | Example Implementation (C/C++)  | Example            |
|---------|---------------------------------|--------------------|
| `id`    | none (resolved at compile time) | `f`                |
| `char`  | `char8_t`                       | `\A`               |
| `int`   | `int64_t`                       | `42`               |
| `float` | `double`                        | `13.37`            |
| `str`   | `char*`, `std::string`          | `"hi"`             |
| `list`  | none (resolved at compile time) | `(f 1)`            |
| `hevec` | `void**`, `std::vector<void*>`  | `["hi" 2]`         |
| `hovec` | `T*`, `std::vector<T>`          | `#[1 2]`           |
| `hemap` | `std::map<hash, void*>`         | `{0 "hi" "bye" 2}` |
| `homap` | `std::map<K, V>`                | `{\x 1 \y 2}`      |

<!--TODO: go into detail about those-->
