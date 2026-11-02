#pragma once

namespace ghost {

// Resolve the game symbols in libminecraftpe.so and hook the player tick.
// Returns false when a required symbol is missing (details go to logcat).
bool install(void* gameLib);

// Ask for ghost mode to be flipped. Safe to call from any thread; the change
// is applied on the game thread during the next local player tick.
void requestToggle();

// True while the player is currently a ghost.
bool isActive();

} // namespace ghost
