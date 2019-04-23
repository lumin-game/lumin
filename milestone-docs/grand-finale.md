# April 19: Grand Finale

## Comparisons to our own timetable

- Shadows look good
- Objects can detect if they are being hit by light in a robust and in a way that feels natural
  - Yes 
- Light and shadows bug-free
- At least one level that fully uses every mechanic in the game
  - Last level uses all the main mechanics of the game (fireflies, moving platforms, lasers, light / dark walls, glass)
- Complete user flow from start screen to level select to levels
  - Implemented start screen and finalized total number of levels
- Level designs that incrementally and intuitively teach game mechanics
  - Took awhile but got it done.
- Fun puzzles
  - We definitely have those.
- Finalize art and music, robust user testing
  - Done
- Final touches to fix any minor bugs
  - Fixed a lot of bugs we had in previous milestones
- Investigate third party engines / tools that support extra lighting options (ambient light, bloom etc.)
  - Instead, we looked at text rendering for the third party tools


## Comparison to milestone requirements
- Development: All features implemented in the previous milestones should be working, or improved upon if it’s the case. 
- Robustness: Sustain progressive, non-repetitive gameplay across one or more levels for 10min including all new features. No verbal explanation should be required at any point during the gameplay.
  - Designed 20 levels with progressive difficulty, should take > 10 minutes
- Usability: Include a self-explanatory tutorial introducing the player to the game mechanics.
  - Added hints for each level to progressively introduce different game entities
- External Integration: Include integration of one or more external tools or libraries (physical simulation (PhysX, Bullet, ODE, etc …), game engines, or alternatives)
  - Text-rendering for current level counter and skips remaining using FreeType library
- Advanced Graphics: Implementation of one or more advanced graphics features including visual effects (Particle Systems, 2.5D(3D) lighting, 2D dynamic shadows) and/or advanced 2D geometric modifications (2D deformations, rigged/skinned motion).
  - Dynamic shadows
- Advanced Gameplay: Implementation of one or more advanced gameplay features including advanced decision making mechanisms based on goals (path planning, A*, or similar), or some form of group behavior if applicable to the game; more complex physics interactions with the environment (e.g. gravity, bouncing, complex dynamics).
  - Object detects if even a little light is making contact (pixel perfect light detection)
  - Fireflies move towards the end of the laser light while it is attracted to the player when they switch to the radius light
  - Space is discretized for collision detection
- Accessibility: evaluate and optimizing user-game interactions (choice of user gestures, ease of navigation, etc …).
  - Incorporated control feedback from playtest sessions
  - Added point-and-click interactions on main menu and level selection menu
- Audio: There should be audio feedback for all meaningful interactions in the game as well as a background music with tones reflecting the current state of the game.
  - Yes, we’ve added background music as well as sound effects for all interactions in the game
