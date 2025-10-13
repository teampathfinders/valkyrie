


# Code Style
## Casing

---
Follow the rust convention for this.   
* `snake_case` for function, variable, and member names. 
* `snake_case` for file/folder names
* `PascalCase` for types (classes, structs, unions, and typedef/using statements all count)
* `SCREAMING_SNAKE_CASE` for global statics/consts (such as `constexpr static int MAX_ENTITIES = 20`)

## Naming

---
* Private class members must start with a prefix of `m_` (e.g. `int m_max_render_distance`)
* Don't contract names/use abbreviations; this rule is fluid and in some cases can be bypassed (such as for Http). This means that contractions such as `int clcr(const class Player&)` are not allowed and should instead be `int calculate_loaded_chunk_radius(const class Player&)`
* Interface and Abstract classes must start with their respective prefixes (`I` for Interfaces, and `A` for Abstract classes)
* Function names must be related to what the function does.

## Coding practices

---

### Error Handling

* Functions which expect to error should be error as value (e.g., returning a `Result<Block*, BlockBoundsError>` from a function which gets a block based on a user provided coordinate). This rule means that any user input we parse (such as packets or commands) must use error as value
* If a function does not throw in any way, it must have the `noexcept` specifier
* Functions are either noexcept or throwing, if a function may throw in debug. It is not a noexcept in release either
* If the function generates new errors (this means a function which bubbles up errors is excluded), it must document that it does this and what error it produces under what conditions.
* All error as value errors must have their error type inherit from a base error class. They must also use our `std::expected` wrapper
> The specific class it should inherit from is `AError` and the `std::expected` wrapper is `Result<E, T>`

### Code Patterns

* Separate "systems" should communicate via events; this means a packet handler should not call the game server handlers directly. It should just fire a `GamePacketEvent` which will be listened for by the implementor.
* Reduce code in headers to only necessary code in the header
* Use forward declarations for types when possible
* Document general APIs
* Write code in a way which reduces the need for comments
* NO `using namespace std;`
* All code written should be namespaced into general sections; the namespace of the classes should come from the parent folder name. (this is only the direct parent, not any higher parents. Example being a class in `src/level/chunk/chunk.hpp` would live in the namespace `valk::chunk` not `valk::level::chunk`)
* All functions must have five or fewer parameters; if more are desired, use a builder pattern or create a configuration struct to be passed to the function instead
* TODO statements must either be removed and completed, or have a reason why they can't be completed before a feature is merged. In cases where a TODO is waiting on another feature, make an issue
* If arguments have specific requirements (such as they are always valid) document this.
* If a parameter is optional, use a raw pointer `*` or an `std::optional` to signify this.
* For raw pointer arguments, it is implied that it does not need ownership of the memory. If this is not the case use an `std::unique_ptr` or document this explicitly
* Make everything const by default

### Dependencies

* Reduce "useless" dependencies. This means dependencies which are trivial to reimplement (example being something such as ZigZag encoding). However, dependencies such as `entt` or `glm` are fine.

### Tests

* Before features are merged, small tests (either compile time tests, or google tests) should be written for each feature unless this is not viable/possible
* Tests should cover all expected use cases. This includes having invalid arguments passed (if this is within the function's contract.)