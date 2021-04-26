
# Prerequisites

The following tools are used for development in this project.

- [Uncrustify](http://uncrustify.sourceforge.net/)
  - For easy installation, use [Choco](https://chocolatey.org/)
  - `choco install uncrustify`

# Installing Git Hooks

To install these Git hooks:

1. Open a Git bash terminal in this repo.
1. `./uncrustify/install_hooks.sh .`

# Manually execute Uncrustify

    uncrustify --replace --no-backup -c .uncrustify.cfg src/*.[h,cpp] test/*.[cpp]
