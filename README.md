# lumin


## Requirements

```
brew install pkg-config glfw3 sdl2 sdl2_mixer
```

## Build Instructions

```
cmake ..
make
./lumin
```


## Mirroring to Stash

```sh
# Set up Stash as a remote
git remote add stash https://stash.ugrad.cs.ubc.ca:8443/scm/cs436d18w2/team03game.git

# Whenever we make updates to master, pull from GitHub and push to Stash
git pull origin
git push stash
```

