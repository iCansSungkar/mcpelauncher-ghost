#pragma once

namespace input {

// Register the keyboard callback on the primary game window. If the window
// does not exist yet, registration is deferred until it is created.
void install();

// The next non-modifier key press (with whatever modifiers are held) becomes
// the new toggle hotkey. onDone is called when the hotkey was captured.
void beginRebind(void (*onDone)());
void cancelRebind();
bool isRebinding();

} // namespace input
