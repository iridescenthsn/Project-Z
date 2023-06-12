# ProjectZ

Developed with Unreal Engine 4

This is a FPS project packed with various mechanics, systems ,AI , UI and 2 game modes

Game modes: Campagin and Zombies

Zombies: A round based zombie game mode(Like COD zombies)

Weapon:

-procedural aim recoil with recoil patterns
-ADS (left click)
-holster weapon when near wall
-Ammo shell ejection on firing
-6 unique weapons(3 hitscans and 3 projectile based)
  -Projectiles include sniper rifle projectile, rocket and a grenade launcher projectile with unique behaviour
-Shooting through walls(you can select which walls are penetrable and how thick of a wall can each weapon penetrate)
-weapon pick ups (while looking at weapons press E to pickup)
-2 weapon slots
-weapon switching, reloading etc(Press 1,2 for switching R for reloading)
-Bullet tracers for hitscan weapons
-Ability to attach suppressor on 2 weapons(Pistol and sniper,Press N)
-bullet decals and impact effects
-Weapon sway(Implemented in code not animation)

Player character:

-sprint(Press L Shift)
-crouch(Press C)
-Physcis based slide (faster and longer when going downhill and slower and shorter when going uphill) (Press crouch button while sprinting)
- 3 jump modes : normal jump - double jump - long jump Press J to switch
-ledge grab and climb (when in air, checks for climbale surface and automatically climbs if climbing is possilble)
-Grab AI dead body and drag it around(Hold F)
-Throw a projectile to distract or damage an enemy AI. Hold G to hold and aim with projectile and release it to throw
-Peak from behind the cover(When close to a cover press the ADS button to peak from the cover)
-Interact with different things in enviroment (Press E)
-Hack Turrets and security cameras which turns on friendly mode (Press X while looking at them)
-Armor and regenerative health (Armor can be filled by interacting with armor buys)

AI characters:

-3 Different Ai characters: Soldier and Turret (Campaign) and Zombie (Zombies mode)
Zombie:
-chase player and do melee attack when in range
-Spawn from a zombie spawner that can be placed in the map

Soldier:
-patrol around givin patrol points
-can carry all of the weapons in game
-3 states: Patrolling - Investigating - chasing
  -Patrolling: moves to givin patrol points in order
  -Investigating: Moves to last known location waits and then finds 3 random points around itself and moves to them and waits few seconds in each location
  -Chasing: Chases player until it gets in range and shoots at them while doing these
  
-Has Sight,Hearing and damage perception:
  -When seeing player enters chasing state - when losing sight of player enters investigating state
  -When hearing sth enters investigating state 
  -When damaged investigates around itself
  
-Reacts to other AI characters dead bodies and enters investigating state

Turret:
-Shoots at player if seen - stops shooting and goes back to original rotation when losing sight
-Can be hacked to friendly mode and shoots at enemy AIs

Ai characters take different amount of damage on hit of differnt body parts(for example headshots deal more damage)

Interactables:

-Press E while looking at interactable to interact
-interactables include buyables like armor buy and ammo buy (for zombie mode), Weapon pickups and automatic doors
-Weapon pickups save the number of bullets


UI:

-Dynamic crosshair
-Weapon info (weapon type and ammo)
-Health and armor
-floating damage points
-interact and buy
-Hit markers
-Round info for zombie mode
-Game over screen
-Pause menu
-Main menu
-Loading screen
