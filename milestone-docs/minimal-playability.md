# February 25: Minimal Playability

## Comparisons to our own timetable

- All "light types" roughly implemented - should have 3 basic levels completed
  - We decided to narrow down to 2 different light types (area light and laser light)
  - We have 3 levels:
    1. covers basics (directional keys, jump)
    2. introduce switch mechanic using light and glass blocks
    3. slightly more advanced level to challenge users to move vertically up a level (platform)
- Object can detect if they are being hit by light
  - Done!
- Core mechanics implementation complete - mirrors, switches, movement between levels, collisions with re-appearing blocks
  - Switches have been implemented, as well as doors to move between levels, and collision on moving blocks
- Start work on implementing AI characters -  fireflies, characters that do specific tasks, any enemies
  - Work has been started for the fireflies

Additionally, we began work on the level selection menu and added a pause menu to display help for the user if needed (i.e. controls, goal of the game)

## Comparison to milestone requirements
- Sustain progressive, non-repetitive gameplay using all required features for 2 min or more (assume that you can provide users with oral instruction).
  - Roughly 2 mins of gameplay is achieved with the 3 levels created which showcase the different mechanics we have implemented so far in each level.
- You should implement state and decision tree driven (possibly randomized) response to user input and game state (create a decision tree data structure and reuse it for multiple entities)
  - Fireflies are attracted towards the user light so if the user is close to the fireflies, they would move closer to the user and even follow the user.
  - States for entities are determined whether they are lit by the light or not (i.e. switches, glass blocks) and the door opens when the corresponding switch is turned on
- Provide extended sprite and background assets set as well as corresponding actions.
  - Abstract entity class now supports loading in multiple textures, which are dynamically swapped based on whether light is shining on that object or not.
- Provide basic user tutorial/help.
  - This is fulfilled by the instructional "pause" screen (which is accessed by pressing "P" during gameplay)
- Stable game code supporting continuing execution and graceful termination.
  - Game is gracefully terminated by pressing "ESC".


## Creative Component
- Implemented AI fireflies, which display flocking movement and move to the nearest light source
