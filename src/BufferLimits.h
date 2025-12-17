#pragma once

// Maximum number of indices allowed to prevent accidental upload of unreasonably large buffers
// (e.g., due to faulty mesh or OBJ parsing). This limit helps catch errors early.
// This value is shared between main.cpp validation and IndexBuffer constructor validation.
constexpr unsigned int MAX_INDEX_COUNT = 500000;
