# March 8: Minimal Playability

## Comparisons to our own timetable

- User testing on the completed levels
  - Done during cross-play sessions
- Design more levels based on user testing
  - Done
- Start discussion and design Start Screen
- Add moving walls / floors
  - Done
- Start making animations for player movement, jumping, falling, idle
- Start making/finding sounds fx
  - Implemented sound effects for the switch
- Set of puzzles that rely on different “light types”
  - New light type is not implemented yet
- Object can detect if they are being hit by light, taking into account blocking walls / shadows
  - Done
- Core mechanics should be free of obvious bugs
  - Done but may need further testing
- AI implementation complete and tested
  - Still being worked on
- Simple sounds when things happen (light mode switching, switch getting lit, door unlocking
  - As mentioned above, we’ve added a sound effect for the switch when it is toggled

## Comparison to milestone requirements

- Provide complete playable prior-milestone implementation
  - Done
- Sustain progressive, non-repetitive gameplay for 4min or more including all new features (with minimal tutorial)
  - 6 Levels that build up in difficulty
- Implement time stepping based physical animation: A subset of the game entities (main or background) should now possess non-trivial physics properties such as linear momentum or angular momentum, and acceleration and act based on those. Specifically, you should implement some form of physical simulation, which can be either background effects (e.g. water, smoke implemented using particles) or active game elements (throwing a ball, swinging a rope, etc…)
  - Fireflies are implemented with flocking particle behavior
- Incorporate one or more polygonal geometric assets
  - Our light mesh is now a polygonal asset
- Implement smooth non-linear motion of one or more assets or characters - Implement an accurate and efficient collision detection method (include multiple moving assets that necessitate collision checks)
  - Movable blocks can move with bezier curves (level 6 showcases this implementation)
- Stable game code supporting continuing execution and graceful termination.
  - Game is gracefully terminated by pressing “ESC”.
  - No known crashes (fixed a graphics crash)


## Creative Component
- Improved level selection and pause screens, added game complete screen
- Added static light source (lantern)
- Finalized level generator
  - Added system for declaring connections between switches and specific game entities so that any level can be represented by a .txt file

