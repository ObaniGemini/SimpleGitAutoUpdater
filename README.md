# SimpleGitAutoUpdater
A simple autoupdater program that takes a simple command input and runs git rev-parse to check if there's any update


# Building

```
make
```

that's it

# Using

This program uses the `exec` API from Linux, so you may need to put the absolute path to your program.
For example, `./gitautoupdater /bin/make` instead of `./gitautoupdater make`

You should also put the executable in the directory where the repo is.
I probably will change this in the future, but for now that's how it is.

The program you put as argument should run the git update, building (if needed) and execution. That's why we recommend you use something like a Makefile.