# April 1: Robust Game

## Comparisons to our own timetable

- Level selection implemented
  - Done but needs to be discussed which approach we want to use
- Transitions between scenes
  - Done
- Implement Start Screen
  - Need to do
- Finish player animations
  - Done
- Nail down level progression to feel good to player while unlocking “light types”
  - To implement for final milestone
- AI implementation bug-free
  - Reworking firefly AI to interact with laser for final milestone
- Add BGM soundtrack
  - Saving this for the final milestone


## Comparison to milestone requirements
- Include complete playable prior-milestone implementation
  - Yes
- Sustain progressive, non-repetitive gameplay for 6min or more including all new features. The game should not terminate early and should allow infinite even if repetitive gameplay
  - Added more levels with the new laser light type and for the usual light source
- Support real-time response rate (i.e. lag-free input).
  - Done
- Include proper memory management (no excessive allocation or leaks). The game should not have any undefined behavior, memory leaks or random crashes. The game should not hog memory even after extended play time.
  - Done
- The game should robustly handle any user input. Unexpected inputs or environment settings should be correctly handled and reported.
  - Handled by a series of checks and flags
- The gameplay should be real-time (no lag). This included improving your collision handling using effective detection strategies. You should support dozens simultaneously moving main or background assets.
  - Done
- The game should allow for some form of state saving for play “reload”. Users should be able to pause and restart in a graceful (if not perfect) manner.
  - Autosave mechanic so the user starts on the last level they entered
- The physical effects should be correctly integrated in time and should not be locked to the machine’s speed by correctly handling the simulation time step and integration.
  - We check the time that has passed between now and the previous frame and use this to update game entities
- Stable game code supporting continuing execution and graceful termination.
  - Still implemented from previous milestones.


## Creative Component
- New laser light type
- Player walking animations
- New art for different switches and top menu UI
