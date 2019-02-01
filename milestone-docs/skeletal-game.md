# February 1: Skeletal Game

## Comparisons to our own timetable

- Basic character movement (WASD, jump)
  - Done
- Designs for some puzzles
  - We have designs for some puzzles, but they are not coded into the game yet, after we were getting text-to-level generator working so we can add level designs in
- Basic collision (player with walls / floors)
  - Done
- Sprites for a few key elements (player, common objects like blocks, switches, door)
  - We have sprites for players, blocks, switches. Others are on the way.
- Basic level with static walls, switches (doesn't have to do anything yet)
  - We have static walls, instead on inserting switches that don't do anything, we focused on creating one parent class that should carry most functionality of interactable objects. This means when we do actually implement switches it should be an easy override of that class and we will get correct functionality as well. This will also make adding further interactable objects much easier.

## Comparison to milestone requirements
- Working application rendering code and shaders for background and sprite assets
  - Our game renders background and assets
- Loading and rendering of textured geometry with correct blending.
  - Our game uses a texture for character and walls
- Working basic 2D transformations
  - Character can move around correctly with transformations
- Keyboard/mouse control of one or more character sprites. This can include changes in the set of rendered objects, - object geometry, position, orientation, textures, colors, and other attributes.
  - Character can be controlled by keyboard (WASD and jump)
- Random or hard-coded action of (other) characters/assets.
  - The light changes with regards to what blocks are around it
- Basic key-frame/state interpolation (smooth movement from point A to point B in Cartesian or angle space).
  - Our movement is smooth, shadow movements are also smooth
- Stable game code supporting continuing execution and graceful termination.
  - Game doesn't crash as far as we know. You can press ESC to quit gracefully

## Creative Component
- Dynamic lighting based on player location; opaque to walls
