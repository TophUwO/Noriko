# Noriko {#mainpage}
Noriko (のり子) is the working title of a cross-platform 2D game engine focused on the recreation of classic top-down RPG games. This documentation is part of the game engine component. Documentation for the runtime and editor components is separate from this one. When there are cross references, they are marked as such.
For feature overviews and more in-depth requirements specifications, please refer to the documents found in the `docs/ooa` directory.
## Build Directory Structure
The following table lists all directories that can be found from Noriko's root directory alongside their purpose.
> This table is for the directory structure of the development repository. The directory structure for the distributed application is documented separately.

|Directory Path |Purpose
|--|--
|`.`|root directory
| `bin` | binary and distribution packages
| `docs` | directory for documentation
| `docs/html` | directory for the documentation generated by [doxygen](<https://www.doxygen.nl/>)
| `docs/ooa` | analysis models are requirement specifications
| `docs/ood` | design models and in-depth developer documentation
| `docs/pages` | directory for supplementary doxygen pages
| `ext` | external components (libraries, ...)
| `include/*` | include directories for the component denoted by `*`
| `proj` | directory for project files
| `res` | static resources built into the binary
| `src/*` | source files for the component denoted by `*`

## API Conventions
The following semantic conventions are valid for the entirety of the public API:
- Public API function names consist of an obligatory `Nk` prefix, a namespace name, and the name of the operation, all in *PascalCase* (for example, `NkVectorCreate`).
- Unless specified otherwise, all functions' behavior is undefined if invalid parameters are passed to it.
- Object destructors are always named `Nk*Destroy` where `*` denotes the namespace name.
- Object destructors canonically accept `NULL`-pointers as function parameters and result in a no-op in this case.
- Types that are not meant to be instantiated statically are modeled opaque.