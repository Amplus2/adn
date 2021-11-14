# adn
_Adscript Data Notation_ is a modified version of
[EDN](https://github.com/edn-format/edn). This spec is currently very sparse,
but still usable.

## Data types
| Type    | Parser Implementation (C++)                | Example            |
|---------|--------------------------------------------|--------------------|
| `id`    | `std::u32string`                           | `f`                |
| `char`  | `char32_t`                                 | `\A`               |
| `int`   | `int64_t` (TODO: bigint)                   | `42`               |
| `float` | `double`                                   | `13.37`            |
| `str`   | `std::u32string`                           | `"hi"`             |
| `list`  | `std::vector<Element>`                     | `(f 1)`            |
| `vec`   | `std::vector<Element>`                     | `["hi" 2]`         |
| `map`   | `std::vector<std::pair<Element, Element>>` | `{0 "hi" "bye" 2}` |

<!--TODO: go into detail about those-->

## Quoting and Hashing
In programming languages from the LISP family, it has been practice for a long
time to use a `'` to distinguish between calling a function and creating a list.
Additionally, languages like Clojure use `#` to distinguish between certain data
types. These behaviors are supported by adn, as `'` and `#` are distinct Lexer
Tokens and added to the next Parser Element. Additionally, we support backquotes
(\`) in the same way, as many LISPs also use those. We currently don't have a
specific unquote character, using `,` would go against treating it as
whitespace, Clojure for example uses `~` instead. If you need unquoting in your
`adn`-based LISP, use an `unquote` function and maybe consider hashed ids
unquoted. If you are really invested in making `adn` unquotes happen, feel free
to open a GitHub issue.
